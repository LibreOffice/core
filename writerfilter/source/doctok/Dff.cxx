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

#include <stdio.h>
#include "Dff.hxx"
#include <doctok/resourceids.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include "doctok/resources.hxx"

namespace writerfilter {
namespace doctok {

typedef boost::shared_ptr<WW8Value> WW8ValueSharedPointer_t;

DffRecord::DffRecord(WW8Stream & rStream, sal_uInt32 nOffset,
                     sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount)
{
}

DffRecord::DffRecord(WW8StructBase * pParent, sal_uInt32 nOffset,
                     sal_uInt32 nCount)
: WW8StructBase(pParent, nOffset, nCount)
{
}

Records_t::iterator DffRecord::begin()
{
    return mRecords.begin();
}

Records_t::iterator DffRecord::end()
{
    return mRecords.end();
}

bool DffRecord::isContainer() const
{
    return getVersion() == 0xf;
}

sal_uInt32 DffRecord::calcSize() const
{
    sal_uInt32 nResult = 0;

    switch (getRecordType())
    {
    case 0xf000:
    case 0xf001:
    case 0xf002:
    case 0xf003:
    case 0xf004:
        nResult = getU32(0x4) + 8;

        break;
    case 0xf700:
        nResult = 8;

        break;
    default:
        nResult = getU32(0x4);

        if (! isContainer())
            nResult += 8;
        break;
    }

    return nResult;
}

sal_uInt32 DffRecord::getVersion() const
{
    return getU8(0x0) & 0xf;
}

sal_uInt32 DffRecord::getInstance() const
{
    return (getU16(0x0) & 0xfff0) >> 4;
}

sal_uInt32 DffRecord::getRecordType() const
{
    return getU16(0x2);
}

Records_t DffRecord::findRecords(sal_uInt32 nType, bool bRecursive, bool bAny)
{
    Records_t aResult;

    findRecords(nType, aResult, bRecursive, bAny);

    return aResult;
}

void DffRecord::findRecords
(sal_uInt32 nType, Records_t & rRecords, bool bRecursive, bool bAny)
{
    Records_t::iterator aIt = begin();

    while (aIt != end())
    {
        Pointer_t pPointer = *aIt;
        if (bAny || pPointer->getRecordType() == nType)
            rRecords.push_back(pPointer);

        if (bRecursive)
            pPointer->findRecords(nType, rRecords, bRecursive,
                                      bAny);

        ++aIt;
    }
}

void DffRecord::resolveChildren(Properties & rHandler)
{
    Records_t::iterator aIt;
    for (aIt = begin(); aIt != end(); ++aIt)
    {
        rHandler.sprm(**aIt);
    }
}

void DffRecord::resolveLocal(Properties &)
{
}

void DffRecord::resolve(Properties & rHandler)
{
    WW8Value::Pointer_t pVal = createValue(getRecordType());
    rHandler.attribute(NS_rtf::LN_dfftype, *pVal);

    pVal = createValue(getInstance());
    rHandler.attribute(NS_rtf::LN_dffinstance, *pVal);

    pVal = createValue(getVersion());
    rHandler.attribute(NS_rtf::LN_dffversion, *pVal);

    pVal = createValue(getU32(0x0));
    rHandler.attribute(NS_rtf::LN_dffheader, *pVal);

    if (isContainer())
    {
        resolveChildren(rHandler);
    }

    resolveLocal(rHandler);

    WW8BinaryObjReference::Pointer_t pBinObjRef
        (new WW8BinaryObjReference(this, 0, getCount()));
    WW8Sprm aSprm(pBinObjRef);

    rHandler.sprm(aSprm);
}

sal_uInt32 DffRecord::getShapeType()
{
    sal_uInt32 nResult = 0;

    Records_t aRecords = findRecords(0xf00a);

    return nResult;
}

sal_uInt32 DffRecord::getShapeId()
{
    sal_uInt32 nResult = 0;

    Records_t aRecords = findRecords(0xf00a);

    return nResult;
}

sal_uInt32 DffRecord::getShapeBid()
{
    sal_uInt32 nResult = 0;

    return nResult;
}

string DffRecord::getType() const
{
    return "DffRecord";
}

Value::Pointer_t DffRecord::getValue()
{
    return Value::Pointer_t();
}

writerfilter::Reference<BinaryObj>::Pointer_t DffRecord::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

writerfilter::Reference<Stream>::Pointer_t DffRecord::getStream()
{
    return writerfilter::Reference<Stream>::Pointer_t();
}

writerfilter::Reference<Properties>::Pointer_t DffRecord::getProps()
{
    return writerfilter::Reference<Properties>::Pointer_t(this->clone());
}

string DffRecord::toString() const
{
    char sBuffer[1024];

    snprintf(sBuffer, sizeof(sBuffer),
             "<dffrecord type=\"%" SAL_PRIuUINT32 "\" instance=\"%" SAL_PRIuUINT32 "\" version=\"%" SAL_PRIuUINT32 "\">\n",
             getRecordType(), getInstance(), getVersion());
    string aResult = sBuffer;


    if (!isContainer())
        aResult += mSequence.toString();
    else
    {
        WW8StructBase::Sequence aSeq(mSequence, 0, 8);
        aResult += aSeq.toString();
    }

    aResult += "</dffrecord>";

    return aResult;
}

string DffRecord::getName() const
{
    return "";
}

Sprm::Kind DffRecord::getKind()
{
    return Sprm::UNKNOWN;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
