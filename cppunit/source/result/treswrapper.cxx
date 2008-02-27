/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treswrapper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:26:03 $
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

#include <cstdarg>
#include <stdlib.h>
#include <stdio.h>

#include <sal/types.h>
#include <cppunit/Test.h>
#include "cppunit/autoregister/htestresult.h"
#include "cppunit/autoregister/callbackfunc_fktptr.h"
#include "cppunit/result/callbackfunc.h"
#include "cppunit/result/TestResult.h"

#include <cppunit/tagvalues.hxx>

namespace
{
    void TestResult_startTest(hTestResult _pResult, hTest _pTest)
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;

        std::string sName = pTest->getName();
        CallbackDispatch(0 /* NULL */, TAG_TYPE, SIGNAL_START_TEST, TAG_RESULT_PTR, _pResult, TAG_NODENAME, sName.c_str(), TAG_DONE);

        pResult->startTest(pTest);
    }

    void TestResult_endTest( hTestResult _pResult, hTest _pTest )
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;
        pResult->endTest(pTest);

        std::string sName = pTest->getName();
        CallbackDispatch(0 /* NULL */, TAG_TYPE, SIGNAL_END_TEST, TAG_RESULT_PTR, _pResult, TAG_NODENAME, sName.c_str(), TAG_DONE);
    }

// -----------------------------------------------------------------------------

    void TestResult_addFailure( hTestResult _pResult, hTest _pTest, hException _pException )
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;
        CppUnit::Exception*      pException = (CppUnit::Exception*)_pException;
        pResult->addFailure(pTest, pException);
    }

    void TestResult_addError( hTestResult _pResult, hTest _pTest, hException _pException )
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;
        CppUnit::Exception*      pException = (CppUnit::Exception*)_pException;
        pResult->addError(pTest, pException);
    }

    sal_Int32 TestResult_shouldStop(hTestResult _pResult)
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        return pResult->shouldStop() == true ? 1 : 0;
    }

// -----------------------------------------------------------------------------
    void TestResult_addInfo( hTestResult _pResult, hTest _pTest, const char* _sInfo )
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;
        // CppUnit::Exception*      pException = (CppUnit::Exception*)_pException;
        pResult->addInfo(pTest, _sInfo);
    }
// -----------------------------------------------------------------------------
    void TestResult_enterNode( hTestResult _pResult, const char* _sNode )
    {
        // signalPushName(getName());
        // CallbackDispatch(NULL, TAG_TYPE, EXECUTION_PUSH_NAME, TAG_RESULT_PTR, _pResult, TAG_NODENAME, _sNode, TAG_DONE);

        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        // CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;
        // CppUnit::Exception*      pException = (CppUnit::Exception*)_pException;
        pResult->enterNode(_sNode);
    }
    void TestResult_leaveNode( hTestResult _pResult, const char* _sNode )
    {
        CppUnit::TestResult* pResult = (CppUnit::TestResult*)_pResult;
        // CppUnit::Test*       pTest   = (CppUnit::Test*)_pTest;
        // CppUnit::Exception*      pException = (CppUnit::Exception*)_pException;
        pResult->leaveNode(_sNode);

        // CallbackDispatch(NULL, TAG_TYPE, EXECUTION_POP_NAME, TAG_RESULT_PTR, _pResult, TAG_DONE);
    }

