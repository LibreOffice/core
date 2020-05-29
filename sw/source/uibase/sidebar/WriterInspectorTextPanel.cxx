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

#include "WriterInspectorTextPanel.hxx"
#include <svx/svxids.hrc>
#include <doc.hxx>
#include <editsh.hxx>
#include <ndtxt.hxx>
#include <fmtftn.hxx>
#include <hintids.hxx>
#include <txatbase.hxx>
#include <IDocumentContentOperations.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <strings.hrc>
#include <editeng/wghtitem.hxx>

using namespace css;

namespace sw::sidebar
{
VclPtr<vcl::Window>
WriterInspectorTextPanel::Create(vcl::Window* pParent,
                                 const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                 SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to WriterInspectorTextPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to WriterInspectorTextPanel::Create",
                                             nullptr, 1);
    if (pBindings == nullptr)
        throw ::com::sun::star::lang::IllegalArgumentException(
            "no SfxBindings given to PageHeaderPanel::Create", nullptr, 0);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame, pBindings);
}
WriterInspectorTextPanel::WriterInspectorTextPanel(
    vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : InspectorTextPanel(pParent, rxFrame)
    , maCharProperty(SID_STYLE_FAMILY1, *pBindings, *this)
{
}

void WriterInspectorTextPanel::NotifyItemUpdate(const sal_uInt16 nSId,
                                                const SfxItemState /*eState*/,
                                                const SfxPoolItem* /*pState*/)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    std::vector<OUString> store;
    switch (nSId)
    {
        case SID_STYLE_FAMILY1:
        {
            if (pDocSh)
            {
                SwDoc* pDoc = pDocSh->GetDoc();
                if (pDoc)
                {
                    /*
                    SwEditShell* pShell = pDoc->GetEditShell();
                    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
                    */
                    uno::Reference<frame::XModel> xModel = pDocSh->GetBaseModel();
                    uno::Reference<text::XTextDocument> xDoc(xModel, uno::UNO_QUERY);
                    uno::Reference<text::XText> xText = xDoc->getText();
                    uno::Reference<text::XTextCursor> m_xCursor = xText->createTextCursor();
                    uno::Reference<beans::XPropertySet> xCursorProp(m_xCursor, uno::UNO_QUERY);
                    uno::Any aFontName = xCursorProp->getPropertyValue("CharFontName");
                    OUString sFontName;
                    aFontName >>= sFontName;

                    // This value does not change on changing the cursor position
                    store.push_back("Font Name    " + sFontName);
                }
            }
        }
        break;
    }
    InspectorTextPanel::updateEntries(store);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
