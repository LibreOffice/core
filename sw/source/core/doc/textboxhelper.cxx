/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <textboxhelper.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtcnct.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
#include <docsh.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <unodraw.hxx>
#include <unotextrange.hxx>
#include <cmdid.h>
#include <unomid.h>
#include <unoprnms.hxx>
#include <mvsave.hxx>
#include <fmtsrnd.hxx>
#include <frmfmt.hxx>
#include <frameformats.hxx>

#include <editeng/unoprnms.hxx>
#include <editeng/memberids.h>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
#include <svl/itemiter.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <sal/log.hxx>
#include <tools/UnitConversion.hxx>
#include <svx/swframetypes.hxx>

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

using namespace com::sun::star;

void SwTextBoxHelper::create(SwFrameFormat* pShape, bool bCopyText)
{
    // If TextBox wasn't enabled previously
    if (pShape->GetAttrSet().HasItem(RES_CNTNT) && pShape->GetOtherTextBoxFormat())
        return;

    // Store the current text content of the shape
    OUString sCopyableText;

    if (bCopyText)
    {
        if (auto pSdrShape = pShape->FindRealSdrObject())
        {
            uno::Reference<text::XText> xSrcCnt(pSdrShape->getWeakUnoShape(), uno::UNO_QUERY);
            auto xCur = xSrcCnt->createTextCursor();
            xCur->gotoStart(false);
            xCur->gotoEnd(true);
            sCopyableText = xCur->getText()->getString();
        }
    }

    // Create the associated TextFrame and insert it into the document.
    uno::Reference<text::XTextContent> xTextFrame(
        SwXServiceProvider::MakeInstance(SwServiceType::TypeTextFrame, *pShape->GetDoc()),
        uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(
        pShape->GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    try
    {
        SdrObject* pSourceSDRShape = pShape->FindRealSdrObject();
        uno::Reference<text::XTextContent> XSourceShape(pSourceSDRShape->getUnoShape(),
                                                        uno::UNO_QUERY_THROW);
        xTextContentAppend->insertTextContentWithProperties(
            xTextFrame, uno::Sequence<beans::PropertyValue>(), XSourceShape->getAnchor());
    }
    catch (uno::Exception&)
    {
        // Before the textframe was appended now it is inserted to the begin of the doc in order
        // to prevent crash when someone removes the para where the textframe anchored:
        uno::Reference<text::XTextCursor> xCursor = xTextDocument->getText()->createTextCursor();
        xCursor->gotoStart(false);
        xTextContentAppend->insertTextContentWithProperties(
            xTextFrame, uno::Sequence<beans::PropertyValue>(), xCursor->getStart());
    }
    // Link FLY and DRAW formats, so it becomes a text box (needed for syncProperty calls).
    uno::Reference<text::XTextFrame> xRealTextFrame(xTextFrame, uno::UNO_QUERY);
    auto pTextFrame = dynamic_cast<SwXTextFrame*>(xRealTextFrame.get());
    assert(nullptr != pTextFrame);
    SwFrameFormat* pFormat = pTextFrame->GetFrameFormat();

    assert(nullptr != dynamic_cast<SwDrawFrameFormat*>(pShape));
    assert(nullptr != dynamic_cast<SwFlyFrameFormat*>(pFormat));

    pShape->SetOtherTextBoxFormat(pFormat);
    pFormat->SetOtherTextBoxFormat(pShape);

    // Initialize properties.
    uno::Reference<beans::XPropertySet> xPropertySet(xTextFrame, uno::UNO_QUERY);
    uno::Any aEmptyBorder = uno::makeAny(table::BorderLine2());
    xPropertySet->setPropertyValue(UNO_NAME_TOP_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_BOTTOM_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_LEFT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_RIGHT_BORDER, aEmptyBorder);

    xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::makeAny(sal_Int32(100)));

    xPropertySet->setPropertyValue(UNO_NAME_SIZE_TYPE, uno::makeAny(text::SizeType::FIX));

    xPropertySet->setPropertyValue(UNO_NAME_SURROUND, uno::makeAny(text::WrapTextMode_THROUGH));

    uno::Reference<container::XNamed> xNamed(xTextFrame, uno::UNO_QUERY);
    xNamed->setName(pShape->GetDoc()->GetUniqueFrameName());

    // Link its text range to the original shape.
    uno::Reference<text::XTextRange> xTextBox(xTextFrame, uno::UNO_QUERY_THROW);
    SwUnoInternalPaM aInternalPaM(*pShape->GetDoc());
    if (sw::XTextRangeToSwPaM(aInternalPaM, xTextBox))
    {
        SwAttrSet aSet(pShape->GetAttrSet());
        SwFormatContent aContent(aInternalPaM.GetNode().StartOfSectionNode());
        aSet.Put(aContent);
        pShape->SetFormatAttr(aSet);
    }

    // Also initialize the properties, which are not constant, but inherited from the shape's ones.
    uno::Reference<drawing::XShape> xShape(pShape->FindRealSdrObject()->getUnoShape(),
                                           uno::UNO_QUERY);
    syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::makeAny(xShape->getSize()));

    uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
    syncProperty(pShape, RES_FOLLOW_TEXT_FLOW, MID_FOLLOW_TEXT_FLOW,
                 xShapePropertySet->getPropertyValue(UNO_NAME_IS_FOLLOWING_TEXT_FLOW));
    syncProperty(pShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE,
                 xShapePropertySet->getPropertyValue(UNO_NAME_ANCHOR_TYPE));
    syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_ORIENT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT));
    syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_RELATION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_RELATION));
    syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_ORIENT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT));
    syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_RELATION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_RELATION));
    syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_POSITION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_POSITION));
    syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_POSITION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_POSITION));
    syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT));
    syncProperty(pShape, RES_TEXT_VERT_ADJUST, 0,
                 xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_VERT_ADJUST));
    text::WritingMode eMode;
    if (xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_WRITINGMODE) >>= eMode)
        syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(sal_Int16(eMode)));

    // TODO: Text dialog attr setting to frame

    // Check if the shape had text before and move it to the new textframe
    if (bCopyText && !sCopyableText.isEmpty())
    {
        auto pSdrShape = pShape->FindRealSdrObject();
        if (pSdrShape)
        {
            auto pSourceText = dynamic_cast<SdrTextObj*>(pSdrShape);
            uno::Reference<text::XTextRange> xDestText(xRealTextFrame, uno::UNO_QUERY);

            xDestText->setString(sCopyableText);

            if (pSourceText)
                pSourceText->SetText(OUString());

            pShape->GetDoc()->getIDocumentState().SetModified();
        }
    }
}

