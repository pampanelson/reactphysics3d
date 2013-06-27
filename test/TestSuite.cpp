/********************************************************************************
* ReactPhysics3D physics library, http://code.google.com/p/reactphysics3d/      *
* Copyright (c) 2010-2013 Daniel Chappuis                                       *
*********************************************************************************
*                                                                               *
* This software is provided 'as-is', without any express or implied warranty.   *
* In no event will the authors be held liable for any damages arising from the  *
* use of this software.                                                         *
*                                                                               *
* Permission is granted to anyone to use this software for any purpose,         *
* including commercial applications, and to alter it and redistribute it        *
* freely, subject to the following restrictions:                                *
*                                                                               *
* 1. The origin of this software must not be misrepresented; you must not claim *
*    that you wrote the original software. If you use this software in a        *
*    product, an acknowledgment in the product documentation would be           *
*    appreciated but is not required.                                           *
*                                                                               *
* 2. Altered source versions must be plainly marked as such, and must not be    *
*    misrepresented as being the original software.                             *
*                                                                               *
* 3. This notice may not be removed or altered from any source distribution.    *
*                                                                               *
********************************************************************************/

// Librairies
#include "TestSuite.h"

using namespace reactphysics3d;

// Constructor
TestSuite::TestSuite(const std::string& name, std::ostream* outputStream)
          : mName(name), mOutputStream(outputStream) {

}

// Return the number of passed tests
long TestSuite::getNbPassedTests() const {

    long nbPassedTests = 0;

    for (size_t i=0; i<mTests.size(); i++) {
        assert(mTests[i]);
        nbPassedTests += mTests[i]->getNbPassedTests();
    }

    return nbPassedTests;
}

// Return the number of failed tests
long TestSuite::getNbFailedTests() const {
    long nbFailedTests = 0;

    for (size_t i=0; i<mTests.size(); i++) {
        assert(mTests[i]);
        nbFailedTests += mTests[i]->getNbFailedTests();
    }

    return nbFailedTests;
}

// Add a unit test in the test suite
void TestSuite::addTest(Test* test) {
    if (test == NULL) {
        throw std::invalid_argument("Error : You cannot add a NULL test in the test suite.");
    }
    else if (mOutputStream != NULL && test->getOutputStream() == NULL) {
        test->setOutputStream(mOutputStream);
    }

    // Add the test to the suite
    mTests.push_back(test);

    // Reset the added test
    test->reset();
}

// Add a test suite to the current test suite
void TestSuite::addTestSuite(const TestSuite& testSuite) {

    // Add each test of the test suite to the current one
    for (size_t i =0; i < testSuite.mTests.size(); i++) {
        assert(testSuite.mTests[i] != NULL);
        addTest(testSuite.mTests[i]);
    }
}

// Launch the tests of the test suite
void TestSuite::run() {

    // Reset all the tests
    reset();

    // Run all the tests
    for (size_t i=0; i < mTests.size(); i++) {
        assert(mTests[i] != NULL);
        mTests[i]->run();
    }
}

// Reset the test suite
void TestSuite::reset() {
    for(size_t i=0; i < mTests.size(); ++i) {
        assert(mTests[i]);
        mTests[i]->reset();
      }
}

// Display the tests report and return the number of failed tests
long TestSuite::report() const {
    if (mOutputStream != NULL) {
        long nbFailedTests = 0;

        *mOutputStream << "Test Suite \"" << mName << "\"\n=====";
        size_t i;
        for (i=0; i < mName.size(); i++) {
            *mOutputStream << "=";
        }
        *mOutputStream << "=" << std::endl;
        for (i=0; i < mTests.size(); i++) {
            assert(mTests[i] != NULL);
            nbFailedTests += mTests[i]->report();
        }
        *mOutputStream << "=====";
        for (i=0; i < mName.size(); i++) {
            *mOutputStream << "=";
        }
        *mOutputStream << "=" << std::endl;

        // Return the number of failed tests
        return nbFailedTests;
    }
    else {
        return getNbFailedTests();
    }
}

// Delete all the tests
void TestSuite::clear() {

    for (size_t i=0; i<mTests.size(); i++) {
        delete mTests[i];
        mTests[i] = NULL;
    }
}