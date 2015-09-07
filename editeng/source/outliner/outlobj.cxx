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

#include <o3tl/cow_wrapper.hxx>
#include <libxml/xmlwriter.h>

OutlinerParaObjData::OutlinerParaObjData( EditTextObject* pEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
    mpEditTextObject(pEditTextObject),
    maParagraphDataVector(rParagraphDataVector),
    mbIsEditDoc(bIsEditDoc)
{
    if( maParagraphDataVector.empty() && (pEditTextObject->GetParagraphCount() != 0) )
        maParagraphDataVector.resize(pEditTextObject->GetParagraphCount());
}

OutlinerParaObjData::OutlinerParaObjData( const OutlinerParaObjData& r ):
    mpEditTextObject(r.mpEditTextObject->Clone()),
    maParagraphDataVector(r.maParagraphDataVector),
    mbIsEditDoc(r.mbIsEditDoc)
{
}

OutlinerParaObjData::~OutlinerParaObjData()
{
    delete mpEditTextObject;
}

bool OutlinerParaObjData::operator==(const OutlinerParaObjData& rCandidate) const
{
    return (*mpEditTextObject == *rCandidate.mpEditTextObject
        && maParagraphDataVector == rCandidate.maParagraphDataVector
        && mbIsEditDoc == rCandidate.mbIsEditDoc);
}

bool OutlinerParaObjData::isWrongListEqual(const OutlinerParaObjData& rCompare) const
{
    return mpEditTextObject->isWrongListEqual(*rCompare.mpEditTextObject);
}

OutlinerParaObject::OutlinerParaObject(
    const EditTextObject& rTextObj, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
    mpImpl(OutlinerParaObjData(rTextObj.Clone(), rParagraphDataVector, bIsEditDoc))
{
}

OutlinerParaObject::OutlinerParaObject( const EditTextObject& rTextObj ) :
    mpImpl(OutlinerParaObjData(rTextObj.Clone(), ParagraphDataVector(), true))
{
}

OutlinerParaObject::OutlinerParaObject( const OutlinerParaObject& r ) :
    mpImpl(r.mpImpl)
{
}

OutlinerParaObject::~OutlinerParaObject()
{
}

OutlinerParaObject& OutlinerParaObject::operator=( const OutlinerParaObject& r )
{
    mpImpl = r.mpImpl;
    return *this;
}

bool OutlinerParaObject::operator==( const OutlinerParaObject& r ) const
{
    return r.mpImpl == mpImpl;
}

// #i102062#
bool OutlinerParaObject::isWrongListEqual( const OutlinerParaObject& r ) const
{
    if (r.mpImpl.same_object(mpImpl))
    {
        return true;
    }

    return mpImpl->isWrongListEqual(*r.mpImpl);
}

sal_uInt16 OutlinerParaObject::GetOutlinerMode() const
{
    return mpImpl->mpEditTextObject->GetUserType();
}

void OutlinerParaObject::SetOutlinerMode(sal_uInt16 nNew)
{
    // create a const pointer to avoid an early call to
    // make_unique() in the dereference of mpImpl
    const ::o3tl::cow_wrapper< OutlinerParaObjData >* pImpl = &mpImpl;
    if ( ( *pImpl )->mpEditTextObject->GetUserType() != nNew )
    {
        mpImpl->mpEditTextObject->SetUserType(nNew);
    }
}

bool OutlinerParaObject::IsVertical() const
{
    return mpImpl->mpEditTextObject->IsVertical();
}

void OutlinerParaObject::SetVertical(bool bNew)
{
    const ::o3tl::cow_wrapper< OutlinerParaObjData >* pImpl = &mpImpl;
    if ( ( *pImpl )->mpEditTextObject->IsVertical() != bNew )
    {
        mpImpl->mpEditTextObject->SetVertical(bNew);
    }
}

sal_Int32 OutlinerParaObject::Count() const
{
    size_t nSize = mpImpl->maParagraphDataVector.size();
    if (nSize > EE_PARA_MAX_COUNT)
    {
        SAL_WARN( "editeng", "OutlinerParaObject::Count - overflow " << nSize);
        return EE_PARA_MAX_COUNT;
    }
    return static_cast<sal_Int32>(nSize);
}

sal_Int16 OutlinerParaObject::GetDepth(sal_Int32 nPara) const
{
    if(0 <= nPara && static_cast<size_t>(nPara) < mpImpl->maParagraphDataVector.size())
    {
        return mpImpl->maParagraphDataVector[nPara].getDepth();
    }
    else
    {
        return -1;
    }
}

const EditTextObject& OutlinerParaObject::GetTextObject() const
{
    return *mpImpl->mpEditTextObject;
}

bool OutlinerParaObject::IsEditDoc() const
{
    return mpImpl->mbIsEditDoc;
}

const ParagraphData& OutlinerParaObject::GetParagraphData(sal_Int32 nIndex) const
{
    if(0 <= nIndex && static_cast<size_t>(nIndex) < mpImpl->maParagraphDataVector.size())
    {
        return mpImpl->maParagraphDataVector[nIndex];
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
    mpImpl->mpEditTextObject->ClearPortionInfo();
}

bool OutlinerParaObject::ChangeStyleSheets(const OUString& rOldName,
    SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily)
{
    return mpImpl->mpEditTextObject->ChangeStyleSheets(rOldName, eOldFamily, rNewName, eNewFamily);
}

void OutlinerParaObject::ChangeStyleSheetName(SfxStyleFamily eFamily,
    const OUString& rOldName, const OUString& rNewName)
{
    mpImpl->mpEditTextObject->ChangeStyleSheetName(eFamily, rOldName, rNewName);
}

void OutlinerParaObject::SetStyleSheets(sal_uInt16 nLevel, const OUString& rNewName,
    const SfxStyleFamily& rNewFamily)
{
    const sal_Int32 nCount(Count());

    if(nCount)
    {
        sal_Int32 nDecrementer(nCount);

        while(nDecrementer > 0)
        {
            if(GetDepth(--nDecrementer) == nLevel)
            {
                mpImpl->mpEditTextObject->SetStyleSheet(nDecrementer, rNewName, rNewFamily);
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