void SwTextBoxHelper::destroy(SwFrameFormat* pShape)
{
    // If a TextBox was enabled previously
    if (pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        SwFrameFormat* pFormat = pShape->GetOtherTextBoxFormat();

        // Unlink the TextBox's text range from the original shape.
        pShape->ResetFormatAttr(RES_CNTNT);

        // Delete the associated TextFrame.
        if (pFormat)
            pShape->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(pFormat);
    }
}

bool SwTextBoxHelper::isTextBox(const SwFrameFormat* pFormat, sal_uInt16 nType)
{
    assert(nType == RES_FLYFRMFMT || nType == RES_DRAWFRMFMT);
    if (!pFormat || pFormat->Which() != nType || !pFormat->GetAttrSet().HasItem(RES_CNTNT))
        return false;

    sal_uInt16 nOtherType = (pFormat->Which() == RES_FLYFRMFMT) ? sal_uInt16(RES_DRAWFRMFMT)
                                                                : sal_uInt16(RES_FLYFRMFMT);
    SwFrameFormat* pOtherFormat = pFormat->GetOtherTextBoxFormat();
    if (!pOtherFormat)
        return false;

    assert(pOtherFormat->Which() == nOtherType);
    if (pOtherFormat->Which() != nOtherType)
        return false;

    const SwFormatContent& rContent = pFormat->GetContent();
    return pOtherFormat->GetAttrSet().HasItem(RES_CNTNT) && pOtherFormat->GetContent() == rContent;
}

bool SwTextBoxHelper::hasTextFrame(const SdrObject* pObj)
{
    if (!pObj)
        return false;

    uno::Reference<drawing::XShape> xShape(pObj->getWeakUnoShape(), uno::UNO_QUERY);
    if (!xShape)
        return false;
    return SwTextBoxHelper::getOtherTextBoxFormat(xShape);
}

sal_Int32 SwTextBoxHelper::getCount(SdrPage const* pPage)
{
    sal_Int32 nRet = 0;
    for (std::size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* p = pPage->GetObj(i);
        if (p && p->IsTextBox())
            continue;
        ++nRet;
    }
    return nRet;
}

sal_Int32 SwTextBoxHelper::getCount(const SwDoc& rDoc)
{
    sal_Int32 nRet = 0;
    const SwFrameFormats& rSpzFrameFormats = *rDoc.GetSpzFrameFormats();
    for (const auto pFormat : rSpzFrameFormats)
    {
        if (isTextBox(pFormat, RES_FLYFRMFMT))
            ++nRet;
    }
    return nRet;
}

