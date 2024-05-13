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

#include <unotools/resmgr.hxx>
#include <utility>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sal/log.hxx>

#include <svtools/ehdl.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/sfxecode.hxx>
#include <memory>
#include <errtxt.hrc>

static DialogMask aWndFunc(
    weld::Window *pWin,            // Parent of the dialog
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
    VclButtonsType eButtonsType = VclButtonsType::NONE;
    bool bAddRetry = false;
    if ( (nFlags & (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry)) == (DialogMask::ButtonsCancel | DialogMask::ButtonsRetry))
    {
        bAddRetry = true;
        eButtonsType = VclButtonsType::Cancel;
    }
    else if ( (nFlags & DialogMask::ButtonsOk) == DialogMask::ButtonsOk )
        eButtonsType = VclButtonsType::Ok;
    else if ( (nFlags & DialogMask::ButtonsYesNo) == DialogMask::ButtonsYesNo )
        eButtonsType = VclButtonsType::YesNo;

    OUString aErr(u"$(ACTION)$(ERROR)"_ustr);
    OUString aAction(rAction);
    if ( !aAction.isEmpty() )
        aAction += ":\n";
    aErr = aErr.replaceAll("$(ACTION)", aAction);
    aErr = aErr.replaceAll("$(ERROR)", rErr);

    VclMessageType eMessageType;
    switch (nFlags & DialogMask(0xf000))
    {
        case DialogMask::MessageError:
            eMessageType = VclMessageType::Error;
            break;

        case DialogMask::MessageWarning:
            eMessageType = VclMessageType::Warning;
            break;

        case DialogMask::MessageInfo:
            eMessageType = VclMessageType::Info;
            break;

        default:
        {
            SAL_WARN( "svtools.misc", "no MessBox type");
            return DialogMask::ButtonsOk;
        }
    }

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin,
                                              eMessageType, eButtonsType, aErr));

    if (bAddRetry)
        xBox->add_button(GetStandardText(StandardButtonType::Retry), RET_RETRY);

    switch(nFlags & DialogMask(0x0f00))
    {
        case DialogMask::ButtonDefaultsOk:
            xBox->set_default_response(RET_OK);
            break;
        case DialogMask::ButtonDefaultsCancel:
            xBox->set_default_response(RET_CANCEL);
            break;
        case DialogMask::ButtonDefaultsYes:
            xBox->set_default_response(RET_YES);
            break;
        case DialogMask::ButtonDefaultsNo:
            xBox->set_default_response(RET_NO);
            break;
        default:
            break;
    }

    DialogMask nRet = DialogMask::NONE;
    switch (xBox->run())
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

    return nRet;
}

SfxErrorHandler::SfxErrorHandler(const ErrMsgCode* pIdPs, ErrCodeArea lStartP, ErrCodeArea lEndP, const std::locale& rLocale)
    : lStart(lStartP), lEnd(lEndP), pIds(pIdPs), aResLocale(rLocale)
{
    ErrorRegistry::RegisterDisplay(&aWndFunc);
}

SfxErrorHandler::~SfxErrorHandler()
{
}

bool SfxErrorHandler::CreateString(const ErrCodeMsg& nErr, OUString &rStr) const

/*  [Description]

    Assemble error string for the ErrorInfo pErr.

    */

{
    ErrCode nErrCode(sal_uInt32(nErr.GetCode()) & ERRCODE_ERROR_MASK);
    if (nErr.GetCode().GetArea() < lStart || lEnd < nErr.GetCode().GetArea())
        return false;
    if(GetErrorString(nErrCode, rStr))
    {
        if(!nErr.GetArg1().isEmpty())
            rStr = rStr.replaceAll("$(ARG1)", nErr.GetArg1());
        if(!nErr.GetArg2().isEmpty())
            rStr = rStr.replaceAll("$(ARG2)", nErr.GetArg2());
        return true;
    }
    return false;
}

void SfxErrorHandler::GetClassString(ErrCodeClass lClassId, OUString &rStr)

/*  [Description]

    Creates the string for the class of the error. Will always
    be read from the resource of the Sfx.

    */

{
    for (const std::pair<TranslateId, ErrCodeClass>* pItem = RID_ERRHDL_CLASS; pItem->first; ++pItem)
    {
        if (pItem->second == lClassId)
        {
            rStr = SvtResId(pItem->first);
            break;
        }
    }
}

bool SfxErrorHandler::GetErrorString(ErrCode lErrId, OUString &rStr) const

/*  [Description]

    Creates the error string for the actual error
    without its class

    */

{
    bool bRet = false;
    rStr = "$(CLASS)$(ERROR)";

    for (const ErrMsgCode* pItem = pIds; pItem->second; ++pItem)
    {
        if (pItem->second.StripWarning() == lErrId.StripWarning())
        {
            rStr = rStr.replaceAll("$(ERROR)", Translate::get(pItem->first, aResLocale));
            bRet = true;
            break;
        }
    }

    if( bRet )
    {
        OUString aErrStr;
        GetClassString(lErrId.GetClass(), aErrStr);
        if(!aErrStr.isEmpty())
            aErrStr += ".\n";
        rStr = rStr.replaceAll("$(CLASS)",aErrStr);
    }

    return bRet;
}

SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, weld::Window *pWindow, const ErrMsgCode* pIdsP, const std::locale& rResLocaleP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), pIds(pIdsP), aResLocale(rResLocaleP)
{
    if (!pIds)
        pIds = RID_ERRCTX;
}


SfxErrorContext::SfxErrorContext(
    sal_uInt16 nCtxIdP, OUString aArg1P, weld::Window *pWindow,
    const ErrMsgCode* pIdsP, const std::locale& rResLocaleP)
:   ErrorContext(pWindow), nCtxId(nCtxIdP), pIds(pIdsP), aResLocale(rResLocaleP),
    aArg1(std::move(aArg1P))
{
    if (!pIds)
        pIds = RID_ERRCTX;
}

bool SfxErrorContext::GetString(const ErrCodeMsg& nErr, OUString &rStr)

/*  [Description]

    Constructs the description of an error context
    */

{
    bool bRet = false;
    for (const ErrMsgCode* pItem = pIds; pItem->second; ++pItem)
    {
        if (sal_uInt32(pItem->second) == nCtxId)
        {
            rStr = Translate::get(pItem->first, aResLocale);
            rStr = rStr.replaceAll("$(ARG1)", aArg1);
            bRet = true;
            break;
        }
    }

    SAL_WARN_IF(!bRet, "svtools.misc", "ErrorContext cannot find the resource");

    if ( bRet )
    {
        sal_uInt16 nId = nErr.IsWarning() ? ERRCTX_WARNING : ERRCTX_ERROR;
        for (const ErrMsgCode* pItem = RID_ERRCTX; pItem->second; ++pItem)
        {
            if (sal_uInt32(pItem->second) == nId)
            {
                rStr = rStr.replaceAll("$(ERR)", Translate::get(pItem->first, aResLocale));
                break;
            }
        }
    }

    // SfxInPlaceClient::DoVerb adds some extra info to report
    if (bRet && nErr.GetCode() == ERRCODE_SO_GENERALERROR && !nErr.GetArg1().isEmpty())
            rStr += "\n" + nErr.GetArg1();

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
