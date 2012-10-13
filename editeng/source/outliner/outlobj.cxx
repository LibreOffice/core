/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#define _OUTLINER_CXX
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <outleeng.hxx>
#include <editeng/editobj.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>

//////////////////////////////////////////////////////////////////////////////

class ImplOutlinerParaObject
{
public:
    // data members
    EditTextObject*                 mpEditTextObject;
    ParagraphDataVector             maParagraphDataVector;
    bool                            mbIsEditDoc;

    // refcounter
    sal_uInt32                      mnRefCount;

    // constuctor
    ImplOutlinerParaObject(EditTextObject* pEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc)
    :   mpEditTextObject(pEditTextObject),
        maParagraphDataVector(rParagraphDataVector),
        mbIsEditDoc(bIsEditDoc),
        mnRefCount(0)
    {
        if( maParagraphDataVector.empty() && (pEditTextObject->GetParagraphCount() != 0) )
            maParagraphDataVector.resize(pEditTextObject->GetParagraphCount());
    }

    // destructor
    ~ImplOutlinerParaObject()
    {
        delete mpEditTextObject;
    }

    bool operator==(const ImplOutlinerParaObject& rCandidate) const
    {
        return (*mpEditTextObject == *rCandidate.mpEditTextObject
            && maParagraphDataVector == rCandidate.maParagraphDataVector
            && mbIsEditDoc == rCandidate.mbIsEditDoc);
    }

    // #i102062#
    bool isWrongListEqual(const ImplOutlinerParaObject& rCompare) const
    {
        return mpEditTextObject->isWrongListEqual(*rCompare.mpEditTextObject);
    }
};

//////////////////////////////////////////////////////////////////////////////

void OutlinerParaObject::ImplMakeUnique()
{
    if(mpImplOutlinerParaObject->mnRefCount)
    {
        ImplOutlinerParaObject* pNew = new ImplOutlinerParaObject(
            mpImplOutlinerParaObject->mpEditTextObject->Clone(),
            mpImplOutlinerParaObject->maParagraphDataVector,
            mpImplOutlinerParaObject->mbIsEditDoc);
        mpImplOutlinerParaObject->mnRefCount--;
        mpImplOutlinerParaObject = pNew;
    }
}

OutlinerParaObject::OutlinerParaObject(const EditTextObject& rEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc)
:   mpImplOutlinerParaObject(new ImplOutlinerParaObject(rEditTextObject.Clone(), rParagraphDataVector, bIsEditDoc))
{
}

OutlinerParaObject::OutlinerParaObject(const OutlinerParaObject& rCandidate)
:   mpImplOutlinerParaObject(rCandidate.mpImplOutlinerParaObject)
{
    mpImplOutlinerParaObject->mnRefCount++;
}

OutlinerParaObject::~OutlinerParaObject()
{
    if(mpImplOutlinerParaObject->mnRefCount)
    {
        mpImplOutlinerParaObject->mnRefCount--;
    }
    else
    {
        delete mpImplOutlinerParaObject;
    }
}

OutlinerParaObject& OutlinerParaObject::operator=(const OutlinerParaObject& rCandidate)
{
    if(rCandidate.mpImplOutlinerParaObject != mpImplOutlinerParaObject)
    {
        if(mpImplOutlinerParaObject->mnRefCount)
        {
            mpImplOutlinerParaObject->mnRefCount--;
        }
        else
        {
            delete mpImplOutlinerParaObject;
        }

        mpImplOutlinerParaObject = rCandidate.mpImplOutlinerParaObject;
        mpImplOutlinerParaObject->mnRefCount++;
    }

    return *this;
}

bool OutlinerParaObject::operator==(const OutlinerParaObject& rCandidate) const
{
    if(rCandidate.mpImplOutlinerParaObject == mpImplOutlinerParaObject)
    {
        return true;
    }

    return (*rCandidate.mpImplOutlinerParaObject == *mpImplOutlinerParaObject);
}

