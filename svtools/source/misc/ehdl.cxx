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

#include <tools/rcid.h>
#include <tools/resary.hxx>
#include <tools/wintypes.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svtools/ehdl.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <svtools/sfxecode.hxx>
#include <memory>
#include "strings.hxx"

static DialogMask aWndFunc(
    vcl::Window *pWin,            // Parent of the dialog
    DialogMask nFlags,
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
    if ( (nFlags & (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry)) == (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry))
        eBits = WB_RETRY_CANCEL;
    else if ( (nFlags & DialogMask::ButtonsOkCancel) == DialogMask::ButtonsOkCancel )
        eBits = WB_OK_CANCEL;
    else if ( (nFlags & DialogMask::ButtonsOk) == DialogMask::ButtonsOk )
        eBits = WB_OK;
    else if ( (nFlags & DialogMask::ButtonsYesNoCancel) == DialogMask::ButtonsYesNoCancel )
        eBits = WB_YES_NO_CANCEL;
    else if ( (nFlags & DialogMask::ButtonsYesNo) == DialogMask::ButtonsYesNo )
        eBits = WB_YES_NO;

    switch(nFlags & DialogMask(0x0f00))
    {
      case DialogMask::ButtonDefaultsOk:
            eBits |= WB_DEF_OK;
            break;

      case DialogMask::ButtonDefaultsCancel:
            eBits |= WB_DEF_CANCEL;
            break;

      case DialogMask::ButtonDefaultsYes:
            eBits |= WB_DEF_YES;
            break;

      case DialogMask::ButtonDefaultsNo:
            eBits |= WB_DEF_NO;
            break;
      default: break;
    }

    OUString aErr(STR_ERR_HDLMESS);
    OUString aAction(rAction);
    if ( !aAction.isEmpty() )
        aAction += ":\n";
    aErr = aErr.replaceAll("$(ACTION)", aAction);
    aErr = aErr.replaceAll("$(ERROR)", rErr);

    VclPtr<MessBox> pBox;
    switch ( nFlags & DialogMask(0xf000) )
    {
        case DialogMask::MessageError:
            pBox.reset(VclPtr<ErrorBox>::Create(pWin, eBits, aErr));
            break;

        case DialogMask::MessageWarning:
            pBox.reset(VclPtr<WarningBox>::Create(pWin, eBits, aErr));
            break;

        case DialogMask::MessageInfo:
            pBox.reset(VclPtr<InfoBox>::Create(pWin, aErr));
            break;

        default:
        {
            SAL_WARN( "svtools.misc", "no MessBox type");
            return DialogMask::ButtonsOk;
        }
    }

    DialogMask nRet = DialogMask::NONE;
    switch ( pBox->Execute() )
    {
        case RET_OK:
            nRet = DialogMask::ButtonsOk;
            break;
        case RET_CANCEL:
            nRet = DialogMask::ButtonsCancel;
            break;
        case RET_RETRY:
            nRet = DialogMask::ButtonsRetry;
            break;
        case RET_YES:
            nRet = DialogMask::ButtonsYes;
            break;
        case RET_NO:
            nRet = DialogMask::ButtonsNo;
            break;
        default:
            SAL_WARN( "svtools.misc", "Unknown MessBox return value" );
            break;
    }
    pBox.disposeAndClear();
    return nRet;
}


SfxErrorHandler::SfxErrorHandler(sal_uInt16 nIdP, ErrCode lStartP, ErrCode lEndP, ResMgr *pMgrP) :

    lStart(lStartP), lEnd(lEndP), nId(nIdP), pMgr(pMgrP), pFreeMgr( nullptr )

{
    ErrorRegistry::RegisterDisplay(&aWndFunc);
    if( ! pMgr )
    {
        pMgr = ResMgr::CreateResMgr("svt", Application::GetSettings().GetUILanguageTag() );
        pFreeMgr.reset(pMgr);
    }
}


SfxErrorHandler::~SfxErrorHandler()
{
}


bool SfxErrorHandler::CreateString(const ErrorInfo *pErr, OUString &rStr) const

/*  [Description]

    Assemble error string for the ErrorInfo pErr.

    */

{
    ErrCode nErrCode = pErr->GetErrorCode().IgnoreWarning();
    if( nErrCode>=lEnd || nErrCode<=lStart )
        return false;
    if(GetErrorString(nErrCode, rStr))
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

void SfxErrorHandler::GetClassString(sal_uLong lClassId, OUString &rStr)

/*  [Description]

    Creates the string for the class of the error. Will always
    be read from the resource of the Sfx.

    */

{
    std::unique_ptr<ResMgr> pResMgr(ResMgr::CreateResMgr("svt", Application::GetSettings().GetUILanguageTag() ));
    if( pResMgr )
    {
        ResStringArray aEr(ResId(RID_ERRHDL, *pResMgr));
        sal_uInt32 nErrIdx = aEr.FindIndex((sal_uInt16)lClassId);
        if (nErrIdx != RESARRAY_INDEX_NOTFOUND)
        {
            rStr = aEr.GetString(nErrIdx);
        }
    }
}

bool SfxErrorHandler::GetErrorString(ErrCode lErrId, OUString &rStr) const

/*  [Description]

    Creates the error string for the actual error
    without its class

    */

{
    SolarMutexGuard aGuard;

    bool bRet = false;
    rStr = RID_ERRHDL_CLASS;

    ResStringArray aEr(ResId(nId, *pMgr));
    sal_uInt32 nErrIdx = aEr.FindIndex((sal_uInt16)(sal_uInt32)lErrId);
    if (nErrIdx != RESARRAY_INDEX_NOTFOUND)
    {
        rStr = rStr.replaceAll("$(ERROR)", aEr.GetString(nErrIdx));
        bRet = true;
    }
    else
        bRet = false;

    if( bRet )
    {
        OUString aErrStr;
        GetClassString((sal_uInt32)lErrId & ERRCODE_CLASS_MASK,
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


bool SfxErrorContext::GetString(ErrCode nErrId, OUString &rStr)

/*  [Description]

    Constructs the description of a error context
    */

{
    bool bRet = false;
    ResMgr* pFreeMgr = nullptr;
    if( ! pMgr )
    {
        pFreeMgr = pMgr = ResMgr::CreateResMgr("svt", Application::GetSettings().GetUILanguageTag() );
    }
    if( pMgr )
    {
        SolarMutexGuard aGuard;

        ResStringArray aTestEr(ResId(nResId, *pMgr));
        sal_uInt32 nErrIdx = aTestEr.FindIndex(nCtxId);
        if (nErrIdx != RESARRAY_INDEX_NOTFOUND)
        {
            rStr = aTestEr.GetString(nErrIdx);
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
            sal_uInt16 nId = nErrId.IsWarning() ? ERRCTX_WARNING : ERRCTX_ERROR;
            ResStringArray aEr(ResId(RID_ERRCTX, *pMgr));
            rStr = rStr.replaceAll("$(ERR)", aEr.GetString(nId));
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
