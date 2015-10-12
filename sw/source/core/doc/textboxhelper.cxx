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
#include <unoprnms.hxx>
#include <dflyobj.hxx>
#include <mvsave.hxx>
#include <sortedobjs.hxx>
#include <cntfrm.hxx>

#include <editeng/unoprnms.hxx>
#include <editeng/charrotateitem.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>
#include <svl/itemiter.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace com::sun::star;

void SwTextBoxHelper::create(SwFrameFormat* pShape)
{
    // If TextBox wasn't enabled previously
    if (!pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        // Create the associated TextFrame and insert it into the document.
        uno::Reference<text::XTextContent> xTextFrame(SwXServiceProvider::MakeInstance(SW_SERVICE_TYPE_TEXTFRAME, *pShape->GetDoc()), uno::UNO_QUERY);
        uno::Reference<text::XTextDocument> xTextDocument(pShape->GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
        uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(), uno::UNO_QUERY);
        xTextContentAppend->appendTextContent(xTextFrame, uno::Sequence<beans::PropertyValue>());

        // Initialize properties.
        uno::Reference<beans::XPropertySet> xPropertySet(xTextFrame, uno::UNO_QUERY);
        uno::Any aEmptyBorder = uno::makeAny(table::BorderLine2());
        xPropertySet->setPropertyValue(UNO_NAME_TOP_BORDER, aEmptyBorder);
        xPropertySet->setPropertyValue(UNO_NAME_BOTTOM_BORDER, aEmptyBorder);
        xPropertySet->setPropertyValue(UNO_NAME_LEFT_BORDER, aEmptyBorder);
        xPropertySet->setPropertyValue(UNO_NAME_RIGHT_BORDER, aEmptyBorder);

        xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::makeAny(sal_Int32(100)));

        xPropertySet->setPropertyValue(UNO_NAME_SIZE_TYPE, uno::makeAny(text::SizeType::FIX));

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
        uno::Reference<drawing::XShape> xShape(pShape->FindRealSdrObject()->getUnoShape(), uno::UNO_QUERY);
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::makeAny(xShape->getSize()));

        uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
        syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_ORIENT, xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT));
        syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_RELATION, xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_RELATION));
        syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_ORIENT, xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT));
        syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_RELATION, xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_RELATION));
        syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_POSITION, xShapePropertySet->getPropertyValue(UNO_NAME_HORI_ORIENT_POSITION));
        syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_POSITION, xShapePropertySet->getPropertyValue(UNO_NAME_VERT_ORIENT_POSITION));
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT, xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT));
        syncProperty(pShape, RES_TEXT_VERT_ADJUST, 0, xShapePropertySet->getPropertyValue(UNO_NAME_TEXT_VERT_ADJUST));
    }
}

void SwTextBoxHelper::destroy(SwFrameFormat* pShape)
{
    // If a TextBox was enabled previously
    if (pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        SwFrameFormat* pFormat = findTextBox(pShape);

        // Unlink the TextBox's text range from the original shape.
        pShape->ResetFormatAttr(RES_CNTNT);

        // Delete the associated TextFrame.
        if (pFormat)
            pShape->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(pFormat);
    }
}

std::set<const SwFrameFormat*> SwTextBoxHelper::findTextBoxes(const SwDoc* pDoc)
{
    std::set<const SwFrameFormat*> aTextBoxes;
    std::map<SwNodeIndex, const SwFrameFormat*> aFlyFormats, aDrawFormats;

    const SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    for (SwFrameFormats::const_iterator it = rSpzFrameFormats.begin(); it != rSpzFrameFormats.end(); ++it)
    {
        const SwFrameFormat* pFormat = *it;

        // A TextBox in the context of this class is a fly frame that has a
        // matching (same RES_CNTNT) draw frame.
        if (!pFormat->GetAttrSet().HasItem(RES_CNTNT) || !pFormat->GetContent().GetContentIdx())
            continue;

        const SwNodeIndex& rIndex = *pFormat->GetContent().GetContentIdx();

        if (pFormat->Which() == RES_FLYFRMFMT)
        {
            if (aDrawFormats.find(rIndex) != aDrawFormats.end())
                aTextBoxes.insert(pFormat);
            else
                aFlyFormats[rIndex] = pFormat;
        }
        else if (pFormat->Which() == RES_DRAWFRMFMT)
        {
            if (aFlyFormats.find(rIndex) != aFlyFormats.end())
                aTextBoxes.insert(aFlyFormats[rIndex]);
            else
                aDrawFormats[rIndex] = pFormat;
        }
    }

    return aTextBoxes;
}