uno::Any SwTextBoxHelper::getByIndex(SdrPage const* pPage, sal_Int32 nIndex)
{
    if (nIndex < 0)
        throw lang::IndexOutOfBoundsException();

    SdrObject* pRet = nullptr;
    sal_Int32 nCount = 0; // Current logical index.
    for (std::size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* p = pPage->GetObj(i);
        if (p && p->IsTextBox())
            continue;
        if (nCount == nIndex)
        {
            pRet = p;
            break;
        }
        ++nCount;
    }

    if (!pRet)
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(uno::Reference<drawing::XShape>(pRet->getUnoShape(), uno::UNO_QUERY));
}

sal_Int32 SwTextBoxHelper::getOrdNum(const SdrObject* pObject)
{
    if (const SdrPage* pPage = pObject->getSdrPageFromSdrObject())
    {
        sal_Int32 nOrder = 0; // Current logical order.
        for (std::size_t i = 0; i < pPage->GetObjCount(); ++i)
        {
            SdrObject* p = pPage->GetObj(i);
            if (p && p->IsTextBox())
                continue;
            if (p == pObject)
                return nOrder;
            ++nOrder;
        }
    }

    SAL_WARN("sw.core", "SwTextBoxHelper::getOrdNum: no page or page doesn't contain the object");
    return pObject->GetOrdNum();
}

void SwTextBoxHelper::getShapeWrapThrough(const SwFrameFormat* pTextBox, bool& rWrapThrough)
{
    SwFrameFormat* pShape = SwTextBoxHelper::getOtherTextBoxFormat(pTextBox, RES_FLYFRMFMT);
    if (pShape)
        rWrapThrough = pShape->GetSurround().GetSurround() == css::text::WrapTextMode_THROUGH;
}

SwFrameFormat* SwTextBoxHelper::getOtherTextBoxFormat(const SwFrameFormat* pFormat,
                                                      sal_uInt16 nType)
{
    if (!isTextBox(pFormat, nType))
        return nullptr;
    return pFormat->GetOtherTextBoxFormat();
}

SwFrameFormat* SwTextBoxHelper::getOtherTextBoxFormat(uno::Reference<drawing::XShape> const& xShape)
{
    auto pShape = dynamic_cast<SwXShape*>(xShape.get());
    if (!pShape)
        return nullptr;

    SwFrameFormat* pFormat = pShape->GetFrameFormat();
    return getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT);
}

uno::Reference<text::XTextFrame>
SwTextBoxHelper::getUnoTextFrame(uno::Reference<drawing::XShape> const& xShape)
{
    if (xShape)
    {
        auto pFrameFormat = SwTextBoxHelper::getOtherTextBoxFormat(xShape);
        if (pFrameFormat)
        {
            auto pSdrObj = pFrameFormat->FindSdrObject();
            if (pSdrObj && pSdrObj->IsTextBox())
            {
                return uno::Reference<css::text::XTextFrame>(pSdrObj->getUnoShape(),
                                                             uno::UNO_QUERY);
            }
        }
    }
    return uno::Reference<css::text::XTextFrame>();
}

template <typename T> static void lcl_queryInterface(const SwFrameFormat* pShape, uno::Any& rAny)
{
    if (SwFrameFormat* pFormat = SwTextBoxHelper::getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
    {
        uno::Reference<T> const xInterface(
            SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat), uno::UNO_QUERY);
        rAny <<= xInterface;
    }
}

uno::Any SwTextBoxHelper::queryInterface(const SwFrameFormat* pShape, const uno::Type& rType)
{
    uno::Any aRet;

    if (rType == cppu::UnoType<css::text::XTextAppend>::get())
    {
        lcl_queryInterface<text::XTextAppend>(pShape, aRet);
    }
    else if (rType == cppu::UnoType<css::text::XText>::get())
    {
        lcl_queryInterface<text::XText>(pShape, aRet);
    }
    else if (rType == cppu::UnoType<css::text::XTextRange>::get())
    {
        lcl_queryInterface<text::XTextRange>(pShape, aRet);
    }

    return aRet;
}

