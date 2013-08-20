/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/mutex.hxx>
#include <tools/rcid.h>
#include <tools/wintypes.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <svtools/ehdl.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <svtools/sfxecode.hxx>

//=========================================================================

static sal_uInt16 aWndFunc(
    Window *pWin,            // Parent des Dialoges
    sal_uInt16 nFlags,
    const OUString &rErr,      // Fehlertext
    const OUString &rAction)   // Actiontext

/*  [Beschreibung]

    Bringt eine Fehlerbox auf den Schirm. Je nach nFlags werden
    Error/ Info usw. Boxen mit den gewuenschten Buttons angezeigt

    Rueckgabewert ist der gedrueckte Button

    */


{
    SolarMutexGuard aGuard;

    // aus den Flags die benoetigten WinBits ermitteln
    WinBits eBits=0;
    if ( (ERRCODE_BUTTON_CANCEL|ERRCODE_BUTTON_RETRY) == (nFlags & (ERRCODE_BUTTON_CANCEL|ERRCODE_BUTTON_RETRY)) )
        eBits = WB_RETRY_CANCEL;
    else if ( ERRCODE_BUTTON_OK_CANCEL == (nFlags & ERRCODE_BUTTON_OK_CANCEL) )
        eBits = WB_OK_CANCEL;
    else if ( ERRCODE_BUTTON_OK == (nFlags & ERRCODE_BUTTON_OK) )
        eBits = WB_OK;
    else if ( ERRCODE_BUTTON_YES_NO_CANCEL == (nFlags & ERRCODE_BUTTON_YES_NO_CANCEL) )
        eBits = WB_YES_NO_CANCEL;
    else if ( ERRCODE_BUTTON_YES_NO == (nFlags & ERRCODE_BUTTON_YES_NO) )
        eBits = WB_YES_NO;

    switch(nFlags & 0x0f00)
    {
      case ERRCODE_BUTTON_DEF_OK:
            eBits |= WB_DEF_OK;
            break;

      case ERRCODE_BUTTON_DEF_CANCEL:
            eBits |= WB_DEF_CANCEL;
            break;

      case ERRCODE_BUTTON_DEF_YES:
            eBits |= WB_DEF_YES;
            break;

      case ERRCODE_BUTTON_DEF_NO:
            eBits |= WB_DEF_NO;
            break;
    }

    OUString aErr(SvtResId(STR_ERR_HDLMESS).toString());
    OUString aAction(rAction);
    if ( !aAction.isEmpty() )
        aAction += ":\n";
    aErr = aErr.replaceAll("$(ACTION)", aAction);
    aErr = aErr.replaceAll("$(ERROR)", rErr);

    MessBox* pBox;
    switch ( nFlags & 0xf000 )
    {
        case ERRCODE_MSG_ERROR:
            pBox = new ErrorBox(pWin, eBits, aErr);
            break;

        case ERRCODE_MSG_WARNING:
            pBox = new WarningBox(pWin, eBits, aErr);
            break;

        case ERRCODE_MSG_INFO:
            pBox = new InfoBox(pWin, aErr);
            break;

        case ERRCODE_MSG_QUERY:
            pBox = new QueryBox(pWin, eBits, aErr);
            break;

        default:
        {
            SAL_WARN( "svtools.misc", "no MessBox type");
            pBox = NULL;
            return ERRCODE_BUTTON_OK;
        }
    }

    sal_uInt16 nRet = RET_CANCEL;
    switch ( pBox->Execute() )
    {
        case RET_OK:
            nRet = ERRCODE_BUTTON_OK;
            break;
        case RET_CANCEL:
            nRet = ERRCODE_BUTTON_CANCEL;
            break;
        case RET_RETRY:
            nRet = ERRCODE_BUTTON_RETRY;
            break;
        case RET_YES:
            nRet = ERRCODE_BUTTON_YES;
            break;
        case RET_NO:
            nRet = ERRCODE_BUTTON_NO;
            break;
        default:
            SAL_WARN( "svtools.misc", "Unknown MessBox return value" );
            break;
    }
    delete pBox;
    return nRet;
}

//-------------------------------------------------------------------------

