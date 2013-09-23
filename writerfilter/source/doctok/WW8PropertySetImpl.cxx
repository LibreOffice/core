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

#include "WW8ResourceModelImpl.hxx"
#include <doctok/resourceids.hxx>
#include <resourcemodel/QNameToString.hxx>
#include "WW8PropertySetImpl.hxx"

namespace writerfilter {
namespace doctok
{


bool operator != (const WW8PropertySetIterator & rA,
                  const WW8PropertySetIterator & rB)
{
    return ! (rA.equal(rB));
}

WW8Property::~WW8Property()
{
}

WW8PropertySet::~WW8PropertySet()
{
}

WW8PropertySetIterator::~WW8PropertySetIterator()
{
}

WW8PropertyImpl::WW8PropertyImpl(const WW8StructBase & rBase,
                                 sal_uInt32 nOffset,
                                 sal_uInt32 nCount)
: WW8StructBase(rBase, nOffset, nCount)
{
}

WW8PropertyImpl::~WW8PropertyImpl()
{
}

sal_uInt32 WW8PropertyImpl::getParam() const
{
    sal_uInt32 nResult = 0;

    switch (get_spra())
    {
    case 0:
    case 1:
        nResult = getU8(2);

        break;

    case 2:
    case 4:
    case 5:
        nResult = getU16(2);

        break;

    case 3:
        nResult = getU32(2);

        break;

    case 7:
        nResult = getU16(2) + (getU8(4) << 16);

        break;

    default:
        break;
    }

    return nResult;
}

WW8Stream::Sequence WW8PropertyImpl::getParams() const
{
    return WW8Stream::Sequence(mSequence, 2, getCount() - 2);
}

sal_uInt32 WW8PropertyImpl::getByteLength() const
{
    sal_uInt32 nParamSize = 0;

    switch (get_spra())
    {
    case 0:
    case 1:
        nParamSize = 1;

        break;

    case 2:
    case 4:
    case 5:
        nParamSize = 2;

        break;

    case 3:
        nParamSize = 4;

        break;

    case 7:
        nParamSize = 3;

        break;
    default:
        switch (getId())
        {
        case 0xd608:
                nParamSize = getU16(2) + 1;
                break;
        default:
                nParamSize = getU8(2) + 1;
                break;
        }

        break;
    }

    return nParamSize + 2;
}

void WW8PropertyImpl::dump(OutputWithDepth<string> & o) const
{
    o.addItem(toString());
}

string WW8PropertyImpl::toString() const
{
    string aResult;

    aResult += "<sprmcommon";

    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), " id=\"%" SAL_PRIuUINT32 "\"", getId());
    aResult += sBuffer;
    aResult += " name=\"";
    aResult += (*SprmIdToString::Instance())(getId());
    aResult += "\"";
    snprintf(sBuffer, sizeof(sBuffer), " sgc=\"%x\"", get_sgc());
    aResult += sBuffer;
    snprintf(sBuffer, sizeof(sBuffer), " spra=\"%x\"", get_spra());
    aResult += sBuffer;
    snprintf(sBuffer, sizeof(sBuffer), " size=\"%" SAL_PRIxUINT32 "\"", getByteLength());
    aResult += sBuffer;
    snprintf(sBuffer, sizeof(sBuffer), " param=\"%" SAL_PRIxUINT32 "\"", getParam());
    aResult += sBuffer;
    aResult += ">\n";

    aResult += mSequence.toString();

    aResult += "</sprmcommon>";

    return aResult;
}

WW8PropertySetImpl::WW8PropertySetImpl(WW8Stream & rStream,
                                       sal_uInt32 nOffset,
                                       sal_uInt32 nCount,
                                       bool bPap)
: WW8StructBase(rStream, nOffset, nCount), mbPap(bPap)
{
}

WW8PropertySetImpl::WW8PropertySetImpl(const WW8StructBase & rBase,
                                       sal_uInt32 nOffset,
                                       sal_uInt32 nCount,
                                       bool bPap)
: WW8StructBase(rBase, nOffset, nCount), mbPap(bPap)
{
}

WW8PropertySetImpl::~WW8PropertySetImpl()
{
}

bool WW8PropertySetImpl::isPap() const
{
    return mbPap;
}

sal_uInt32 WW8PropertySetImpl::get_istd() const
{
    sal_uInt32 nResult = 0;

    if (mbPap)
        nResult = getU16(0);

    return nResult;
}

WW8PropertySetIterator::Pointer_t WW8PropertySetImpl::begin()
{
    return WW8PropertySetIterator::Pointer_t
        (new WW8PropertySetIteratorImpl(this, mbPap ? 2 : 0));
}

