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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "sfx2/macrconf.hxx"
#include "sfx2/app.hxx"

#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/basmgr.hxx>
#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>

//==========================================================================

SbMethod* SfxQueryMacro_Impl( BasicManager* pMgr , const String& rMacro,
    const String &rLibName, const String& rModule )
{
    IntlWrapper aIntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const CollatorWrapper* pCollator = aIntlWrapper.getCollator();
    sal_uInt16 nLibCount = pMgr->GetLibCount();
    for ( sal_uInt16 nLib = 0; nLib < nLibCount; ++nLib )
    {
        if ( COMPARE_EQUAL == pCollator->compareString( pMgr->GetLibName( nLib ), rLibName ) )
        {
            StarBASIC* pLib = pMgr->GetLib( nLib );
            if( !pLib )
            {
                pMgr->LoadLib( nLib );
                pLib = pMgr->GetLib( nLib );
            }

            if( pLib )
            {
                sal_uInt16 nModCount = pLib->GetModules()->Count();
                for( sal_uInt16 nMod = 0; nMod < nModCount; ++nMod )
                {
                    SbModule* pMod = (SbModule*)pLib->GetModules()->Get( nMod );
                    if ( pMod && COMPARE_EQUAL == pCollator->compareString( pMod->GetName(), rModule ) )
                    {
                        SbMethod* pMethod = (SbMethod*)pMod->Find( rMacro, SbxCLASS_METHOD );
                        if( pMethod )
                            return pMethod;
                    }
                }
            }
        }
    }
    return 0;
}

SbMethod* SfxQueryMacro( BasicManager* pMgr , const String& rMacro )
{
    sal_uInt16 nLast = 0;
    String aMacro = rMacro;
    String aLibName = aMacro.GetToken( 0, '.', nLast );
    String aModule = aMacro.GetToken( 0, '.', nLast );
    aMacro.Erase( 0, nLast );

    return SfxQueryMacro_Impl( pMgr, aMacro, aLibName, aModule );
}

//==========================================================================

ErrCode SfxMacroConfig::Call(
    SbxObject* /*pVCtrl*/,
    const String& rCode,
    BasicManager* pMgr,
    SbxArray *pArgs,
    SbxValue *pRet )
{
    SfxApplication *pApp = SFX_APP();
    pApp->EnterBasicCall();
    SbMethod* pMethod = SfxQueryMacro( pMgr, rCode );
    ErrCode nErr = 0;
    if( pMethod )
    {
        if ( pArgs )
            pMethod->SetParameters( pArgs );
        nErr = pMethod->Call( pRet );
    }
    else
        nErr = ERRCODE_BASIC_PROC_UNDEFINED;

    pApp->LeaveBasicCall();
    return nErr;
}


