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
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#ifndef _SBX_HXX //autogen
#include <basic/sbx.hxx>
#endif
#include <svl/intitem.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#ifndef _BASIC_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif

#include <osl/mutex.hxx>

#include <com/sun/star/script/XEngine.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#ifndef GCC
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <unotools/intlwrapper.hxx>

#include <sfx2/msgpool.hxx>
#include <sfx2/macrconf.hxx>
#include "sfxbasic.hxx"
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include "minfitem.hxx"
#include "imgmgr.hxx"
#include <sfx2/evntconf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/viewfrm.hxx>
#include <appdata.hxx>
#include "objshimp.hxx"
#include <sfx2/request.hxx>

static const sal_uInt16 nCompatVersion = 2;
static const sal_uInt16 nVersion = 3;

// Static member
SfxMacroConfig* SfxMacroConfig::pMacroConfig = NULL;

SfxMacroConfig* SfxMacroConfig::GetOrCreate()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if ( !pMacroConfig )
        pMacroConfig = new SfxMacroConfig;
    return pMacroConfig;
}

void SfxMacroConfig::Release_Impl()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    DELETEZ( pMacroConfig );
}

//==========================================================================

struct SfxMacroConfig_Impl
{
    SfxMacroInfoArr_Impl    aArr;
    sal_uInt32                  nEventId;
    sal_Bool                    bWaitingForCallback;

                            SfxMacroConfig_Impl()
                            : nEventId( 0 )
                            , bWaitingForCallback( sal_False )
                            {}
};

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

SfxMacroInfo::SfxMacroInfo( const String& rURL ) :
    pHelpText(0),
    nRefCnt(0),
    bAppBasic(TRUE),
    nSlotId(0),
    pSlot(0)
{
    if ( rURL.CompareToAscii( "macro:", 6 ) == COMPARE_EQUAL )
    {
        String aTmp = rURL.Copy( 6 );
        if ( aTmp.GetTokenCount('/') > 3 )
        {
            // 'macro:///lib.mod.proc(args)' => Macro via App-BASIC-Mgr
            // 'macro://[docname|.]/lib.mod.proc(args)' => Macro via zugehoerigen Doc-BASIC-Mgr
            if ( aTmp.CompareToAscii("///", 3 ) != COMPARE_EQUAL )
                bAppBasic = FALSE;
            aTmp = rURL.GetToken( 3, '/' );
            if ( aTmp.GetTokenCount('.') == 3 )
            {
                aLibName = aTmp.GetToken( 0, '.' );
                aModuleName = aTmp.GetToken( 1, '.' );
                aMethodName = aTmp.GetToken( 2, '.' );

                // Remove arguments to be compatible
                aMethodName.SearchAndReplaceAscii( "()", String(), sal::static_int_cast< xub_StrLen >(std::max( aMethodName.Len()-2, 0 )));
            }
        }

        DBG_ASSERT( aLibName.Len() && aModuleName.Len() && aMethodName.Len(), "Wrong macro URL!" );
    }
    else
        aMethodName = rURL;
}

SfxMacroInfo::SfxMacroInfo( bool _bAppBasic ) :
    pHelpText(0),
    nRefCnt(0),
    bAppBasic(_bAppBasic),
    nSlotId(0),
    pSlot(0)
{}

//==========================================================================

SfxMacroInfo::SfxMacroInfo(bool _bAppBasic, const String& rLibName,
        const String& rModuleName, const String& rMethodName) :
    pHelpText(0),
    nRefCnt(0),
    bAppBasic(_bAppBasic),
    aLibName(rLibName),
    aModuleName(rModuleName),
    aMethodName(rMethodName),
    nSlotId(0),
    pSlot(0)
{
}

//==========================================================================

SfxMacroInfo::SfxMacroInfo(bool _bAppBasic, const String& rQualifiedName )
:   pHelpText(0),
    nRefCnt(0),
    bAppBasic(_bAppBasic),
    nSlotId(0),
    pSlot(0)
{
    sal_uInt16 nCount = rQualifiedName.GetTokenCount('.');
    aMethodName = rQualifiedName.GetToken( nCount-1, '.' );
    if ( nCount > 1 )
        aModuleName = rQualifiedName.GetToken( nCount-2, '.' );
    if ( nCount > 2 )
        aLibName = rQualifiedName.GetToken( 0, '.' );
}

