/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/dispatch.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/menu.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <sfxlocal.hrc>
#include <sfx2/sfxresid.hxx>
#include "NotebookBarPopupMenu.hxx"

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;

NotebookBarPopupMenu::NotebookBarPopupMenu(ResId aRes)
    : PopupMenu(aRes)
{
    if (SfxViewFrame::Current())
    {
        for (int i = 0; i < GetItemCount(); ++i)
        {
            const SfxPoolItem* pItem;
            SfxItemState eState = SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(GetItemId(i), pItem);
            if (eState == SfxItemState::DISABLED)
                this->EnableItem(GetItemId(i), false);
            else
                this->EnableItem(GetItemId(i));
        }
    }
    else
    {
        HideItem(SID_UNDO);
        HideItem(SID_REDO);
        HideItem(SID_PRINTDOC);
        HideItem(SID_SAVEDOC);
    }
    EnableItem(SID_OPTIONS);
}

void NotebookBarPopupMenu::Execute(NotebookBar* pNotebookbar,
                            css::uno::Reference<css::frame::XFrame>& xFrame)
{
    if (pNotebookbar)
    {
        sal_uInt16 nSelected = PopupMenu::Execute(pNotebookbar, Point(0, 40));

        if (nSelected)
        {
            OUString aCommandURL = GetItemCommand(nSelected);
            css::util::URL aUrl;
            aUrl.Complete = aCommandURL;

            Reference<css::util::XURLTransformer> xURLTransformer(
                css::util::URLTransformer::create(comphelper::getProcessComponentContext()));
            if (xURLTransformer.is())
                xURLTransformer->parseStrict(aUrl);

            css::uno::Reference<css::frame::XDispatch> xDispatch;
            css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(xFrame, UNO_QUERY);

            if (xDispatchProvider.is())
            {
                xDispatch = xDispatchProvider->queryDispatch(aUrl, OUString(), 0);
            }
            if (xDispatch.is())
            {
                Sequence<com::sun::star::beans::PropertyValue> aArgs;
                xDispatch->dispatch(aUrl, aArgs);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
