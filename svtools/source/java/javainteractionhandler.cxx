/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"



#include <svtools/svtools.hrc>
#include <tools/resid.hxx>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/java/JavaNotFoundException.hpp>
#include <com/sun/star/java/InvalidJavaSettingsException.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <com/sun/star/java/RestartRequiredException.hpp>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vos/mutex.hxx>
#include <tools/string.hxx>
#include <tools/rcid.h>
#include <jvmfwk/framework.h>

#include <svtools/svtdata.hxx>
#include <svtools/javainteractionhandler.hxx>
#include <svtools/javacontext.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;

namespace svt
{

JavaInteractionHandler::JavaInteractionHandler():
    m_aRefCount(0),
    m_bShowErrorsOnce(false),
    m_bJavaDisabled_Handled(false),
    m_bInvalidSettings_Handled(false),
    m_bJavaNotFound_Handled(false),
    m_bVMCreationFailure_Handled(false),
    m_bRestartRequired_Handled(false),
    m_nResult_JavaDisabled(RET_NO)
{
}

JavaInteractionHandler::JavaInteractionHandler(bool bReportErrorOnce) :
    m_aRefCount(0),
    m_bShowErrorsOnce(bReportErrorOnce),
    m_bJavaDisabled_Handled(false),
    m_bInvalidSettings_Handled(false),
    m_bJavaNotFound_Handled(false),
    m_bVMCreationFailure_Handled(false),
    m_bRestartRequired_Handled(false),
    m_nResult_JavaDisabled(RET_NO)
{
}

JavaInteractionHandler::~JavaInteractionHandler()
{
}

Any SAL_CALL JavaInteractionHandler::queryInterface(const Type& aType )
    throw (RuntimeException)
{
    if (aType == getCppuType(reinterpret_cast<Reference<XInterface>*>(0)))
        return Any( static_cast<XInterface*>(this), aType);
    else if (aType == getCppuType(reinterpret_cast<Reference<XInteractionHandler>*>(0)))
        return Any( static_cast<XInteractionHandler*>(this), aType);
    return Any();
}

void SAL_CALL JavaInteractionHandler::acquire(  ) throw ()
{
    osl_incrementInterlockedCount( &m_aRefCount );
}

void SAL_CALL JavaInteractionHandler::release(  ) throw ()
{
    if (! osl_decrementInterlockedCount( &m_aRefCount ))
        delete this;
}


void SAL_CALL JavaInteractionHandler::handle( const Reference< XInteractionRequest >& Request ) throw (RuntimeException)
{
    Any anyExc = Request->getRequest();
    Sequence< Reference< XInteractionContinuation > > aSeqCont = Request->getContinuations();

    Reference< XInteractionAbort > abort;
    Reference< XInteractionRetry > retry;
    sal_Int32 i;

    for ( i = 0; i < aSeqCont.getLength(); i++ )
    {
        abort = Reference< XInteractionAbort>::query( aSeqCont[i]);
        if ( abort.is() )
            break;
    }

    for ( i= 0; i < aSeqCont.getLength(); i++)
    {
        retry= Reference<XInteractionRetry>::query( aSeqCont[i]);
        if ( retry.is() )
            break;
    }

    com::sun::star::java::JavaNotFoundException e1;
    com::sun::star::java::InvalidJavaSettingsException e2;
     com::sun::star::java::JavaDisabledException                e3;
    com::sun::star::java::JavaVMCreationFailureException    e4;
    com::sun::star::java::RestartRequiredException e5;
    // Try to recover the Exception type in the any and
    // react accordingly.
    sal_uInt16      nResult = RET_CANCEL;
    ::rtl::OUString    aParameter;

    if ( anyExc >>= e1 )
    {
        if( ! (m_bShowErrorsOnce && m_bJavaNotFound_Handled))
        {
           // No suitable JRE found
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            m_bJavaNotFound_Handled = true;
            WarningBox aWarningBox( NULL, SvtResId( WARNINGBOX_JAVANOTFOUND ) );
            String aTitle( SvtResId( STR_WARNING_JAVANOTFOUND ) );
            aWarningBox.SetText( aTitle );
            nResult = aWarningBox.Execute();
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e2 )
    {
        if( !(m_bShowErrorsOnce && m_bInvalidSettings_Handled))
        {
           // javavendors.xml was updated and Java has not been configured yet
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            m_bInvalidSettings_Handled = true;
            WarningBox aWarningBox( NULL, SvtResId( WARNINGBOX_INVALIDJAVASETTINGS ) );
            String aTitle( SvtResId(STR_WARNING_INVALIDJAVASETTINGS));
            aWarningBox.SetText( aTitle );
            nResult = aWarningBox.Execute();
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e3 )
    {
        if( !(m_bShowErrorsOnce && m_bJavaDisabled_Handled))
        {
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            m_bJavaDisabled_Handled = true;
            // Java disabled. Give user a chance to enable Java inside Office.
            QueryBox aQueryBox( NULL, SvtResId( QBX_JAVADISABLED ) );
            String aTitle( SvtResId( STR_QUESTION_JAVADISABLED ) );
            aQueryBox.SetText( aTitle );
            nResult = aQueryBox.Execute();
            if ( nResult == RET_YES )
            {
                jfw_setEnabled(sal_True);
            }

            m_nResult_JavaDisabled = nResult;

        }
        else
        {
            nResult = m_nResult_JavaDisabled;
        }
    }
    else if ( anyExc >>= e4 )
    {
        if( !(m_bShowErrorsOnce && m_bVMCreationFailure_Handled))
        {
            // Java not correctly installed, or damaged
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            m_bVMCreationFailure_Handled = true;
            ErrorBox aErrorBox( NULL, SvtResId( ERRORBOX_JVMCREATIONFAILED ) );
            String aTitle( SvtResId( STR_ERROR_JVMCREATIONFAILED ) );
            aErrorBox.SetText( aTitle );
            nResult = aErrorBox.Execute();
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e5 )
    {
        if( !(m_bShowErrorsOnce && m_bRestartRequired_Handled))
        {
            // a new JRE was selected, but office needs to be restarted
            //before it can be used.
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            m_bRestartRequired_Handled = true;
            ErrorBox aErrorBox(NULL, SvtResId( ERRORBOX_RESTARTREQUIRED ) );
            String aTitle( SvtResId( STR_ERROR_RESTARTREQUIRED ) );
            aErrorBox.SetText( aTitle );
            nResult = aErrorBox.Execute();
        }
        else
        {
            nResult = RET_OK;
        }
    }

    if ( nResult == RET_CANCEL || nResult == RET_NO)
    {
        // Unknown exception type or user wants to cancel
        if ( abort.is() )
            abort->select();
    }
    else // nResult == RET_OK
    {
        // User selected OK => retry Java usage
        if ( retry.is() )
            retry->select();
    }
}

}