tools::Rectangle SwTextBoxHelper::getTextRectangle(SwFrameFormat* pShape, bool bAbsolute)
{
    tools::Rectangle aRet;
    aRet.SetEmpty();
    auto pSdrShape = pShape->FindRealSdrObject();
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(pSdrShape);
    if (pCustomShape)
    {
        // Need to temporarily release the lock acquired in
        // SdXMLShapeContext::AddShape(), otherwise we get an empty rectangle,
        // see EnhancedCustomShapeEngine::getTextBounds().
        uno::Reference<document::XActionLockable> xLockable(pCustomShape->getUnoShape(),
                                                            uno::UNO_QUERY);
        sal_Int16 nLocks = 0;
        if (xLockable.is())
            nLocks = xLockable->resetActionLocks();
        pCustomShape->GetTextBounds(aRet);
        if (nLocks)
            xLockable->setActionLocks(nLocks);
    }
    else if (pSdrShape)
    {
        // fallback - get *any* bound rect we can possibly get hold of
        aRet = pSdrShape->GetCurrentBoundRect();
    }

    if (!bAbsolute && pSdrShape)
    {
        // Relative, so count the logic (reference) rectangle, see the EnhancedCustomShape2d ctor.
        Point aPoint(pSdrShape->GetSnapRect().Center());
        Size aSize(pSdrShape->GetLogicRect().GetSize());
        aPoint.AdjustX(-(aSize.Width() / 2));
        aPoint.AdjustY(-(aSize.Height() / 2));
        tools::Rectangle aLogicRect(aPoint, aSize);
        aRet.Move(-1 * aLogicRect.Left(), -1 * aLogicRect.Top());
    }

    return aRet;
}

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, std::u16string_view rPropertyName,
                                   const css::uno::Any& rValue)
{
    // Textframes does not have valid horizontal adjust property, so map it to paragraph adjust property
    if (rPropertyName == UNO_NAME_TEXT_HORZADJUST)
    {
        SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT);
        if (!pFormat)
            return;

        auto xTextFrame = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);
        uno::Reference<text::XTextCursor> xCursor = xTextFrame->getText()->createTextCursor();

        // Select all paragraphs in the textframe
        xCursor->gotoStart(false);
        xCursor->gotoEnd(true);
        uno::Reference<beans::XPropertySet> xFrameParaProps(xCursor, uno::UNO_QUERY);

        // And simply map the property
        const auto eValue = rValue.get<drawing::TextHorizontalAdjust>();
        switch (eValue)
        {
            case drawing::TextHorizontalAdjust::TextHorizontalAdjust_CENTER:
                xFrameParaProps->setPropertyValue(
                    UNO_NAME_PARA_ADJUST,
                    uno::makeAny(style::ParagraphAdjust::ParagraphAdjust_CENTER)); //3
                break;
            case drawing::TextHorizontalAdjust::TextHorizontalAdjust_LEFT:
                xFrameParaProps->setPropertyValue(
                    UNO_NAME_PARA_ADJUST,
                    uno::makeAny(style::ParagraphAdjust::ParagraphAdjust_LEFT)); //0
                break;
            case drawing::TextHorizontalAdjust::TextHorizontalAdjust_RIGHT:
                xFrameParaProps->setPropertyValue(
                    UNO_NAME_PARA_ADJUST,
                    uno::makeAny(style::ParagraphAdjust::ParagraphAdjust_RIGHT)); //1
                break;
            default:
                SAL_WARN("sw.core",
                         "SwTextBoxHelper::syncProperty: unhandled TextHorizontalAdjust: "
                             << static_cast<sal_Int32>(eValue));
                break;
        }
        return;
    }

    if (rPropertyName == u"CustomShapeGeometry")
    {
        // CustomShapeGeometry changes the textbox position offset and size, so adjust both.
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::Any());

        SdrObject* pObject = pShape->FindRealSdrObject();
        if (pObject)
        {
            tools::Rectangle aRectangle(pObject->GetSnapRect());
            syncProperty(
                pShape, RES_HORI_ORIENT, MID_HORIORIENT_POSITION,
                uno::makeAny(static_cast<sal_Int32>(convertTwipToMm100(aRectangle.Left()))));
            syncProperty(
                pShape, RES_VERT_ORIENT, MID_VERTORIENT_POSITION,
                uno::makeAny(static_cast<sal_Int32>(convertTwipToMm100(aRectangle.Top()))));
        }

        SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT);
        if (!pFormat)
            return;

        comphelper::SequenceAsHashMap aCustomShapeGeometry(rValue);
        auto it = aCustomShapeGeometry.find("TextPreRotateAngle");
        if (it == aCustomShapeGeometry.end())
        {
            it = aCustomShapeGeometry.find("TextRotateAngle");
        }

        if (it != aCustomShapeGeometry.end())
        {
            auto nAngle = it->second.has<sal_Int32>() ? it->second.get<sal_Int32>() : 0;
            if (nAngle == 0)
            {
                nAngle = it->second.has<double>() ? it->second.get<double>() : 0;
            }

            sal_Int16 nDirection = 0;
            switch (nAngle)
            {
                case -90:
                    nDirection = text::WritingMode2::TB_RL;
                    break;
                case -270:
                    nDirection = text::WritingMode2::BT_LR;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled property value: "
                                        "CustomShapeGeometry:TextPreRotateAngle: "
                                            << nAngle);
                    break;
            }

            if (nDirection)
            {
                syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(nDirection));
            }
        }
    }
    else if (rPropertyName == UNO_NAME_TEXT_VERT_ADJUST)
        syncProperty(pShape, RES_TEXT_VERT_ADJUST, 0, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_AUTOGROWHEIGHT)
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_LEFTDIST)
        syncProperty(pShape, RES_BOX, LEFT_BORDER_DISTANCE, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_RIGHTDIST)
        syncProperty(pShape, RES_BOX, RIGHT_BORDER_DISTANCE, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_UPPERDIST)
        syncProperty(pShape, RES_BOX, TOP_BORDER_DISTANCE, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_LOWERDIST)
        syncProperty(pShape, RES_BOX, BOTTOM_BORDER_DISTANCE, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_WRITINGMODE)
    {
        text::WritingMode eMode;
        sal_Int16 eMode2;
        if (rValue >>= eMode)
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(sal_Int16(eMode)));
        else if (rValue >>= eMode2)
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(eMode2));
    }
    else
        SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled property: "
                                << static_cast<OUString>(rPropertyName));
}

