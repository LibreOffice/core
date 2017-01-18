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
#include <vcl/settings.hxx>

#include <svtools/ehdl.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <svtools/sfxecode.hxx>
#include <memory>


static sal_uInt16 aWndFunc(
    vcl::Window *pWin,            // Parent of the dialog
    sal_uInt16 nFlags,
    const OUString &rErr,      // error text
    const OUString &rAction)   // action text

/*  [Description]

    Draw an errorbox on the screen. Depending on nFlags
    Error/Info etc. boxes with the requested buttons are shown.

    Returnvalue is the button pressed

    */


{
    SolarMutexGuard aGuard;

    // determine necessary WinBits from the flags
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

    VclPtr<MessBox> pBox;
    switch ( nFlags & 0xf000 )
    {
        case ERRCODE_MSG_ERROR:
            pBox.reset(VclPtr<ErrorBox>::Create(pWin, eBits, aErr));
            break;

        case ERRCODE_MSG_WARNING:
            pBox.reset(VclPtr<WarningBox>::Create(pWin, eBits, aErr));
            break;

        case ERRCODE_MSG_INFO:
            pBox.reset(VclPtr<InfoBox>::Create(pWin, aErr));
            break;

        case ERRCODE_MSG_QUERY:
            pBox.reset(VclPtr<QueryBox>::Create(pWin, eBits, aErr));
            break;

        default:
        {
            SAL_WARN( "svtools.misc", "no MessBox type");
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
    pBox.disposeAndClear();
    return nRet;
}


SfxErrorHandler::SfxErrorHandler(sal_uInt16 nIdP, sal_uLong lStartP, sal_uLong lEndP, ResMgr *pMgrP) :

    lStart(lStartP), lEnd(lEndP), nId(nIdP), pMgr(pMgrP), pFreeMgr( nullptr )

{
    RegisterDisplay(&aWndFunc);
    if( ! pMgr )
    {
        pMgr = ResMgr::CreateResMgr("ofa", Application::GetSettings().GetUILanguageTag() );
        pFreeMgr.reset(pMgr);
    }
}


SfxErrorHandler::~SfxErrorHandler()
{
}


bool SfxErrorHandler::CreateString(
    const ErrorInfo *pErr, OUString &rStr, sal_uInt16& nFlags) const

/*  [Description]

    Assemble error string for the ErrorInfo pErr.

    */

{
    sal_uLong nErrCode = pErr->GetErrorCode() & ERRCODE_ERROR_MASK;
    if( nErrCode>=lEnd || nErrCode<=lStart )
        return false;
    if(GetErrorString(nErrCode, rStr, nFlags))
    {
        const StringErrorInfo *pStringInfo = dynamic_cast<const StringErrorInfo *>(pErr);
        if(pStringInfo)
        {
            rStr = rStr.replaceAll("$(ARG1)", pStringInfo->GetErrorString());
        }
        else
        {
            const TwoStringErrorInfo * pTwoStringInfo = dynamic_cast<const TwoStringErrorInfo* >(pErr);
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


class ResString: public OUString

/*  [Description]

    Helpclass to read a string and optional ExtraData from
    a string Resource.

    */

{
    sal_uInt16 nFlags;
  public:
    sal_uInt16 GetFlags() const {return nFlags;}
    const OUString & GetString() const {return *this;}
    explicit ResString( ResId &rId);
};


ResString::ResString(ResId & rId):
    OUString(rId.SetAutoRelease(false).toString()),
    nFlags(0)
{
    ResMgr * pResMgr = rId.GetResMgr();
     // String ctor temporarily sets global ResManager
    if (pResMgr->GetRemainSize())
        nFlags = sal_uInt16(pResMgr->ReadShort());
    rId.SetAutoRelease(true);
    pResMgr->PopContext();
}


struct ErrorResource_Impl : private Resource

/*  [Description]

    Helpclass for access to string sub-resources of a resource
    */

{

    ResId aResId;

    ErrorResource_Impl(ResId& rErrIdP, sal_uInt16 nId)
        : Resource(rErrIdP),aResId(nId,*rErrIdP.GetResMgr()){}

    ~ErrorResource_Impl() { FreeResource(); }

    ResString GetResString() { return ResString( aResId ); }
    operator bool()          { return IsAvailableRes(aResId.SetRT(RSC_STRING)); }

};


void SfxErrorHandler::GetClassString(sal_uLong lClassId, OUString &rStr)

/*  [Description]

    Creates the string for the class of the error. Will always
    be read from the resource of the Sfx.

    */

{
    std::unique_ptr<ResMgr> pResMgr(ResMgr::CreateResMgr("ofa", Application::GetSettings().GetUILanguageTag() ));
    if( pResMgr )
    {
        ResId aId(RID_ERRHDL, *pResMgr );
        ErrorResource_Impl aEr(aId, (sal_uInt16)lClassId);
        if(aEr)
        {
            rStr = aEr.GetResString().GetString();
        }
    }
}


bool SfxErrorHandler::GetErrorString(
    sal_uLong lErrId, OUString &rStr, sal_uInt16 &nFlags) const

/*  [Description]

    Creates the error string for the actual error
    without its class

    */

{
    SolarMutexGuard aGuard;

    bool bRet = false;
    rStr = SvtResId(RID_ERRHDL_CLASS).toString();
    ResId aResId(nId, *pMgr);

    {
        ErrorResource_Impl aEr(aResId, (sal_uInt16)lErrId);
        if(aEr)
        {
            ResString aErrorString(aEr.GetResString());

            sal_uInt16 nResFlags = aErrorString.GetFlags();
            if ( nResFlags )
                nFlags = nResFlags;
            rStr = rStr.replaceAll("$(ERROR)", aErrorString.GetString());
            bRet = true;
        }
        else
            bRet = false;
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


SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, vcl::Window *pWindow, sal_uInt16 nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP)
{
    if( nResId==USHRT_MAX )
        nResId=RID_ERRCTX;
}


SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, const OUString &aArg1P, vcl::Window *pWindow,
    sal_uInt16 nResIdP, ResMgr *pMgrP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), nResId(nResIdP), pMgr(pMgrP),
    aArg1(aArg1P)
{
    if( nResId==USHRT_MAX )
        nResId=RID_ERRCTX;
}


bool SfxErrorContext::GetString(sal_uLong nErrId, OUString &rStr)

/*  [Description]

    Constructs the description of a error context
    */

{
    bool bRet = false;
    ResMgr* pFreeMgr = nullptr;
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
            rStr = aTestEr.GetResString().GetString();
            rStr = rStr.replaceAll("$(ARG1)", aArg1);
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
            rStr = rStr.replaceAll("$(ERR)", aEr.GetResString().GetString());
        }
    }

    if( pFreeMgr )
    {
        delete pFreeMgr;
        pMgr = nullptr;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
