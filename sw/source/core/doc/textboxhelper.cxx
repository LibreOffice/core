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
#include <ndtxt.hxx>
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

#include <editeng/unoprnms.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/memberids.h>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
#include <svl/itemiter.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <sal/log.hxx>

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

using namespace com::sun::star;

void SwTextBoxHelper::create(SwFrameFormat* pShape)
{
    // If TextBox wasn't enabled previously
    if (pShape->GetAttrSet().HasItem(RES_CNTNT))
        return;

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

sal_Int32 SwTextBoxHelper::getCount(const SwDoc* pDoc)
{
    sal_Int32 nRet = 0;
    const SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
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

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, const OUString& rPropertyName,
                                   const css::uno::Any& rValue)
{
    if (rPropertyName == "CustomShapeGeometry")
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

        if (SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
        {
            comphelper::SequenceAsHashMap aCustomShapeGeometry(rValue);
            // That would be the btLr text direction which we don't support at a frame level, so do it at a character level.
            if (aCustomShapeGeometry.find("TextPreRotateAngle") != aCustomShapeGeometry.end()
                && aCustomShapeGeometry["TextPreRotateAngle"].get<sal_Int32>() == -270)
            {
                if (const SwNodeIndex* pNodeIndex = pFormat->GetContent().GetContentIdx())
                {
                    SwPaM aPaM(*pFormat->GetDoc()->GetNodes()[pNodeIndex->GetIndex() + 1], 0);
                    aPaM.SetMark();
                    if (SwTextNode* pMark
                        = pFormat->GetDoc()
                              ->GetNodes()[pNodeIndex->GetNode().EndOfSectionIndex() - 1]
                              ->GetTextNode())
                    {
                        aPaM.GetMark()->nNode = *pMark;
                        aPaM.GetMark()->nContent.Assign(pMark, pMark->GetText().getLength());
                        SvxCharRotateItem aItem(900, false, RES_CHRATR_ROTATE);
                        pFormat->GetDoc()->getIDocumentContentOperations().InsertPoolItem(aPaM,
                                                                                          aItem);
                    }
                }
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
}

void SwTextBoxHelper::getProperty(SwFrameFormat const* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                                  css::uno::Any& rValue)
{
    if (!pShape)
        return;

    nMemberID &= ~CONVERT_TWIPS;

    if (SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
    {
        if (nWID == RES_CHAIN)
        {
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
            }
        }
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

    if (SwFrameFormat* pFormat = getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT))
    {
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
                }
                break;
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
                        if (aValue.get<text::TextContentAnchorType>()
                            == text::TextContentAnchorType_AS_CHARACTER)
                        {
                            uno::Reference<beans::XPropertySet> const xPropertySet(
                                SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat),
                                uno::UNO_QUERY);
                            xPropertySet->setPropertyValue(
                                UNO_NAME_SURROUND, uno::makeAny(text::WrapTextMode_THROUGH));
                            return;
                        }
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
                }
                break;
            case RES_OPAQUE:
                aPropertyName = UNO_NAME_OPAQUE;
                break;
        }

        if (!aPropertyName.isEmpty())
        {
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
                        awt::Size aSize(TWIPS_TO_MM(aRect.getWidth()),
                                        TWIPS_TO_MM(aRect.getHeight()));
                        aValue <<= aSize;
                    }
                }
            }

            uno::Reference<beans::XPropertySet> const xPropertySet(
                SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat), uno::UNO_QUERY);
            xPropertySet->setPropertyValue(aPropertyName, aValue);
        }
    }
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

void SwTextBoxHelper::resetLink(SwFrameFormat* pShape,
                                std::map<const SwFrameFormat*, SwFormatContent>& rOldContent)
{
    if (pShape->Which() == RES_DRAWFRMFMT)
    {
        if (pShape->GetContent().GetContentIdx())
            rOldContent.insert(std::make_pair(pShape, pShape->GetContent()));
        pShape->ResetFormatAttr(RES_CNTNT);
    }
}

void SwTextBoxHelper::restoreLinks(std::set<ZSortFly>& rOld, std::vector<SwFrameFormat*>& rNew,
                                   SavedLink& rSavedLinks, SavedContent& rResetContent)
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
        if (rResetContent.find(rIt.GetFormat()) != rResetContent.end())
            const_cast<SwFrameFormat*>(rIt.GetFormat())
                ->SetFormatAttr(rResetContent[rIt.GetFormat()]);
        ++i;
    }
}

void SwTextBoxHelper::syncFlyFrameAttr(SwFrameFormat& rShape, SfxItemSet const& rSet)
{
    if (SwFrameFormat* pFormat = getOtherTextBoxFormat(&rShape, RES_DRAWFRMFMT))
    {
        SfxItemSet aTextBoxSet(pFormat->GetDoc()->GetAttrPool(), aFrameFormatSetRange);

        SfxItemIter aIter(rSet);
        sal_uInt16 nWhich = aIter.GetCurItem()->Which();
        do
        {
            switch (nWhich)
            {
                case RES_VERT_ORIENT:
                {
                    auto& rOrient = static_cast<const SwFormatVertOrient&>(*aIter.GetCurItem());
                    SwFormatVertOrient aOrient(rOrient);

                    tools::Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
                    if (!aRect.IsEmpty())
                        aOrient.SetPos(aOrient.GetPos() + aRect.getY());

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
                    auto& rOrient = static_cast<const SwFormatHoriOrient&>(*aIter.GetCurItem());
                    SwFormatHoriOrient aOrient(rOrient);

                    tools::Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
                    if (!aRect.IsEmpty())
                        aOrient.SetPos(aOrient.GetPos() + aRect.getX());

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
                default:
                    SAL_WARN("sw.core",
                             "SwTextBoxHelper::syncFlyFrameAttr: unhandled which-id: " << nWhich);
                    break;
            }

            if (aIter.IsAtEnd())
                break;
        } while (0 != (nWhich = aIter.NextItem()->Which()));

        if (aTextBoxSet.Count())
            pFormat->GetDoc()->SetFlyFrameAttr(*pFormat, aTextBoxSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
