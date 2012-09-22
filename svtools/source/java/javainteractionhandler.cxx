/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <tools/rcid.h>
#include <jvmfwk/framework.h>

#include <svtools/svtresid.hxx>
#include <svtools/javainteractionhandler.hxx>
#include <svtools/javacontext.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;

namespace svt
{

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
    osl_atomic_increment( &m_aRefCount );
}

void SAL_CALL JavaInteractionHandler::release(  ) throw ()
{
    if (! osl_atomic_decrement( &m_aRefCount ))
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

    if ( anyExc >>= e1 )
    {
        if( ! (m_bShowErrorsOnce && m_bJavaNotFound_Handled))
        {
           // No suitable JRE found
            SolarMutexGuard aSolarGuard;
            m_bJavaNotFound_Handled = true;
            WarningBox aWarningBox( NULL, SvtResId( WARNINGBOX_JAVANOTFOUND ) );
            aWarningBox.SetText(SvtResId(STR_WARNING_JAVANOTFOUND).toString());
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
            SolarMutexGuard aSolarGuard;
            m_bInvalidSettings_Handled = true;
#ifdef MACOSX
            WarningBox aWarningBox( NULL, SvtResId( WARNINGBOX_INVALIDJAVASETTINGS_MAC ) );
#else
            WarningBox aWarningBox( NULL, SvtResId( WARNINGBOX_INVALIDJAVASETTINGS ) );
#endif
            aWarningBox.SetText(SvtResId(STR_WARNING_INVALIDJAVASETTINGS).toString());
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
            SolarMutexGuard aSolarGuard;
            m_bJavaDisabled_Handled = true;
            // Java disabled. Give user a chance to enable Java inside Office.
            QueryBox aQueryBox( NULL, SvtResId( QBX_JAVADISABLED ) );
            aQueryBox.SetText(SvtResId( STR_QUESTION_JAVADISABLED ).toString());
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
            SolarMutexGuard aSolarGuard;
            m_bVMCreationFailure_Handled = true;
#ifdef MACOSX
            ErrorBox aErrorBox( NULL, SvtResId( ERRORBOX_JVMCREATIONFAILED_MAC ) );
#else
            ErrorBox aErrorBox( NULL, SvtResId( ERRORBOX_JVMCREATIONFAILED ) );
#endif
            aErrorBox.SetText(SvtResId( STR_ERROR_JVMCREATIONFAILED ).toString());
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
            SolarMutexGuard aSolarGuard;
            m_bRestartRequired_Handled = true;
            ErrorBox aErrorBox(NULL, SvtResId( ERRORBOX_RESTARTREQUIRED ) );
            aErrorBox.SetText(SvtResId( STR_ERROR_RESTARTREQUIRED ).toString());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
