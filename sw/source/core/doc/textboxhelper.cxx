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
#include <fmtfollowtextflow.hxx>
#include <frmfmt.hxx>
#include <frameformats.hxx>
#include <dflyobj.hxx>
#include <swtable.hxx>

#include <editeng/unoprnms.hxx>
#include <editeng/memberids.h>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
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

    // If TextBox wasn't enabled previously
    if (pShape->GetOtherTextBoxFormats() && pShape->GetOtherTextBoxFormats()->GetTextBox(pObject))
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
    xTextContentAppend->appendTextContent(xTextFrame, uno::Sequence<beans::PropertyValue>());

    // Link FLY and DRAW formats, so it becomes a text box (needed for syncProperty calls).
    uno::Reference<text::XTextFrame> xRealTextFrame(xTextFrame, uno::UNO_QUERY);
    auto pTextFrame = dynamic_cast<SwXTextFrame*>(xRealTextFrame.get());
    assert(nullptr != pTextFrame);
    SwFrameFormat* pFormat = pTextFrame->GetFrameFormat();

    assert(nullptr != dynamic_cast<SwDrawFrameFormat*>(pShape));
    assert(nullptr != dynamic_cast<SwFlyFrameFormat*>(pFormat));

    if (!pShape->GetOtherTextBoxFormats())
    {
        auto pTextBox = std::make_shared<SwTextBoxNode>(SwTextBoxNode(pShape));
        pTextBox->AddTextBox(pObject, pFormat);
        pShape->SetOtherTextBoxFormats(pTextBox);
        pFormat->SetOtherTextBoxFormats(pTextBox);
    }
    else
    {
        auto& pTextBox = pShape->GetOtherTextBoxFormats();
        pTextBox->AddTextBox(pObject, pFormat);
        pFormat->SetOtherTextBoxFormats(pTextBox);
    }
    // Initialize properties.
    uno::Reference<beans::XPropertySet> xPropertySet(xTextFrame, uno::UNO_QUERY);
    uno::Any aEmptyBorder{ table::BorderLine2() };
    xPropertySet->setPropertyValue(UNO_NAME_TOP_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_BOTTOM_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_LEFT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_RIGHT_BORDER, aEmptyBorder);

    xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::Any(sal_Int32(100)));

    xPropertySet->setPropertyValue(UNO_NAME_SIZE_TYPE, uno::Any(text::SizeType::FIX));

    xPropertySet->setPropertyValue(UNO_NAME_SURROUND, uno::Any(text::WrapTextMode_THROUGH));

    uno::Reference<container::XNamed> xNamed(xTextFrame, uno::UNO_QUERY);
    assert(!xNamed->getName().isEmpty());
    (void)xNamed;

    // Link its text range to the original shape.
    uno::Reference<text::XTextRange> xTextBox(xTextFrame, uno::UNO_QUERY_THROW);
    SwUnoInternalPaM aInternalPaM(*pShape->GetDoc());
    if (sw::XTextRangeToSwPaM(aInternalPaM, xTextBox))
    {
        SwAttrSet aSet(pShape->GetAttrSet());
        SwFormatContent aContent(aInternalPaM.GetPointNode().StartOfSectionNode());
        aSet.Put(aContent);
        pShape->SetFormatAttr(aSet);
    }

    DoTextBoxZOrderCorrection(pShape, pObject);

    // Also initialize the properties, which are not constant, but inherited from the shape's ones.
    uno::Reference<drawing::XShape> xShape(pObject->getUnoShape(), uno::UNO_QUERY);
    syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::Any(xShape->getSize()), pObject);

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
        syncProperty(pShape, RES_FRAMEDIR, 0, uno::Any(sal_Int16(eMode)), pObject);

    changeAnchor(pShape, pObject);
    syncTextBoxSize(pShape, pObject);

    // Check if the shape had text before and move it to the new textframe
    if (!bCopyText || sCopyableText.isEmpty())
        return;

    if (pObject)
    {
        auto pSourceText = DynCastSdrTextObj(pObject);
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

    // If there is a format, check if the shape already has a textbox assigned to.
    if (auto& pTextBoxNode = pShapeFormat->GetOtherTextBoxFormats())
    {
        // If it has a texbox, destroy it.
        if (pTextBoxNode->GetTextBox(pObj))
            pTextBoxNode->DelTextBox(pObj, true);
        // And set the new one.
        pTextBoxNode->AddTextBox(pObj, pFormat);
        pFormat->SetOtherTextBoxFormats(pTextBoxNode);
    }
    else
    {
        // If the shape do not have a texbox node and textbox,
        // create that for the shape.
        auto pTextBox = std::make_shared<SwTextBoxNode>(SwTextBoxNode(pShapeFormat));
        pTextBox->AddTextBox(pObj, pFormat);
        pShapeFormat->SetOtherTextBoxFormats(pTextBox);
        pFormat->SetOtherTextBoxFormats(pTextBox);
    }
    // Initialize its properties
    uno::Reference<beans::XPropertySet> xPropertySet(xNew, uno::UNO_QUERY);
    uno::Any aEmptyBorder{ table::BorderLine2() };
    xPropertySet->setPropertyValue(UNO_NAME_TOP_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_BOTTOM_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_LEFT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_RIGHT_BORDER, aEmptyBorder);
    xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::Any(sal_Int32(100)));
    xPropertySet->setPropertyValue(UNO_NAME_SIZE_TYPE, uno::Any(text::SizeType::FIX));
    xPropertySet->setPropertyValue(UNO_NAME_SURROUND, uno::Any(text::WrapTextMode_THROUGH));
    // Add a new name to it
    uno::Reference<container::XNamed> xNamed(xNew, uno::UNO_QUERY);
    assert(!xNamed->getName().isEmpty());
    (void)xNamed;
    // And sync. properties.
    uno::Reference<drawing::XShape> xShape(pObj->getUnoShape(), uno::UNO_QUERY);
    syncProperty(pShapeFormat, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::Any(xShape->getSize()), pObj);
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
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_VERT_ADJUST, uno::Any(aVertAdj));
    text::WritingMode eMode;
    if (xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_WRITINGMODE) >>= eMode)
        syncProperty(pShapeFormat, RES_FRAMEDIR, 0, uno::Any(sal_Int16(eMode)), pObj);

    // Do sync for the new textframe.
    synchronizeGroupTextBoxProperty(&changeAnchor, pShapeFormat, pObj);
    synchronizeGroupTextBoxProperty(&syncTextBoxSize, pShapeFormat, pObj);

    updateTextBoxMargin(pObj);
}

