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

#include <fuexecuteinteraction.hxx>

#include <app.hrc>
#include <config_features.h>
#include <avmedia/mediawindow.hxx>
#include <basic/sbstar.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <tools/urlobj.hxx>
#include <o3tl/string_view.hxx>

#include <DrawViewShell.hxx>
#include <GraphicDocShell.hxx>
#include <ViewShell.hxx>
#include <anminfo.hxx>
#include <drawdoc.hxx>
#include <drawview.hxx>
#include <pgjump.hxx>

#include <com/sun/star/media/XPlayer.hpp>

using namespace css;

namespace sd
{
FuExecuteInteraction::FuExecuteInteraction(ViewShell* pViewSh, ::sd::Window* pWin,
                                           ::sd::View* pView, SdDrawDocument* pDoc,
                                           SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuExecuteInteraction::Create(ViewShell* pViewSh, ::sd::Window* pWin,
                                                    ::sd::View* pView, SdDrawDocument* pDoc,
                                                    SfxRequest& rReq)
{
    rtl::Reference<FuPoor> xFunc(new FuExecuteInteraction(pViewSh, pWin, pView, pDoc, rReq));
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuExecuteInteraction::DoExecute(SfxRequest&)
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if (rMarkList.GetMarkCount() != 1)
        return;

    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if (dynamic_cast<const GraphicDocShell*>(mpDocSh) != nullptr
        || dynamic_cast<const DrawView*>(mpView) == nullptr)
        return;

    assert(mpDocSh);

    SdAnimationInfo* pInfo = SdDrawDocument::GetAnimationInfo(pObj);
    if (!pInfo)
        return;

    switch (pInfo->meClickAction)
    {
        case presentation::ClickAction_BOOKMARK:
        {
            // Jump to Bookmark (Page or Object)
            SfxStringItem aItem(SID_NAVIGATOR_OBJECT, pInfo->GetBookmark());
            mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_OBJECT, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
        }
        break;

        case presentation::ClickAction_DOCUMENT:
        {
            OUString sBookmark(pInfo->GetBookmark());
            // Jump to document
            if (!sBookmark.isEmpty())
            {
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxStringItem aStrItem(SID_FILE_NAME, sBookmark);
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem(SID_BROWSE, true);
                pFrame->GetDispatcher()->ExecuteList(
                    SID_OPENDOC, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                    { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });
            }
        }
        break;

        case presentation::ClickAction_PREVPAGE:
        {
            // Jump to the previous page
            SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_PREVIOUS);
            mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_PAGE, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
        }
        break;

        case presentation::ClickAction_NEXTPAGE:
        {
            // Jump to the next page
            SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_NEXT);
            mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_PAGE, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
        }
        break;

        case presentation::ClickAction_FIRSTPAGE:
        {
            // Jump to the first page
            SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_FIRST);
            mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_PAGE, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
        }
        break;

        case presentation::ClickAction_LASTPAGE:
        {
            // Jump to the last page
            SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_LAST);
            mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_NAVIGATOR_PAGE, SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
        }
        break;

        case presentation::ClickAction_SOUND:
        {
#if HAVE_FEATURE_AVMEDIA
            try
            {
                mxPlayer.set(avmedia::MediaWindow::createPlayer(pInfo->GetBookmark(), "" /*TODO?*/),
                             uno::UNO_SET_THROW);
                mxPlayer->start();
            }
            catch (uno::Exception&)
            {
            }
#endif
        }
        break;

        case presentation::ClickAction_VERB:
        {
            // Assign verb
            mpView->UnmarkAll();
            mpView->MarkObj(pObj, mpView->GetSdrPageView());
            DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(mpViewShell);
            pDrViewSh->DoVerb(static_cast<sal_Int16>(pInfo->mnVerb));
        }
        break;

        case presentation::ClickAction_PROGRAM:
        {
            OUString aBaseURL = GetDocSh()->GetMedium()->GetBaseURL();
            INetURLObject aURL(::URIHelper::SmartRel2Abs(
                INetURLObject(aBaseURL), pInfo->GetBookmark(), URIHelper::GetMaybeFileHdl(), true,
                false, INetURLObject::EncodeMechanism::WasEncoded,
                INetURLObject::DecodeMechanism::Unambiguous));

            if (INetProtocol::File == aURL.GetProtocol())
            {
                if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
                {
                    SfxStringItem aUrl(SID_FILE_NAME,
                                       aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE));
                    SfxBoolItem aBrowsing(SID_BROWSE, true);

                    pViewFrm->GetDispatcher()->ExecuteList(
                        SID_OPENDOC, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aUrl, &aBrowsing });
                }
            }
        }
        break;

#if HAVE_FEATURE_SCRIPTING
        case presentation::ClickAction_MACRO:
        {
            // Execute macro
            OUString aMacro = pInfo->GetBookmark();

            if (SfxApplication::IsXScriptURL(aMacro))
            {
                uno::Any aRet;
                uno::Sequence<sal_Int16> aOutArgsIndex;
                uno::Sequence<uno::Any> aParams;
                uno::Sequence<uno::Any> aOutArgs;

                mpDocSh->CallXScript(aMacro, aParams, aRet, aOutArgsIndex, aOutArgs);
            }
            else
            {
                // aMacro has got following format:
                // "Macroname.Modulname.Libname.Documentname" or
                // "Macroname.Modulname.Libname.Applicationname"
                sal_Int32 nIdx{ 0 };
                const std::u16string_view aMacroName = o3tl::getToken(aMacro, 0, '.', nIdx);
                const std::u16string_view aModulName = o3tl::getToken(aMacro, 0, '.', nIdx);

                // Currently the "Call" method only resolves modulename+macroname
                mpDocSh->GetBasic()->Call(OUString::Concat(aModulName) + "." + aMacroName);
            }
        }
        break;
#endif

        default:
            break;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