std::set<const SwFrameFormat*> SwTextBoxHelper::findTextBoxes(const SwNode& rNode)
{
    const SwDoc* pDoc = rNode.GetDoc();
    const SwContentNode* pContentNode = 0;
    const SwContentFrm* pContentFrm = 0;
    bool bHaveViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    if (bHaveViewShell && (pContentNode = rNode.GetContentNode()) && (pContentFrm = pContentNode->getLayoutFrm(pDoc->getIDocumentLayoutAccess().GetCurrentLayout())))
    {
        // We can use the layout information to iterate over only the frames which are anchored to us.
        std::set<const SwFrameFormat*> aRet;
        const SwSortedObjs* pSortedObjs = pContentFrm->GetDrawObjs();
        if (pSortedObjs)
        {
            for (size_t i = 0; i < pSortedObjs->size(); ++i)
            {
                SwAnchoredObject* pAnchoredObject = (*pSortedObjs)[i];
                SwFrameFormat* pTextBox = findTextBox(&pAnchoredObject->GetFrameFormat());
                if (pTextBox)
                    aRet.insert(pTextBox);
            }
        }
        return aRet;
    }
    else
        // If necessary, here we could manually limit the returned set to the
        // ones which are anchored to rNode, but currently no need to do so.
        return findTextBoxes(pDoc);
}

std::map<SwFrameFormat*, SwFrameFormat*> SwTextBoxHelper::findShapes(const SwDoc* pDoc)
{
    std::map<SwFrameFormat*, SwFrameFormat*> aRet;

    const SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    for (SwFrameFormats::const_iterator it = rSpzFrameFormats.begin(); it != rSpzFrameFormats.end(); ++it)
    {
        SwFrameFormat* pTextBox = findTextBox(*it);
        if (pTextBox)
            aRet[pTextBox] = *it;
    }

    return aRet;
}

/// If the passed SdrObject is in fact a TextFrame, that is used as a TextBox.
bool lcl_isTextBox(SdrObject* pSdrObject, std::set<const SwFrameFormat*>& rTextBoxes)
{
    SwVirtFlyDrawObj* pObject = dynamic_cast<SwVirtFlyDrawObj*>(pSdrObject);
    return pObject && rTextBoxes.find(pObject->GetFormat()) != rTextBoxes.end();
}

bool SwTextBoxHelper::isTextBox(const SdrObject* pObject)
{
    const SwVirtFlyDrawObj* pVirtFlyDrawObj = dynamic_cast<const SwVirtFlyDrawObj*>(pObject);
    if (!pVirtFlyDrawObj)
        return false;
    std::set<const SwFrameFormat*> aTextBoxes = findTextBoxes(pVirtFlyDrawObj->GetFormat()->GetDoc());
    return aTextBoxes.find(pVirtFlyDrawObj->GetFormat()) != aTextBoxes.end();
}

sal_Int32 SwTextBoxHelper::getCount(SdrPage* pPage, std::set<const SwFrameFormat*>& rTextBoxes)
{
    sal_Int32 nRet = 0;
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        if (lcl_isTextBox(pPage->GetObj(i), rTextBoxes))
            continue;
        ++nRet;
    }
    return nRet;
}

uno::Any SwTextBoxHelper::getByIndex(SdrPage* pPage, sal_Int32 nIndex, std::set<const SwFrameFormat*>& rTextBoxes) throw(lang::IndexOutOfBoundsException)
{
    if (nIndex < 0)
        throw lang::IndexOutOfBoundsException();

    SdrObject* pRet = 0;
    sal_Int32 nCount = 0; // Current logical index.
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        if (lcl_isTextBox(pPage->GetObj(i), rTextBoxes))
            continue;
        if (nCount == nIndex)
        {
            pRet = pPage->GetObj(i);
            break;
        }
        ++nCount;
    }

    if (!pRet)
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(uno::Reference<drawing::XShape>(pRet->getUnoShape(), uno::UNO_QUERY));
}

