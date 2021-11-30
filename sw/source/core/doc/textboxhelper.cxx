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
#include <dflyobj.hxx>

#include <editeng/unoprnms.hxx>
#include <editeng/memberids.h>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svl/itemiter.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <sal/log.hxx>
#include <tools/UnitConversion.hxx>
#include <svx/swframetypes.hxx>
#include <drawdoc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <frmatr.hxx>

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

using namespace com::sun::star;

void SwTextBoxHelper::create(SwFrameFormat* pShape, SdrObject* pObject, bool bCopyText)
{
    assert(pShape);
    assert(pObject);

    const bool bIsGroupObj = dynamic_cast<SdrObjGroup*>(pObject->getParentSdrObjectFromSdrObject());

    // If TextBox wasn't enabled previously
    if (pShape->GetOtherTextBoxFormat() && pShape->GetOtherTextBoxFormat()->GetTextBox(pObject))
        return;

    // Store the current text content of the shape
    OUString sCopyableText;

    if (bCopyText)
    {
        if (pObject)
        {
            uno::Reference<text::XText> xSrcCnt(pObject->getWeakUnoShape().get(), uno::UNO_QUERY);
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
        uno::Reference<text::XTextContent> XSourceShape(pObject->getUnoShape(),
                                                        uno::UNO_QUERY_THROW);
        xTextContentAppend->insertTextContentWithProperties(
            xTextFrame, uno::Sequence<beans::PropertyValue>(), XSourceShape->getAnchor());
    }
    catch (uno::Exception&)
    {
        xTextContentAppend->appendTextContent(xTextFrame, uno::Sequence<beans::PropertyValue>());
    }
    // Link FLY and DRAW formats, so it becomes a text box (needed for syncProperty calls).
    uno::Reference<text::XTextFrame> xRealTextFrame(xTextFrame, uno::UNO_QUERY);
    auto pTextFrame = dynamic_cast<SwXTextFrame*>(xRealTextFrame.get());
    assert(nullptr != pTextFrame);
    SwFrameFormat* pFormat = pTextFrame->GetFrameFormat();

    assert(nullptr != dynamic_cast<SwDrawFrameFormat*>(pShape));
    assert(nullptr != dynamic_cast<SwFlyFrameFormat*>(pFormat));

    if (!pShape->GetOtherTextBoxFormat())
    {
        auto* pTextBox = new SwTextBoxNode(pShape);
        pTextBox->AddTextBox(pObject, pFormat);
        pShape->SetOtherTextBoxFormat(pTextBox);
        pFormat->SetOtherTextBoxFormat(pTextBox);
    }
    else
    {
        auto* pTextBox = pShape->GetOtherTextBoxFormat();
        pTextBox->AddTextBox(pObject, pFormat);
        pShape->SetOtherTextBoxFormat(pTextBox);
        pFormat->SetOtherTextBoxFormat(pTextBox);
    }
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

    DoTextBoxZOrderCorrection(pShape, pObject);

    // Also initialize the properties, which are not constant, but inherited from the shape's ones.
    uno::Reference<drawing::XShape> xShape(pObject->getUnoShape(), uno::UNO_QUERY);
    syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::makeAny(xShape->getSize()), pObject);

    uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
    syncProperty(pShape, RES_FOLLOW_TEXT_FLOW, MID_FOLLOW_TEXT_FLOW,
                 xShapePropertySet->getPropertyValue(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), pObject);
    syncProperty(pShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE,
                 xShapePropertySet->getPropertyValue(UNO_NAME_ANCHOR_TYPE), pObject);
    syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_ORIENT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT), pObject);
    syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_RELATION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_RELATION), pObject);
    syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_ORIENT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT), pObject);
    syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_RELATION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_RELATION), pObject);
    syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_POSITION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_POSITION), pObject);
    syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_POSITION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_POSITION), pObject);
    syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT), pObject);
    syncProperty(pShape, RES_TEXT_VERT_ADJUST, 0,
                 xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_VERT_ADJUST), pObject);
    text::WritingMode eMode;
    if (xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_WRITINGMODE) >>= eMode)
        syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(sal_Int16(eMode)), pObject);

    if (bIsGroupObj)
        doTextBoxPositioning(pShape, pObject);

    // Check if the shape had text before and move it to the new textframe
    if (!bCopyText || sCopyableText.isEmpty())
        return;

    if (pObject)
    {
        auto pSourceText = dynamic_cast<SdrTextObj*>(pObject);
        uno::Reference<text::XTextRange> xDestText(xRealTextFrame, uno::UNO_QUERY);

        xDestText->setString(sCopyableText);

        if (pSourceText)
            pSourceText->SetText(OUString());

        pShape->GetDoc()->getIDocumentState().SetModified();
    }
}