// -----------------------------------------------------------------------------
    sal_Int32 TestResult_StarterA(TagHelper const& _aTagItems)
    {
        sal_Int32 nRetValue = 0;
        TagData nTagType     = _aTagItems.GetTagData(TAG_TYPE, 0);
        hTestResult pResult  = (hTestResult) _aTagItems.GetTagData(TAG_RESULT_PTR, 0 /* NULL */);
        CppUnit::Test* pTest = (CppUnit::Test*) _aTagItems.GetTagData(TAG_TEST_PTR, 0 /* NULL */);

        try
        {
            switch(nTagType)
            {
            case RESULT_START:
                TestResult_startTest(pResult, pTest);
                break;

            case RESULT_END:
                TestResult_endTest(pResult, pTest);
                break;

            case RESULT_ADD_FAILURE:
            {
                hException pException = (hException) _aTagItems.GetTagData(TAG_EXCEPTION, 0);
                TestResult_addFailure(pResult, pTest, pException);
                break;
            }

            case RESULT_ADD_ERROR:
            {
                hException pException = (hException) _aTagItems.GetTagData(TAG_EXCEPTION, 0);
                TestResult_addError(pResult, pTest, pException);
                break;
            }

            case RESULT_ADD_INFO:
            {
                const char* pInfo = (const char* ) _aTagItems.GetTagData(TAG_INFO, 0);
                TestResult_addInfo(pResult, pTest, pInfo);
                break;
            }

            case RESULT_ENTER_NODE:
            {
                const char* pNode = (const char* )_aTagItems.GetTagData(TAG_NODENAME, 0);
                TestResult_enterNode(pResult, pNode);
                break;
            }

            case RESULT_LEAVE_NODE:
            {
                const char* pNode = (const char* ) _aTagItems.GetTagData(TAG_NODENAME, 0);
                TestResult_leaveNode(pResult, pNode);
                break;
            }

            case RESULT_SHOULD_STOP:
                nRetValue = TestResult_shouldStop(pResult);
                break;

            default:
                fprintf(stderr, "TestResult_StarterA: Can't handle the tag type %8x\n", SAL_STATIC_CAST(int, nTagType));
                fflush(stderr);
                // throw std::exception(/*std::string("Unknown TYPE_TAG Exception.")*/);
            }
        }
        catch (std::exception &e)
        {
            fprintf(stderr, "TestResult_StarterA: Exception caught: %s\n", e.what());
            // throw e;
        }

        return nRetValue;
    }
} // namespace anonymous

// --------------------------------- Prototypes ---------------------------------

sal_Int32 SignalHandlerA( TagHelper const& _aTagItems );
sal_Int32 ExecutionA( TagHelper const& _aTagItems );

// -----------------------------------------------------------------------------
sal_Int32 CallbackDispatch(int x, ...)
{
    (void) x; // avoid warning

    // The idea behind TagItems are from the Amiga OS.
    // Due to the fact that the following code does not find my ok
    // I decide to wrote a simple helper
    //
    // struct TagItem *tags = (struct TagItem *)&x + 1;

    TagHelper aTagItems;

    // the following code could also be in a #define, so other functions could
    // use this, but at the moment this function a the only one.

    // FILL_TAGS(aTagItems, x);
    std::va_list  args;
    va_start( args, x );
    Tag nTag;
    do
    {
        nTag = va_arg(args, Tag);
        if (nTag != TAG_DONE)
        {
            TagData nValue = va_arg(args, TagData);
            // printf("Tag: %8x Value:%8x\n", nTag, nValue);
            aTagItems.insert(nTag, nValue);
        }
    } while (nTag != TAG_DONE);

    va_end   ( args );

    // FILL_TAGS:END

    // printf(".\n");

    sal_Int32 nRetValue = 0;
    Tag nPreTag = aTagItems.GetTagData(TAG_TYPE);
    if ( (nPreTag & TAG_RESULT) == TAG_RESULT)
    {
        nRetValue = TestResult_StarterA( aTagItems );
    }
    else if ((nPreTag & TAG_SIGNAL) == TAG_SIGNAL)
    {
        nRetValue = SignalHandlerA(aTagItems);
    }
    else if (( nPreTag & TAG_EXECUTION) == TAG_EXECUTION)
    {
        nRetValue = ExecutionA(aTagItems);
    }
    else
    {
        fprintf(stderr, "CallbackDispatch: First tag is unknown %8x\n", SAL_STATIC_CAST(int, nPreTag));
        // unknown TAG_TYPE
    }
    return nRetValue;
}

