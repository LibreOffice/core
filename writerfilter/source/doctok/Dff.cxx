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

#include <stdio.h>
#include "Dff.hxx"
#include <doctok/resourceids.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include "resources.hxx"

namespace writerfilter {
namespace doctok {

typedef boost::shared_ptr<WW8Value> WW8ValueSharedPointer_t;

DffRecord::DffRecord(WW8Stream & rStream, sal_uInt32 nOffset, 
                     sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount), bInitialized(false)
{
}

DffRecord::DffRecord(WW8StructBase * pParent, sal_uInt32 nOffset, 
                     sal_uInt32 nCount)
: WW8StructBase(pParent, nOffset, nCount), bInitialized(false)
{
}

Records_t::iterator DffRecord::begin()
{
    if (! bInitialized)
        initChildren();

    return mRecords.begin();
}

Records_t::iterator DffRecord::end()
{
    if (! bInitialized)
        initChildren();

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

void DffRecord::initChildren()
{
    if (isContainer())
    {
        sal_uInt32 nOffset = 8;
        sal_uInt32 nCount = calcSize();
        
        while (nCount - nOffset >= 8)
        {
            sal_uInt32 nSize = 0;
            boost::shared_ptr<DffRecord> pRec
                (createDffRecord(this, nOffset, &nSize));

            if (nSize == 0)
                break;
            
            mRecords.push_back(pRec);
            
            nOffset += nSize;
        }
    }

    bInitialized = true;
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

#if 1
    WW8BinaryObjReference::Pointer_t pBinObjRef
        (new WW8BinaryObjReference(this, 0, getCount()));
    WW8Sprm aSprm(pBinObjRef);

    rHandler.sprm(aSprm);
#endif
}

sal_uInt32 DffRecord::getShapeType()
{
    sal_uInt32 nResult = 0;

    Records_t aRecords = findRecords(0xf00a);

    if (aRecords.size() > 0)
    {
        DffFSP * pDffFSP = dynamic_cast<DffFSP*>((*aRecords.begin()).get());
        nResult = pDffFSP->get_shptype();
    }

    return nResult;
}

sal_uInt32 DffRecord::getShapeId()
{
    sal_uInt32 nResult = 0;

    Records_t aRecords = findRecords(0xf00a);

    if (aRecords.size() > 0)
    {
        DffFSP * pDffFSP = dynamic_cast<DffFSP*>((*aRecords.begin()).get());
        nResult = pDffFSP->get_shpid();
    }

    return nResult;
}

class DffOPTHandler : public Properties
{
    map<int, WW8ValueSharedPointer_t> mMap;
    int nId;

public:
    DffOPTHandler() : nId(0) {}
    virtual ~DffOPTHandler() {}

    virtual void attribute(Id name, Value & val)
    {
        switch (name)
        {
        case NS_rtf::LN_shppid:
            nId = val.getInt();
            break;
        case NS_rtf::LN_shpvalue:
            {
                WW8Value & rTmpVal = dynamic_cast<WW8Value &>(val);
                WW8ValueSharedPointer_t 
                    pVal(dynamic_cast<WW8Value *>(rTmpVal.clone()));
                mMap[nId] = pVal;
            }
        }
    }

    virtual void sprm(Sprm & /*sprm_*/)
    {
    }

    WW8ValueSharedPointer_t & getValue(int nId_)
    {
        return mMap[nId_];
    }

};

sal_uInt32 DffRecord::getShapeBid()
{
    sal_uInt32 nResult = 0;

    if (getShapeType() == 75)
    {
        Records_t aRecords = findRecords(0xf00b);

        if (aRecords.size() > 0)
        {
            DffOPTHandler aHandler;
            DffOPT * pOpts = dynamic_cast<DffOPT*>((*aRecords.begin()).get());

            sal_uInt32 nCount = pOpts->get_property_count();

            for (sal_uInt32 n = 0; n < nCount; ++n)
            {
                pOpts->get_property(n)->resolve(aHandler);
            }

            WW8ValueSharedPointer_t pVal = aHandler.getValue(260);

            if (pVal.get() != NULL)
                nResult = pVal->getInt();
        }
    }
        
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

DffBlock::DffBlock(WW8Stream & rStream, sal_uInt32 nOffset, 
                   sal_uInt32 nCount, sal_uInt32 nPadding)
: WW8StructBase(rStream, nOffset, nCount), bInitialized(false),
  mnPadding(nPadding)
{
}

DffBlock::DffBlock(WW8StructBase * pParent, sal_uInt32 nOffset, 
                   sal_uInt32 nCount, sal_uInt32 nPadding)
: WW8StructBase(pParent, nOffset, nCount), bInitialized(false), 
  mnPadding(nPadding)
{
}

DffBlock::DffBlock(const DffBlock & rSrc)
: WW8StructBase(rSrc), writerfilter::Reference<Properties>(rSrc), 
  bInitialized(false), mnPadding(rSrc.mnPadding)
{
}

void DffBlock::initChildren()
{
    sal_uInt32 nOffset = 0;
    sal_uInt32 nCount = getCount();
    
    while (nOffset < nCount)
    {
        sal_uInt32 nSize = 0;
        DffRecord::Pointer_t pDffRecord
            (createDffRecord(this, nOffset, &nSize));

        if (nSize == 0)
            break;

        mRecords.push_back(pDffRecord);
        
        nOffset +=  nSize + mnPadding;
    }    

    bInitialized = true;
}

Records_t DffBlock::findRecords(sal_uInt32 nType, bool bRecursive, bool bAny)
{
    Records_t aResult;

    findRecords(nType, aResult, bRecursive, bAny);

    return aResult;
}

void DffBlock::findRecords
(sal_uInt32 nType, Records_t & rRecords, bool bRecursive, bool bAny)
{
    Records_t::iterator aIt = begin();

    while (aIt != end())
    {
        DffRecord::Pointer_t pPointer(*aIt);

        if (bAny || pPointer->getRecordType() == nType)
            rRecords.push_back(pPointer);

        if (bRecursive)
            pPointer->findRecords(nType, rRecords, bRecursive, 
                                  bAny);
        
        ++aIt;
    }
}

void DffBlock::resolve(Properties & rHandler)
{
    Records_t::iterator aIt;

    for (aIt = begin(); aIt != end(); ++aIt)
    {
        DffRecord * pDff = aIt->get();
        rHandler.sprm(*pDff);
    }
}

DffRecord::Pointer_t DffBlock::getShape(sal_uInt32 nSpid)
{
    DffRecord::Pointer_t pResult;

    Records_t aRecords = findRecords(0xf004);
    Records_t::iterator aIt;
    for (aIt = aRecords.begin(); aIt != aRecords.end(); ++aIt)
    {
        DffRecord::Pointer_t pPointer = *aIt;

        Records_t aFSPs = pPointer->findRecords(0xf00a);
        Records_t::iterator aItFSP = aFSPs.begin();
        
        if (aItFSP != aFSPs.end())
        {
            DffFSP * pFSP = dynamic_cast<DffFSP *>((*aItFSP).get());

            if (pFSP->get_shpid() == nSpid)
            {
                pResult = pPointer;

                break;
            }
        }
    }

    return pResult;
}

DffRecord::Pointer_t DffBlock::getBlip(sal_uInt32 nBlip)
{
    DffRecord::Pointer_t pResult;

    if (nBlip > 0)
    {
        nBlip--;

        Records_t aRecords = findRecords(0xf007);
        
        if (nBlip < aRecords.size())
        {
            pResult = aRecords[nBlip];
        }
    }

    return pResult;
}

Records_t::iterator DffBlock::begin()
{
    if (! bInitialized)
        initChildren();

    return mRecords.begin();
}

Records_t::iterator DffBlock::end()
{
    if (! bInitialized)
        initChildren();

    return mRecords.end();
}

string DffBlock::getType() const
{
    return "DffBlock";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