void SwTextBoxHelper::getProperty(SwFrameFormat const* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                                  css::uno::Any& rValue)
{
    if (!pShape)
        return;

    nMemberID &= ~CONVERT_TWIPS;

    SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT);
    if (!pFormat)
        return;

    if (nWID != RES_CHAIN)
        return;

    switch (nMemberID)
    {
        case MID_CHAIN_PREVNAME:
        case MID_CHAIN_NEXTNAME:
        {
            const SwFormatChain& rChain = pFormat->GetChain();
            rChain.QueryValue(rValue, nMemberID);
        }
        break;
        case MID_CHAIN_NAME:
            rValue <<= pFormat->GetName();
            break;
        default:
            SAL_WARN("sw.core", "SwTextBoxHelper::getProperty: unhandled member-id: "
                                    << static_cast<sal_uInt16>(nMemberID));
            break;
    }
}

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                                   const css::uno::Any& rValue)
{
    // No shape yet? Then nothing to do, initial properties are set by create().
    if (!pShape)
        return;

    uno::Any aValue(rValue);
    nMemberID &= ~CONVERT_TWIPS;

    SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT);
    if (!pFormat)
        return;

    OUString aPropertyName;
    bool bAdjustX = false;
    bool bAdjustY = false;
    bool bAdjustSize = false;
    switch (nWID)
    {
        case RES_HORI_ORIENT:
            switch (nMemberID)
            {
                case MID_HORIORIENT_ORIENT:
                    aPropertyName = UNO_NAME_HORI_ORIENT;
                    break;
                case MID_HORIORIENT_RELATION:
                    aPropertyName = UNO_NAME_HORI_ORIENT_RELATION;
                    break;
                case MID_HORIORIENT_POSITION:
                    aPropertyName = UNO_NAME_HORI_ORIENT_POSITION;
                    bAdjustX = true;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        case RES_LR_SPACE:
        {
            switch (nMemberID)
            {
                case MID_L_MARGIN:
                    aPropertyName = UNO_NAME_LEFT_MARGIN;
                    break;
                case MID_R_MARGIN:
                    aPropertyName = UNO_NAME_RIGHT_MARGIN;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        }
        case RES_VERT_ORIENT:
            switch (nMemberID)
            {
                case MID_VERTORIENT_ORIENT:
                    aPropertyName = UNO_NAME_VERT_ORIENT;
                    break;
                case MID_VERTORIENT_RELATION:
                    aPropertyName = UNO_NAME_VERT_ORIENT_RELATION;
                    break;
                case MID_VERTORIENT_POSITION:
                    aPropertyName = UNO_NAME_VERT_ORIENT_POSITION;
                    bAdjustY = true;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        case RES_FRM_SIZE:
            switch (nMemberID)
            {
                case MID_FRMSIZE_IS_AUTO_HEIGHT:
                    aPropertyName = UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT;
                    break;
                case MID_FRMSIZE_REL_HEIGHT_RELATION:
                    aPropertyName = UNO_NAME_RELATIVE_HEIGHT_RELATION;
                    break;
                case MID_FRMSIZE_REL_WIDTH_RELATION:
                    aPropertyName = UNO_NAME_RELATIVE_WIDTH_RELATION;
                    break;
                default:
                    aPropertyName = UNO_NAME_SIZE;
                    bAdjustSize = true;
                    break;
            }
            break;
        case RES_ANCHOR:
            switch (nMemberID)
            {
                case MID_ANCHOR_ANCHORTYPE:
                {
                    uno::Reference<beans::XPropertySet> const xPropertySet(
                        SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat),
                        uno::UNO_QUERY);
                    // Surround (Wrap) has to be THROUGH always:
                    xPropertySet->setPropertyValue(UNO_NAME_SURROUND,
                                                   uno::makeAny(text::WrapTextMode_THROUGH));
                    // Use At_Char anchor instead of As_Char anchoring:
                    if (aValue.get<text::TextContentAnchorType>()
                        == text::TextContentAnchorType::TextContentAnchorType_AS_CHARACTER)
                    {
                        xPropertySet->setPropertyValue(
                            UNO_NAME_ANCHOR_TYPE,
                            uno::makeAny(
                                text::TextContentAnchorType::TextContentAnchorType_AT_PARAGRAPH));
                    }
                    else // Otherwise copy the anchor type of the shape
                    {
                        xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, aValue);
                    }
                    // After anchoring the position must be set as well:
                    if (aValue.get<text::TextContentAnchorType>()
                        == text::TextContentAnchorType::TextContentAnchorType_AT_PAGE)
                    {
                        xPropertySet->setPropertyValue(
                            UNO_NAME_ANCHOR_PAGE_NO,
                            uno::makeAny(pShape->GetAnchor().GetPageNum()));
                    }

                    return;
                }
                break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        case FN_TEXT_RANGE:
        {
            uno::Reference<text::XTextRange> xRange;
            rValue >>= xRange;
            SwUnoInternalPaM aInternalPaM(*pFormat->GetDoc());
            if (sw::XTextRangeToSwPaM(aInternalPaM, xRange))
            {
                SwFormatAnchor aAnchor(pFormat->GetAnchor());
                aAnchor.SetAnchor(aInternalPaM.Start());
                pFormat->SetFormatAttr(aAnchor);
            }
        }
        break;
        case RES_CHAIN:
            switch (nMemberID)
            {
                case MID_CHAIN_PREVNAME:
                    aPropertyName = UNO_NAME_CHAIN_PREV_NAME;
                    break;
                case MID_CHAIN_NEXTNAME:
                    aPropertyName = UNO_NAME_CHAIN_NEXT_NAME;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        case RES_TEXT_VERT_ADJUST:
            aPropertyName = UNO_NAME_TEXT_VERT_ADJUST;
            break;
        case RES_BOX:
            switch (nMemberID)
            {
                case LEFT_BORDER_DISTANCE:
                    aPropertyName = UNO_NAME_LEFT_BORDER_DISTANCE;
                    break;
                case RIGHT_BORDER_DISTANCE:
                    aPropertyName = UNO_NAME_RIGHT_BORDER_DISTANCE;
                    break;
                case TOP_BORDER_DISTANCE:
                    aPropertyName = UNO_NAME_TOP_BORDER_DISTANCE;
                    break;
                case BOTTOM_BORDER_DISTANCE:
                    aPropertyName = UNO_NAME_BOTTOM_BORDER_DISTANCE;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        case RES_OPAQUE:
            aPropertyName = UNO_NAME_OPAQUE;
            break;
        case RES_FRAMEDIR:
            aPropertyName = UNO_NAME_WRITING_MODE;
            break;
        case RES_WRAP_INFLUENCE_ON_OBJPOS:
            switch (nMemberID)
            {
                case MID_ALLOW_OVERLAP:
                    aPropertyName = UNO_NAME_ALLOW_OVERLAP;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << static_cast<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        default:
            SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled which-id: "
                                    << nWID << " (member-id: " << static_cast<sal_uInt16>(nMemberID)
                                    << ")");
            break;
    }

    if (aPropertyName.isEmpty())
        return;

    // Position/size should be the text position/size, not the shape one as-is.
    if (bAdjustX || bAdjustY || bAdjustSize)
    {
        tools::Rectangle aRect = getTextRectangle(pShape, /*bAbsolute=*/false);
        if (!aRect.IsEmpty())
        {
            if (bAdjustX || bAdjustY)
            {
                sal_Int32 nValue;
                if (aValue >>= nValue)
                {
                    if (bAdjustX)
                        nValue += TWIPS_TO_MM(aRect.getX());
                    else if (bAdjustY)
                        nValue += TWIPS_TO_MM(aRect.getY());
                    aValue <<= nValue;
                }
            }
            else if (bAdjustSize)
            {
                awt::Size aSize(TWIPS_TO_MM(aRect.getWidth()), TWIPS_TO_MM(aRect.getHeight()));
                aValue <<= aSize;
            }
        }
    }

    uno::Reference<beans::XPropertySet> const xPropertySet(
        SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat), uno::UNO_QUERY);
    xPropertySet->setPropertyValue(aPropertyName, aValue);
}

void SwTextBoxHelper::saveLinks(const SwFrameFormats& rFormats,
                                std::map<const SwFrameFormat*, const SwFrameFormat*>& rLinks)
{
    for (const auto pFormat : rFormats)
    {
        if (SwFrameFormat* pTextBox = getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT))
            rLinks[pFormat] = pTextBox;
    }
}

void SwTextBoxHelper::restoreLinks(std::set<ZSortFly>& rOld, std::vector<SwFrameFormat*>& rNew,
                                   SavedLink& rSavedLinks)
{
    std::size_t i = 0;
    for (const auto& rIt : rOld)
    {
        auto aTextBoxIt = rSavedLinks.find(rIt.GetFormat());
        if (aTextBoxIt != rSavedLinks.end())
        {
            std::size_t j = 0;
            for (const auto& rJt : rOld)
            {
                if (rJt.GetFormat() == aTextBoxIt->second)
                    rNew[i]->SetFormatAttr(rNew[j]->GetContent());
                ++j;
            }
        }
        ++i;
    }
}

text::TextContentAnchorType SwTextBoxHelper::mapAnchorType(const RndStdIds& rAnchorID)
{
    text::TextContentAnchorType aAnchorType;
    switch (rAnchorID)
    {
        case RndStdIds::FLY_AS_CHAR:
            aAnchorType = text::TextContentAnchorType::TextContentAnchorType_AS_CHARACTER;
            break;
        case RndStdIds::FLY_AT_CHAR:
            aAnchorType = text::TextContentAnchorType::TextContentAnchorType_AT_CHARACTER;
            break;
        case RndStdIds::FLY_AT_PARA:
            aAnchorType = text::TextContentAnchorType::TextContentAnchorType_AT_PARAGRAPH;
            break;
        case RndStdIds::FLY_AT_PAGE:
            aAnchorType = text::TextContentAnchorType::TextContentAnchorType_AT_PAGE;
            break;
        case RndStdIds::FLY_AT_FLY:
            aAnchorType = text::TextContentAnchorType::TextContentAnchorType_AT_FRAME;
            break;
        default:
            aAnchorType = text::TextContentAnchorType::TextContentAnchorType_AT_PARAGRAPH;
            SAL_WARN("sw.core", "SwTextBoxHelper::mapAnchorType: Unknown AnchorType!");
            break;
    }
    return aAnchorType;
}

void SwTextBoxHelper::syncFlyFrameAttr(SwFrameFormat& rShape, SfxItemSet const& rSet)
{
    SwFrameFormat* pFormat = getOtherTextBoxFormat(&rShape, RES_DRAWFRMFMT);
    if (!pFormat)
        return;

    SfxItemSet aTextBoxSet(pFormat->GetDoc()->GetAttrPool(), aFrameFormatSetRange);

    SfxItemIter aIter(rSet);
    const SfxPoolItem* pItem = aIter.GetCurItem();

    do
    {
        switch (pItem->Which())
        {
            case RES_VERT_ORIENT:
            {
                // The new position can be with anchor changing so sync it!
                const text::TextContentAnchorType aNewAnchorType
                    = mapAnchorType(rShape.GetAnchor().GetAnchorId());
                syncProperty(&rShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, uno::Any(aNewAnchorType));
                auto& rOrient = static_cast<const SwFormatVertOrient&>(*pItem);
                SwFormatVertOrient aOrient(rOrient);

                tools::Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                    aOrient.SetPos(aOrient.GetPos() + aRect.getY());

                if (rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE
                    && rShape.GetAnchor().GetPageNum() != 0)
                    aOrient.SetRelationOrient(rShape.GetVertOrient().GetRelationOrient());
                aTextBoxSet.Put(aOrient);

                // restore height (shrunk for extending beyond the page bottom - tdf#91260)
                SwFormatFrameSize aSize(pFormat->GetFrameSize());
                if (!aRect.IsEmpty())
                {
                    aSize.SetHeight(aRect.getHeight());
                    aTextBoxSet.Put(aSize);
                }
            }
            break;
            case RES_HORI_ORIENT:
            {
                // The new position can be with anchor changing so sync it!
                const text::TextContentAnchorType aNewAnchorType
                    = mapAnchorType(rShape.GetAnchor().GetAnchorId());
                syncProperty(&rShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, uno::Any(aNewAnchorType));
                auto& rOrient = static_cast<const SwFormatHoriOrient&>(*pItem);
                SwFormatHoriOrient aOrient(rOrient);

                tools::Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                    aOrient.SetPos(aOrient.GetPos() + aRect.getX());

                if (rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE
                    && rShape.GetAnchor().GetPageNum() != 0)
                    aOrient.SetRelationOrient(rShape.GetHoriOrient().GetRelationOrient());
                aTextBoxSet.Put(aOrient);
            }
            break;
            case RES_FRM_SIZE:
            {
                // In case the shape got resized, then we need to adjust both
                // the position and the size of the textbox (e.g. larger
                // rounded edges of a rectangle -> need to push right/down the
                // textbox).
                SwFormatVertOrient aVertOrient(rShape.GetVertOrient());
                SwFormatHoriOrient aHoriOrient(rShape.GetHoriOrient());
                SwFormatFrameSize aSize(pFormat->GetFrameSize());

                tools::Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                {
                    aVertOrient.SetPos(aVertOrient.GetPos() + aRect.getY());
                    aTextBoxSet.Put(aVertOrient);

                    aHoriOrient.SetPos(aHoriOrient.GetPos() + aRect.getX());
                    aTextBoxSet.Put(aHoriOrient);

                    aSize.SetWidth(aRect.getWidth());
                    aSize.SetHeight(aRect.getHeight());
                    aTextBoxSet.Put(aSize);
                }
            }
            break;
            case RES_ANCHOR:
            {
                auto& rAnchor = static_cast<const SwFormatAnchor&>(*pItem);
                if (rAnchor == rShape.GetAnchor())
                // the anchor have to be synced
                {
                    const text::TextContentAnchorType aNewAnchorType
                        = mapAnchorType(rShape.GetAnchor().GetAnchorId());
                    syncProperty(&rShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE,
                                 uno::Any(aNewAnchorType));
                }
                else
                {
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncFlyFrameAttr: The anchor of the "
                                        "shape different from the textframe!");
                }
            }
            break;
            default:
                SAL_WARN("sw.core", "SwTextBoxHelper::syncFlyFrameAttr: unhandled which-id: "
                                        << pItem->Which());
                break;
        }

        pItem = aIter.NextItem();
    } while (pItem && (0 != pItem->Which()));

    if (aTextBoxSet.Count())
        pFormat->GetDoc()->SetFlyFrameAttr(*pFormat, aTextBoxSet);
}