void SwTextBoxHelper::set(SwFrameFormat* pShapeFormat, SdrObject* pObj,
                          uno::Reference<text::XTextFrame> xNew)
{
    // Do not set invalid data
    assert(pShapeFormat && pObj && xNew);
    // Firstly find the format of the new textbox.
    SwFrameFormat* pFormat = nullptr;
    if (auto pTextFrame = dynamic_cast<SwXTextFrame*>(xNew.get()))
        pFormat = pTextFrame->GetFrameFormat();
    if (!pFormat)
        return;
    std::vector<std::pair<beans::Property, uno::Any>> aOldProps;
    // If there is a format, check if the shape already has a textbox assigned to.
    if (auto pTextBoxNode = pShapeFormat->GetOtherTextBoxFormat())
    {
        // If it has a texbox, destroy it.
        if (pTextBoxNode->GetTextBox(pObj))
        {
            auto xOldFrame
                = pObj->getUnoShape()->queryInterface(cppu::UnoType<text::XTextRange>::get());
            if (xOldFrame.hasValue())
            {
                uno::Reference<beans::XPropertySet> xOldprops(xOldFrame, uno::UNO_QUERY);
                uno::Reference<beans::XPropertyState> xOldPropStates(xOldFrame, uno::UNO_QUERY);
                for (auto& rProp : xOldprops->getPropertySetInfo()->getProperties())
                {
                    try
                    {
                        if (xOldPropStates->getPropertyState(rProp.Name)
                            == beans::PropertyState::PropertyState_DIRECT_VALUE)
                            aOldProps.push_back(
                                std::pair(rProp, xOldprops->getPropertyValue(rProp.Name)));
                    }
                    catch (...)
                    {
                    }
                }
            }
            destroy(pShapeFormat, pObj);
        }
        // And set the new one.
        pTextBoxNode->AddTextBox(pObj, pFormat);
        pFormat->SetOtherTextBoxFormat(pTextBoxNode);
    }
    else
    {
        // If the shape do not have a texbox node and textbox,
        // create that for the shape.
        auto* pTextBox = new SwTextBoxNode(pShapeFormat);
        pTextBox->AddTextBox(pObj, pFormat);
        pShapeFormat->SetOtherTextBoxFormat(pTextBox);
        pFormat->SetOtherTextBoxFormat(pTextBox);
    }
    // Initialize its properties
    uno::Reference<beans::XPropertySet> xPropertySet(xNew, uno::UNO_QUERY);
    uno::Any aEmptyBorder = uno::makeAny(table::BorderLine2());
    xPropertySet->setPropertyValue(UNO_NAME_TOP_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_BOTTOM_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_LEFT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_RIGHT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::makeAny(sal_Int32(100)));
    xPropertySet->setPropertyValue(UNO_NAME_SIZE_TYPE, uno::makeAny(text::SizeType::FIX));
    xPropertySet->setPropertyValue(UNO_NAME_SURROUND, uno::makeAny(text::WrapTextMode_THROUGH));
    // Add a new name to it
    uno::Reference<container::XNamed> xNamed(xNew, uno::UNO_QUERY);
    xNamed->setName(pShapeFormat->GetDoc()->GetUniqueFrameName());
    // And sync. properties.
    uno::Reference<drawing::XShape> xShape(pObj->getUnoShape(), uno::UNO_QUERY);
    syncProperty(pShapeFormat, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::makeAny(xShape->getSize()),
                 pObj);
    uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
    syncProperty(pShapeFormat, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE,
                 xShapePropertySet->getPropertyValue(UNO_NAME_ANCHOR_TYPE), pObj);
    syncProperty(pShapeFormat, RES_HORI_ORIENT, MID_HORIORIENT_ORIENT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT), pObj);
    syncProperty(pShapeFormat, RES_HORI_ORIENT, MID_HORIORIENT_RELATION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_RELATION), pObj);
    syncProperty(pShapeFormat, RES_VERT_ORIENT, MID_VERTORIENT_ORIENT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT), pObj);
    syncProperty(pShapeFormat, RES_VERT_ORIENT, MID_VERTORIENT_RELATION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_RELATION), pObj);
    syncProperty(pShapeFormat, RES_HORI_ORIENT, MID_HORIORIENT_POSITION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_POSITION), pObj);
    syncProperty(pShapeFormat, RES_VERT_ORIENT, MID_VERTORIENT_POSITION,
                 xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_POSITION), pObj);
    syncProperty(pShapeFormat, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT,
                 xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT), pObj);
    drawing::TextVerticalAdjust aVertAdj = drawing::TextVerticalAdjust_CENTER;
    if ((uno::Reference<beans::XPropertyState>(xShape, uno::UNO_QUERY_THROW))
            ->getPropertyState(UNO_NAME_TEXT_VERT_ADJUST)
        != beans::PropertyState::PropertyState_DEFAULT_VALUE)
    {
        aVertAdj = xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_VERT_ADJUST)
                       .get<drawing::TextVerticalAdjust>();
    }
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_VERT_ADJUST, uno::makeAny(aVertAdj));
    text::WritingMode eMode;
    if (xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_WRITINGMODE) >>= eMode)
        syncProperty(pShapeFormat, RES_FRAMEDIR, 0, uno::makeAny(sal_Int16(eMode)), pObj);
    if (aOldProps.size())
    {
        for (auto& rProp : aOldProps)
        {
            try
            {
                xPropertySet->setPropertyValue(rProp.first.Name, rProp.second);
            }
            catch (...)
            {
            }
        }
    }
    if (pFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE
        && pFormat->GetAnchor().GetPageNum() == 0)
    {
        pFormat->SetFormatAttr(SwFormatAnchor(RndStdIds::FLY_AT_PAGE, 1));
    }
    // Do sync for the new textframe.
    synchronizeGroupTextBoxProperty(&changeAnchor, pShapeFormat, pObj);
}

