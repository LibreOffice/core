/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <outleeng.hxx>
#include <editeng/editobj.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>

/**
 * This is the guts of OutlinerParaObject, refcounted and shared among
 * multiple instances of OutlinerParaObject.
 */
class OutlinerParaObjData
{
public:
    // data members
    EditTextObject*                 mpEditTextObject;
    ParagraphDataVector             maParagraphDataVector;
    bool                            mbIsEditDoc;

    // refcounter
    sal_uInt32                      mnRefCount;

    // constuctor
    OutlinerParaObjData(EditTextObject* pEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc)
    :   mpEditTextObject(pEditTextObject),
        maParagraphDataVector(rParagraphDataVector),
        mbIsEditDoc(bIsEditDoc),
        mnRefCount(0)
    {
        if( maParagraphDataVector.empty() && (pEditTextObject->GetParagraphCount() != 0) )
            maParagraphDataVector.resize(pEditTextObject->GetParagraphCount());
    }

    // destructor
    ~OutlinerParaObjData()
    {
        delete mpEditTextObject;
    }

    bool operator==(const OutlinerParaObjData& rCandidate) const
    {
        return (*mpEditTextObject == *rCandidate.mpEditTextObject
            && maParagraphDataVector == rCandidate.maParagraphDataVector
            && mbIsEditDoc == rCandidate.mbIsEditDoc);
    }

    // #i102062#
    bool isWrongListEqual(const OutlinerParaObjData& rCompare) const
    {
        return mpEditTextObject->isWrongListEqual(*rCompare.mpEditTextObject);
    }
};

struct OutlinerParaObject::Impl
{
    OutlinerParaObjData* mpData;

    Impl( const EditTextObject& rTextObj, const ParagraphDataVector& rParaData, bool bIsEditDoc ) :
        mpData(new OutlinerParaObjData(rTextObj.Clone(), rParaData, bIsEditDoc)) {}

    Impl( const EditTextObject& rTextObj ) :
        mpData(new OutlinerParaObjData(rTextObj.Clone(), ParagraphDataVector(), true)) {}

    Impl( const Impl& r ) : mpData(r.mpData)
    {
        mpData->mnRefCount++;
    }

    ~Impl()
    {
        if (mpData->mnRefCount)
            mpData->mnRefCount--;
        else
            delete mpData;
    }
};

void OutlinerParaObject::ImplMakeUnique()
{
    if (mpImpl->mpData->mnRefCount)
    {
        OutlinerParaObjData* pNew = new OutlinerParaObjData(
            mpImpl->mpData->mpEditTextObject->Clone(),
            mpImpl->mpData->maParagraphDataVector,
            mpImpl->mpData->mbIsEditDoc);
        mpImpl->mpData->mnRefCount--;
        mpImpl->mpData = pNew;
    }
}