void SwTextBoxHelper::destroy(const SwFrameFormat* pShape, const SdrObject* pObject)
{
    // If a TextBox was enabled previously
    auto& pTextBox = pShape->GetOtherTextBoxFormats();
    if (pTextBox)
    {
        // Unlink the TextBox's text range from the original shape.
        // Delete the associated TextFrame.
        pTextBox->DelTextBox(pObject, true);
    }
}

bool SwTextBoxHelper::isTextBox(const SwFrameFormat* pFormat, sal_uInt16 nType,
                                const SdrObject* pObject)
{
    DBG_TESTSOLARMUTEX();
    assert(nType == RES_FLYFRMFMT || nType == RES_DRAWFRMFMT);
    if (!pFormat || pFormat->Which() != nType)
        return false;

    auto& pTextBox = pFormat->GetOtherTextBoxFormats();
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
    for (const rtl::Reference<SdrObject>& p : *pPage)
    {
        if (p && p->IsTextBox())
            continue;
        ++nRet;
    }
    return nRet;
}

sal_Int32 SwTextBoxHelper::getCount(const SwDoc& rDoc)
{
    sal_Int32 nRet = 0;
    for (const sw::SpzFrameFormat* pFormat : *rDoc.GetSpzFrameFormats())
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
    for (const rtl::Reference<SdrObject>& p : *pPage)
    {
        if (p && p->IsTextBox())
            continue;
        if (nCount == nIndex)
        {
            pRet = p.get();
            break;
        }
        ++nCount;
    }

    if (!pRet)
        throw lang::IndexOutOfBoundsException();

    return uno::Any(uno::Reference<drawing::XShape>(pRet->getUnoShape(), uno::UNO_QUERY));
}