void SwTextBoxHelper::destroy(const SwFrameFormat* pShape, const SdrObject* pObject)
{
    // If a TextBox was enabled previously
    auto pTextBox = pShape->GetOtherTextBoxFormat();
    if (pTextBox && pTextBox->IsTextBoxActive(pObject))
    {
        // Unlink the TextBox's text range from the original shape.
        pTextBox->SetTextBoxInactive(pObject);

        // Delete the associated TextFrame.
        pTextBox->DelTextBox(pObject);
    }
}

bool SwTextBoxHelper::isTextBox(const SwFrameFormat* pFormat, sal_uInt16 nType,
                                const SdrObject* pObject)
{
    SolarMutexGuard aGuard;
    assert(nType == RES_FLYFRMFMT || nType == RES_DRAWFRMFMT);
    if (!pFormat || pFormat->Which() != nType)
        return false;

    auto pTextBox = pFormat->GetOtherTextBoxFormat();
    if (!pTextBox)
        return false;

    if (nType == RES_DRAWFRMFMT)
    {
        if (pObject)
            return pTextBox->GetTextBox(pObject);
        if (auto pObj = pFormat->FindRealSdrObject())
            return pTextBox->GetTextBox(pObj);
    }

    if (nType == RES_FLYFRMFMT)
    {
        return pTextBox->GetOwnerShape();
    }

    return false;
}

bool SwTextBoxHelper::hasTextFrame(const SdrObject* pObj)
{
    if (!pObj)
        return false;

    uno::Reference<drawing::XShape> xShape(pObj->getWeakUnoShape().get(), uno::UNO_QUERY);
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
                                                      sal_uInt16 nType, const SdrObject* pObject)
{
    SolarMutexGuard aGuard;
    if (!isTextBox(pFormat, nType, pObject))
        return nullptr;

    if (nType == RES_DRAWFRMFMT)
    {
        if (pObject)
            return pFormat->GetOtherTextBoxFormat()->GetTextBox(pObject);
        if (pFormat->FindRealSdrObject())
            return pFormat->GetOtherTextBoxFormat()->GetTextBox(pFormat->FindRealSdrObject());
        return nullptr;
    }
    if (nType == RES_FLYFRMFMT)
    {
        return pFormat->GetOtherTextBoxFormat()->GetOwnerShape();
    }
    return nullptr;
}

SwFrameFormat* SwTextBoxHelper::getOtherTextBoxFormat(uno::Reference<drawing::XShape> const& xShape)
{
    auto pShape = dynamic_cast<SwXShape*>(xShape.get());
    if (!pShape)
        return nullptr;

    SwFrameFormat* pFormat = pShape->GetFrameFormat();
    return getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT,
                                 SdrObject::getSdrObjectFromXShape(xShape));
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
            if (pSdrObj)
            {
                return { pSdrObj->getUnoShape(), uno::UNO_QUERY };
            }
        }
    }
    return {};
}

template <typename T>
static void lcl_queryInterface(const SwFrameFormat* pShape, uno::Any& rAny, SdrObject* pObj)
{
    if (SwFrameFormat* pFormat
        = SwTextBoxHelper::getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        uno::Reference<T> const xInterface(
            SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat), uno::UNO_QUERY);
        rAny <<= xInterface;
    }
}

uno::Any SwTextBoxHelper::queryInterface(const SwFrameFormat* pShape, const uno::Type& rType,
                                         SdrObject* pObj)
{
    uno::Any aRet;

    if (rType == cppu::UnoType<css::text::XTextAppend>::get())
    {
        lcl_queryInterface<text::XTextAppend>(pShape, aRet, pObj);
    }
    else if (rType == cppu::UnoType<css::text::XText>::get())
    {
        lcl_queryInterface<text::XText>(pShape, aRet, pObj);
    }
    else if (rType == cppu::UnoType<css::text::XTextRange>::get())
    {
        lcl_queryInterface<text::XTextRange>(pShape, aRet, pObj);
    }

    return aRet;
}

