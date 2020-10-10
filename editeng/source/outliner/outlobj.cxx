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

#include <editeng/editdata.hxx>

#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <o3tl/cow_wrapper.hxx>
#include <o3tl/safeint.hxx>
#include <libxml/xmlwriter.h>

OutlinerParaObjData::OutlinerParaObjData( std::unique_ptr<EditTextObject> pEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
    mpEditTextObject(std::move(pEditTextObject)),
    maParagraphDataVector(rParagraphDataVector),
    mbIsEditDoc(bIsEditDoc)
{
    if( maParagraphDataVector.empty() && (mpEditTextObject->GetParagraphCount() != 0) )
        maParagraphDataVector.resize(mpEditTextObject->GetParagraphCount());
}

OutlinerParaObjData::OutlinerParaObjData( const OutlinerParaObjData& r ):
    mpEditTextObject(r.mpEditTextObject->Clone()),
    maParagraphDataVector(r.maParagraphDataVector),
    mbIsEditDoc(r.mbIsEditDoc)
{
}

OutlinerParaObjData::~OutlinerParaObjData()
{
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
    std::unique_ptr<EditTextObject> xTextObj, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc ) :
    mpImpl(OutlinerParaObjData(std::move(xTextObj), rParagraphDataVector, bIsEditDoc))
{
}

OutlinerParaObject::OutlinerParaObject( const EditTextObject& rTextObj ) :
    mpImpl(OutlinerParaObjData(rTextObj.Clone(), ParagraphDataVector(), true))
{
}

OutlinerParaObject::OutlinerParaObject( std::unique_ptr<EditTextObject> pTextObj ) :
    mpImpl(OutlinerParaObjData(std::move(pTextObj), ParagraphDataVector(), true))
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

OutlinerMode OutlinerParaObject::GetOutlinerMode() const
{
    return mpImpl->mpEditTextObject->GetUserType();
}

void OutlinerParaObject::SetOutlinerMode(OutlinerMode nNew)
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

bool OutlinerParaObject::GetDirectVertical() const
{
    return mpImpl->mpEditTextObject->GetDirectVertical();
}

bool OutlinerParaObject::IsTopToBottom() const
{
    return mpImpl->mpEditTextObject->IsTopToBottom();
}

void OutlinerParaObject::SetVertical(bool bNew)
{
    const ::o3tl::cow_wrapper< OutlinerParaObjData >* pImpl = &mpImpl;
    if ( ( *pImpl )->mpEditTextObject->IsVertical() != bNew)
    {
        mpImpl->mpEditTextObject->SetVertical(bNew);
    }
}
void OutlinerParaObject::SetRotation(TextRotation nRotation)
{
    mpImpl->mpEditTextObject->SetRotation(nRotation);
}

TextRotation OutlinerParaObject::GetRotation() const
{
    return mpImpl->mpEditTextObject->GetRotation();
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
    if(0 <= nPara && o3tl::make_unsigned(nPara) < mpImpl->maParagraphDataVector.size())
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

const ParagraphData& OutlinerParaObject::GetParagraphData(sal_Int32 nIndex) const
{
    if(0 <= nIndex && o3tl::make_unsigned(nIndex) < mpImpl->maParagraphDataVector.size())
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("OutlinerParaObject"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    mpImpl->mpEditTextObject->dumpAsXml(pWriter);
    for (ParagraphData const & p : mpImpl->maParagraphDataVector)
        Paragraph(p).dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