sal_Int32 SwTextBoxHelper::getOrdNum(const SdrObject* pObject)
{
    if (const SdrPage* pPage = pObject->getSdrPageFromSdrObject())
    {
        sal_Int32 nOrder = 0; // Current logical order.
        for (const rtl::Reference<SdrObject>& p : *pPage)
        {
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
            return pFormat->GetOtherTextBoxFormats()->GetTextBox(pObject);
        if (pFormat->FindRealSdrObject())
            return pFormat->GetOtherTextBoxFormats()->GetTextBox(pFormat->FindRealSdrObject());
        return nullptr;
    }
    if (nType == RES_FLYFRMFMT)
    {
        return pFormat->GetOtherTextBoxFormats()->GetOwnerShape();
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
            getXWeak(SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat).get()),
            uno::UNO_QUERY);
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

tools::Rectangle SwTextBoxHelper::getRelativeTextRectangle(SdrObject* pShape)
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

    if (pShape)
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
                    uno::Any(style::ParagraphAdjust::ParagraphAdjust_CENTER)); //3
                break;
            case drawing::TextHorizontalAdjust::TextHorizontalAdjust_LEFT:
                xFrameParaProps->setPropertyValue(
                    UNO_NAME_PARA_ADJUST,
                    uno::Any(style::ParagraphAdjust::ParagraphAdjust_LEFT)); //0
                break;
            case drawing::TextHorizontalAdjust::TextHorizontalAdjust_RIGHT:
                xFrameParaProps->setPropertyValue(
                    UNO_NAME_PARA_ADJUST,
                    uno::Any(style::ParagraphAdjust::ParagraphAdjust_RIGHT)); //1
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
            syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_POSITION,
                         uno::Any(static_cast<sal_Int32>(convertTwipToMm100(aRectangle.Left()))));
            syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_POSITION,
                         uno::Any(static_cast<sal_Int32>(convertTwipToMm100(aRectangle.Top()))));
        }

        SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj);
        if (!pFormat)
            return;

        // Older documents or documents in ODF strict do not have WritingMode, but have used the
        // TextRotateAngle values -90 and -270 to emulate these text directions of frames.
        // ToDo: Is TextPreRotateAngle needed for diagrams or can it be removed?
        comphelper::SequenceAsHashMap aCustomShapeGeometry(rValue);
        auto it = aCustomShapeGeometry.find(u"TextPreRotateAngle"_ustr);
        if (it == aCustomShapeGeometry.end())
        {
            it = aCustomShapeGeometry.find(u"TextRotateAngle"_ustr);
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
                    nDirection = text::WritingMode2::TB_RL90;
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
                syncProperty(pShape, RES_FRAMEDIR, 0, uno::Any(nDirection), pObj);
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
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::Any(sal_Int16(eMode)), pObj);
        else if (rValue >>= eMode2)
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::Any(eMode2), pObj);
    }
    else if (rPropertyName == u"WritingMode")
    {
        sal_Int16 eMode2;
        if (rValue >>= eMode2)
            syncProperty(pShape, RES_FRAMEDIR, 0, uno::Any(eMode2), pObj);
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

    rtl::Reference<SwXTextFrame> xPropertySet
        = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);

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
                    changeAnchor(pShape, pObj);
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
            = getRelativeTextRectangle(pObj ? pObj : pShape->FindRealSdrObject());
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
                awt::Size aSize(convertTwipToMm100(aRect.getOpenWidth()),
                                convertTwipToMm100(aRect.getOpenHeight()));
                aValue <<= aSize;
            }
        }
    }
    auto aGuard = SwTextBoxLockGuard(*pShape->GetOtherTextBoxFormats());
    rtl::Reference<SwXTextFrame> const xPropertySet
        = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);
    xPropertySet->setPropertyValue(aPropertyName, aValue);
}

void SwTextBoxHelper::saveLinks(const sw::FrameFormats<sw::SpzFrameFormat*>& rFormats,
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
    const bool bLayoutInCell = rShape.GetFollowTextFlow().GetValue()
                               && rShape.GetAnchor().GetAnchorNode()
                               && rShape.GetAnchor().GetAnchorNode()->FindTableNode();
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
                if (bInlineAnchored || bLayoutInCell)
                    return;
                SwFormatVertOrient aOrient(pItem->StaticWhichCast(RES_VERT_ORIENT));

                tools::Rectangle aRect
                    = getRelativeTextRectangle(pObj ? pObj : rShape.FindRealSdrObject());
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
                    aSize.SetHeight(aRect.getOpenHeight());
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
                if (bInlineAnchored || bLayoutInCell)
                    return;
                SwFormatHoriOrient aOrient(pItem->StaticWhichCast(RES_HORI_ORIENT));

                tools::Rectangle aRect
                    = getRelativeTextRectangle(pObj ? pObj : rShape.FindRealSdrObject());
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

                tools::Rectangle aRect
                    = getRelativeTextRectangle(pObj ? pObj : rShape.FindRealSdrObject());
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

                    aSize.SetWidth(aRect.getOpenWidth());
                    aSize.SetHeight(aRect.getOpenHeight());
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
    {
        auto aGuard = SwTextBoxLockGuard(*rShape.GetOtherTextBoxFormats());
        pFormat->SetFormatAttr(aTextBoxSet);
    }
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
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_LEFTDIST), pObj);
    syncProperty(pParentFormat, UNO_NAME_TEXT_RIGHTDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_RIGHTDIST), pObj);
    syncProperty(pParentFormat, UNO_NAME_TEXT_UPPERDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_UPPERDIST), pObj);
    syncProperty(pParentFormat, UNO_NAME_TEXT_LOWERDIST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_LOWERDIST), pObj);

    // Sync the text aligning
    syncProperty(pParentFormat, UNO_NAME_TEXT_VERTADJUST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_VERTADJUST), pObj);
    syncProperty(pParentFormat, UNO_NAME_TEXT_HORZADJUST,
                 xPropertySet->getPropertyValue(UNO_NAME_TEXT_HORZADJUST), pObj);

    // tdf137803: Sync autogrow:
    const bool bIsAutoGrow
        = xPropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT).get<bool>();
    const bool bIsAutoWrap = xPropertySet->getPropertyValue(UNO_NAME_TEXT_WORDWRAP).get<bool>();

    syncProperty(pParentFormat, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT, uno::Any(bIsAutoGrow),
                 pObj);

    syncProperty(pParentFormat, RES_FRM_SIZE, MID_FRMSIZE_WIDTH_TYPE,
                 uno::Any(bIsAutoWrap ? text::SizeType::FIX : text::SizeType::MIN), pObj);

    changeAnchor(pParentFormat, pObj);
    DoTextBoxZOrderCorrection(pParentFormat, pObj);
}