tools::Rectangle SwTextBoxHelper::getTextRectangle(SdrObject* pShape, bool bAbsolute)
{
    tools::Rectangle aRet;
    aRet.SetEmpty();

    assert(pShape);

    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(pShape);
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
    else if (pShape)
    {
        // fallback - get *any* bound rect we can possibly get hold of
        aRet = pShape->GetCurrentBoundRect();
    }

    if (!bAbsolute && pShape)
    {
        // Relative, so count the logic (reference) rectangle, see the EnhancedCustomShape2d ctor.
        Point aPoint(pShape->GetSnapRect().Center());
        Size aSize(pShape->GetLogicRect().GetSize());
        aPoint.AdjustX(-(aSize.Width() / 2));
        aPoint.AdjustY(-(aSize.Height() / 2));
        tools::Rectangle aLogicRect(aPoint, aSize);
        aRet.Move(-1 * aLogicRect.Left(), -1 * aLogicRect.Top());
    }

    return aRet;
}

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, std::u16string_view rPropertyName,
                                   const css::uno::Any& rValue, SdrObject* pObj)
{
    // Textframes does not have valid horizontal adjust property, so map it to paragraph adjust property
    if (rPropertyName == UNO_NAME_TEXT_HORZADJUST)
    {
        SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj);
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

        SdrObject* pObject = pObj ? pObj : pShape->FindRealSdrObject();
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

        SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj);
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
                syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(nDirection), pObj);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_TEXT_VERT_ADJUST)
        syncProperty(pShape, RES_TEXT_VERT_ADJUST, 0, rValue, pObj);
    else if (rPropertyName == UNO_NAME_TEXT_AUTOGROWHEIGHT)
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT, rValue, pObj);
    else if (rPropertyName == UNO_NAME_TEXT_LEFTDIST)
        syncProperty(pShape, RES_BOX, LEFT_BORDER_DISTANCE, rValue, pObj);
    else if (rPropertyName == UNO_NAME_TEXT_RIGHTDIST)
        syncProperty(pShape, RES_BOX, RIGHT_BORDER_DISTANCE, rValue, pObj);
    else if (rPropertyName == UNO_NAME_TEXT_UPPERDIST)
        syncProperty(pShape, RES_BOX, TOP_BORDER_DISTANCE, rValue, pObj);
    else if (rPropertyName == UNO_NAME_TEXT_LOWERDIST)
        syncProperty(pShape, RES_BOX, BOTTOM_BORDER_DISTANCE, rValue, pObj);
    else if (rPropertyName == UNO_NAME_TEXT_WRITINGMODE)
    {
        text::WritingMode eMode;
        sal_Int16 eMode2;
        if (rValue >>= eMode)
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(sal_Int16(eMode)), pObj);
        else if (rValue >>= eMode2)
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::makeAny(eMode2), pObj);
    }
    else
        SAL_INFO("sw.core", "SwTextBoxHelper::syncProperty: unhandled property: "
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
                                    << o3tl::narrowing<sal_uInt16>(nMemberID));
            break;
    }
}