//==========================================================================

SfxMacroInfo::SfxMacroInfo(SfxMacroInfo& rOther) :
    pHelpText(0),
    nRefCnt(0),
    bAppBasic(rOther.bAppBasic),
    aLibName(rOther.aLibName),
    aModuleName(rOther.aModuleName),
    aMethodName(rOther.aMethodName),
    nSlotId(rOther.nSlotId),
    pSlot(0)
{}

//==========================================================================

SfxMacroInfo::~SfxMacroInfo()
{
    delete pSlot;
    delete pHelpText;
}

//==========================================================================

sal_Bool SfxMacroInfo::operator==(const SfxMacroInfo& rOther) const
{
    if ( GetQualifiedName() == rOther.GetQualifiedName() &&
            bAppBasic   == rOther.bAppBasic )
        return sal_True;
    else
        return sal_False;
}

//==========================================================================

String SfxMacroInfo::GetMacroName() const
{
    String aMacroName = aMethodName;
    aMacroName += '(';
    aMacroName += aLibName;
    aMacroName += '.';
    aMacroName += aModuleName;
    aMacroName += ')';
    return aMacroName;
}

//==========================================================================

String SfxMacroInfo::GetQualifiedName() const
{
    String aMacroName;
    if( aMacroName.Len() || aLibName.Len() )
    {
        // Altes Format
        aMacroName = aLibName;
        aMacroName += '.';
        aMacroName += aModuleName;
        aMacroName += '.';
    }

    // Wg. ::com::sun::star::script::JavaScript kein Zerlegen des Strings mehr
    aMacroName += aMethodName;
    return aMacroName;
}

String SfxMacroInfo::GetFullQualifiedName() const
{
    // Liefert nur Unsinn, wenn f"ur ein ::com::sun::star::script::JavaScript aufgerufen !
    String aRet;
    if ( bAppBasic )
        aRet = SFX_APP()->GetName();
    aRet += '.';
    aRet += GetQualifiedName();
    return aRet;
}

String SfxMacroInfo::GetURL() const
{
    if ( !aLibName.Len() )
        return aMethodName;

    // 'macro:///lib.mod.proc(args)' => Macro via App-BASIC-Mgr
    // 'macro://[docname|.]/lib.mod.proc(args)' => Macro via zugehoerigen Doc-BASIC-Mgr
    // 'macro://obj.method(args)' => Object via App-BASIC-Mgr
    String aURL( String::CreateFromAscii("macro://") );
    if ( !bAppBasic )
        aURL += '.';
    aURL += '/';
    aURL += aLibName;
    aURL += '.';
    aURL += aModuleName;
    aURL += '.';
    aURL += aMethodName;
    aURL += String::CreateFromAscii("()");

    return aURL;
}

//==========================================================================

BasicManager* SfxMacroInfo::GetBasicManager() const
{
    if (bAppBasic)
    {
        return SFX_APP()->GetBasicManager();
    }
    else
    {
        SfxObjectShell *pCurrDocShell = SfxObjectShell::Current();
        return pCurrDocShell ? pCurrDocShell->GetBasicManager() :
                               SFX_APP()->GetBasicManager();
    }
}

//==========================================================================

String SfxMacroInfo::GetBasicName() const
{
    if (bAppBasic)
    {
        return SFX_APP()->GetName();
    }
    else
    {
        SfxObjectShell *pCurrDocShell = SfxObjectShell::Current();
        if ( pCurrDocShell )
            return pCurrDocShell->GetTitle();
        else
            return SFX_APP()->GetName();
    }
}

String SfxMacroInfo::GetHelpText() const
{
    if ( pHelpText )
        return *pHelpText;
    return String();
}