bool SwTextBoxHelper::changeAnchor(SwFrameFormat* pShape, SdrObject* pObj)
{
    if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        if (!isAnchorSyncNeeded(pShape, pFormat))
        {
            doTextBoxPositioning(pShape, pObj);
            DoTextBoxZOrderCorrection(pShape, pObj);
            if (pShape->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR
                && pFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_CHAR
                && pFormat->GetVertOrient().GetRelationOrient() != text::RelOrientation::PRINT_AREA)
            {
                SwFormatVertOrient aTmp = pFormat->GetVertOrient();
                aTmp.SetRelationOrient(text::RelOrientation::PRINT_AREA);
                pFormat->SetFormatAttr(aTmp);
            }

            return false;
        }

        const SwFormatAnchor& rOldAnch = pFormat->GetAnchor();
        const SwFormatAnchor& rNewAnch = pShape->GetAnchor();

        const auto pOldCnt = rOldAnch.GetContentAnchor();
        const auto pNewCnt = rNewAnch.GetContentAnchor();

        const uno::Any aShapeHorRelOrient(pShape->GetHoriOrient().GetRelationOrient());

        try
        {
            auto aGuard = SwTextBoxLockGuard(*pShape->GetOtherTextBoxFormats());
            ::sw::UndoGuard const UndoGuard(pShape->GetDoc()->GetIDocumentUndoRedo());
            rtl::Reference<SwXTextFrame> const xPropertySet
                = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);
            if (pOldCnt && rNewAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE
                && rNewAnch.GetPageNum())
            {
                uno::Any aValue(text::TextContentAnchorType_AT_PAGE);
                xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION, aShapeHorRelOrient);
                xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, aValue);
                xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_PAGE_NO,
                                               uno::Any(rNewAnch.GetPageNum()));
            }
            else if (rOldAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE && pNewCnt)
            {
                if (rNewAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                {
                    assert(pNewCnt);
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
                    assert(pNewCnt);
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
                    if (rNewAnch.GetAnchorId() == RndStdIds::FLY_AT_PAGE
                        && rNewAnch.GetPageNum() == 0)
                    {
                        pFormat->SetFormatAttr(SwFormatAnchor(RndStdIds::FLY_AT_PAGE, 1));
                    }
                    else
                        pFormat->SetFormatAttr(pShape->GetAnchor());
                }
            }
        }
        catch (uno::Exception& e)
        {
            SAL_WARN("sw.core", "SwTextBoxHelper::changeAnchor(): " << e.Message);
        }

        doTextBoxPositioning(pShape, pObj);
        DoTextBoxZOrderCorrection(pShape, pObj);
        return true;
    }

    return false;
}