css::uno::Any SwTextBoxHelper::getProperty(SwFrameFormat const* pShape, const OUString& rPropName)
{
    if (!pShape)
        return {};

    SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT);
    if (!pFormat)
        return {};

    uno::Reference<beans::XPropertySet> const xPropertySet(
        SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat), uno::UNO_QUERY);

    return xPropertySet->getPropertyValue(rPropName);
}

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                                   const css::uno::Any& rValue, SdrObject* pObj)
{
    // No shape yet? Then nothing to do, initial properties are set by create().
    if (!pShape)
        return;

    uno::Any aValue(rValue);
    nMemberID &= ~CONVERT_TWIPS;

    SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj);
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
                    if (pShape->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)
                        aPropertyName = UNO_NAME_HORI_ORIENT_RELATION;
                    else
                        return;
                    break;
                case MID_HORIORIENT_POSITION:
                    aPropertyName = UNO_NAME_HORI_ORIENT_POSITION;
                    bAdjustX = true;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
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
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
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
                    if (pShape->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)
                        aPropertyName = UNO_NAME_VERT_ORIENT_RELATION;
                    else
                        return;
                    break;
                case MID_VERTORIENT_POSITION:
                    aPropertyName = UNO_NAME_VERT_ORIENT_POSITION;
                    bAdjustY = true;
                    break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        case RES_FRM_SIZE:
            switch (nMemberID)
            {
                case MID_FRMSIZE_WIDTH_TYPE:
                    aPropertyName = UNO_NAME_WIDTH_TYPE;
                    break;
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
                    setWrapThrough(pShape);
                    changeAnchor(pShape, pObj);
                    doTextBoxPositioning(pShape, pObj);

                    return;
                }
                break;
                default:
                    SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled member-id: "
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
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
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
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
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
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
                                            << o3tl::narrowing<sal_uInt16>(nMemberID)
                                            << " (which-id: " << nWID << ")");
                    break;
            }
            break;
        default:
            SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: unhandled which-id: "
                                    << nWID << " (member-id: "
                                    << o3tl::narrowing<sal_uInt16>(nMemberID) << ")");
            break;
    }

    if (aPropertyName.isEmpty())
        return;

    // Position/size should be the text position/size, not the shape one as-is.
    if (bAdjustX || bAdjustY || bAdjustSize)
    {
        changeAnchor(pShape, pObj);
        tools::Rectangle aRect
            = getTextRectangle(pObj ? pObj : pShape->FindRealSdrObject(), /*bAbsolute=*/false);
        if (!aRect.IsEmpty())
        {
            if (bAdjustX || bAdjustY)
            {
                sal_Int32 nValue;
                if (aValue >>= nValue)
                {
                    nValue += convertTwipToMm100(bAdjustX ? aRect.Left() : aRect.Top());
                    aValue <<= nValue;
                }
            }
            else if (bAdjustSize)
            {
                awt::Size aSize(convertTwipToMm100(aRect.getWidth()),
                                convertTwipToMm100(aRect.getHeight()));
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

void SwTextBoxHelper::syncFlyFrameAttr(SwFrameFormat& rShape, SfxItemSet const& rSet,
                                       SdrObject* pObj)
{
    SwFrameFormat* pFormat = getOtherTextBoxFormat(&rShape, RES_DRAWFRMFMT, pObj);
    if (!pFormat)
        return;

    const bool bInlineAnchored = rShape.GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR;
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
                syncProperty(&rShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, uno::Any(aNewAnchorType),
                             pObj);
                if (bInlineAnchored)
                    return;
                SwFormatVertOrient aOrient(pItem->StaticWhichCast(RES_VERT_ORIENT));

                tools::Rectangle aRect = getTextRectangle(pObj ? pObj : rShape.FindRealSdrObject(),
                                                          /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                    aOrient.SetPos(aOrient.GetPos() + aRect.Top());

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
                syncProperty(&rShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, uno::Any(aNewAnchorType),
                             pObj);
                if (bInlineAnchored)
                    return;
                SwFormatHoriOrient aOrient(pItem->StaticWhichCast(RES_HORI_ORIENT));

                tools::Rectangle aRect = getTextRectangle(pObj ? pObj : rShape.FindRealSdrObject(),
                                                          /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                    aOrient.SetPos(aOrient.GetPos() + aRect.Left());

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

                tools::Rectangle aRect = getTextRectangle(pObj ? pObj : rShape.FindRealSdrObject(),
                                                          /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                {
                    if (!bInlineAnchored)
                    {
                        aVertOrient.SetPos(
                            (pObj ? pObj->GetRelativePos().getX() : aVertOrient.GetPos())
                            + aRect.Top());
                        aHoriOrient.SetPos(
                            (pObj ? pObj->GetRelativePos().getY() : aHoriOrient.GetPos())
                            + aRect.Left());

                        aTextBoxSet.Put(aVertOrient);
                        aTextBoxSet.Put(aHoriOrient);
                    }

                    aSize.SetWidth(aRect.getWidth());
                    aSize.SetHeight(aRect.getHeight());
                    aTextBoxSet.Put(aSize);
                }
            }
            break;
            case RES_ANCHOR:
            {
                if (pItem->StaticWhichCast(RES_ANCHOR) == rShape.GetAnchor())
                // the anchor have to be synced
                {
                    const text::TextContentAnchorType aNewAnchorType
                        = mapAnchorType(rShape.GetAnchor().GetAnchorId());
                    syncProperty(&rShape, RES_ANCHOR, MID_ANCHOR_ANCHORTYPE,
                                 uno::Any(aNewAnchorType), pObj);
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
        pFormat->SetFormatAttr(aTextBoxSet);
    //pFormat->GetDoc()->SetFlyFrameAttr(*pFormat, aTextBoxSet);

    DoTextBoxZOrderCorrection(&rShape, pObj);
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

    // tdf137803: Sync autogrow:
    const bool bIsAutoGrow
        = xPropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT).get<bool>();
    const bool bIsAutoWrap = xPropertySet->getPropertyValue(UNO_NAME_TEXT_WORDWRAP).get<bool>();

    syncProperty(pParentFormat, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT, uno::Any(bIsAutoGrow));

    syncProperty(pParentFormat, RES_FRM_SIZE, MID_FRMSIZE_WIDTH_TYPE,
                 uno::Any(bIsAutoWrap ? text::SizeType::FIX : text::SizeType::MIN));

    changeAnchor(pParentFormat, pObj);
    DoTextBoxZOrderCorrection(pParentFormat, pObj);
}

bool SwTextBoxHelper::setWrapThrough(SwFrameFormat* pShape)
{
    OUString sErrMsg;
    if (isTextBoxShapeHasValidTextFrame(pShape))
    {
        if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
        {
            ::sw::UndoGuard const UndoGuard(pShape->GetDoc()->GetIDocumentUndoRedo());
            if (auto xFrame = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat))
                try
                {
                    uno::Reference<beans::XPropertySet> const xPropertySet(xFrame, uno::UNO_QUERY);
                    xPropertySet->setPropertyValue(UNO_NAME_SURROUND,
                                                   uno::makeAny(text::WrapTextMode_THROUGH));
                    return true;
                }
                catch (uno::Exception& e)
                {
                    sErrMsg = "Exception caught: " + e.Message;
                }
            else
                sErrMsg = "No XTextFrame!";
        }
        else
            sErrMsg = "No Other TextBox Format!";
    }
    else
        sErrMsg = "Not a Valid TextBox object!";

    SAL_WARN("sw.core", "SwTextBoxHelper::setWrapThrough: " << sErrMsg);
    return false;
}

bool SwTextBoxHelper::changeAnchor(SwFrameFormat* pShape, SdrObject* pObj)
{
    if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        const SwFormatAnchor& rOldAnch = pFormat->GetAnchor();
        const SwFormatAnchor& rNewAnch = pShape->GetAnchor();

        const auto pOldCnt = rOldAnch.GetContentAnchor();
        const auto pNewCnt = rNewAnch.GetContentAnchor();

        const uno::Any aShapeHorRelOrient
            = uno::makeAny(pShape->GetHoriOrient().GetRelationOrient());

        if (isAnchorTypeDifferent(pShape) || (pObj && pObj != pShape->FindRealSdrObject()))
        {
            try
            {
                ::sw::UndoGuard const UndoGuard(pShape->GetDoc()->GetIDocumentUndoRedo());
                uno::Reference<beans::XPropertySet> const xPropertySet(
                    SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat), uno::UNO_QUERY);
                if (pOldCnt && rNewAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE
                    && rNewAnch.GetPageNum())
                {
                    uno::Any aValue(text::TextContentAnchorType_AT_PAGE);
                    xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION,
                                                   aShapeHorRelOrient);
                    xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, aValue);
                    xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_PAGE_NO,
                                                   uno::Any(rNewAnch.GetPageNum()));
                }
                else if (rOldAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE && pNewCnt)
                {
                    if (rNewAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                    {
                        uno::Any aValue(text::TextContentAnchorType_AT_CHARACTER);
                        xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, aValue);
                        xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION,
                                                       uno::Any(text::RelOrientation::CHAR));
                        xPropertySet->setPropertyValue(UNO_NAME_VERT_ORIENT_RELATION,
                                                       uno::Any(text::RelOrientation::PRINT_AREA));
                        SwFormatAnchor aPos(pFormat->GetAnchor());
                        aPos.SetAnchor(pNewCnt);
                        pFormat->SetFormatAttr(aPos);
                    }
                    else
                    {
                        uno::Any aValue(mapAnchorType(rNewAnch.GetAnchorId()));
                        xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION,
                                                       aShapeHorRelOrient);
                        xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, aValue);
                        pFormat->SetFormatAttr(rNewAnch);
                    }
                }
                else
                {
                    if (rNewAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                    {
                        uno::Any aValue(text::TextContentAnchorType_AT_CHARACTER);
                        xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, aValue);
                        xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION,
                                                       uno::Any(text::RelOrientation::CHAR));
                        xPropertySet->setPropertyValue(UNO_NAME_VERT_ORIENT_RELATION,
                                                       uno::Any(text::RelOrientation::PRINT_AREA));
                        SwFormatAnchor aPos(pFormat->GetAnchor());
                        aPos.SetAnchor(pNewCnt);
                        pFormat->SetFormatAttr(aPos);
                    }
                    else
                    {
                        xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION,
                                                       aShapeHorRelOrient);
                        pFormat->SetFormatAttr(pShape->GetAnchor());
                    }
                }
            }
            catch (uno::Exception& e)
            {
                SAL_WARN("sw.core", "SwTextBoxHelper::changeAnchor(): " << e.Message);
            }
        }

        return doTextBoxPositioning(pShape, pObj) && DoTextBoxZOrderCorrection(pShape, pObj);
    }

    return false;
}

