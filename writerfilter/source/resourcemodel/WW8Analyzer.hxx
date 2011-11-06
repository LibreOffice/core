/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    virtual void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void endShape( );

    void dumpStats(ostream & o) const;
};
}

#endif // INCLUDED_WW8_ANALYZER_HXX
