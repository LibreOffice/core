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

#ifndef INCLUDED_WW8_ANALYZER_HXX
#define INCLUDED_WW8_ANALYZER_HXX

#include <resourcemodel/WW8ResourceModel.hxx>

#include <boost/unordered_set.hpp>
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

    typedef boost::unordered_set<sal_uInt32, boost::hash<sal_uInt32>, eqSalUInt32> IdSet;
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
    virtual void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void endShape( );

    void dumpStats(ostream & o) const;
};
}

#endif // INCLUDED_WW8_ANALYZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
