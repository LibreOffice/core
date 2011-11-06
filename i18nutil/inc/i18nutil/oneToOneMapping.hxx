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


#ifndef INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX
#define INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX

#include <utility>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class widthfolding;

typedef std::pair< sal_Unicode, sal_Unicode > OneToOneMappingTable_t;

#define MAKE_PAIR(item1,item2) std::make_pair< sal_Unicode, sal_Unicode >((sal_Unicode)item1,(sal_Unicode)item2)

typedef sal_Int8 UnicodePairFlag;
typedef struct _UnicodePairWithFlag
{
    sal_Unicode     first;
    sal_Unicode     second;
    UnicodePairFlag flag;
} UnicodePairWithFlag;

class oneToOneMapping
{
private:
    // no copy, no substitution
    oneToOneMapping( const oneToOneMapping& );
    oneToOneMapping& operator=( const oneToOneMapping& );
public:
    oneToOneMapping( OneToOneMappingTable_t *rpTable, const size_t rnSize, const size_t rnUnitSize = sizeof(OneToOneMappingTable_t) );
    virtual ~oneToOneMapping();

    // make index for fast search
    // bluedawrf: not used
//        void makeIndex();

    // binary search
    virtual sal_Unicode find( const sal_Unicode nKey ) const;

    // translator
    sal_Unicode operator[] ( const sal_Unicode nKey ) const { return find( nKey ); };

protected:
    OneToOneMappingTable_t *mpTable;
    size_t                  mnSize;
};

class oneToOneMappingWithFlag : public oneToOneMapping
{
    friend class widthfolding;

private:
    // no copy, no substitution
    oneToOneMappingWithFlag( const oneToOneMappingWithFlag& );
    oneToOneMappingWithFlag& operator=( const oneToOneMappingWithFlag& );
public:
    oneToOneMappingWithFlag( UnicodePairWithFlag *rpTableWF, const size_t rnSize, const UnicodePairFlag rnFlag );
    virtual ~oneToOneMappingWithFlag();

    // make index for fast search
    void makeIndex();

    // index search
    virtual sal_Unicode find( const sal_Unicode nKey ) const;
protected:
    UnicodePairWithFlag  *mpTableWF;
    UnicodePairFlag       mnFlag;
    UnicodePairWithFlag **mpIndex[256];
    sal_Bool              mbHasIndex;
};

} } } }

#endif // _I18N_TRANSLITERATION_ONETOONEMAPPING_HXX_
