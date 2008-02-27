/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tresstatewrapper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:23:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

// this functions build the wrapper, to handle old function calls to rtl_tres_state
// IMPORTANT: Due to the fact that rtl_tres_state is in the sal.lib
// you have to rename rtl_tres_state to c_rtl_tres_state
//
// return c_rtl_tres_state
// (
//     hRtlTestResult,
//     pData->length == 0 &&
//     ! *pData->buffer,
//     "New OString containing no characters",
//     "ctor_001"
// )

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cppunit/Test.h"
#include "cppunit/autoregister/htestresult.h"
#include "cppunit/autoregister/callbackfunc_fktptr.h"
#include "cppunit/TestAssert.h"

#define LOAD_TRESSTATEWRAPPER_LOCAL
#include "testshl/tresstatewrapper.hxx"
#undef LOAD_TRESSTATEWRAPPER_LOCAL

#include "cppunit/tagvalues.hxx"
#include "cppunit/externcallbackfunc.hxx"

// -----------------------------------------------------------------------------
// Wrapper class for the CppUnit::Test, because the new TestResult
// expect a CppUnit::Test interface

class WrappedTest : public CppUnit::Test
{
    std::string m_sName;
public:
    WrappedTest(std::string const& _sName)
            :m_sName(_sName){}

    virtual                       ~WrappedTest () {}
    virtual void                  run (hTestResult /*pResult*/){}
    virtual int                   countTestCases () const {return 1;}
    virtual std::string           getName () const {return m_sName;}
    virtual std::string           toString () const {return "WrappedTest";}
};

// -----------------------------------------------------------------------------
rtl_tres_state_start::rtl_tres_state_start(hTestResult _aResult, const sal_Char* _pName)
        :m_pName(_pName),
         m_aResult(_aResult)
{
    // (pTestResult_EnterNode)(m_aResult, m_pName);
    (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ENTER_NODE, TAG_RESULT_PTR, m_aResult, TAG_NODENAME, m_pName, TAG_DONE);
}
rtl_tres_state_start::~rtl_tres_state_start()
{
    // (pTestResult_LeaveNode)(m_aResult, m_pName);
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_LEAVE_NODE, TAG_RESULT_PTR, m_aResult, TAG_NODENAME, m_pName, TAG_DONE);
}

// --------------------------------- C-Wrapper ---------------------------------
typedef std::vector<rtl_tres_state_start*> rtl_tres_states;
rtl_tres_states m_StateStack;

extern "C" void c_rtl_tres_state_start(hTestResult _aResult, const sal_Char* _pName)
{
    rtl_tres_state_start* pState = new rtl_tres_state_start(_aResult, _pName);
    m_StateStack.push_back(pState);
}

extern "C" void c_rtl_tres_state_end(hTestResult, const sal_Char*)
{
    rtl_tres_state_start* pState = m_StateStack.back();

    delete pState;
    m_StateStack.pop_back();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ------------------------- Wrapper for rtl_tres_state -------------------------
// -----------------------------------------------------------------------------

sal_Bool SAL_CALL c_rtl_tres_state(
    hTestResult pResult,
    sal_Bool bSuccess,
    char const * pTestMsg,
    char const * pFuncName)
{
    CppUnit::Test* pTest = new WrappedTest(pFuncName);

    sal_Int32 nValue = (pCallbackFunc)(0 /* NULL */, TAG_TYPE, EXECUTION_CHECK, TAG_RESULT_PTR, pResult, TAG_NODENAME, pTest->getName().c_str(), TAG_DONE);
    if (nValue == DO_NOT_EXECUTE)
    {
        return false;
    }

    // This code is copied from CppUnit::TestCase and less adapted, to work with the
    // old test code.
    // (pTestResult_StartTest)(pResult, pTest);
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_START, TAG_RESULT_PTR, pResult, TAG_TEST_PTR, pTest, TAG_DONE);

    try
    {
        CPPUNIT_ASSERT_MESSAGE( pTestMsg, bSuccess ? true : false );
        // (pTestResult_AddInfo)(pResult, pTest, "#PASSED#");
        (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_ADD_INFO, TAG_RESULT_PTR, pResult, TAG_TEST_PTR, pTest, TAG_INFO, "PASSED", TAG_DONE);
    }
    catch ( CppUnit::Exception &e )
    {
        CppUnit::Exception *copy = e.clone();
        // (pTestResult_AddFailure)( pResult, pTest, copy );
        (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_FAILURE, TAG_RESULT_PTR, pResult, TAG_TEST_PTR, pTest, TAG_EXCEPTION, copy, TAG_DONE );
    }
    catch ( std::exception &e )
    {
        //(pTestResult_AddError)( pResult, pTest, new CppUnit::Exception( e.what() ) );
        (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_ERROR, TAG_RESULT_PTR, pResult, TAG_TEST_PTR, pTest, TAG_EXCEPTION, new CppUnit::Exception( e.what() ), TAG_DONE );
    }
    catch (...)
    {
        CppUnit::Exception *e = new CppUnit::Exception( "caught unknown exception" );
        // (pTestResult_AddError)( pResult, pTest, e );
        (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_ERROR, TAG_RESULT_PTR, pResult, TAG_TEST_PTR, pTest, TAG_EXCEPTION, e, TAG_DONE );
    }


    // (pTestResult_EndTest)( pResult, pTest );
    (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_END, TAG_RESULT_PTR, pResult, TAG_TEST_PTR, pTest, TAG_DONE );

    return bSuccess;
}