// #i102062#
bool OutlinerParaObject::isWrongListEqual(const OutlinerParaObject& rCompare) const
{
    if(rCompare.mpImplOutlinerParaObject == mpImplOutlinerParaObject)
    {
        return true;
    }

    return mpImplOutlinerParaObject->isWrongListEqual(*rCompare.mpImplOutlinerParaObject);
}

sal_uInt16 OutlinerParaObject::GetOutlinerMode() const
{
    return mpImplOutlinerParaObject->mpEditTextObject->GetUserType();
}

void OutlinerParaObject::SetOutlinerMode(sal_uInt16 nNew)
{
    if(mpImplOutlinerParaObject->mpEditTextObject->GetUserType() != nNew)
    {
        ImplMakeUnique();
        mpImplOutlinerParaObject->mpEditTextObject->SetUserType(nNew);
    }
}

bool OutlinerParaObject::IsVertical() const
{
    return mpImplOutlinerParaObject->mpEditTextObject->IsVertical();
}

void OutlinerParaObject::SetVertical(bool bNew)
{
    if((bool)mpImplOutlinerParaObject->mpEditTextObject->IsVertical() != bNew)
    {
        ImplMakeUnique();
        mpImplOutlinerParaObject->mpEditTextObject->SetVertical(bNew);
    }
}

sal_uInt32 OutlinerParaObject::Count() const
{
    return mpImplOutlinerParaObject->maParagraphDataVector.size();
}

sal_Int16 OutlinerParaObject::GetDepth(sal_uInt16 nPara) const
{
    if(nPara < mpImplOutlinerParaObject->maParagraphDataVector.size())
    {
        return mpImplOutlinerParaObject->maParagraphDataVector[nPara].getDepth();
    }
    else
    {
        return -1;
    }
}

const EditTextObject& OutlinerParaObject::GetTextObject() const
{
    return *mpImplOutlinerParaObject->mpEditTextObject;
}

bool OutlinerParaObject::IsEditDoc() const
{
    return mpImplOutlinerParaObject->mbIsEditDoc;
}

const ParagraphData& OutlinerParaObject::GetParagraphData(sal_uInt32 nIndex) const
{
    if(nIndex < mpImplOutlinerParaObject->maParagraphDataVector.size())
    {
        return mpImplOutlinerParaObject->maParagraphDataVector[nIndex];
    }
    else
    {
        OSL_FAIL("OutlinerParaObject::GetParagraphData: Access out of range (!)");
        static ParagraphData aEmptyParagraphData;
        return aEmptyParagraphData;
    }
}

void OutlinerParaObject::ClearPortionInfo()
{
    ImplMakeUnique();
    mpImplOutlinerParaObject->mpEditTextObject->ClearPortionInfo();
}

bool OutlinerParaObject::ChangeStyleSheets(const OUString& rOldName,
    SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    ImplMakeUnique();
    return mpImplOutlinerParaObject->mpEditTextObject->ChangeStyleSheets(rOldName, eOldFamily, rNewName, eNewFamily);
}

void OutlinerParaObject::ChangeStyleSheetName(SfxStyleFamily eFamily,
    const OUString& rOldName, const OUString& rNewName)
{
    ImplMakeUnique();
    mpImplOutlinerParaObject->mpEditTextObject->ChangeStyleSheetName(eFamily, rOldName, rNewName);
}

void OutlinerParaObject::SetStyleSheets(sal_uInt16 nLevel, const OUString rNewName,
    const SfxStyleFamily& rNewFamily)
{
    const sal_uInt32 nCount(mpImplOutlinerParaObject->maParagraphDataVector.size());

    if(nCount)
    {
        ImplMakeUnique();
        sal_uInt16 nDecrementer(sal::static_int_cast< sal_uInt16 >(nCount));

        for(;nDecrementer;)
        {
            if(GetDepth(--nDecrementer) == nLevel)
            {
                mpImplOutlinerParaObject->mpEditTextObject->SetStyleSheet(nDecrementer, rNewName, rNewFamily);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