bool SwTextBoxHelper::doTextBoxPositioning(SwFrameFormat* pShape, SdrObject* pObj)
{
    const bool bIsGroupObj = (pObj != pShape->FindRealSdrObject()) && pObj;
    if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        ::sw::UndoGuard const UndoGuard(pShape->GetDoc()->GetIDocumentUndoRedo());
        if (pShape->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
        {
            tools::Rectangle aRect(
                getTextRectangle(pObj ? pObj : pShape->FindRealSdrObject(), false));

            auto nLeftSpace = pShape->GetLRSpace().GetLeft();

            SwFormatHoriOrient aNewHOri(pFormat->GetHoriOrient());
            aNewHOri.SetPos(aRect.Left() + nLeftSpace);

            SwFormatVertOrient aNewVOri(pFormat->GetVertOrient());
            aNewVOri.SetPos(aRect.Top() + pShape->GetVertOrient().GetPos());

            // tdf#140598: Do not apply wrong rectangle position.
            if (aRect.TopLeft() != Point(0, 0))
            {
                pFormat->SetFormatAttr(aNewHOri);
                pFormat->SetFormatAttr(aNewVOri);
            }
            else
                SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: Repositioning failed!");
        }
        else
        {
            tools::Rectangle aRect(
                getTextRectangle(pObj ? pObj : pShape->FindRealSdrObject(), false));

            // tdf#140598: Do not apply wrong rectangle position.
            if (aRect.TopLeft() != Point(0, 0) || bIsGroupObj)
            {
                SwFormatHoriOrient aNewHOri(pShape->GetHoriOrient());
                aNewHOri.SetPos(
                    (bIsGroupObj && pObj ? pObj->GetRelativePos().getX() : aNewHOri.GetPos())
                    + aRect.Left());
                SwFormatVertOrient aNewVOri(pShape->GetVertOrient());
                aNewVOri.SetPos(
                    (bIsGroupObj && pObj ? pObj->GetRelativePos().getY() : aNewVOri.GetPos())
                    + aRect.Top());

                pFormat->SetFormatAttr(aNewHOri);
                pFormat->SetFormatAttr(aNewVOri);
            }
            else
                SAL_WARN("sw.core", "SwTextBoxHelper::syncProperty: Repositioning failed!");
        }
        return true;
    }

    return false;
}