SfxErrorHandler::SfxErrorHandler(sal_uInt16 nIdP, sal_uLong lStartP, sal_uLong lEndP, ResMgr *pMgrP) :

    lStart(lStartP), lEnd(lEndP), nId(nIdP), pMgr(pMgrP), pFreeMgr( NULL )

{
    RegisterDisplay(&aWndFunc);
    if( ! pMgr )
    {
        pFreeMgr = pMgr = ResMgr::CreateResMgr("ofa", Application::GetSettings().GetUILanguageTag() );
    }
}

//-------------------------------------------------------------------------

SfxErrorHandler::~SfxErrorHandler()
{
    delete pFreeMgr;
}

//-------------------------------------------------------------------------

bool SfxErrorHandler::CreateString(
    const ErrorInfo *pErr, OUString &rStr, sal_uInt16& nFlags) const

/*  [Beschreibung]

    Der Fehlerstring fuer die ErrorInfo pErr wird zusammengesetzt.

    */

{
    sal_uLong nErrCode = pErr->GetErrorCode() & ERRCODE_ERROR_MASK;
    if( nErrCode>=lEnd || nErrCode<=lStart )
        return false;
    MessageInfo *pMsgInfo=PTR_CAST(MessageInfo,pErr);
    if(pMsgInfo)
    {
        if(GetMessageString(nErrCode, rStr, nFlags))
        {
            rStr = rStr.replaceAll("$(ARG1)", pMsgInfo->GetMessageArg());
            return true;
        }
    }
    else if(GetErrorString(nErrCode, rStr, nFlags))
    {
        StringErrorInfo *pStringInfo=PTR_CAST(StringErrorInfo,pErr);
        if(pStringInfo)
        {
            rStr = rStr.replaceAll(OUString("$(ARG1)"),
                                      pStringInfo->GetErrorString());
        }
        else
        {
            TwoStringErrorInfo * pTwoStringInfo = PTR_CAST(TwoStringErrorInfo,
                                                           pErr);
            if (pTwoStringInfo)
            {
                rStr = rStr.replaceAll("$(ARG1)", pTwoStringInfo->GetArg1());
                rStr = rStr.replaceAll("$(ARG2)", pTwoStringInfo->GetArg2());
            }
        }
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------

class ResString: public String

/*  [Beschreibung]

    Hilfsklasse zum Auslesen eines Strings und optionaler ExtraData aus
    einer String Resource.

    */

{
    sal_uInt16 nFlags;
  public:
    sal_uInt16 GetFlags() const {return nFlags;}
    const String & GetString() const {return *this;}
    ResString( ResId &rId);
};

//-------------------------------------------------------------------------

ResString::ResString(ResId & rId):
    String(rId.SetAutoRelease(sal_False).toString()),
    nFlags(0)
{
    ResMgr * pResMgr = rId.GetResMgr();
     // String ctor temporarily sets global ResManager
    if (pResMgr->GetRemainSize())
        nFlags = sal_uInt16(pResMgr->ReadShort());
    rId.SetAutoRelease(sal_True);
    pResMgr->PopContext();
}

//-------------------------------------------------------------------------

struct ErrorResource_Impl : private Resource

/*  [Beschreibung]

    Hilfsklasse zum Zugriff auf String SubResourcen einer Resource
    */

{

    ResId aResId;

    ErrorResource_Impl(ResId& rErrIdP, sal_uInt16 nId)
        : Resource(rErrIdP),aResId(nId,*rErrIdP.GetResMgr()){}

    ~ErrorResource_Impl() { FreeResource(); }

    operator ResString(){ return ResString( aResId ); }
    operator sal_Bool(){return IsAvailableRes(aResId.SetRT(RSC_STRING));}

};


sal_Bool SfxErrorHandler::GetClassString(sal_uLong lClassId, OUString &rStr) const

/*  [Beschreibung]

    Erzeugt den String fuer die Klasse des Fehlers. Wird immer aus der
    Resource des Sfx gelesen

    */

{
    sal_Bool bRet = sal_False;
    ResMgr* pResMgr = ResMgr::CreateResMgr("ofa", Application::GetSettings().GetUILanguageTag() );
    if( pResMgr )
    {
        ResId aId(RID_ERRHDL, *pResMgr );
        ErrorResource_Impl aEr(aId, (sal_uInt16)lClassId);
        if(aEr)
        {
            rStr=((ResString)aEr).GetString();
            bRet = sal_True;
        }
    }
    delete pResMgr;
    return bRet;
}

//-------------------------------------------------------------------------

sal_Bool SfxErrorHandler::GetMessageString(
    sal_uLong lErrId, OUString &rStr, sal_uInt16 &nFlags) const

/*  [Beschreibung]

    Erzeugt den String fuer die Ausgabe in einer MessageBox

    */

{
    sal_Bool bRet = sal_False;
    ResId *pResId= new ResId(nId, *pMgr);

    ErrorResource_Impl aEr(*pResId, (sal_uInt16)lErrId);
    if(aEr)
    {
        ResString aErrorString(aEr);
        sal_uInt16 nResFlags = aErrorString.GetFlags();
        if( nResFlags )
            nFlags=aErrorString.GetFlags();
        rStr = aErrorString.GetString();
        bRet = sal_True;
    }

    delete pResId;
    return bRet;
}

//-------------------------------------------------------------------------

sal_Bool SfxErrorHandler::GetErrorString(
    sal_uLong lErrId, OUString &rStr, sal_uInt16 &nFlags) const

/*  [Beschreibung]
    Erzeugt den Fehlerstring fuer den eigentlichen Fehler ohne
    dessen Klasse

    */

{
    SolarMutexGuard aGuard;

    sal_Bool bRet = sal_False;
    rStr=SvtResId(RID_ERRHDL_CLASS).toString();
    ResId aResId(nId, *pMgr);

    {
        ErrorResource_Impl aEr(aResId, (sal_uInt16)lErrId);
        if(aEr)
        {
            ResString aErrorString(aEr);

            sal_uInt16 nResFlags = aErrorString.GetFlags();
            if ( nResFlags )
                nFlags = nResFlags;
            rStr = rStr.replaceAll(OUString("$(ERROR)"), aErrorString.GetString());
            bRet = sal_True;
        }
        else
            bRet = sal_False;
    }

    if( bRet )
    {
        OUString aErrStr;
        GetClassString(lErrId & ERRCODE_CLASS_MASK,
                       aErrStr);
        if(!aErrStr.isEmpty())
            aErrStr += ".\n";
        rStr = rStr.replaceAll("$(CLASS)",aErrStr);
    }

    return bRet;
}

//-------------------------------------------------------------------------

SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, Window *pWindow, sal_uInt16 nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP)
{
    if( nResId==USHRT_MAX )
        nResId=RID_ERRCTX;
}

//-------------------------------------------------------------------------

SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, const OUString &aArg1P, Window *pWindow,
    sal_uInt16 nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP),
    aArg1(aArg1P)
{
    if( nResId==USHRT_MAX )
        nResId=RID_ERRCTX;
}

