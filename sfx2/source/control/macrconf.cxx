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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbx.hxx>
#include <svl/intitem.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbuno.hxx>

#include <osl/mutex.hxx>

#include <com/sun/star/script/XEngine.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <comphelper/processfactory.hxx>
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
    sal_uInt32              nEventId;
    sal_Bool                bWaitingForCallback;

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

ErrCode SfxCallMacro( BasicManager* pMgr, const String& rCode,
                      SbxArray *pArgs, SbxValue *pRet )
{
    ErrCode nErr;
    SfxApplication *pApp = SFX_APP();
    pApp->EnterBasicCall();
    SbMethod* pMethod = SfxQueryMacro( pMgr, rCode );
    if ( pMethod )
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
            // 'macro://[docname|.]/lib.mod.proc(args)' => Macro via included Doc-BASIC-Mgr
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
:       pHelpText(0),
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

    // due to ::com::sun::star::script::JavaScript
    // no more disassembly of the string
    aMacroName += aMethodName;
    return aMacroName;
}

String SfxMacroInfo::GetFullQualifiedName() const
{
    // Returns only nonsense, when called for a
    // ::com::sun::star::script::JavaScript !
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
    // 'macro://[docname|.]/lib.mod.proc(args)' => Macro via included Doc-BASIC-Mgr
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

SFX_STATE_STUB( SfxApplication, MacroState_Impl )
SFX_EXEC_STUB( SfxApplication, MacroExec_Impl )

sal_uInt16 SfxMacroConfig::GetSlotId(SfxMacroInfoPtr pInfo)
{
    sal_uInt16 nCount = pImp->aArr.Count();      // Search for Macro
    sal_uInt16 i;
    for (i=0; i<nCount; i++)
        if ((*(pImp->aArr)[i]) == (*pInfo))
            break;

    if (i == nCount)
    {
        // Macro still unknown
        nCount = aIdArray.Count();
        sal_uInt16 n;
        for (n=0; n<nCount; n++) // Seearch for free SlotId
            if (aIdArray[n] > SID_MACRO_START + n)
                break;

        sal_uInt16 nNewSlotId = SID_MACRO_START + n;
        if ( nNewSlotId > SID_MACRO_END )
            return 0;
        aIdArray.Insert( SID_MACRO_START + n, n );

        SfxSlot *pNewSlot = new SfxSlot;
        pNewSlot->nSlotId = SID_MACRO_START + n;
        pNewSlot->nGroupId = 0;
        pNewSlot->nFlags = SFX_SLOT_ASYNCHRON;
        pNewSlot->nMasterSlotId = 0;
        pNewSlot->nValue = 0;
        pNewSlot->fnExec = SFX_STUB_PTR(SfxApplication,MacroExec_Impl);
        pNewSlot->fnState = SFX_STUB_PTR(SfxApplication,MacroState_Impl);
        pNewSlot->pType = 0; HACK(SFX_TYPE(SfxVoidItem))
        pNewSlot->pName = pNewSlot->pMethodName = U2S(pInfo->aMethodName).getStr();
        pNewSlot->pLinkedSlot = 0;
        pNewSlot->nArgDefCount = 0;
        pNewSlot->pFirstArgDef = 0;
        pNewSlot->pUnoName = 0;

        if (nCount)
        {
            SfxSlot *pSlot = (pImp->aArr)[0]->pSlot;
            pNewSlot->pNextSlot = pSlot->pNextSlot;
            pSlot->pNextSlot = pNewSlot;
        }
        else
            pNewSlot->pNextSlot = pNewSlot;

        // Take over Macro
        SfxMacroInfoPtr pNewInfo = new SfxMacroInfo(*pInfo);
        pNewInfo->nSlotId = SID_MACRO_START + n;
        pImp->aArr.Insert(pNewInfo,n);
        pNewInfo->pSlot = pNewSlot;
        pInfo->nSlotId = pNewInfo->nSlotId;
        pNewInfo->nRefCnt++;
    }
    else
    {
        pInfo->nSlotId = (pImp->aArr)[i]->nSlotId;
        (pImp->aArr)[i]->nRefCnt++;
    }

    return pInfo->nSlotId;
}

//==========================================================================

void SfxMacroConfig::ReleaseSlotId(sal_uInt16 nId)
{
    DBG_ASSERT( IsMacroSlot( nId ), "SlotId is no Macro!");

    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        SfxMacroInfo *pInfo = (pImp->aArr)[i];
        if (pInfo->nSlotId == nId)
        {
            pInfo->nRefCnt--;
            if (pInfo->nRefCnt == 0)
            {
                // Slot is no longer referenced, so get
                SfxSlot *pSlot = pInfo->pSlot;

                // Take out Slot from the concatenation
                while (pSlot->pNextSlot != pInfo->pSlot)
                    pSlot = (SfxSlot*) pSlot->pNextSlot;
                pSlot->pNextSlot = pInfo->pSlot->pNextSlot;

                // Slot close itself briefly
                pSlot = pInfo->pSlot;
                pSlot->pNextSlot = pSlot;

                // Remove Macro info from array so that it can not cause
                // any harm
                pImp->aArr.Remove(i);

                // Release SlotId again
                sal_uInt16 nIdCount = aIdArray.Count();
                for (sal_uInt16 n=0; n<nIdCount; n++)
                {
                    if (aIdArray[n] == nId)
                    {
                        aIdArray.Remove(n);
                        break;
                    }
                }

                // Unless the application is not shut down, then
                // the Slot has to be deleted asynchronously if cancelled in
                // its own Execute!
                if ( !SFX_APP()->Get_Impl()->bInQuit )
                    pImp->nEventId = Application::PostUserEvent( LINK(this, SfxMacroConfig, EventHdl_Impl), pInfo );
                else
                    EventHdl_Impl( pInfo );
            }
            return;
        }
    }

    OSL_FAIL("Macro-SlotId is not found!");
}

