/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Analyzer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:15:57 $
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

#ifndef INCLUDED_WW8_ANALYZER_HXX
#define INCLUDED_WW8_ANALYZER_HXX

#include <resourcemodel/WW8ResourceModel.hxx>

#include <hash_set>
#include <map>

namespace writerfilter
{
using namespace std;

struct eqSalUInt32
{
    bool operator () (sal_uInt32 n1, sal_uInt32 n2) const;
};

class WW8Analyzer : public Properties, public Table,
                    public BinaryObj, public Stream
{
    typedef map<sal_uInt32, sal_uInt32> SprmMap;

    typedef hash_set<sal_uInt32, hash<sal_uInt32>, eqSalUInt32> IdSet;
    typedef map<Id, sal_uInt32> AttributeMap;

    mutable SprmMap mSprmMap;
    IdSet mSprmIdSet;
    mutable AttributeMap mAttributeMap;
    IdSet mAttributeIdSet;

public:
    WW8Analyzer();
    virtual ~WW8Analyzer();

    // Properties

    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table

    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // BinaryObj

    virtual void data(const sal_uInt8* buf, size_t len,
                      writerfilter::Reference<Properties>::Pointer_t ref);

    // Stream

    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void table(Id name,
                       writerfilter::Reference<Table>::Pointer_t ref);
    virtual void substream(Id name,
                           writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & info);

    void dumpStats(ostream & o) const;
};
}

#endif // INCLUDED_WW8_ANALYZER_HXX
