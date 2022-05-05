/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <textboxhandler.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <svx/svdobj.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextFrame.hpp>

#include <unocoll.hxx>
#include <unoframe.hxx>

SwTextBoxHandler::SwTextBoxHandler(SwFrameFormat& rFormat)
    : SwClient(&rFormat)
    , m_rShape(rFormat)
    , m_bLocked(false)
{
}

SwTextBoxHandler::~SwTextBoxHandler()
{
    if (auto pModify = GetRegisteredIn())
        pModify->Remove(this);

    DestroyAll();
}

void SwTextBoxHandler::Create(SdrObject* pObj)
{
    if (IsLocked())
        return;

    if (Has(pObj))
        return;

    OUString sOldText;
    if (pObj->HasText() && pObj->getUnoShape())
    {
        uno::Reference<text::XText> xSourceText(pObj->getUnoShape(), uno::UNO_QUERY_THROW);
        sOldText = xSourceText->getString();
    }

    uno::Reference<text::XTextFrame> xTextFrame;
    try
    {
        uno::Reference<text::XTextDocument> xTextDocument(
            m_rShape.GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
        xTextFrame.set(
            SwXServiceProvider::MakeInstance(SwServiceType::TypeTextFrame, *m_rShape.GetDoc()),
            uno::UNO_QUERY);
        uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
        xTextContentAppend->appendTextContent(xTextFrame, {});
    }
    catch (...)
    {
        throw SwTextBoxHandlingException("Exception during textbox creation!");
    }

    try
    {
        Replace(pObj, xTextFrame);
    }
    catch (...)
    {
        throw SwTextBoxHandlingException("Exception during textbox replacement!");
    }

    try
    {
        Sync(m_rShape.GetAttrSet(), pObj);
    }
    catch (...)
    {
        throw SwTextBoxHandlingException("Exception during textbox property sync!");
    }
}

void SwTextBoxHandler::Destroy(const SdrObject* pObj)
{
    if (IsLocked())
        return;

    if (!Has(pObj))
        return;

    Lock();

    m_TextBoxTable.erase(pObj);
    m_rShape.GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(Get(pObj));

    Unlock();
}

void SwTextBoxHandler::DestroyAll()
{
    if (IsLocked())
        return;

    Lock();
    for (auto& rElement : m_TextBoxTable)
    {
        m_rShape.GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(rElement.second);
    }
    m_TextBoxTable.clear();
    Unlock();
}

void SwTextBoxHandler::Replace(SdrObject* pObj, uno::Reference<text::XTextFrame>& rNew)
{
    if (IsLocked())
        return;

    if (!pObj || !rNew)
        throw SwTextBoxHandlingException("No textbox to replace!");

    if (Has(pObj))
        Destroy(pObj);

    auto pNewTextBox = dynamic_cast<SwXTextFrame*>(rNew.get());
    if (!pNewTextBox)
        throw SwTextBoxHandlingException("This is not a textframe!");

    auto pNewFormat = pNewTextBox->GetFrameFormat();

    if (pNewFormat)
        Set(pObj, pNewFormat);
}

void SwTextBoxHandler::Sync(const SfxItemSet& rSet, const SdrObject* pObj)
{
    if (IsLocked())
        return;

    if (!rSet.Count())
        return;

    if (!Has(pObj))
        return;

    auto pTextBox = Get(pObj);

    Lock();

    pTextBox->SetFormatAttr(rSet);

    Unlock();
}

void SwTextBoxHandler::SyncAll(const SfxItemSet& rSet)
{
    if (IsLocked())
        return;

    for (auto& rTextBox : m_TextBoxTable)
        Sync(rSet, rTextBox.first);
}

void SwTextBoxHandler::Set(SdrObject* pObj, SwFrameFormat* pNew)
{
    if (IsLocked())
        return;

    if (!pObj || !pNew)
        throw SwTextBoxHandlingException("No textbox to set!");

    if (Has(pObj))
        Destroy(pObj);

    Lock();
    m_TextBoxTable.insert(std::pair(pObj, pNew));
    Unlock();
}

void SwTextBoxHandler::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    const auto& rOwner = static_cast<SwModify&>(m_rShape);
    if (&rOwner == &rMod)
    {
        if (IsLocked())
            return;

        if (rHint.GetId() == SfxHintId::SwLegacyModify)
        {
            const auto& rLegacyHint = dynamic_cast<const sw::LegacyModifyHint&>(rHint);
            if (rLegacyHint.m_pNew && rLegacyHint.m_pNew->Which() == RES_ATTRSET_CHG)
            {
                auto pAttrSetChg = dynamic_cast<const SwAttrSetChg*>(rLegacyHint.m_pNew);

                if (pAttrSetChg && pAttrSetChg->GetChgSet())
                    SyncAll(*pAttrSetChg->GetChgSet());
            }
        }
    }
    SwClient::SwClientNotify(rMod, rHint);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