std::optional<bool> SwTextBoxHelper::isAnchorTypeDifferent(const SwFrameFormat* pShape)
{
    std::optional<bool> bRet;
    if (isTextBoxShapeHasValidTextFrame(pShape))
    {
        if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
        {
            if (pShape->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                bRet = (pFormat->GetAnchor().GetAnchorId() != RndStdIds::FLY_AT_CHAR
                        && pFormat->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR);
            else
                bRet = pFormat->GetAnchor().GetAnchorId() != pShape->GetAnchor().GetAnchorId();
        }
    }
    return bRet;
}

bool SwTextBoxHelper::syncTextBoxSize(SwFrameFormat* pShape, SdrObject* pObj)
{
    if (!pShape || !pObj)
        return false;

    if (auto pTextBox = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        const auto& rSize = getTextRectangle(pObj, false).GetSize();
        if (!rSize.IsEmpty())
        {
            SwFormatFrameSize aSize(pTextBox->GetFrameSize());
            aSize.SetSize(rSize);
            return pTextBox->SetFormatAttr(aSize);
        }
    }

    return false;
}

bool SwTextBoxHelper::isTextBoxShapeHasValidTextFrame(const SwFrameFormat* pShape)
{
    if (pShape && pShape->Which() == RES_DRAWFRMFMT)
        if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
            if (pFormat && pFormat->Which() == RES_FLYFRMFMT)
                return true;
            else
                SAL_WARN("sw.core", "SwTextBoxHelper::isTextBoxShapeHasValidTextFrame: "
                                    "Shape does not have valid textframe!");
        else
            SAL_WARN("sw.core", "SwTextBoxHelper::isTextBoxShapeHasValidTextFrame: "
                                "Shape does not have associated frame!");
    else
        SAL_WARN("sw.core", "SwTextBoxHelper::isTextBoxShapeHasValidTextFrame: Not valid shape!");
    return false;
}

bool SwTextBoxHelper::DoTextBoxZOrderCorrection(SwFrameFormat* pShape, const SdrObject* pObj)
{
    // TODO: do this with group shape textboxes.
    SdrObject* pShpObj = nullptr;
    //if (pObj)
    //    pShpObj = pObj;
    //else
    pShpObj = pShape->FindRealSdrObject();

    if (pShpObj)
    {
        if (SdrObject* pFrmObj
            = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj)->FindRealSdrObject())
        {
            // Get the draw model from the doc
            SwDrawModel* pDrawModel
                = pShape->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
            if (pDrawModel)
            {
                // Not really sure this will work all page, but it seems it will.
                auto pPage = pDrawModel->GetPage(0);
                // Recalc all Zorders
                pPage->RecalcObjOrdNums();
                // Here is a counter avoiding running to in infinity:
                sal_uInt16 nIterator = 0;
                // If the shape is behind the frame, is good, but if there are some objects
                // between of them that is wrong so put the frame exactly one level higher
                // than the shape.
                if (pFrmObj->GetOrdNum() > pShpObj->GetOrdNum())
                    pPage->SetObjectOrdNum(pFrmObj->GetOrdNum(), pShpObj->GetOrdNum() + 1);
                else
                    // Else, if the frame is behind the shape, bring to the front of it.
                    while (pFrmObj->GetOrdNum() <= pShpObj->GetOrdNum())
                    {
                        pPage->SetObjectOrdNum(pFrmObj->GetOrdNum(), pFrmObj->GetOrdNum() + 1);
                        // If there is any problem with the indexes, do not run over the infinity
                        if (pPage->GetObjCount() == pFrmObj->GetOrdNum())
                            break;
                        ++nIterator;
                        if (nIterator > 300)
                            break; // Do not run to infinity
                    }
                pPage->RecalcObjOrdNums();
                return true; // Success
            }
            SAL_WARN("sw.core", "SwTextBoxHelper::DoTextBoxZOrderCorrection(): "
                                "No Valid Draw model for SdrObject for the shape!");
        }
        SAL_WARN("sw.core", "SwTextBoxHelper::DoTextBoxZOrderCorrection(): "
                            "No Valid SdrObject for the frame!");
    }
    SAL_WARN("sw.core", "SwTextBoxHelper::DoTextBoxZOrderCorrection(): "
                        "No Valid SdrObject for the shape!");

    return false;
}