String SfxMacroConfig::RequestHelp( sal_uInt16 nId )
{
    SfxMacroInfo *pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo( nId );
    if ( !pInfo )
        return String();

    if ( !pInfo->pHelpText )
    {
        SbMethod *pMethod =
            SfxQueryMacro_Impl( pInfo->GetBasicManager(), pInfo->aMethodName,
                pInfo->aLibName, pInfo->aModuleName );
        if ( pMethod && pMethod->GetInfo() )
            pInfo->pHelpText = new String( pMethod->GetInfo()->GetComment() );
    }

    return pInfo->GetHelpText();
}

void SfxMacroInfo::SetHelpText( const String& rName )
{
    if ( !pHelpText )
        pHelpText = new String;
    *pHelpText = rName;
}

sal_Bool SfxMacroInfo::Compare( const SvxMacro& rMacro ) const
{
    String aName = rMacro.GetLibName();
    aName += '.';
    aName += rMacro.GetMacName();
    if ( GetFullQualifiedName() == aName )
        return sal_True;
    return sal_False;
}

//==========================================================================

SfxMacroConfig::SfxMacroConfig()
{
    pImp = new SfxMacroConfig_Impl;
}

//==========================================================================

SfxMacroConfig::~SfxMacroConfig()
{
    if ( pImp->nEventId )
        Application::RemoveUserEvent( pImp->nEventId );
    delete pImp;
}

//==========================================================================

void SfxMacroConfig::ReleaseSlotId(sal_uInt16 nId)
{
    DBG_ASSERT( IsMacroSlot( nId ), "SlotId ist kein Macro!");

    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        SfxMacroInfo *pInfo = (pImp->aArr)[i];
        if (pInfo->nSlotId == nId)
        {
            pInfo->nRefCnt--;
            if (pInfo->nRefCnt == 0)
            {
                // Slot wird nicht mehr referenziert, also holen
                SfxSlot *pSlot = pInfo->pSlot;

                // Slot aus der Verkettung rausnehmen
                while (pSlot->pNextSlot != pInfo->pSlot)
                    pSlot = (SfxSlot*) pSlot->pNextSlot;
                pSlot->pNextSlot = pInfo->pSlot->pNextSlot;

                // Slot selbst kurz schlie\sen
                pSlot = pInfo->pSlot;
                pSlot->pNextSlot = pSlot;

                // MacroInfo aus Array entfernen, damit sie kein Unheil
                // anrichten kann
                pImp->aArr.Remove(i);

                // SlotId wieder freigeben
                sal_uInt16 nIdCount = aIdArray.Count();
                for (sal_uInt16 n=0; n<nIdCount; n++)
                {
                    if (aIdArray[n] == nId)
                    {
                        aIdArray.Remove(n);
                        break;
                    }
                }

                // Sofern nicht die Applikation heruntergefahren wird, mu\s
                // der Slot asynchron gel"oscht werden, falls er in seinem
                // eigenen Execute abgeschossen wird!
                if ( !SFX_APP()->Get_Impl()->bInQuit )
                    pImp->nEventId = Application::PostUserEvent( LINK(this, SfxMacroConfig, EventHdl_Impl), pInfo );
                else
                    EventHdl_Impl( pInfo );
            }
            return;
        }
    }

    DBG_ERROR("Macro-SlotId nicht gefunden!");
}

//==========================================================================

void SfxMacroConfig::RegisterSlotId(sal_uInt16 nId)
{
    DBG_ASSERT( IsMacroSlot( nId ), "SlotId ist kein Macro!");

    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        if ((pImp->aArr)[i]->nSlotId == nId)
        {
            (pImp->aArr)[i]->nRefCnt++;
            return;
        }
    }

    DBG_ERROR("Macro-SlotId nicht gefunden!");
}

//==========================================================================

SfxMacroInfo* SfxMacroConfig::GetMacroInfo(sal_uInt16 nId) const
{
    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ((pImp->aArr)[i]->nSlotId == nId)
            return (pImp->aArr)[i];

    return 0;
}

IMPL_LINK( SfxMacroConfig, EventHdl_Impl, SfxMacroInfo*, pInfo )
{
    delete pInfo;
    pImp->nEventId = 0;
    return 0;
}

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


sal_Bool SfxMacroConfig::IsMacroSlot( sal_uInt16 nId )
{
    return ( nId >= SID_MACRO_START && nId <= SID_MACRO_END );
}