WW8PropertySetIterator::Pointer_t WW8PropertySetImpl::end()
{
    return WW8PropertySetIterator::Pointer_t
        (new WW8PropertySetIteratorImpl(this, getCount()));
}

WW8PropertySetIteratorImpl::~WW8PropertySetIteratorImpl()
{
}

WW8Property::Pointer_t
WW8PropertySetImpl::getAttribute(sal_uInt32 nOffset) const
{
    WW8PropertyImpl aTmpAttr(*this, nOffset, 3);

    sal_uInt32 nLength = aTmpAttr.getByteLength();

    if (nOffset + nLength > getCount())
        nLength = getCount() - nOffset;

    return WW8Property::Pointer_t
        (new WW8PropertyImpl(*this, nOffset, nLength));
}

void WW8PropertySetImpl::dump(OutputWithDepth<string> & o) const
{
    WW8StructBase::dump(o);

    WW8PropertySetIterator::Pointer_t pIt =
        const_cast<WW8PropertySetImpl *>(this)->begin();
    WW8PropertySetIterator::Pointer_t pItEnd =
        const_cast<WW8PropertySetImpl *>(this)->end();

    while((*pIt) != (*pItEnd))
    {
        WW8Property::Pointer_t pAttr = pIt->get();

        pAttr->dump(o);
        ++(*pIt);
    }
}

void WW8PropertySetImpl::dots(ostream & o)
{
    WW8PropertySetIterator::Pointer_t pIt = begin();
    WW8PropertySetIterator::Pointer_t pItEnd = end();

    while((*pIt) != (*pItEnd))
    {
        WW8Property::Pointer_t pAttr = pIt->get();

        o << "." << endl;

        ++(*pIt);
    }
}

string WW8PropertySetImpl::getType() const
{
    return "WW8PropertySetImpl";
}

void WW8PropertySetImpl::resolveLocal(Sprm & sprm, Properties & rHandler)
{
    switch (sprm.getId())
    {
    case 0x6a03:
        {
            Value::Pointer_t pValue = sprm.getValue();
        }
        break;
    case 0x806:
        break;
    case 0x6646:
        {
            WW8Stream::Pointer_t pStream;

            if (pStream.get() != NULL)
            {
                Value::Pointer_t pValue = sprm.getValue();
                sal_uInt32 nOffset = pValue->getInt();
                WW8StructBase aStruct(*pStream, nOffset, 2);
                sal_uInt16 nCount = aStruct.getU16(0);

                {
                    WW8PropertySetImpl * pPropSet =
                    new WW8PropertySetImpl(*pStream, nOffset + 2, nCount);

                    pPropSet->resolve(rHandler);
                }
            }
        }
        break;
    default:
        break;
    }
}

void WW8PropertySetImpl::resolve(Properties & rHandler)
{
    if (getCount() >= (isPap() ? 5U : 3U))
    {
        WW8PropertySetIterator::Pointer_t pIt = begin();
        WW8PropertySetIterator::Pointer_t pItEnd = end();

        if (isPap())
        {
            WW8Value::Pointer_t pValue = createValue(getU16(0));
            rHandler.attribute(NS_rtf::LN_ISTD, *pValue);
        }

        while((*pIt) != (*pItEnd))
        {
            WW8Sprm aSprm(pIt->get());

            rHandler.sprm(aSprm);

            resolveLocal(aSprm, rHandler);

            ++(*pIt);
        }
    }
}

WW8PropertySetIterator & WW8PropertySetIteratorImpl::operator++ ()
{
    WW8Property::Pointer_t pTmpAttr = mpAttrSet->getAttribute(mnOffset);

    mnOffset += dynamic_cast<WW8PropertyImpl *>(pTmpAttr.get())->
        getByteLength();

    if (mnOffset > mpAttrSet->getCount() ||
        mpAttrSet->getCount() - mnOffset < 3)
        mnOffset = mpAttrSet->getCount();

    return *this;
}

WW8Property::Pointer_t WW8PropertySetIteratorImpl::get() const
{
    return mpAttrSet->getAttribute(mnOffset);
}

bool WW8PropertySetIteratorImpl::equal
(const WW8PropertySetIterator & rIt) const
{
    const WW8PropertySetIteratorImpl & rMyIt =
        dynamic_cast<const WW8PropertySetIteratorImpl &>(rIt);

    return mpAttrSet == rMyIt.mpAttrSet && mnOffset == rMyIt.mnOffset;
}

string WW8PropertySetIteratorImpl::toString() const
{
    string sResult = "";

    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "(%" SAL_PRIuUINT32 ")", mnOffset);
    sResult += sBuffer;

    return sResult;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