bool SwTextBoxHelper::doTextBoxPositioning(SwFrameFormat* pShape, SdrObject* pObj)
{
    // Set the position of the textboxes according to the position of its shape-pair
    const bool bIsGroupObj = (pObj != pShape->FindRealSdrObject()) && pObj;
    if (auto pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        // Do not create undo entry for the positioning
        ::sw::UndoGuard const UndoGuard(pShape->GetDoc()->GetIDocumentUndoRedo());
        auto aGuard = SwTextBoxLockGuard(*pShape->GetOtherTextBoxFormats());
        // Special treatment for AS_CHAR textboxes:
        if (pShape->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
        {
            // Get the text area of the shape
            tools::Rectangle aRect
                = getRelativeTextRectangle(pObj ? pObj : pShape->FindRealSdrObject());

            // Get the left spacing of the text area of the shape
            auto nLeftSpace = pShape->GetLRSpace().GetLeft();

            // Set the textbox position at the X-axis:
            SwFormatHoriOrient aNewHOri(pFormat->GetHoriOrient());
            if (bIsGroupObj && aNewHOri.GetHoriOrient() != text::HoriOrientation::NONE)
                aNewHOri.SetHoriOrient(text::HoriOrientation::NONE);
            aNewHOri.SetPos(aRect.Left() + nLeftSpace
                            + (bIsGroupObj ? pObj->GetRelativePos().getX() : 0));
            SwFormatVertOrient aNewVOri(pFormat->GetVertOrient());

            // Special handling of group textboxes
            if (bIsGroupObj)
            {
                // There are the following cases:
                // case 1: The textbox should be in that position where the shape is.
                // case 2: The shape has negative offset so that have to be subtracted
                // case 3: The shape and its parent shape also has negative offset, so subtract
                aNewVOri.SetPos(
                    ((pObj->GetRelativePos().getY()) > 0
                         ? (pShape->GetVertOrient().GetPos() > 0
                                ? pObj->GetRelativePos().getY()
                                : pObj->GetRelativePos().getY() - pShape->GetVertOrient().GetPos())
                         : (pShape->GetVertOrient().GetPos() > 0
                                ? 0 // Is this can be a variation?
                                : pObj->GetRelativePos().getY() - pShape->GetVertOrient().GetPos()))
                    + aRect.Top());
            }
            else
            {
                // Simple textboxes: vertical position equals to the vertical offset of the shape
                aNewVOri.SetPos(
                    ((pShape->GetVertOrient().GetPos()) > 0 ? pShape->GetVertOrient().GetPos() : 0)
                    + aRect.Top());
            }

            // Special cases when the shape is aligned to the line
            if (pShape->GetVertOrient().GetVertOrient() != text::VertOrientation::NONE)
            {
                aNewVOri.SetVertOrient(text::VertOrientation::NONE);
                switch (pShape->GetVertOrient().GetVertOrient())
                {
                    // Top aligned shape
                    case text::VertOrientation::TOP:
                    case text::VertOrientation::CHAR_TOP:
                    case text::VertOrientation::LINE_TOP:
                    {
                        aNewVOri.SetPos(aNewVOri.GetPos() - pShape->GetFrameSize().GetHeight());
                        break;
                    }
                    // Bottom aligned shape
                    case text::VertOrientation::BOTTOM:
                    case text::VertOrientation::CHAR_BOTTOM:
                    case text::VertOrientation::LINE_BOTTOM:
                    {
                        aNewVOri.SetPos(aNewVOri.GetPos() + pShape->GetFrameSize().GetHeight());
                        break;
                    }
                    // Center aligned shape
                    case text::VertOrientation::CENTER:
                    case text::VertOrientation::CHAR_CENTER:
                    case text::VertOrientation::LINE_CENTER:
                    {
                        aNewVOri.SetPos(aNewVOri.GetPos()
                                        + std::lroundf(pShape->GetFrameSize().GetHeight() / 2));
                        break;
                    }
                    default:
                        break;
                }
            }

            pFormat->SetFormatAttr(aNewHOri);
            pFormat->SetFormatAttr(aNewVOri);
        }
        // Other cases when the shape has different anchor from AS_CHAR
        else
        {
            // Text area of the shape
            tools::Rectangle aRect
                = getRelativeTextRectangle(pObj ? pObj : pShape->FindRealSdrObject());

            // X Offset of the shape spacing
            auto nLeftSpace = pShape->GetLRSpace().GetLeft();

            // Set the same position as the (child) shape has
            SwFormatHoriOrient aNewHOri(pShape->GetHoriOrient());
            if (bIsGroupObj && aNewHOri.GetHoriOrient() != text::HoriOrientation::NONE)
                aNewHOri.SetHoriOrient(text::HoriOrientation::NONE);

            aNewHOri.SetPos(
                (bIsGroupObj && pObj ? pObj->GetRelativePos().getX() : aNewHOri.GetPos())
                + aRect.Left());
            SwFormatVertOrient aNewVOri(pShape->GetVertOrient());
            aNewVOri.SetPos(
                (bIsGroupObj && pObj ? pObj->GetRelativePos().getY() : aNewVOri.GetPos())
                + aRect.Top());

            // Get the distance of the child shape inside its parent
            const auto& nInshapePos
                = pObj ? pObj->GetRelativePos() - pShape->FindRealSdrObject()->GetRelativePos()
                       : Point();

            // Special case: the shape has relative position from the page
            if (pShape->GetHoriOrient().GetRelationOrient() == text::RelOrientation::PAGE_FRAME
                && pShape->GetAnchor().GetAnchorId() != RndStdIds::FLY_AT_PAGE)
            {
                aNewHOri.SetRelationOrient(text::RelOrientation::PAGE_FRAME);
                aNewHOri.SetPos(pShape->GetHoriOrient().GetPos() + nInshapePos.getX()
                                + aRect.Left());
            }

            if (pShape->GetVertOrient().GetRelationOrient() == text::RelOrientation::PAGE_FRAME
                && pShape->GetAnchor().GetAnchorId() != RndStdIds::FLY_AT_PAGE)
            {
                aNewVOri.SetRelationOrient(text::RelOrientation::PAGE_FRAME);
                aNewVOri.SetPos(pShape->GetVertOrient().GetPos() + nInshapePos.getY()
                                + aRect.Top());
            }

            // Other special case: shape is inside a table or floating table following the text flow
            if (pShape->GetFollowTextFlow().GetValue() && pShape->GetAnchor().GetAnchorNode()
                && pShape->GetAnchor().GetAnchorNode()->FindTableNode())
            {
                // Table position
                Point nTableOffset;
                // Floating table
                if (auto pFly
                    = pShape->GetAnchor().GetAnchorNode()->FindTableNode()->FindFlyStartNode())
                {
                    if (auto pFlyFormat = pFly->GetFlyFormat())
                    {
                        nTableOffset.setX(pFlyFormat->GetHoriOrient().GetPos());
                        nTableOffset.setY(pFlyFormat->GetVertOrient().GetPos());
                    }
                }
                else
                // Normal table
                {
                    auto pTableNode = pShape->GetAnchor().GetAnchorNode()->FindTableNode();
                    if (auto pTableFormat = pTableNode->GetTable().GetFrameFormat())
                    {
                        nTableOffset.setX(pTableFormat->GetHoriOrient().GetPos());
                        nTableOffset.setY(pTableFormat->GetVertOrient().GetPos());
                    }
                }

                // Add the table positions to the textbox.
                aNewHOri.SetPos(aNewHOri.GetPos() + nTableOffset.getX() + nLeftSpace);
                if (pShape->GetVertOrient().GetRelationOrient() == text::RelOrientation::PAGE_FRAME
                    || pShape->GetVertOrient().GetRelationOrient()
                           == text::RelOrientation::PAGE_PRINT_AREA)
                    aNewVOri.SetPos(aNewVOri.GetPos() + nTableOffset.getY());
            }

            pFormat->SetFormatAttr(aNewHOri);
            pFormat->SetFormatAttr(aNewVOri);
        }
        return true;
    }

    return false;
}

bool SwTextBoxHelper::syncTextBoxSize(SwFrameFormat* pShape, SdrObject* pObj)
{
    if (!pShape || !pObj)
        return false;

    if (auto pTextBox = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj))
    {
        auto aGuard = SwTextBoxLockGuard(*pShape->GetOtherTextBoxFormats());
        const auto& rSize = getRelativeTextRectangle(pObj).GetSize();
        if (!rSize.IsEmpty())
        {
            SwFormatFrameSize aSize(pTextBox->GetFrameSize());
            aSize.SetSize(rSize);
            return pTextBox->SetFormatAttr(aSize);
        }
    }

    return false;
}