//-------------------------------------------------------------------------

bool SfxErrorContext::GetString(sal_uLong nErrId, OUString &rStr)

/*  [Beschreibung]

    Baut die Beschreibung eines ErrorContextes auf
    */

{
    bool bRet = false;
    ResMgr* pFreeMgr = NULL;
    if( ! pMgr )
    {
        pFreeMgr = pMgr = ResMgr::CreateResMgr("ofa", Application::GetSettings().GetUILanguageTag() );
    }
    if( pMgr )
    {
        SolarMutexGuard aGuard;

        ResId aResId( nResId, *pMgr );

        ErrorResource_Impl aTestEr( aResId, nCtxId );
        if ( aTestEr )
        {
            rStr = ( (ResString)aTestEr ).GetString();
            rStr = rStr.replaceAll(OUString("$(ARG1)"), aArg1 );
            bRet = true;
        }
        else
        {
            SAL_WARN( "svtools.misc", "ErrorContext cannot find the resource" );
            bRet = false;
        }

        if ( bRet )
        {
            sal_uInt16 nId = ( nErrId & ERRCODE_WARNING_MASK ) ? ERRCTX_WARNING : ERRCTX_ERROR;
            ResId aSfxResId( RID_ERRCTX, *pMgr );
            ErrorResource_Impl aEr( aSfxResId, nId );
            rStr = rStr.replaceAll( OUString("$(ERR)"), ( (ResString)aEr ).GetString() );
        }
    }

    if( pFreeMgr )
    {
        delete pFreeMgr;
        pMgr = NULL;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
