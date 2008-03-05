/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Analyzer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:10:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

void WW8Analyzer::dumpStats(ostream & o) const
{
    {
        for (IdSet::const_iterator aIt = mSprmIdSet.begin();
             aIt != mSprmIdSet.end(); aIt++)
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
             aIt != mAttributeIdSet.end(); aIt++)
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

Stream::Pointer_t createAnalyzer()
{
    return Stream::Pointer_t(new WW8Analyzer());
}

}
