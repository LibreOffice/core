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
#include <svl/itemiter.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <tools/gen.hxx>
#include <tools/UnitConversion.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

#include <unocoll.hxx>
#include <unoframe.hxx>
#include <unoprnms.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/memberids.h>
#include <unomid.h>
#include <fmtcnct.hxx>

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
        throw SwTextBoxHandlingException(u"Exception during textbox creation!");
    }

    try
    {
        Replace(pObj, xTextFrame);
    }
    catch (...)
    {
        throw SwTextBoxHandlingException(u"Exception during textbox replacement!");
    }
}

void SwTextBoxHandler::Destroy(const SdrObject* pObj)
{
    if (IsLocked())
        return;

    if (!Has(pObj))
        return;

    Lock();

    auto pTextBox = Get(pObj);
    m_TextBoxTable.erase(pObj);
    m_rShape.GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(pTextBox);

    Unlock();
}

void SwTextBoxHandler::Del(SwFrameFormat* pFormat)
{
    if (IsLocked())
        return;
    Lock();

    for (const auto& rElem : m_TextBoxTable)
        if (rElem.second == pFormat)
            m_TextBoxTable.erase(rElem.first);

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

void SwTextBoxHandler::Replace(const SdrObject* pObj, const uno::Reference<text::XTextFrame>& rNew)
{
    if (IsLocked())
        return;

    if (!pObj || !rNew)
        throw SwTextBoxHandlingException(u"No textbox to replace!");

    auto pNewTextBox = dynamic_cast<SwXTextFrame*>(rNew.get());
    if (!pNewTextBox)
        throw SwTextBoxHandlingException(u"This is not a textframe!");

    auto pNewFormat = pNewTextBox->GetFrameFormat();

    if (pNewFormat)
        Set(pObj, pNewFormat);

    uno::Reference<beans::XPropertySet> xPropertySet(rNew, uno::UNO_QUERY);
    uno::Any aEmptyBorder = uno::makeAny(table::BorderLine2());
    xPropertySet->setPropertyValue(UNO_NAME_TOP_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_BOTTOM_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_LEFT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_RIGHT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::makeAny(sal_Int32(100)));

    // Add a new name to it
    uno::Reference<container::XNamed> xNamed(rNew, uno::UNO_QUERY);
    xNamed->setName(m_rShape.GetDoc()->GetUniqueFrameName());

    try
    {
        SfxItemSetFixed<RES_FRM_SIZE, RES_ANCHOR> aSycSet(m_rShape.GetDoc()->GetAttrPool());
        aSycSet.Put(SwFormatAnchor());
        aSycSet.Put(SwFormatFrameSize());
        aSycSet.Put(SwFormatVertOrient());
        aSycSet.Put(SwFormatHoriOrient());
        Sync(aSycSet, pObj);
    }
    catch (...)
    {
        throw SwTextBoxHandlingException(u"Exception during textbox property sync!");
    }
}

void SwTextBoxHandler::Sync(const SfxItemSet& rSet, const SdrObject* pObj)
{
    if (IsLocked())
        return;

    if (!rSet.Count())
        return;

    if (!Has(pObj))
        return;

    Lock();

    // TODO: Improve sync with adjustment of the value of the poolItem.
    auto aIter = SfxItemIter(rSet);
    auto pItem = aIter.GetCurItem();
    while (pItem)
    {
        switch (pItem->Which())
        {
            case RES_ANCHOR:
            {
                HandleAnchorChange(pObj);
                HandlePosChange(pObj);
                HandleSorroundChange(pObj);
                break;
            }
            case RES_HORI_ORIENT:
            case RES_VERT_ORIENT:
            {
                HandlePosChange(pObj);
                HandleAnchorChange(pObj);
                HandlePosChange(pObj);
                HandleSorroundChange(pObj);
                break;
            }
            case RES_FRM_SIZE:
            {
                HandleSizeChange(pObj);
                break;
            }
            case RES_SURROUND:
            {
                HandleSorroundChange(pObj);
                break;
            }

            default:
            {
                break;
            }
        }

        pItem = aIter.NextItem();
    }

    Unlock();
}

void SwTextBoxHandler::SyncAll(const SfxItemSet& rSet)
{
    if (IsLocked())
        return;

    for (auto& rTextBox : m_TextBoxTable)
        Sync(rSet, rTextBox.first);
}

void SwTextBoxHandler::Set(const SdrObject* pObj, SwFrameFormat* pNew)
{
    if (IsLocked())
        return;

    if (!pObj || !pNew)
        throw SwTextBoxHandlingException(u"No textbox to set!");

    if (Has(pObj))
        Destroy(pObj);

    Lock();
    pNew->SetTextBoxHandler(m_rShape.GetTextBoxHandler());
    m_TextBoxTable.insert(std::pair(pObj, pNew));
    Unlock();
}

css::uno::Any SwTextBoxHandler::QueryInterface(const SdrObject* pObj,
                                               const css::uno::Type& rInterfaceType) const
{
    if (!pObj)
        throw SwTextBoxHandlingException(u"No object given for query!");

    if (!Has(pObj))
        return uno::Any();

    uno::Reference<text::XTextFrame> xTextFrame
        = SwXTextFrame::CreateXTextFrame(*m_rShape.GetDoc(), Get(pObj));
    if (xTextFrame)
    {
        uno::Any aRet;

        try
        {
            if (rInterfaceType == cppu::UnoType<text::XText>::get())
                aRet <<= uno::Reference<text::XText>(xTextFrame, uno::UNO_QUERY_THROW);

            if (rInterfaceType == cppu::UnoType<text::XTextRange>::get())
                aRet <<= uno::Reference<text::XText>(xTextFrame, uno::UNO_QUERY_THROW);

            if (rInterfaceType == cppu::UnoType<text::XTextAppend>::get())
                aRet <<= uno::Reference<text::XText>(xTextFrame, uno::UNO_QUERY_THROW);
        }
        catch (...)
        {
            throw SwTextBoxHandlingException(u"Exception during interface query!");
        }

        return aRet;
    }

    return uno::Any();
}

css::uno::Any SwTextBoxHandler::GetProperty(const SdrObject* pObj,
                                            const OUString& rPropertyName) const
{
    if (!pObj)
        throw SwTextBoxHandlingException(u"No object given!");

    auto pTextBox = Get(pObj);
    if (!pTextBox)
        return uno::Any();

    uno::Reference<beans::XPropertySet> xProps;

    try
    {
        xProps.set(SwXTextFrame::CreateXTextFrame(*m_rShape.GetDoc(), pTextBox),
                   uno::UNO_QUERY_THROW);
    }
    catch (...)
    {
        throw SwTextBoxHandlingException(u"Exception during property query!");
    }

    return xProps->getPropertyValue(rPropertyName);
}

css::uno::Any SwTextBoxHandler::GetProperty(const SdrObject* pObj, const sal_uInt16& rnResourceID,
                                            const sal_uInt16& nMemberID) const
{
    if (!pObj)
        throw SwTextBoxHandlingException(u"No object given!");

    auto pTextBox = Get(pObj);
    if (!pTextBox)
        return uno::Any();

    uno::Any aRet;
    switch (rnResourceID)
    {
        case RES_CHAIN:
        {
            switch (nMemberID)
            {
                case MID_CHAIN_PREVNAME:
                case MID_CHAIN_NEXTNAME:
                {
                    const SwFormatChain& rChain = pTextBox->GetChain();
                    rChain.QueryValue(aRet, nMemberID);
                }
                break;
                case MID_CHAIN_NAME:
                    aRet <<= pTextBox->GetName();
                    break;
                default:

                    break;
            }
            break;
        }
        case RES_LR_SPACE:
        {
            switch (nMemberID)
            {
                case MID_L_MARGIN:
                    aRet = GetProperty(pObj, UNO_NAME_LEFT_MARGIN);
                    break;
                case MID_R_MARGIN:
                    aRet = GetProperty(pObj, UNO_NAME_RIGHT_MARGIN);
                    break;
                default:

                    break;
            }
            break;
        }

        case RES_FRM_SIZE:
            switch (nMemberID)
            {
                case MID_FRMSIZE_WIDTH_TYPE:
                    aRet = GetProperty(pObj, UNO_NAME_WIDTH_TYPE);
                    break;
                case MID_FRMSIZE_IS_AUTO_HEIGHT:
                    aRet = GetProperty(pObj, UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT);
                    break;
                case MID_FRMSIZE_REL_HEIGHT_RELATION:
                    aRet = GetProperty(pObj, UNO_NAME_RELATIVE_HEIGHT_RELATION);
                    break;
                case MID_FRMSIZE_REL_WIDTH_RELATION:
                    aRet = GetProperty(pObj, UNO_NAME_RELATIVE_WIDTH_RELATION);
                    break;
                default:
                    aRet = GetProperty(pObj, UNO_NAME_SIZE);
                    break;
            }
            break;

        case RES_TEXT_VERT_ADJUST:
            aRet = GetProperty(pObj, UNO_NAME_TEXT_VERT_ADJUST);
            break;
        case RES_BOX:
            switch (nMemberID)
            {
                case LEFT_BORDER_DISTANCE:
                    aRet = GetProperty(pObj, UNO_NAME_LEFT_BORDER_DISTANCE);
                    break;
                case RIGHT_BORDER_DISTANCE:
                    aRet = GetProperty(pObj, UNO_NAME_RIGHT_BORDER_DISTANCE);
                    break;
                case TOP_BORDER_DISTANCE:
                    aRet = GetProperty(pObj, UNO_NAME_TOP_BORDER_DISTANCE);
                    break;
                case BOTTOM_BORDER_DISTANCE:
                    aRet = GetProperty(pObj, UNO_NAME_BOTTOM_BORDER_DISTANCE);
                    break;
                default:
                    break;
            }
            break;
        case RES_OPAQUE:
            aRet = GetProperty(pObj, UNO_NAME_OPAQUE);
            break;
        case RES_FRAMEDIR:
            aRet = GetProperty(pObj, UNO_NAME_WRITING_MODE);
            break;
        case RES_WRAP_INFLUENCE_ON_OBJPOS:
            switch (nMemberID)
            {
                case MID_ALLOW_OVERLAP:
                    aRet = GetProperty(pObj, UNO_NAME_ALLOW_OVERLAP);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return aRet;
}

void SwTextBoxHandler::SetProperty(const SdrObject* pObj, const OUString& rPropertyName,
                                   const uno::Any& rValue)
{
    if (!pObj)
        throw SwTextBoxHandlingException(u"No object given!");

    auto pTextBox = Get(pObj);
    if (!pTextBox)
        return;

    uno::Reference<beans::XPropertySet> xProps;

    try
    {
        xProps.set(SwXTextFrame::CreateXTextFrame(*m_rShape.GetDoc(), pTextBox),
                   uno::UNO_QUERY_THROW);
    }
    catch (...)
    {
        throw SwTextBoxHandlingException(u"Exception during property query!");
    }

    xProps->setPropertyValue(rPropertyName, rValue);
}

void SwTextBoxHandler::SetProperty(const SdrObject* pObj, const sal_uInt16& rnResourceID,
                                   const sal_uInt16& nMemberID, const uno::Any& rValue)
{
    if (!pObj)
        throw SwTextBoxHandlingException(u"No object given!");

    auto pTextBox = Get(pObj);
    if (!pTextBox)
        return;

    switch (rnResourceID)
    {
        case RES_CHAIN:
        {
            switch (nMemberID)
            {
                case MID_CHAIN_PREVNAME:
                    SetProperty(pObj, UNO_NAME_CHAIN_PREV_NAME, rValue);
                    break;
                case MID_CHAIN_NEXTNAME:
                    SetProperty(pObj, UNO_NAME_CHAIN_NEXT_NAME, rValue);
                    break;
                case MID_CHAIN_NAME:
                    break;
                default:
                    break;
            }
            break;
        }
        case RES_LR_SPACE:
        {
            switch (nMemberID)
            {
                case MID_L_MARGIN:
                    SetProperty(pObj, UNO_NAME_LEFT_MARGIN, rValue);
                    break;
                case MID_R_MARGIN:
                    SetProperty(pObj, UNO_NAME_RIGHT_MARGIN, rValue);
                    break;
                default:

                    break;
            }
            break;
        }

        case RES_FRM_SIZE:
            switch (nMemberID)
            {
                case MID_FRMSIZE_WIDTH_TYPE:
                    SetProperty(pObj, UNO_NAME_WIDTH_TYPE, rValue);
                    break;
                case MID_FRMSIZE_IS_AUTO_HEIGHT:
                    SetProperty(pObj, UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT, rValue);
                    break;
                case MID_FRMSIZE_REL_HEIGHT_RELATION:
                    SetProperty(pObj, UNO_NAME_RELATIVE_HEIGHT_RELATION, rValue);
                    break;
                case MID_FRMSIZE_REL_WIDTH_RELATION:
                    SetProperty(pObj, UNO_NAME_RELATIVE_WIDTH_RELATION, rValue);
                    break;
                default:
                    SetProperty(pObj, UNO_NAME_SIZE, rValue);
                    break;
            }
            break;

        case RES_TEXT_VERT_ADJUST:
            SetProperty(pObj, UNO_NAME_TEXT_VERT_ADJUST, rValue);
            break;
        case RES_BOX:
            switch (nMemberID)
            {
                case LEFT_BORDER_DISTANCE:
                    SetProperty(pObj, UNO_NAME_LEFT_BORDER_DISTANCE, rValue);
                    break;
                case RIGHT_BORDER_DISTANCE:
                    SetProperty(pObj, UNO_NAME_RIGHT_BORDER_DISTANCE, rValue);
                    break;
                case TOP_BORDER_DISTANCE:
                    SetProperty(pObj, UNO_NAME_TOP_BORDER_DISTANCE, rValue);
                    break;
                case BOTTOM_BORDER_DISTANCE:
                    SetProperty(pObj, UNO_NAME_BOTTOM_BORDER_DISTANCE, rValue);
                    break;
                default:
                    break;
            }
            break;
        case RES_OPAQUE:
            SetProperty(pObj, UNO_NAME_OPAQUE, rValue);
            break;
        case RES_FRAMEDIR:
            SetProperty(pObj, UNO_NAME_WRITING_MODE, rValue);
            break;
        case RES_WRAP_INFLUENCE_ON_OBJPOS:
            switch (nMemberID)
            {
                case MID_ALLOW_OVERLAP:
                    SetProperty(pObj, UNO_NAME_ALLOW_OVERLAP, rValue);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
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

void SwTextBoxHandler::HandleAnchorChange(const SdrObject* pObj)
{
    auto pTextBox = Get(pObj);
    if (m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
    {
        SwFormatAnchor aNew;
        aNew.SetType(RndStdIds::FLY_AT_CHAR);
        aNew.SetAnchor(m_rShape.GetAnchor().GetContentAnchor());
        pTextBox->SetFormatAttr(aNew);
    }
    else
        pTextBox->SetFormatAttr(m_rShape.GetAnchor());
}

void SwTextBoxHandler::HandlePosChange(const SdrObject* pObj)
{
    auto pTextBox = Get(pObj);
    const bool bGroup = (Count() > size_t(1));

    {
        SwFormatHoriOrient aNew;
        tools::Rectangle aTextArea
            = SwTextBoxHelper::getRelativeTextRectangle(const_cast<SdrObject*>(pObj));
        aNew.SetRelationOrient((m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                                   ? (text::RelOrientation::CHAR)
                                   : (m_rShape.GetHoriOrient().GetRelationOrient()));
        aNew.SetHoriOrient((m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                               ? (text::VertOrientation::NONE)
                               : (m_rShape.GetHoriOrient().GetHoriOrient()));
        aNew.SetPos(
            (m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                ? (bGroup ? pObj->GetRelativePos().getX() + aTextArea.Left() : aTextArea.Left())
                : ((bGroup ? pObj->GetRelativePos().getX() : m_rShape.GetHoriOrient().GetPos())
                   + aTextArea.Left()));

        if (m_rShape.GetHoriOrient().GetRelationOrient() == text::RelOrientation::PAGE_FRAME
            && m_rShape.GetAnchor().GetAnchorId() != RndStdIds::FLY_AT_PAGE)
        {
            aNew.SetRelationOrient(text::RelOrientation::PAGE_FRAME);
            aNew.SetPos(m_rShape.GetHoriOrient().GetPos() + aTextArea.Left());
        }

        pTextBox->SetFormatAttr(aNew);
    }

    {
        SwFormatVertOrient aNew;
        tools::Rectangle aTextArea
            = SwTextBoxHelper::getRelativeTextRectangle(const_cast<SdrObject*>(pObj));
        aNew.SetRelationOrient((m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                                   ? (text::RelOrientation::CHAR)
                                   : (m_rShape.GetVertOrient().GetRelationOrient()));
        aNew.SetVertOrient((m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                               ? (text::VertOrientation::NONE)
                               : (m_rShape.GetVertOrient().GetVertOrient()));

        if (m_rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
        {
            if (bGroup)
            {
                // There are the following cases:
                // case 1: The textbox should be in that position where the shape is.
                // case 2: The shape has negative offset so that have to be subtracted
                // case 3: The shape and its parent shape also has negative offset, so subtract
                aNew.SetPos(
                    ((pObj->GetRelativePos().getY()) > 0
                         ? (m_rShape.GetVertOrient().GetPos() > 0
                                ? pObj->GetRelativePos().getY()
                                : pObj->GetRelativePos().getY() - m_rShape.GetVertOrient().GetPos())
                         : (m_rShape.GetVertOrient().GetPos() > 0
                                ? 0 // Is this can be a variation?
                                : pObj->GetRelativePos().getY()
                                      - m_rShape.GetVertOrient().GetPos()))
                    + aTextArea.Top());
            }
            else
            {
                aNew.SetPos(aTextArea.Top());
            }

            if (m_rShape.GetVertOrient().GetVertOrient() != text::VertOrientation::NONE)
            {
                aNew.SetVertOrient(text::VertOrientation::NONE);
                switch (m_rShape.GetVertOrient().GetVertOrient())
                {
                        // Top aligned shape
                    case text::VertOrientation::TOP:
                    case text::VertOrientation::CHAR_TOP:
                    case text::VertOrientation::LINE_TOP:
                    {
                        aNew.SetPos(aNew.GetPos() - m_rShape.GetFrameSize().GetHeight());
                        break;
                    }
                    // Bottom aligned shape
                    case text::VertOrientation::BOTTOM:
                    case text::VertOrientation::CHAR_BOTTOM:
                    case text::VertOrientation::LINE_BOTTOM:
                    {
                        aNew.SetPos(aNew.GetPos() + m_rShape.GetFrameSize().GetHeight());
                        break;
                    }
                    // Center aligned shape
                    case text::VertOrientation::CENTER:
                    case text::VertOrientation::CHAR_CENTER:
                    case text::VertOrientation::LINE_CENTER:
                    {
                        aNew.SetPos(aNew.GetPos()
                                    + std::lroundf(m_rShape.GetFrameSize().GetHeight() / 2));
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        else
        {
            aNew.SetPos((bGroup ? pObj->GetRelativePos().getY() : m_rShape.GetVertOrient().GetPos())
                        + aTextArea.Top());
        }

        if (m_rShape.GetVertOrient().GetRelationOrient() == text::RelOrientation::PAGE_FRAME
            && m_rShape.GetAnchor().GetAnchorId() != RndStdIds::FLY_AT_PAGE)
        {
            aNew.SetRelationOrient(text::RelOrientation::PAGE_FRAME);
            aNew.SetPos(m_rShape.GetVertOrient().GetPos() + aTextArea.Top());
        }

        pTextBox->SetFormatAttr(aNew);
    }
}

void SwTextBoxHandler::HandleSizeChange(const SdrObject* pObj)
{
    auto pTextBox = Get(pObj);
    auto pTextArea = SwTextBoxHelper::getRelativeTextRectangle(const_cast<SdrObject*>(pObj));
    pTextBox->SetFormatAttr(SwFormatFrameSize(SwFrameSize::Fixed, pTextArea.GetSize().Width(),
                                              pTextArea.GetSize().Height()));
}

void SwTextBoxHandler::HandleSorroundChange(const SdrObject* pObj)
{
    auto pTextBox = Get(pObj);
    pTextBox->SetFormatAttr(SwFormatSurround(text::WrapTextMode::WrapTextMode_THROUGH));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
