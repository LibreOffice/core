/*************************************************************************
 *
 *  $RCSfile: javainteractionhandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:10:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/



#include "svtools.hrc"

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/java/JavaNotFoundException.hpp>
#include <com/sun/star/java/InvalidJavaSettingsException.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <com/sun/star/java/RestartRequiredException.hpp>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif
#ifndef INCLUDED_JVMFWK_FRAMEWORK_H
#include <jvmfwk/framework.h>
#endif

#include "svtdata.hxx"
#include "javainteractionhandler.hxx"
#include "javacontext.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::task;

namespace svt
{

JavaInteractionHandler::JavaInteractionHandler()
{
    JavaInteractionHandler(false);
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
    USHORT      nResult = RET_CANCEL;
    OUString    aParameter;

    if ( anyExc >>= e1 )
    {
        if( ! (m_bShowErrorsOnce && m_bJavaNotFound_Handled))
        {
           // No suitable JRE found
            vos::OGuard aSolarGuard( Application::GetSolarMutex() );
            m_bJavaNotFound_Handled = true;
            //We first try to get the patch resource svp680xxx.res
            //If the resource is not found then svt680xxx.res is used
            ResId idWBX = SvtResId(WARNINGBOX_JAVANOTFOUND);
            SvpResId pidPatchWBX(WARNINGBOX_JAVANOTFOUND);
            pidPatchWBX.SetRT(RSC_WARNINGBOX);
            ResMgr *pMgrWB = pidPatchWBX.GetResMgr();
            if (pMgrWB && pMgrWB->IsAvailable(pidPatchWBX))
                idWBX = pidPatchWBX;
            WarningBox aWarningBox( NULL, idWBX);

            String aTitle;
            SvpResId pidString(STR_WARNING_JAVANOTFOUND);
            pidString.SetRT(RSC_STRING);
            ResMgr *pmgr = pidString.GetResMgr();
            if ( pmgr && pmgr->IsAvailable(pidString))
                aTitle = String(pidString);
            else
                aTitle = String( SvtResId( STR_WARNING_JAVANOTFOUND ));

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
            //We first try to get the patch resource svp680xxx.res
            //If the resource is not found then svt680xxx.res is used
            ResId idWBX = SvtResId(WARNINGBOX_INVALIDJAVASETTINGS);
            SvpResId pidPatchWBX(WARNINGBOX_INVALIDJAVASETTINGS);
            pidPatchWBX.SetRT(RSC_WARNINGBOX);
            ResMgr *pMgrWB = pidPatchWBX.GetResMgr();
            if (pMgrWB && pMgrWB->IsAvailable(pidPatchWBX))
                idWBX = pidPatchWBX;
            WarningBox aWarningBox( NULL, idWBX);

            String aTitle;
            SvpResId pidString(STR_WARNING_INVALIDJAVASETTINGS);
            pidString.SetRT(RSC_STRING);
            ResMgr *pmgr = pidString.GetResMgr();
            if ( pmgr && pmgr->IsAvailable(pidString))
                aTitle = String(pidString);
            else
                aTitle = String( SvtResId(STR_WARNING_INVALIDJAVASETTINGS));

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
            //We first try to get the patch resource svp680xxx.res
            //If the resource is not found then svt680xxx.res is used
            ResId idQBX = SvtResId( QBX_JAVADISABLED );
            SvpResId pidPatchQBX(QBX_JAVADISABLED);
            pidPatchQBX.SetRT(RSC_QUERYBOX);
            ResMgr *pMgrQB = pidPatchQBX.GetResMgr();

            if (pMgrQB && pMgrQB->IsAvailable(pidPatchQBX))
                idQBX = pidPatchQBX;

            QueryBox aQueryBox(NULL, idQBX);

            String aTitle;

            SvpResId pidString(STR_QUESTION_JAVADISABLED);
            pidString.SetRT(RSC_STRING);
            ResMgr *pmgr = pidString.GetResMgr();
            if ( pmgr && pmgr->IsAvailable(pidString))
                aTitle = String(pidString);
            else
                aTitle = String( SvtResId( STR_QUESTION_JAVADISABLED ));

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
            //We first try to get the patch resource svp680xxx.res
            //If the resource is not found then svt680xxx.res is used
            ResId idEBX = SvtResId(ERRORBOX_JVMCREATIONFAILED);
            SvpResId pidPatchEBX(ERRORBOX_JVMCREATIONFAILED);
            pidPatchEBX.SetRT(RSC_ERRORBOX);
            ResMgr *pMgrEB = pidPatchEBX.GetResMgr();
            if (pMgrEB && pMgrEB->IsAvailable(pidPatchEBX))
                idEBX = pidPatchEBX;
            ErrorBox aErrorBox( NULL, idEBX);

            String aTitle;
            SvpResId pidString(STR_ERROR_JVMCREATIONFAILED);
            pidString.SetRT(RSC_STRING);
            ResMgr *pmgr = pidString.GetResMgr();
            if ( pmgr && pmgr->IsAvailable(pidString))
                aTitle = String(pidString);
            else
                aTitle = String( SvtResId(STR_ERROR_JVMCREATIONFAILED));

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
            //We first try to get the patch resource svp680xxx.res
            //If the resource is not found then svt680xxx.res is used
            ResId idEBX = SvtResId(ERRORBOX_RESTARTREQUIRED);
            SvpResId pidPatchEBX(ERRORBOX_RESTARTREQUIRED);
            pidPatchEBX.SetRT(RSC_ERRORBOX);
            ResMgr *pMgrEB = pidPatchEBX.GetResMgr();
            if (pMgrEB && pMgrEB->IsAvailable(pidPatchEBX))
                idEBX = pidPatchEBX;
            ErrorBox aErrorBox(NULL, idEBX);

            String aTitle;
            SvpResId pidString(STR_ERROR_RESTARTREQUIRED);
            pidString.SetRT(RSC_STRING);
            ResMgr *pmgr = pidString.GetResMgr();
            if ( pmgr && pmgr->IsAvailable(pidString))
                aTitle = String(pidString);
            else
                aTitle = String( SvtResId(STR_ERROR_RESTARTREQUIRED));

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