sal_Int32 SwTextBoxHelper::getOrdNum(const SdrObject* pObject, std::set<const SwFrameFormat*>& rTextBoxes)
{
    if (const SdrPage* pPage = pObject->GetPage())
    {
        sal_Int32 nOrder = 0; // Current logical order.
        for (size_t i = 0; i < pPage->GetObjCount(); ++i)
        {
            if (lcl_isTextBox(pPage->GetObj(i), rTextBoxes))
                continue;
            if (pPage->GetObj(i) == pObject)
                return nOrder;
            ++nOrder;
        }
    }

    SAL_WARN("sw.core", "SwTextBoxHelper::getOrdNum: no page or page doesn't contain the object");
    return pObject->GetOrdNum();
}

SwFrameFormat* SwTextBoxHelper::findTextBox(uno::Reference<drawing::XShape> xShape)
{
    SwXShape* pShape = dynamic_cast<SwXShape*>(xShape.get());
    if (!pShape)
        return 0;

    return findTextBox(pShape->GetFrameFormat());
}

SwFrameFormat* SwTextBoxHelper::findTextBox(const SwFrameFormat* pShape)
{
    SwFrameFormat* pRet = 0;

    // Only draw frames can have TextBoxes.
    if (pShape && pShape->Which() == RES_DRAWFRMFMT && pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        const SwFormatContent& rContent = pShape->GetContent();
        const SwFrameFormats& rSpzFrameFormats = *pShape->GetDoc()->GetSpzFrameFormats();
        for (SwFrameFormats::const_iterator it = rSpzFrameFormats.begin(); it != rSpzFrameFormats.end(); ++it)
        {
            SwFrameFormat* pFormat = *it;
            // Only a fly frame can be a TextBox.
            if (pFormat->Which() == RES_FLYFRMFMT && pFormat->GetAttrSet().HasItem(RES_CNTNT) && pFormat->GetContent() == rContent)
            {
                pRet = pFormat;
                break;
            }
        }
    }

    return pRet;
}

template < typename T >
void lcl_queryInterface(SwFrameFormat* pShape, uno::Any& rAny)
{
    if (SwFrameFormat* pFormat = SwTextBoxHelper::findTextBox(pShape))
    {
        uno::Reference<T> const xInterface(
            SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat),
            uno::UNO_QUERY);
        rAny <<= xInterface;
    }
}

uno::Any SwTextBoxHelper::queryInterface(SwFrameFormat* pShape, const uno::Type& rType)
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