bool SwTextBoxHelper::DoTextBoxZOrderCorrection(SwFrameFormat* pShape, const SdrObject* pObj)
{
    // TODO: do this with group shape textboxes.
    SdrObject* pShpObj = nullptr;

    pShpObj = pShape->FindRealSdrObject();

    if (pShpObj)
    {
        auto pTextBox = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT, pObj);
        if (!pTextBox)
            return false;
        SdrObject* pFrmObj = pTextBox->FindRealSdrObject();
        if (!pFrmObj)
        {
            // During loading there is no ready SdrObj for z-ordering, so create and cache it here
            pFrmObj
                = SwXTextFrame::GetOrCreateSdrObject(*dynamic_cast<SwFlyFrameFormat*>(pTextBox));
        }
        if (pFrmObj)
        {
            // Get the draw model from the doc
            SwDrawModel* pDrawModel
                = pShape->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
            if (pDrawModel)
            {
                // Not really sure this will work on all pages, but it seems it will.
                auto pPage = pDrawModel->GetPage(0);
                // Recalc all Z-orders
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
                                                      SwFrameFormat* pFormat, SdrObject* pObj)
{
    if (auto pChildren = pObj->getChildrenOfSdrObject())
    {
        for (const rtl::Reference<SdrObject>& pChildObj : *pChildren)
            synchronizeGroupTextBoxProperty(pFunc, pFormat, pChildObj.get());
    }
    else
    {
        (*pFunc)(pFormat, pObj);
    }
}

std::vector<SwFrameFormat*> SwTextBoxHelper::CollectTextBoxes(const SdrObject* pGroupObject,
                                                              SwFrameFormat* pFormat)
{
    std::vector<SwFrameFormat*> vRet;
    if (auto pChildren = pGroupObject->getChildrenOfSdrObject())
    {
        for (const rtl::Reference<SdrObject>& pObj : *pChildren)
        {
            auto pChildTextBoxes = CollectTextBoxes(pObj.get(), pFormat);
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

bool SwTextBoxHelper::isAnchorSyncNeeded(const SwFrameFormat* pFirst, const SwFrameFormat* pSecond)
{
    if (!pFirst)
        return false;

    if (!pSecond)
        return false;

    if (pFirst == pSecond)
        return false;

    if (!pFirst->GetOtherTextBoxFormats())
        return false;

    if (!pSecond->GetOtherTextBoxFormats())
        return false;

    if (pFirst->GetOtherTextBoxFormats() != pSecond->GetOtherTextBoxFormats())
        return false;

    if (pFirst->GetOtherTextBoxFormats()->GetOwnerShape() == pSecond
        || pFirst == pSecond->GetOtherTextBoxFormats()->GetOwnerShape())
    {
        const auto& rShapeAnchor
            = pFirst->Which() == RES_DRAWFRMFMT ? pFirst->GetAnchor() : pSecond->GetAnchor();
        const auto& rFrameAnchor
            = pFirst->Which() == RES_FLYFRMFMT ? pFirst->GetAnchor() : pSecond->GetAnchor();

        if (rShapeAnchor.GetAnchorId() == rFrameAnchor.GetAnchorId())
        {
            if (rShapeAnchor.GetAnchorNode() && rFrameAnchor.GetAnchorNode())
            {
                if (*rShapeAnchor.GetContentAnchor() != *rFrameAnchor.GetContentAnchor())
                    return true;

                return false;
            }

            if (rShapeAnchor.GetAnchorId() == RndStdIds::FLY_AT_PAGE
                && rFrameAnchor.GetAnchorId() == RndStdIds::FLY_AT_PAGE)
            {
                if (rShapeAnchor.GetPageNum() == rFrameAnchor.GetPageNum())
                    return false;
                else
                    return true;
            }

            return true;
        }

        if (rShapeAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR
            && rFrameAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR)
        {
            if (rShapeAnchor.GetAnchorNode() && rFrameAnchor.GetAnchorNode())
            {
                if (*rShapeAnchor.GetContentAnchor() != *rFrameAnchor.GetContentAnchor())
                    return true;

                return false;
            }
        }
        return true;
    }
    return false;
}

SwTextBoxNode::SwTextBoxNode(SwFrameFormat* pOwnerShape)
{
    assert(pOwnerShape);
    assert(pOwnerShape->Which() == RES_DRAWFRMFMT);

    m_bIsCloningInProgress = false;
    m_bLock = false;

    m_pOwnerShapeFormat = pOwnerShape;
    if (!m_pTextBoxes.empty())
        m_pTextBoxes.clear();
}

SwTextBoxNode::~SwTextBoxNode()
{
    if (!m_pTextBoxes.empty())
    {
        SAL_WARN("sw.core", "SwTextBoxNode::~SwTextBoxNode(): Text-Box-Vector still not empty!");
        assert(false);
    }
}

void SwTextBoxNode::AddTextBox(SdrObject* pDrawObject, SwFrameFormat* pNewTextBox)
{
    assert(pNewTextBox);
    assert(pNewTextBox->Which() == RES_FLYFRMFMT);

    assert(pDrawObject);

    SwTextBoxElement aElem;
    aElem.m_pDrawObject = pDrawObject;
    aElem.m_pTextBoxFormat = pNewTextBox;

    for (const auto& rE : m_pTextBoxes)
    {
        if (rE.m_pDrawObject == pDrawObject || rE.m_pTextBoxFormat == pNewTextBox)
        {
            SAL_WARN("sw.core", "SwTextBoxNode::AddTextBox(): Already exist!");
            return;
        }
    }

    auto pSwFlyDraw = dynamic_cast<SwFlyDrawObj*>(pDrawObject);
    if (pSwFlyDraw)
    {
        pSwFlyDraw->SetTextBox(true);
    }
    m_pTextBoxes.push_back(aElem);
}

void SwTextBoxNode::DelTextBox(const SdrObject* pDrawObject, bool bDelFromDoc)
{
    assert(pDrawObject);
    if (m_pTextBoxes.empty())
        return;

    for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end();)
    {
        if (it->m_pDrawObject == pDrawObject)
        {
            if (bDelFromDoc)
            {
                it->m_pTextBoxFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(
                    it->m_pTextBoxFormat);
                // What about m_pTextBoxes? So, when the DelLayoutFormat() removes the format
                // then the ~SwFrameFormat() will call this method again to remove the entry.
                return;
            }
            else
            {
                it = m_pTextBoxes.erase(it);
                return;
            }
        }
        ++it;
    }

    SAL_WARN("sw.core", "SwTextBoxNode::DelTextBox(): Not found!");
}

void SwTextBoxNode::DelTextBox(const SwFrameFormat* pTextBox, bool bDelFromDoc)
{
    if (m_pTextBoxes.empty())
        return;

    for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end();)
    {
        if (it->m_pTextBoxFormat == pTextBox)
        {
            if (bDelFromDoc)
            {
                it->m_pTextBoxFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(
                    it->m_pTextBoxFormat);
                // What about m_pTextBoxes? So, when the DelLayoutFormat() removes the format
                // then the ~SwFrameFormat() will call this method again to remove the entry.
                return;
            }
            else
            {
                it = m_pTextBoxes.erase(it);
                return;
            }
        }
        ++it;
    }

    SAL_WARN("sw.core", "SwTextBoxNode::DelTextBox(): Not found!");
}

SwFrameFormat* SwTextBoxNode::GetTextBox(const SdrObject* pDrawObject) const
{
    assert(pDrawObject);
    assert(m_pOwnerShapeFormat);

    if (auto& pTextBoxes = m_pOwnerShapeFormat->GetOtherTextBoxFormats())
    {
        if (size_t(pTextBoxes.use_count()) != pTextBoxes->GetTextBoxCount() + size_t(1))
        {
            SAL_WARN("sw.core", "SwTextBoxNode::GetTextBox(): RefCount and TexBox count mismatch!");
        }
    }

    if (m_bLock)
        return nullptr;

    if (!m_pTextBoxes.empty())
    {
        for (auto it = m_pTextBoxes.begin(); it != m_pTextBoxes.end(); it++)
        {
            if (it->m_pDrawObject == pDrawObject)
            {
                return it->m_pTextBoxFormat;
            }
        }
        SAL_WARN("sw.core", "SwTextBoxNode::GetTextBox(): Not found!");
    }

    return nullptr;
}

void SwTextBoxNode::ClearAll()
{
    // If this called from ~SwDoc(), then only the address entries
    // have to be removed, the format will be deleted by the
    // the mpSpzFrameFormatTable->DeleteAndDestroyAll() in ~SwDoc()!
    if (m_pOwnerShapeFormat->GetDoc()->IsInDtor())
    {
        m_pTextBoxes.clear();
        return;
    }

    // For loop control
    sal_uInt16 nLoopCount = 0;

    // Reference not enough, copy needed.
    const size_t nTextBoxCount = m_pTextBoxes.size();

    // For loop has problems: When one entry deleted, the iterator has
    // to be refreshed according to the new situation. So using While() instead.
    while (!m_pTextBoxes.empty())
    {
        // Delete the last textbox of the vector from the doc
        // (what will call deregister in ~SwFrameFormat()
        m_pOwnerShapeFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(
            m_pTextBoxes.back().m_pTextBoxFormat);

        // Check if we are looping
        if (nLoopCount > (nTextBoxCount + 1))
        {
            SAL_WARN("sw.core", "SwTextBoxNode::ClearAll(): Maximum loop count reached!");
            break;
        }
        else
        {
            nLoopCount++;
        }
    }

    // Ensure the vector is empty.
    if (!m_pTextBoxes.empty())
    {
        SAL_WARN("sw.core", "SwTextBoxNode::ClearAll(): Text-Box-Vector still not empty!");
        assert(false);
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

void SwTextBoxNode::Clone(SwDoc* pDoc, const SwFormatAnchor& rNewAnc, SwFrameFormat* o_pTarget,
                          bool bSetAttr, bool bMakeFrame) const
{
    if (!o_pTarget || !pDoc)
        return;

    if (o_pTarget->Which() != RES_DRAWFRMFMT)
        return;

    if (m_bIsCloningInProgress)
        return;

    m_bIsCloningInProgress = true;

    Clone_Impl(pDoc, rNewAnc, o_pTarget, m_pOwnerShapeFormat->FindSdrObject(),
               o_pTarget->FindSdrObject(), bSetAttr, bMakeFrame);

    m_bIsCloningInProgress = false;

    for (auto& rElem : m_pTextBoxes)
    {
        SwTextBoxHelper::changeAnchor(m_pOwnerShapeFormat, rElem.m_pDrawObject);
        SwTextBoxHelper::doTextBoxPositioning(m_pOwnerShapeFormat, rElem.m_pDrawObject);
        SwTextBoxHelper::DoTextBoxZOrderCorrection(m_pOwnerShapeFormat, rElem.m_pDrawObject);
        SwTextBoxHelper::syncTextBoxSize(m_pOwnerShapeFormat, rElem.m_pDrawObject);
    }
}

void SwTextBoxNode::Clone_Impl(SwDoc* pDoc, const SwFormatAnchor& rNewAnc, SwFrameFormat* o_pTarget,
                               const SdrObject* pSrcObj, SdrObject* pDestObj, bool bSetAttr,
                               bool bMakeFrame) const
{
    if (!pSrcObj || !pDestObj)
        return;

    auto pSrcList = pSrcObj->getChildrenOfSdrObject();
    auto pDestList = pDestObj->getChildrenOfSdrObject();

    if (pSrcList && pDestList)
    {
        if (pSrcList->GetObjCount() != pDestList->GetObjCount())
        {
            SAL_WARN("sw.core", "SwTextBoxNode::Clone_Impl(): Difference between the shapes!");
            return;
        }

        for (auto itSrc = pSrcList->begin(), itDest = pDestList->begin(); itSrc != pSrcList->end();
             ++itSrc, ++itDest)
        {
            Clone_Impl(pDoc, rNewAnc, o_pTarget, itSrc->get(), itDest->get(), bSetAttr, bMakeFrame);
        }
        return;
    }

    if (!pSrcList && !pDestList)
    {
        if (auto pSrcFormat = GetTextBox(pSrcObj))
        {
            SwFormatAnchor aNewAnchor(rNewAnc);
            if (aNewAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
            {
                aNewAnchor.SetType(RndStdIds::FLY_AT_CHAR);

                if (!bMakeFrame)
                    bMakeFrame = true;
            }

            if (auto pTargetFormat = pDoc->getIDocumentLayoutAccess().CopyLayoutFormat(
                    *pSrcFormat, aNewAnchor, bSetAttr, bMakeFrame))
            {
                if (!o_pTarget->GetOtherTextBoxFormats())
                {
                    auto pNewTextBoxes = std::make_shared<SwTextBoxNode>(SwTextBoxNode(o_pTarget));
                    o_pTarget->SetOtherTextBoxFormats(pNewTextBoxes);
                    pNewTextBoxes->AddTextBox(pDestObj, pTargetFormat);
                    pTargetFormat->SetOtherTextBoxFormats(pNewTextBoxes);
                }
                else
                {
                    o_pTarget->GetOtherTextBoxFormats()->AddTextBox(pDestObj, pTargetFormat);
                    pTargetFormat->SetOtherTextBoxFormats(o_pTarget->GetOtherTextBoxFormats());
                }
                o_pTarget->SetFormatAttr(pTargetFormat->GetContent());
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
