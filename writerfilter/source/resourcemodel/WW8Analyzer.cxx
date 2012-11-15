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

#include <WW8Analyzer.hxx>
#include <doctok/resourceids.hxx>
#include <resourcemodel/QNameToString.hxx>

namespace writerfilter
{
bool eqSalUInt32::operator () (sal_uInt32 n1, sal_uInt32 n2) const
{
    return n1 == n2;
}

WW8Analyzer::WW8Analyzer()
{
}

WW8Analyzer::~WW8Analyzer()
{
    dumpStats(cout);
}

void WW8Analyzer::attribute(Id name, Value & val)
{
    string aAttrName = (*QNameToString::Instance())(name);
    string aStr;

    if (aAttrName.length() > 6)
        aStr = aAttrName.substr(4, 2);
    else
        logger("DEBUG", "WW8Analyzer::attribute:" + aAttrName);

    bool bAdd = false;
    if (aStr.compare("LC") == 0 || aStr.compare("FC") == 0)
    {
        if (val.getInt() != 0)
        {
            bAdd = true;
        }
    }
    else
    {
        bAdd = true;
    }

    if (bAdd)
    {
        if (mAttributeMap.count(name) > 0)
        {
            sal_uInt32 nCount = mAttributeMap[name] + 1;
            mAttributeMap[name] = nCount;
        }
        else
            mAttributeMap[name] = 1;

        mAttributeIdSet.insert(name);
    }
}

void WW8Analyzer::sprm(Sprm & sprm_)
{
    if (mSprmMap.count(sprm_.getId()) > 0)
    {
        sal_uInt32 nCount = mSprmMap[sprm_.getId()] + 1;
        mSprmMap[sprm_.getId()] = nCount;
    }
    else
        mSprmMap[sprm_.getId()] = 1;

    mSprmIdSet.insert(sprm_.getId());

    writerfilter::Reference<Properties>::Pointer_t pProps = sprm_.getProps();

    if (pProps.get() != NULL)
    {
        pProps->resolve(*this);
    }

}

void WW8Analyzer::entry(int /*pos*/, ::writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::data(const sal_uInt8 * /*buf*/, size_t /*len*/,
                       ::writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}

void WW8Analyzer::startSectionGroup()
{
}

void WW8Analyzer::endSectionGroup()
{
}

void WW8Analyzer::startParagraphGroup()
{
}

void WW8Analyzer::endParagraphGroup()
{
}

void WW8Analyzer::startCharacterGroup()
{
}

void WW8Analyzer::endCharacterGroup()
{
}

void WW8Analyzer::text(const sal_uInt8 * /*data*/, size_t /*len*/)
{
}

void WW8Analyzer::utext(const sal_uInt8 * /*data*/, size_t /*len*/)
{
}

void WW8Analyzer::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::table(Id /*id*/, writerfilter::Reference<Table>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::substream(Id /*name*/,
                            writerfilter::Reference<Stream>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::info(const string & /*info*/)
{
}

void WW8Analyzer::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > /*xShape*/ )
{
}

void WW8Analyzer::endShape( )
{
}

void WW8Analyzer::dumpStats(ostream & o) const
{
    {
        for (IdSet::const_iterator aIt = mSprmIdSet.begin();
             aIt != mSprmIdSet.end(); ++aIt)
        {
            sal_uInt32 aId = *aIt;

            o << "<sprm>" << endl
              << "<id>" << hex << aId << "</id>" << endl
              << "<name>" << (*SprmIdToString::Instance())(aId)
              << "</name>"  << endl
              << "<count>" << dec << mSprmMap[aId] << "</count>"
              << endl
              << "</sprm>" << endl;
        }
    }

    {
        for (IdSet::const_iterator aIt = mAttributeIdSet.begin();
             aIt != mAttributeIdSet.end(); ++aIt)
        {
            sal_uInt32 aId = *aIt;

            o << "<attribute>" << endl
              << "<name>" << (*QNameToString::Instance())(aId) << "</name>"
              << endl
              << "<count>" << dec << mAttributeMap[aId] << "</count>"
              << endl
              << "</attribute>" << endl;
        }
    }

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