void SwTextBoxHelper::updateTextBoxMargin(SdrObject* pObj)
{
    if (!pObj)
        return;
    uno::Reference<drawing::XShape> xShape(pObj->getUnoShape(), uno::UNO_QUERY);
    if (!xShape)
        return;
    uno::Reference<beans::XPropertySet> const xPropertySet(xShape, uno::UNO_QUERY);

    auto pParentFormat = getOtherTextBoxFormat(getOtherTextBoxFormat(xShape), RES_FLYFRMFMT);
    if (!pParentFormat)
        return;

    // Sync the padding
    syncProperty(pParentFormat, UNO_NAME_TEXT_LEFTDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_LEFTDIST));
    syncProperty(pParentFormat, UNO_NAME_TEXT_RIGHTDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_RIGHTDIST));
    syncProperty(pParentFormat, UNO_NAME_TEXT_UPPERDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_UPPERDIST));
    syncProperty(pParentFormat, UNO_NAME_TEXT_LOWERDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_LOWERDIST));

    // Sync the text aligning
    syncProperty(pParentFormat, UNO_NAME_TEXT_VERTADJUST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_VERTADJUST));
    syncProperty(pParentFormat, UNO_NAME_TEXT_HORZADJUST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_HORZADJUST));

    //FIXME: Sync autogrow: needs repositioning after sync
    //syncProperty(pParentFormat, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT,
    //             xPropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