Rectangle SwTextBoxHelper::getTextRectangle(SwFrameFormat* pShape, bool bAbsolute)
{
    Rectangle aRet;
    aRet.SetEmpty();
    SdrObjCustomShape* pCustomShape = dynamic_cast<SdrObjCustomShape*>(pShape->FindRealSdrObject());
    if (pCustomShape)
    {
        // Need to temporarily release the lock acquired in
        // SdXMLShapeContext::AddShape(), otherwise we get an empty rectangle,
        // see EnhancedCustomShapeEngine::getTextBounds().
        uno::Reference<document::XActionLockable> xLockable(pCustomShape->getUnoShape(), uno::UNO_QUERY);
        sal_Int16 nLocks = 0;
        if (xLockable.is())
            nLocks = xLockable->resetActionLocks();
        pCustomShape->GetTextBounds(aRet);
        if (nLocks)
            xLockable->setActionLocks(nLocks);
    }

    if (!bAbsolute && pCustomShape)
    {
        // Relative, so count the logic (reference) rectangle, see the EnhancedCustomShape2d ctor.
        Point aPoint(pCustomShape->GetSnapRect().Center());
        Size aSize(pCustomShape->GetLogicRect().GetSize());
        aPoint.X() -= aSize.Width() / 2;
        aPoint.Y() -= aSize.Height() / 2;
        Rectangle aLogicRect(aPoint, aSize);
        aRet.Move(-1 * aLogicRect.Left(), -1 * aLogicRect.Top());
    }

    return aRet;
}

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, const OUString& rPropertyName, const css::uno::Any& rValue)
{
    if (rPropertyName == "CustomShapeGeometry")
    {
        // CustomShapeGeometry changes the textbox position offset and size, so adjust both.
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::Any());

        SdrObject* pObject = pShape->FindRealSdrObject();
        if (pObject)
        {
            Rectangle aRectangle(pObject->GetSnapRect());
            syncProperty(pShape, RES_HORI_ORIENT, MID_HORIORIENT_POSITION, uno::makeAny(static_cast<sal_Int32>(convertTwipToMm100(aRectangle.Left()))));
            syncProperty(pShape, RES_VERT_ORIENT, MID_VERTORIENT_POSITION, uno::makeAny(static_cast<sal_Int32>(convertTwipToMm100(aRectangle.Top()))));
        }

        if (SwFrameFormat* pFormat = findTextBox(pShape))
        {
            comphelper::SequenceAsHashMap aCustomShapeGeometry(rValue);
            // That would be the btLr text direction which we don't support at a frame level, so do it at a character level.
            if (aCustomShapeGeometry.find("TextPreRotateAngle") != aCustomShapeGeometry.end() && aCustomShapeGeometry["TextPreRotateAngle"].get<sal_Int32>() == -270)
            {
                if (const SwNodeIndex* pNodeIndex = pFormat->GetContent().GetContentIdx())
                {
                    SwPaM aPaM(*pFormat->GetDoc()->GetNodes()[pNodeIndex->GetIndex() + 1], 0);
                    aPaM.SetMark();
                    if (SwTextNode* pMark = pFormat->GetDoc()->GetNodes()[pNodeIndex->GetNode().EndOfSectionIndex() - 1]->GetTextNode())
                    {
                        aPaM.GetMark()->nNode = *pMark;
                        aPaM.GetMark()->nContent.Assign(pMark, pMark->GetText().getLength());
                        SvxCharRotateItem aItem(900, false, RES_CHRATR_ROTATE);
                        pFormat->GetDoc()->getIDocumentContentOperations().InsertPoolItem(aPaM, aItem);
                    }
                }
            }
        }
    }
    else if (rPropertyName == UNO_NAME_TEXT_VERT_ADJUST)
        syncProperty(pShape, RES_TEXT_VERT_ADJUST, 0, rValue);
    else if (rPropertyName == UNO_NAME_TEXT_AUTOGROWHEIGHT)
        syncProperty(pShape, RES_FRM_SIZE, MID_FRMSIZE_IS_AUTO_HEIGHT, rValue);
}

void SwTextBoxHelper::getProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberId, css::uno::Any& rValue)
{
    if (!pShape)
        return;

    nMemberId &= ~CONVERT_TWIPS;

    if (SwFrameFormat* pFormat = findTextBox(pShape))
    {
        if (nWID == RES_CHAIN)
        {
            switch (nMemberId)
            {
            case MID_CHAIN_PREVNAME:
            case MID_CHAIN_NEXTNAME:
            {
                const SwFormatChain& rChain = pFormat->GetChain();
                rChain.QueryValue(rValue, nMemberId);
            }
            break;
            case MID_CHAIN_NAME:
                rValue = uno::makeAny(pFormat->GetName());
                break;
            }
        }
    }
}

void SwTextBoxHelper::syncProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberId, const css::uno::Any& rValue)
{
    // No shape yet? Then nothing to do, initial properties are set by create().
    if (!pShape)
        return;

    uno::Any aValue(rValue);
    nMemberId &= ~CONVERT_TWIPS;

    if (SwFrameFormat* pFormat = findTextBox(pShape))
    {
        OUString aPropertyName;
        bool bAdjustX = false;
        bool bAdjustY = false;
        bool bAdjustSize = false;
        switch (nWID)
        {
        case RES_HORI_ORIENT:
            switch (nMemberId)
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
            switch (nMemberId)
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
            switch (nMemberId)
            {
            case MID_FRMSIZE_IS_AUTO_HEIGHT:
                aPropertyName = UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT;
                break;
            default:
                aPropertyName = UNO_NAME_SIZE;
                bAdjustSize = true;
                break;
            }
            break;
        case RES_ANCHOR:
            switch (nMemberId)
            {
            case MID_ANCHOR_ANCHORTYPE:
                if (aValue.get<text::TextContentAnchorType>() == text::TextContentAnchorType_AS_CHARACTER)
                {
                    uno::Reference<beans::XPropertySet> const xPropertySet(
                        SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat),
                        uno::UNO_QUERY);
                    xPropertySet->setPropertyValue(UNO_NAME_SURROUND, uno::makeAny(text::WrapTextMode_THROUGHT));
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
            switch (nMemberId)
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
        }

        if (!aPropertyName.isEmpty())
        {
            // Position/size should be the text position/size, not the shape one as-is.
            if (bAdjustX || bAdjustY || bAdjustSize)
            {
                Rectangle aRect = getTextRectangle(pShape, /*bAbsolute=*/false);
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
                SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat),
                uno::UNO_QUERY);
            xPropertySet->setPropertyValue(aPropertyName, aValue);
        }
    }
}