void SwTextBoxHelper::synchronizeGroupTextBoxProperty(bool pFunc(SwFrameFormat*, SdrObject*),
                                                      SwFrameFormat* pShape, SdrObject* pObj)
{
    if (auto pChildren = pObj->getChildrenOfSdrObject())
    {
        for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
            synchronizeGroupTextBoxProperty(pFunc, pShape, pChildren->GetObj(i));
    }
    else
    {
        (*pFunc)(pShape, pObj);
    }
}

std::vector<SwFrameFormat*> SwTextBoxHelper::CollectTextBoxes(SdrObject* pGroupObject,
                                                              SwFrameFormat* pFormat)
{
    std::vector<SwFrameFormat*> vRet;
    if (auto pChildren = pGroupObject->getChildrenOfSdrObject())
    {
        for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
        {
            auto pChildTextBoxes = CollectTextBoxes(pChildren->GetObj(i), pFormat);
            for (auto& rChildTextBox : pChildTextBoxes)
                vRet.push_back(rChildTextBox);
        }
    }
    else
    {
        if (isTextBox(pFormat, RES_DRAWFRMFMT, pGroupObject))
            vRet.push_back(getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT, pGroupObject));
    }
    return vRet;
}

SwTextBoxNode::SwTextBoxNode(SwFrameFormat* pOwnerShape)
{
    assert(pOwnerShape);
    assert(pOwnerShape->Which() == RES_DRAWFRMFMT);

    m_pOwnerShapeFormat = pOwnerShape;
    if (!m_pTextBoxes.empty())
        m_pTextBoxes.clear();
}

SwTextBoxNode::~SwTextBoxNode()
{
    m_pTextBoxes.clear();

    if (m_pOwnerShapeFormat && m_pOwnerShapeFormat->GetOtherTextBoxFormat())
        m_pOwnerShapeFormat->SetOtherTextBoxFormat(nullptr);
}

void SwTextBoxNode::AddTextBox(SdrObject* pDrawObject, SwFrameFormat* pNewTextBox)
{
    assert(pNewTextBox);
    assert(pNewTextBox->Which() == RES_FLYFRMFMT);

    assert(pDrawObject);

    SwTextBoxElement aElem;
    aElem.m_bIsActive = true;
    aElem.m_pDrawObject = pDrawObject;
    aElem.m_pTextBoxFormat = pNewTextBox;
    auto pSwFlyDraw = dynamic_cast<SwFlyDrawObj*>(pDrawObject);
    if (pSwFlyDraw)
    {
        pSwFlyDraw->SetTextBox(true);
    }
    m_pTextBoxes.push_back(aElem);
}

void SwTextBoxNode::DelTextBox(const SdrObject* pDrawObject)
{
    assert(pDrawObject);
    if (m_pTextBoxes.empty())
        return;

    for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end();)
    {
        if (it->m_pDrawObject == pDrawObject)
        {
            m_pOwnerShapeFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(
                it->m_pTextBoxFormat);
            it = m_pTextBoxes.erase(it);
            break;
        }
        ++it;
    }
}

SwFrameFormat* SwTextBoxNode::GetTextBox(const SdrObject* pDrawObject) const
{
    assert(pDrawObject);
    if (!m_pTextBoxes.empty())
    {
        for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end(); it++)
        {
            if (it->m_pDrawObject == pDrawObject)
            {
                return it->m_pTextBoxFormat;
            }
        }
    }
    return nullptr;
}

bool SwTextBoxNode::IsTextBoxActive(const SdrObject* pDrawObject) const
{
    assert(pDrawObject);

    if (!m_pTextBoxes.empty())
    {
        for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end(); it++)
        {
            if (it->m_pDrawObject == pDrawObject)
            {
                return it->m_bIsActive;
            }
        }
    }
    return false;
}

void SwTextBoxNode::SetTextBoxActive(const SdrObject* pDrawObject)
{
    assert(pDrawObject);

    if (!m_pTextBoxes.empty())
    {
        for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end(); it++)
        {
            if (it->m_pDrawObject == pDrawObject)
            {
                it->m_bIsActive = true;
            }
        }
    }
}

void SwTextBoxNode::SetTextBoxInactive(const SdrObject* pDrawObject)
{
    assert(pDrawObject);

    if (!m_pTextBoxes.empty())
    {
        for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end(); it++)
        {
            if (it->m_pDrawObject == pDrawObject)
            {
                it->m_bIsActive = false;
            }
        }
    }
}

bool SwTextBoxNode::IsGroupTextBox() const { return m_pTextBoxes.size() > 1; }

std::map<SdrObject*, SwFrameFormat*> SwTextBoxNode::GetAllTextBoxes() const
{
    std::map<SdrObject*, SwFrameFormat*> aRet;
    for (auto& rElem : m_pTextBoxes)
    {
        aRet.emplace(rElem.m_pDrawObject, rElem.m_pTextBoxFormat);
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