//==========================================================================

void SfxMacroConfig::RegisterSlotId(sal_uInt16 nId)
{
    DBG_ASSERT( IsMacroSlot( nId ), "SlotId is no Macro!");

    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        if ((pImp->aArr)[i]->nSlotId == nId)
        {
            (pImp->aArr)[i]->nRefCnt++;
            return;
        }
    }

    OSL_FAIL("Macro-SlotId is not found!");
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

//==========================================================================

const SfxMacroInfo* SfxMacroConfig::GetMacroInfo_Impl( const SvxMacro *pMacro ) const
{
    sal_uInt16 nCount = pImp->aArr.Count();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ((pImp->aArr)[i]->Compare(*pMacro) )
            return (pImp->aArr)[i];
    return 0;
}

//==========================================================================

sal_Bool SfxMacroConfig::ExecuteMacro( sal_uInt16 nId, const String& rArgs ) const
{
    const SfxMacroInfo* pInfo = GetMacroInfo( nId );
    if ( !pInfo )
        return sal_False;

    SfxObjectShell* pSh = SfxObjectShell::Current();

    SvxMacro aMacro( pInfo->GetQualifiedName(), pInfo->GetBasicName(), STARBASIC );
    sal_Bool bRet = ExecuteMacro( pSh, &aMacro, rArgs );

    // Release, because a register was created in the dispatcher Execute
    ((SfxMacroConfig*)this)->ReleaseSlotId( nId );
    return bRet;
}