void SwTextBoxHelper::saveLinks(const SwFrameFormats& rFormats, std::map<const SwFrameFormat*, const SwFrameFormat*>& rLinks)
{
    for (size_t i = 0; i < rFormats.size(); ++i)
    {
        const SwFrameFormat* pFormat = rFormats[i];
        if (pFormat->Which() != RES_DRAWFRMFMT)
            continue;
        if (SwFrameFormat* pTextBox = findTextBox(pFormat))
            rLinks[pFormat] = pTextBox;
    }
}

void SwTextBoxHelper::resetLink(SwFrameFormat* pShape, std::map<const SwFrameFormat*, SwFormatContent>& rMap)
{
    if (pShape->Which() == RES_DRAWFRMFMT)
    {
        if (pShape->GetContent().GetContentIdx())
            rMap.insert(std::make_pair(pShape, pShape->GetContent()));
        pShape->ResetFormatAttr(RES_CNTNT);
    }
}

void SwTextBoxHelper::restoreLinks(std::set<_ZSortFly>& rOld, std::vector<SwFrameFormat*>& rNew, SavedLink& rSavedLinks, SavedContent& rOldContent)
{
    size_t i = 0;
    for (std::set<_ZSortFly>::iterator aSetIt = rOld.begin(); aSetIt != rOld.end(); ++aSetIt, ++i)
    {
        SavedLink::iterator aTextBoxIt = rSavedLinks.find(aSetIt->GetFormat());
        if (aTextBoxIt != rSavedLinks.end())
        {
            size_t j = 0;
            for (std::set<_ZSortFly>::iterator aSetJt = rOld.begin(); aSetJt != rOld.end(); ++aSetJt, ++j)
            {
                if (aSetJt->GetFormat() == aTextBoxIt->second)
                    rNew[i]->SetFormatAttr(rNew[j]->GetContent());
            }
        }
        if (rOldContent.find(aSetIt->GetFormat()) != rOldContent.end())
            const_cast<SwFrameFormat*>(aSetIt->GetFormat())->SetFormatAttr(rOldContent[aSetIt->GetFormat()]);
    }
}

void SwTextBoxHelper::syncFlyFrmAttr(SwFrameFormat& rShape, SfxItemSet& rSet)
{
    if (SwFrameFormat* pFormat = findTextBox(&rShape))
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
                const SwFormatVertOrient& rOrient = static_cast<const SwFormatVertOrient&>(*aIter.GetCurItem());
                SwFormatVertOrient aOrient(rOrient);

                Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
                if (!aRect.IsEmpty())
                    aOrient.SetPos(aOrient.GetPos() + aRect.getY());

                aTextBoxSet.Put(aOrient);

                // restore height (shrinked for extending beyond the page bottom - tdf#91260)
                SwFormatFrmSize aSize(pFormat->GetFrmSize());
                if (!aRect.IsEmpty())
                {
                    aSize.SetHeight(aRect.getHeight());
                    aTextBoxSet.Put(aSize);
                }
            }
            break;
            case RES_HORI_ORIENT:
            {
                const SwFormatHoriOrient& rOrient = static_cast<const SwFormatHoriOrient&>(*aIter.GetCurItem());
                SwFormatHoriOrient aOrient(rOrient);

                Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
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
                SwFormatFrmSize aSize(pFormat->GetFrmSize());

                Rectangle aRect = getTextRectangle(&rShape, /*bAbsolute=*/false);
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
                SAL_WARN("sw.core", "SwTextBoxHelper::syncFlyFrmAttr: unhandled which-id: " << nWhich);
                break;
            }

            if (aIter.IsAtEnd())
                break;
        }
        while (0 != (nWhich = aIter.NextItem()->Which()));

        if (aTextBoxSet.Count())
            pFormat->GetDoc()->SetFlyFrmAttr(*pFormat, aTextBoxSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
