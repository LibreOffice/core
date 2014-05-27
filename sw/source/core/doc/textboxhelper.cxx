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
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unomid.h>
#include <dflyobj.hxx>

#include <svx/svdoashp.hxx>
#include <svx/unopage.hxx>
#include <svx/svdpage.hxx>

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/SizeType.hpp>
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
        xPropertySet->setPropertyValue("TopBorder", aEmptyBorder);
        xPropertySet->setPropertyValue("BottomBorder", aEmptyBorder);
        xPropertySet->setPropertyValue("LeftBorder", aEmptyBorder);
        xPropertySet->setPropertyValue("RightBorder", aEmptyBorder);

        xPropertySet->setPropertyValue("FillTransparence", uno::makeAny(sal_Int32(100)));

        xPropertySet->setPropertyValue("SizeType", uno::makeAny(text::SizeType::FIX));

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
    return uno::makeAny(uno::Reference<drawing::XShape>(pRet->getUnoShape(), uno::UNO_QUERY));
}

SwFrmFmt* SwTextBoxHelper::findTextBox(SwFrmFmt* pShape)
{
    SwFrmFmt* pRet = 0;

    if (pShape->GetAttrSet().HasItem(RES_CNTNT))
    {
        const SwFmtCntnt& rCntnt = pShape->GetCntnt();
        SwFrmFmts& rSpzFrmFmts = *pShape->GetDoc()->GetSpzFrmFmts();
        for (SwFrmFmts::iterator it = rSpzFrmFmts.begin(); it != rSpzFrmFmts.end(); ++it)
        {
            SwFrmFmt* pFmt = *it;
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


    return aRet;
}

void SwTextBoxHelper::syncProperty(SwFrmFmt* pShape, sal_uInt16 nWID, sal_uInt8 nMemberId, const OUString& rPropertyName, const css::uno::Any& rValue)
{
    // No shape yet? Then nothing to do, initial properties are set by create().
    if (!pShape)
        return;

    uno::Any aValue(rValue);
    nMemberId &= ~CONVERT_TWIPS;

    if (SwFrmFmt* pFmt = findTextBox(pShape))
    {
        bool bSync = false;
        bool bAdjustX = false;
        bool bAdjustY = false;
        bool bAdjustSize = false;
        switch (nWID)
        {
        case RES_HORI_ORIENT:
            switch (nMemberId)
            {
            case MID_HORIORIENT_ORIENT:
            case MID_HORIORIENT_RELATION:
                bSync = true;
                break;
            case MID_HORIORIENT_POSITION:
                bSync = true;
                bAdjustX = true;
                break;
            }
            break;
        case RES_VERT_ORIENT:
            switch (nMemberId)
            {
            case MID_VERTORIENT_ORIENT:
            case MID_VERTORIENT_RELATION:
                bSync = true;
                break;
            case MID_VERTORIENT_POSITION:
                bSync = true;
                bAdjustY = true;
                break;
            }
            break;
        case RES_FRM_SIZE:
            bSync = true;
            bAdjustSize = true;
            break;
        }

        if (bSync)
        {
            // Position/size should be the text position/size, not the shape one as-is.
            if (bAdjustX || bAdjustY || bAdjustSize)
            {
                SdrObjCustomShape* pCustomShape = dynamic_cast<SdrObjCustomShape*>(pShape->FindRealSdrObject());
                if (pCustomShape)
                {
                    Rectangle aRect;
                    pCustomShape->GetTextBounds(aRect);

                    if (bAdjustX || bAdjustY)
                    {
                        sal_Int32 nValue;
                        if (aValue >>= nValue)
                        {
                            if (bAdjustX)
                                nValue = TWIPS_TO_MM(aRect.getX());
                            else if (bAdjustY)
                                nValue = TWIPS_TO_MM(aRect.getY());
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
            xPropertySet->setPropertyValue(rPropertyName, aValue);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
