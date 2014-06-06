/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <textboxhelper.hxx>
#include <frmfmt.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <unodraw.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unomid.h>
#include <unoprnms.hxx>
#include <dflyobj.hxx>

#include <editeng/unoprnms.hxx>
#include <svx/svdoashp.hxx>
#include <svx/unopage.hxx>
#include <svx/svdpage.hxx>

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace com::sun::star;

void SwTextBoxHelper::create(SwFrmFmt* pShape)
{
    // If TextBox wasn't enabled previously
    if (!pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        // Create the associated TextFrame and insert it into the document.
        uno::Reference<text::XTextContent> xTextFrame(SwXServiceProvider::MakeInstance(SW_SERVICE_TYPE_TEXTFRAME, pShape->GetDoc()), uno::UNO_QUERY);
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

        // Link its text range to the original shape.
        uno::Reference<text::XTextRange> xTextBox(xTextFrame, uno::UNO_QUERY_THROW);
        SwUnoInternalPaM aInternalPaM(*pShape->GetDoc());
        if (sw::XTextRangeToSwPaM(aInternalPaM, xTextBox))
        {
            SwAttrSet aSet(pShape->GetAttrSet());
            SwFmtCntnt aCntnt(aInternalPaM.GetNode()->StartOfSectionNode());
            aSet.Put(aCntnt);
            pShape->SetFmtAttr(aSet);
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
    }
}

void SwTextBoxHelper::destroy(SwFrmFmt* pShape)
{
    // If a TextBox was enabled previously
    if (pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        SwFrmFmt* pFmt = findTextBox(pShape);

        // Unlink the TextBox's text range from the original shape.
        pShape->ResetFmtAttr(RES_CNTNT);

        // Delete the associated TextFrame.
        if (pFmt)
            pShape->GetDoc()->DelLayoutFmt(pFmt);
    }
}

std::list<SwFrmFmt*> SwTextBoxHelper::findTextBoxes(const SwDoc* pDoc)
{
    std::list<SwFrmFmt*> aRet;

    const SwFrmFmts& rSpzFrmFmts = *pDoc->GetSpzFrmFmts();
    for (SwFrmFmts::const_iterator it = rSpzFrmFmts.begin(); it != rSpzFrmFmts.end(); ++it)
    {
        SwFrmFmt* pTextBox = findTextBox(*it);
        if (pTextBox)
            aRet.push_back(pTextBox);
    }

    return aRet;
}

/// If the passed SdrObject is in fact a TextFrame, that is used as a TextBox.
bool lcl_isTextBox(SdrObject* pSdrObject, std::list<SwFrmFmt*>& rTextBoxes)
{
    SwVirtFlyDrawObj* pObject = PTR_CAST(SwVirtFlyDrawObj, pSdrObject);
    return pObject && std::find(rTextBoxes.begin(), rTextBoxes.end(), pObject->GetFmt()) != rTextBoxes.end();
}

sal_Int32 SwTextBoxHelper::getCount(SdrPage* pPage, std::list<SwFrmFmt*>& rTextBoxes)
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

uno::Any SwTextBoxHelper::getByIndex(SdrPage* pPage, sal_Int32 nIndex, std::list<SwFrmFmt*>& rTextBoxes) throw(lang::IndexOutOfBoundsException)
{
    if (nIndex < 0 || nIndex >= getCount(pPage, rTextBoxes))
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
    assert(pRet);
    return pRet ? uno::makeAny(uno::Reference<drawing::XShape>(pRet->getUnoShape(), uno::UNO_QUERY)) : uno::Any();
}

SwFrmFmt* SwTextBoxHelper::findTextBox(uno::Reference<drawing::XShape> xShape)
{
    SwXShape* pShape = dynamic_cast<SwXShape*>(xShape.get());
    if (!pShape)
        return 0;

    return findTextBox(pShape->GetFrmFmt());
}

SwFrmFmt* SwTextBoxHelper::findTextBox(SwFrmFmt* pShape)
{
    SwFrmFmt* pRet = 0;

    // Only draw frames can have TextBoxes.
    if (pShape->Which() == RES_DRAWFRMFMT && pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        const SwFmtCntnt& rCntnt = pShape->GetCntnt();
        SwFrmFmts& rSpzFrmFmts = *pShape->GetDoc()->GetSpzFrmFmts();
        for (SwFrmFmts::iterator it = rSpzFrmFmts.begin(); it != rSpzFrmFmts.end(); ++it)
        {
            SwFrmFmt* pFmt = *it;
            // Only a fly frame can be a TextBox.
            if (pFmt->Which() == RES_FLYFRMFMT && pFmt->GetAttrSet().HasItem(RES_CNTNT) && pFmt->GetCntnt() == rCntnt)
            {
                pRet = pFmt;
                break;
            }
        }
    }

    return pRet;
}

template < typename T >
void lcl_queryInterface(SwFrmFmt* pShape, uno::Any& rAny)
{
    if (SwFrmFmt* pFmt = SwTextBoxHelper::findTextBox(pShape))
    {
        uno::Reference<T> xInterface(static_cast<cppu::OWeakObject*>(SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM)), uno::UNO_QUERY);
        rAny <<= xInterface;
    }
}

uno::Any SwTextBoxHelper::queryInterface(SwFrmFmt* pShape, const uno::Type& rType)
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

Rectangle SwTextBoxHelper::getTextRectangle(SwFrmFmt* pShape, bool bAbsolute)
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

void SwTextBoxHelper::syncProperty(SwFrmFmt* pShape, const OUString& rPropertyName, const css::uno::Any& /*rValue*/)
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
    }
}

void SwTextBoxHelper::syncProperty(SwFrmFmt* pShape, sal_uInt16 nWID, sal_uInt8 nMemberId, const css::uno::Any& rValue)
{
    // No shape yet? Then nothing to do, initial properties are set by create().
    if (!pShape)
        return;

    uno::Any aValue(rValue);
    nMemberId &= ~CONVERT_TWIPS;

    if (SwFrmFmt* pFmt = findTextBox(pShape))
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
            aPropertyName = UNO_NAME_SIZE;
            bAdjustSize = true;
            break;
        case RES_ANCHOR:
            switch (nMemberId)
            {
            case MID_ANCHOR_ANCHORTYPE:
                if (aValue.get<text::TextContentAnchorType>() == text::TextContentAnchorType_AS_CHARACTER)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(static_cast<cppu::OWeakObject*>(SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM)), uno::UNO_QUERY);
                    xPropertySet->setPropertyValue(UNO_NAME_SURROUND, uno::makeAny(text::WrapTextMode_THROUGHT));
                    return;
                }
                break;
            }
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

            uno::Reference<beans::XPropertySet> xPropertySet(static_cast<cppu::OWeakObject*>(SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM)), uno::UNO_QUERY);
            xPropertySet->setPropertyValue(aPropertyName, aValue);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
