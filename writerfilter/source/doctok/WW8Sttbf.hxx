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



#ifndef INCLUDED_WW8_STTBF_HXX
#define INCLUDED_WW8_STTBF_HXX

#include <WW8StructBase.hxx>
#include <WW8ResourceModelImpl.hxx>

#include <boost/shared_ptr.hpp>
#include <vector>

namespace writerfilter {
namespace doctok
{

using namespace ::std;

/**
   A string table in file.

   The string table contains strings that each can have extra data.
 */
class WW8Sttbf : public WW8StructBase
{
    /// true if strings contain two-byte characters
    bool mbComplex;

    /// the number of entries
    sal_uInt32 mnCount;

    /// the size of the extra data (per string)
    sal_uInt32 mnExtraDataCount;

    /// offsets for the strings
    vector<sal_uInt32> mEntryOffsets;

    /// offsets for the extra data
    vector<sal_uInt32> mExtraOffsets;

    /**
       Return offset of an entry.

       @param nPos    the index of the entry
    */
    sal_uInt32 getEntryOffset(sal_uInt32 nPos) const;

    /**
       Return offset of extra data of an entry.

       @param nPos    the index of the entry
     */
    sal_uInt32 getExtraOffset(sal_uInt32 nPos) const;

public:
    typedef boost::shared_ptr<WW8Sttbf> Pointer_t;

    WW8Sttbf(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount);

    /**
       Return the number of entries.
    */
    sal_uInt32 getEntryCount() const;

    /**
       Return the string of an entry.

       @param nPos     the index of the entry
     */
    rtl::OUString getEntry(sal_uInt32 nPos) const;

    /**
       Return the extra data of an entry.

       @param nPos     the index of the entry
    */
    WW8StructBase::Pointer_t getExtraData(sal_uInt32 nPos);
};

class WW8SttbTableResource : public ::writerfilter::Reference<Table>
{
    WW8Sttbf::Pointer_t mpSttbf;

public:
    WW8SttbTableResource(WW8Sttbf::Pointer_t pSttbf);
    virtual ~WW8SttbTableResource();

    void resolve(Table & rTable);

    string getType() const;
};

class WW8StringProperty : public ::writerfilter::Reference<Properties>
{
    sal_uInt32 mnId;
    WW8StringValue::Pointer_t mpValue;

public:
    WW8StringProperty(sal_uInt32 nId, WW8StringValue::Pointer_t pValue);
    virtual ~WW8StringProperty();

    void resolve(Properties & rProperties);

    string getType() const;
};

}}
#endif // INCLUDED_WW8_STTBF