sal_Bool SfxMacroConfig::ExecuteMacro( SfxObjectShell *pSh, const SvxMacro* pMacro, const String& /*rArgs*/ ) const
{
    SfxApplication *pApp = SFX_APP();

    // Name of the Macros or Scripts (ScriptCode)
    String aCode( pMacro->GetMacName() );
    ErrCode nErr = ERRCODE_NONE;

    // Is it a Basic-Macro ?
    ScriptType eSType = pMacro->GetScriptType();
    sal_Bool bIsBasic = eSType == STARBASIC;
    sal_Bool bIsStarScript = ( eSType == EXTENDED_STYPE && pMacro->GetLibName().SearchAscii( "StarScript" ) != STRING_NOTFOUND );
    sal_Bool bIsBasicLibBased = bIsBasic || bIsStarScript || !pSh;

    if ( bIsBasicLibBased )
    {
        pApp->EnterBasicCall();
        BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
        if( bIsBasic )
        {
            // BasicManager of the Document?
            BasicManager *pMgr = pSh ? pSh->GetBasicManager() : NULL;

            // As the name has unfortunately been changed often ...
            if( SFX_APP()->GetName() == pMacro->GetLibName() ||
                    pMacro->GetLibName().EqualsAscii("StarDesktop") )
                pMgr = pAppMgr;
            else if ( pMgr == pAppMgr )
                pMgr = NULL;

            if ( pSh && pMgr && pMgr != pAppMgr )
            {
                if ( !pSh->AdjustMacroMode( String() ) )
                    return sal_False;
            }

            if ( pSh && pMgr && pMgr == pAppMgr )
            {
                ::com::sun::star::uno::Any aOldThisComponent = pAppMgr->SetGlobalUNOConstant( "ThisComponent", makeAny( pSh->GetModel() ) );
                nErr = Call( 0, aCode, pMgr );
                pAppMgr->SetGlobalUNOConstant( "ThisComponent", aOldThisComponent );
            }
            else if ( pMgr )
                nErr = Call( 0, aCode, pMgr );
            else
                nErr = SbxERR_NO_METHOD;

        }

        pApp->LeaveBasicCall();
    }
    else
    {
        nErr = SbxERR_NO_METHOD;
    }

    return ( nErr == ERRCODE_NONE );
}

sal_Bool SfxMacroConfig::CheckMacro( SfxObjectShell *pSh, const SvxMacro* pMacro ) const
{
    SfxApplication *pApp = SFX_APP();

    // Name of Macros or Scripts (ScriptCode)
    String aCode( pMacro->GetMacName() );
    ErrCode nErr = ERRCODE_NONE;

    // BasicManager of Document or Application
    pApp->EnterBasicCall();
    BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
    BasicManager *pMgr = pSh ? pSh->GetBasicManager() : NULL;

    // As the name has unfortunately been changed often ...
    if( SFX_APP()->GetName() == pMacro->GetLibName() ||
            pMacro->GetLibName().EqualsAscii("StarDesktop") )
        pMgr = pAppMgr;
    else if ( pMgr == pAppMgr )
        pMgr = NULL;

    if ( !pMgr || !SfxQueryMacro( pMgr, aCode ) )
        nErr = SbxERR_NO_METHOD;
    pApp->LeaveBasicCall();
    return ( nErr == ERRCODE_NONE );
}

//==========================================================================

sal_Bool SfxMacroConfig::CheckMacro( sal_uInt16 nId ) const
{
    const SfxMacroInfo* pInfo = GetMacroInfo( nId );
    if ( !pInfo )
        return sal_False;

    // only initialize Basic, when default is not
    // ::com::sun::star::script::JavaScript; then the Basic has to be created
    // anyway in IsBasic()
    SfxObjectShell* pSh = SfxObjectShell::Current();

    SfxApplication *pApp = SFX_APP();
    pApp->EnterBasicCall();

    // BasicManager of Document or Application
    BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
    BasicManager *pMgr = pSh ? pSh->GetBasicManager() : NULL;

    if( SFX_APP()->GetName() == pInfo->GetBasicName() )
        pMgr = SFX_APP()->GetBasicManager();
    else if ( pMgr == pAppMgr )
        pMgr = NULL;

    String aFull( pInfo->GetQualifiedName() );
    sal_Bool bIsBasic = pMgr ? IsBasic( 0, aFull, pMgr ) : sal_False;
    pApp->LeaveBasicCall();
    return bIsBasic;
}

//==========================================================================

IMPL_LINK( SfxMacroConfig, CallbackHdl_Impl, SfxMacroConfig*, pConfig )
{
    (void)pConfig; // unused
    pImp->bWaitingForCallback = sal_False;
    return 0;
}

IMPL_LINK( SfxMacroConfig, EventHdl_Impl, SfxMacroInfo*, pInfo )
{
    delete pInfo;
    pImp->nEventId = 0;
    return 0;
}

sal_Bool SfxMacroConfig::IsBasic(
    SbxObject* /*pVCtrl*/,
    const String& rCode,
    BasicManager* pMgr )
{
    sal_Bool bFound;
    SFX_APP()->EnterBasicCall();
    bFound = SfxQueryMacro( pMgr, rCode ) != 0;
    SFX_APP()->LeaveBasicCall();
    return bFound;
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
