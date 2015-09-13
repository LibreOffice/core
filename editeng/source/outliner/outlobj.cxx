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

#include <boost/intrusive_ptr.hpp>
#include <libxml/xmlwriter.h>

/**
 * This is the guts of OutlinerParaObject, refcounted and shared among
 * multiple instances of OutlinerParaObject.
 */
struct OutlinerParaObjData
{
    // data members
    EditTextObject*                 mpEditTextObject;
    ParagraphDataVector             maParagraphDataVector;
    bool                            mbIsEditDoc;

    // refcounter
    mutable size_t mnRefCount;

    // constuctor
    OutlinerParaObjData( EditTextObject* pEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
        mpEditTextObject(pEditTextObject),
        maParagraphDataVector(rParagraphDataVector),
        mbIsEditDoc(bIsEditDoc),
        mnRefCount(0)
    {
        if( maParagraphDataVector.empty() && (pEditTextObject->GetParagraphCount() != 0) )
            maParagraphDataVector.resize(pEditTextObject->GetParagraphCount());
    }

    OutlinerParaObjData( const OutlinerParaObjData& r ) :
        mpEditTextObject(r.mpEditTextObject->Clone()),
        maParagraphDataVector(r.maParagraphDataVector),
        mbIsEditDoc(r.mbIsEditDoc),
        mnRefCount(0)
    {}

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

inline void intrusive_ptr_add_ref(const OutlinerParaObjData* p)
{
    ++p->mnRefCount;
}

inline void intrusive_ptr_release(const OutlinerParaObjData* p)
{
    --p->mnRefCount;
    if (!p->mnRefCount)
        delete p;
}

struct OutlinerParaObject::Impl
{
    typedef boost::intrusive_ptr<OutlinerParaObjData> DataRef;
    DataRef mxData;

    Impl( const EditTextObject& rTextObj, const ParagraphDataVector& rParaData, bool bIsEditDoc ) :
        mxData(new OutlinerParaObjData(rTextObj.Clone(), rParaData, bIsEditDoc)) {}

    explicit Impl(const EditTextObject& rTextObj) :
        mxData(new OutlinerParaObjData(rTextObj.Clone(), ParagraphDataVector(), true)) {}

    Impl( const Impl& r ) : mxData(r.mxData) {}

    ~Impl() {}
};

void OutlinerParaObject::ImplMakeUnique()
{
    mpImpl->mxData.reset(new OutlinerParaObjData(*mpImpl->mxData));
}

OutlinerParaObject::OutlinerParaObject(
    const EditTextObject& rTextObj, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
    mpImpl(new Impl(rTextObj, rParagraphDataVector, bIsEditDoc)) {}

OutlinerParaObject::OutlinerParaObject( const EditTextObject& rTextObj ) :
    mpImpl(new Impl(rTextObj))
{
}

OutlinerParaObject::OutlinerParaObject( const OutlinerParaObject& r ) :
    mpImpl(new Impl(*r.mpImpl)) {}

OutlinerParaObject::~OutlinerParaObject()
{
    delete mpImpl;
}

OutlinerParaObject& OutlinerParaObject::operator=( const OutlinerParaObject& r )
{
    mpImpl->mxData = r.mpImpl->mxData;
    return *this;
}

bool OutlinerParaObject::operator==( const OutlinerParaObject& r ) const
{
    if (r.mpImpl->mxData.get() == mpImpl->mxData.get())
    {
        return true;
    }

    return (*r.mpImpl->mxData == *mpImpl->mxData);
}

// #i102062#
bool OutlinerParaObject::isWrongListEqual( const OutlinerParaObject& r ) const
{
    if (r.mpImpl->mxData.get() == mpImpl->mxData.get())
    {
        return true;
    }

    return mpImpl->mxData->isWrongListEqual(*r.mpImpl->mxData);
}

sal_uInt16 OutlinerParaObject::GetOutlinerMode() const
{
    return mpImpl->mxData->mpEditTextObject->GetUserType();
}

void OutlinerParaObject::SetOutlinerMode(sal_uInt16 nNew)
{
    if (mpImpl->mxData->mpEditTextObject->GetUserType() != nNew)
    {
        ImplMakeUnique();
        mpImpl->mxData->mpEditTextObject->SetUserType(nNew);
    }
}

bool OutlinerParaObject::IsVertical() const
{
    return mpImpl->mxData->mpEditTextObject->IsVertical();
}

void OutlinerParaObject::SetVertical(bool bNew)
{
    if (mpImpl->mxData->mpEditTextObject->IsVertical() != bNew)
    {
        ImplMakeUnique();
        mpImpl->mxData->mpEditTextObject->SetVertical(bNew);
    }
}

sal_Int32 OutlinerParaObject::Count() const
{
    size_t nSize = mpImpl->mxData->maParagraphDataVector.size();
    if (nSize > EE_PARA_MAX_COUNT)
    {
        SAL_WARN( "editeng", "OutlinerParaObject::Count - overflow " << nSize);
        return EE_PARA_MAX_COUNT;
    }
    return static_cast<sal_Int32>(nSize);
}

sal_Int16 OutlinerParaObject::GetDepth(sal_Int32 nPara) const
{
    if(0 <= nPara && static_cast<size_t>(nPara) < mpImpl->mxData->maParagraphDataVector.size())
    {
        return mpImpl->mxData->maParagraphDataVector[nPara].getDepth();
    }
    else
    {
        return -1;
    }
}

const EditTextObject& OutlinerParaObject::GetTextObject() const
{
    return *mpImpl->mxData->mpEditTextObject;
}

bool OutlinerParaObject::IsEditDoc() const
{
    return mpImpl->mxData->mbIsEditDoc;
}

const ParagraphData& OutlinerParaObject::GetParagraphData(sal_Int32 nIndex) const
{
    if(0 <= nIndex && static_cast<size_t>(nIndex) < mpImpl->mxData->maParagraphDataVector.size())
    {
        return mpImpl->mxData->maParagraphDataVector[nIndex];
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
    mpImpl->mxData->mpEditTextObject->ClearPortionInfo();
}

bool OutlinerParaObject::ChangeStyleSheets(const OUString& rOldName,
    SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    ImplMakeUnique();
    return mpImpl->mxData->mpEditTextObject->ChangeStyleSheets(rOldName, eOldFamily, rNewName, eNewFamily);
}

void OutlinerParaObject::ChangeStyleSheetName(SfxStyleFamily eFamily,
    const OUString& rOldName, const OUString& rNewName)
{
    ImplMakeUnique();
    mpImpl->mxData->mpEditTextObject->ChangeStyleSheetName(eFamily, rOldName, rNewName);
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
                mpImpl->mxData->mpEditTextObject->SetStyleSheet(nDecrementer, rNewName, rNewFamily);
            }
        }
    }
}

void OutlinerParaObject::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("outlinerParaObject"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    GetTextObject().dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