OutlinerParaObject::OutlinerParaObject(
    const EditTextObject& rTextObj, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
    mpImpl(new Impl(rTextObj, rParagraphDataVector, bIsEditDoc)) {}

OutlinerParaObject::OutlinerParaObject( const EditTextObject& rTextObj ) :
    mpImpl(new Impl(rTextObj))
{
}

OutlinerParaObject::OutlinerParaObject(const OutlinerParaObject& rCandidate) :
    mpImpl(new Impl(*rCandidate.mpImpl)) {}

OutlinerParaObject::~OutlinerParaObject()
{
    delete mpImpl;
}

OutlinerParaObject& OutlinerParaObject::operator=(const OutlinerParaObject& rCandidate)
{
    if(rCandidate.mpImpl->mpData != mpImpl->mpData)
    {
        if (mpImpl->mpData->mnRefCount)
        {
            mpImpl->mpData->mnRefCount--;
        }
        else
        {
            delete mpImpl->mpData;
        }

        mpImpl->mpData = rCandidate.mpImpl->mpData;
        mpImpl->mpData->mnRefCount++;
    }

    return *this;
}

bool OutlinerParaObject::operator==(const OutlinerParaObject& rCandidate) const
{
    if (rCandidate.mpImpl->mpData == mpImpl->mpData)
    {
        return true;
    }

    return (*rCandidate.mpImpl->mpData == *mpImpl->mpData);
}

// #i102062#
bool OutlinerParaObject::isWrongListEqual(const OutlinerParaObject& rCompare) const
{
    if (rCompare.mpImpl->mpData == mpImpl->mpData)
    {
        return true;
    }

    return mpImpl->mpData->isWrongListEqual(*rCompare.mpImpl->mpData);
}

sal_uInt16 OutlinerParaObject::GetOutlinerMode() const
{
    return mpImpl->mpData->mpEditTextObject->GetUserType();
}

void OutlinerParaObject::SetOutlinerMode(sal_uInt16 nNew)
{
    if (mpImpl->mpData->mpEditTextObject->GetUserType() != nNew)
    {
        ImplMakeUnique();
        mpImpl->mpData->mpEditTextObject->SetUserType(nNew);
    }
}

bool OutlinerParaObject::IsVertical() const
{
    return mpImpl->mpData->mpEditTextObject->IsVertical();
}

void OutlinerParaObject::SetVertical(bool bNew)
{
    if((bool)mpImpl->mpData->mpEditTextObject->IsVertical() != bNew)
    {
        ImplMakeUnique();
        mpImpl->mpData->mpEditTextObject->SetVertical(bNew);
    }
}

sal_Int32 OutlinerParaObject::Count() const
{
    size_t nSize = mpImpl->mpData->maParagraphDataVector.size();
    if (nSize > EE_PARA_MAX_COUNT)
    {
        SAL_WARN( "editeng", "OutlinerParaObject::Count - overflow " << nSize);
        return EE_PARA_MAX_COUNT;
    }
    return static_cast<sal_Int32>(nSize);
}

sal_Int16 OutlinerParaObject::GetDepth(sal_Int32 nPara) const
{
    if(0 <= nPara && static_cast<size_t>(nPara) < mpImpl->mpData->maParagraphDataVector.size())
    {
        return mpImpl->mpData->maParagraphDataVector[nPara].getDepth();
    }
    else
    {
        return -1;
    }
}

const EditTextObject& OutlinerParaObject::GetTextObject() const
{
    return *mpImpl->mpData->mpEditTextObject;
}

bool OutlinerParaObject::IsEditDoc() const
{
    return mpImpl->mpData->mbIsEditDoc;
}

const ParagraphData& OutlinerParaObject::GetParagraphData(sal_Int32 nIndex) const
{
    if(0 <= nIndex && static_cast<size_t>(nIndex) < mpImpl->mpData->maParagraphDataVector.size())
    {
        return mpImpl->mpData->maParagraphDataVector[nIndex];
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
    mpImpl->mpData->mpEditTextObject->ClearPortionInfo();
}

bool OutlinerParaObject::ChangeStyleSheets(const OUString& rOldName,
    SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    ImplMakeUnique();
    return mpImpl->mpData->mpEditTextObject->ChangeStyleSheets(rOldName, eOldFamily, rNewName, eNewFamily);
}

void OutlinerParaObject::ChangeStyleSheetName(SfxStyleFamily eFamily,
    const OUString& rOldName, const OUString& rNewName)
{
    ImplMakeUnique();
    mpImpl->mpData->mpEditTextObject->ChangeStyleSheetName(eFamily, rOldName, rNewName);
}

void OutlinerParaObject::SetStyleSheets(sal_uInt16 nLevel, const OUString& rNewName,
    const SfxStyleFamily& rNewFamily)
{
    const sal_Int32 nCount(Count());

    if(nCount)
    {
        ImplMakeUnique();
        sal_Int32 nDecrementer(nCount);

        for(;nDecrementer;)
        {
            if(GetDepth(--nDecrementer) == nLevel)
            {
                mpImpl->mpData->mpEditTextObject->SetStyleSheet(nDecrementer, rNewName, rNewFamily);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
