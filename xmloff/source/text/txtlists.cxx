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


#include <txtlists.hxx>
#include <comphelper/random.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>

#include <xmloff/txtimp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "XMLTextListItemContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "txtparai.hxx"


using namespace ::com::sun::star;


XMLTextListsHelper::XMLTextListsHelper()
   :  mpProcessedLists( nullptr ),
      msLastProcessedListId(),
      msListStyleOfLastProcessedList(),
      // Inconsistent behavior regarding lists (#i92811#)
      mpMapListIdToListStyleDefaultListId( nullptr ),
      mpContinuingLists( nullptr ),
      mpListStack( nullptr )
{
}

XMLTextListsHelper::~XMLTextListsHelper()
{
    if ( mpProcessedLists )
    {
        mpProcessedLists->clear();
        delete mpProcessedLists;
    }
    // Inconsistent behavior regarding lists (#i92811#)#
    if ( mpMapListIdToListStyleDefaultListId )
    {
        mpMapListIdToListStyleDefaultListId->clear();
        delete mpMapListIdToListStyleDefaultListId;
    }
    if ( mpContinuingLists )
    {
        mpContinuingLists->clear();
        delete mpContinuingLists;
    }
    if ( mpListStack )
    {
        mpListStack->clear();
        delete mpListStack;
    }
}

void XMLTextListsHelper::PushListContext(
    XMLTextListBlockContext *i_pListBlock)
{
    mListStack.push(::boost::make_tuple(i_pListBlock,
        static_cast<XMLTextListItemContext*>(nullptr),
        static_cast<XMLNumberedParaContext*>(nullptr)));
}

void XMLTextListsHelper::PushListContext(
    XMLNumberedParaContext *i_pNumberedParagraph)
{
    mListStack.push(::boost::make_tuple(
        static_cast<XMLTextListBlockContext*>(nullptr),
        static_cast<XMLTextListItemContext*>(nullptr), i_pNumberedParagraph));
}

void XMLTextListsHelper::PopListContext()
{
    assert(mListStack.size());
    if ( !mListStack.empty())
        mListStack.pop();
}

void XMLTextListsHelper::ListContextTop(
    XMLTextListBlockContext*& o_pListBlockContext,
    XMLTextListItemContext*& o_pListItemContext,
    XMLNumberedParaContext*& o_pNumberedParagraphContext )
{
    if ( !mListStack.empty() ) {
        o_pListBlockContext =
            static_cast<XMLTextListBlockContext*>(&mListStack.top().get<0>());
        o_pListItemContext  =
            static_cast<XMLTextListItemContext *>(&mListStack.top().get<1>());
        o_pNumberedParagraphContext =
            static_cast<XMLNumberedParaContext *>(&mListStack.top().get<2>());
    }
}

void XMLTextListsHelper::SetListItem( XMLTextListItemContext *i_pListItem )
{
    // may be cleared by ListBlockContext for upper list...
    if (i_pListItem) {
        assert(mListStack.size());
        assert(mListStack.top().get<0>() &&
            "internal error: SetListItem: mListStack has no ListBlock");
        assert(!mListStack.top().get<1>() &&
            "error: SetListItem: list item already exists");
    }
    if ( !mListStack.empty() ) {
        mListStack.top().get<1>() = i_pListItem;
    }
}

// Handling for parameter <sListStyleDefaultListId> (#i92811#)
void XMLTextListsHelper::KeepListAsProcessed( const OUString& sListId,
                                              const OUString& sListStyleName,
                                              const OUString& sContinueListId,
                                              const OUString& sListStyleDefaultListId )
{
    if ( IsListProcessed( sListId ) )
    {
        assert(false &&
                    "<XMLTextListsHelper::KeepListAsProcessed(..)> - list id already added" );
        return;
    }

    if ( mpProcessedLists == nullptr )
    {
        mpProcessedLists = new tMapForLists();
    }

    ::std::pair< OUString, OUString >
                                aListData( sListStyleName, sContinueListId );
    (*mpProcessedLists)[ sListId ] = aListData;

    msLastProcessedListId = sListId;
    msListStyleOfLastProcessedList = sListStyleName;

    // Inconsistent behavior regarding lists (#i92811#)
    if ( !sListStyleDefaultListId.isEmpty())
    {
        if ( mpMapListIdToListStyleDefaultListId == nullptr )
        {
            mpMapListIdToListStyleDefaultListId = new tMapForLists();
        }

        if ( mpMapListIdToListStyleDefaultListId->find( sListStyleName ) ==
                                mpMapListIdToListStyleDefaultListId->end() )
        {
            ::std::pair< OUString, OUString >
                                aListIdMapData( sListId, sListStyleDefaultListId );
            (*mpMapListIdToListStyleDefaultListId)[ sListStyleName ] =
                                                                aListIdMapData;
        }
    }
}

bool XMLTextListsHelper::IsListProcessed( const OUString& sListId ) const
{
    if ( mpProcessedLists == nullptr )
    {
        return false;
    }

    return mpProcessedLists->find( sListId ) != mpProcessedLists->end();
}

OUString XMLTextListsHelper::GetListStyleOfProcessedList(
                                            const OUString& sListId ) const
{
    if ( mpProcessedLists != nullptr )
    {
        tMapForLists::const_iterator aIter = mpProcessedLists->find( sListId );
        if ( aIter != mpProcessedLists->end() )
        {
            return (*aIter).second.first;
        }
    }

    return OUString();
}

OUString XMLTextListsHelper::GetContinueListIdOfProcessedList(
                                            const OUString& sListId ) const
{
    if ( mpProcessedLists != nullptr )
    {
        tMapForLists::const_iterator aIter = mpProcessedLists->find( sListId );
        if ( aIter != mpProcessedLists->end() )
        {
            return (*aIter).second.second;
        }
    }

    return OUString();
}



OUString XMLTextListsHelper::GenerateNewListId() const
{
    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);
    OUString sTmpStr( "list" );

    if (bHack)
    {
        static sal_Int64 nIdCounter = SAL_CONST_INT64(5000000000);
        sTmpStr += OUString::number(nIdCounter++);
    }
    else
    {
        // Value of xml:id in element <text:list> has to be a valid ID type (#i92478#)
        sal_Int64 n = ::tools::Time( ::tools::Time::SYSTEM ).GetTime();
        n += Date( Date::SYSTEM ).GetDate();
        n += comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max());
        // Value of xml:id in element <text:list> has to be a valid ID type (#i92478#)
        sTmpStr += OUString::number( n );
    }

    OUString sNewListId( sTmpStr );
    if ( mpProcessedLists != nullptr )
    {
        long nHitCount = 0;
        while ( mpProcessedLists->find( sNewListId ) != mpProcessedLists->end() )
        {
            ++nHitCount;
            sNewListId = sTmpStr;
            sNewListId += OUString::number( nHitCount );
        }
    }

    return sNewListId;
}

// Provide list id for a certain list block for import (#i92811#)
OUString XMLTextListsHelper::GetListIdForListBlock( XMLTextListBlockContext& rListBlock )
{
    OUString sListBlockListId( rListBlock.GetContinueListId() );
    if ( sListBlockListId.isEmpty() )
    {
        sListBlockListId = rListBlock.GetListId();
    }

    if ( mpMapListIdToListStyleDefaultListId != nullptr )
    {
        if ( !sListBlockListId.isEmpty() )
        {
            const OUString sListStyleName =
                                GetListStyleOfProcessedList( sListBlockListId );

            tMapForLists::const_iterator aIter =
                    mpMapListIdToListStyleDefaultListId->find( sListStyleName );
            if ( aIter != mpMapListIdToListStyleDefaultListId->end() )
            {
                if ( (*aIter).second.first == sListBlockListId )
                {
                    sListBlockListId = (*aIter).second.second;
                }
            }
        }
    }

    return sListBlockListId;
}

void XMLTextListsHelper::StoreLastContinuingList( const OUString& sListId,
                                                  const OUString& sContinuingListId )
{
    if ( mpContinuingLists == nullptr )
    {
        mpContinuingLists = new tMapForContinuingLists();
    }

    (*mpContinuingLists)[ sListId ] = sContinuingListId;
}

OUString XMLTextListsHelper::GetLastContinuingListId(
                                                const OUString& sListId ) const
{
    if ( mpContinuingLists != nullptr)
    {
        tMapForContinuingLists::const_iterator aIter =
                                                mpContinuingLists->find( sListId );
        if ( aIter != mpContinuingLists->end() )
        {
            return (*aIter).second;
        }
    }

    return sListId;
}

void XMLTextListsHelper::PushListOnStack( const OUString& sListId,
                                          const OUString& sListStyleName )
{
    if ( mpListStack == nullptr )
    {
        mpListStack = new tStackForLists();
    }
    ::std::pair< OUString, OUString >
                                aListData( sListId, sListStyleName );
    mpListStack->push_back( aListData );
}
void XMLTextListsHelper::PopListFromStack()
{
    if ( mpListStack != nullptr &&
         mpListStack->size() > 0 )
    {
        mpListStack->pop_back();
    }
}

bool XMLTextListsHelper::EqualsToTopListStyleOnStack( const OUString& sListId ) const
{
    return mpListStack != nullptr && sListId == mpListStack->back().second;
}

OUString
XMLTextListsHelper::GetNumberedParagraphListId(
    const sal_uInt16 i_Level,
    const OUString& i_StyleName)
{
    if (i_StyleName.isEmpty()) {
        SAL_INFO("xmloff.text", "invalid numbered-paragraph: no style-name");
    }
    if (!i_StyleName.isEmpty()
        && (i_Level < mLastNumberedParagraphs.size())
        && (mLastNumberedParagraphs[i_Level].first == i_StyleName) )
    {
        assert(!mLastNumberedParagraphs[i_Level].second.isEmpty() &&
            "internal error: numbered-paragraph style-name but no list-id?");
        return mLastNumberedParagraphs[i_Level].second;
    } else {
        return GenerateNewListId();
    }
}

static void
ClampLevel(uno::Reference<container::XIndexReplace> const& i_xNumRules,
    sal_Int16 & io_rLevel)
{
    assert(i_xNumRules.is());
    if (i_xNumRules.is()) {
        const sal_Int32 nLevelCount( i_xNumRules->getCount() );
        if ( io_rLevel >= nLevelCount ) {
            io_rLevel = sal::static_int_cast< sal_Int16 >(nLevelCount-1);
        }
    }
}

uno::Reference<container::XIndexReplace>
XMLTextListsHelper::EnsureNumberedParagraph(
    SvXMLImport & i_rImport,
    const OUString& i_ListId,
    sal_Int16 & io_rLevel, const OUString& i_StyleName)
{
    assert(!i_ListId.isEmpty());
    assert(io_rLevel >= 0);
    NumParaList_t & rNPList( mNPLists[i_ListId] );
    const OUString none; // default
    if ( rNPList.empty() ) {
        // create default list style for top level
        sal_Int16 lev(0);
        rNPList.push_back(::std::make_pair(none,
            MakeNumRule(i_rImport, nullptr, none, none, lev) ));
    }
    // create num rule first because this might clamp the level...
    uno::Reference<container::XIndexReplace> xNumRules;
    if ((0 == io_rLevel) || rNPList.empty() || !i_StyleName.isEmpty()) {
        // no parent to inherit from, or explicit style given => new numrules!
        // index of parent: level - 1, but maybe that does not exist
        const size_t parent( std::min(static_cast<size_t>(io_rLevel),
            rNPList.size()) - 1 );
        xNumRules = MakeNumRule(i_rImport,
            io_rLevel > 0 ? rNPList[parent].second : nullptr,
            io_rLevel > 0 ? rNPList[parent].first  : none,
            i_StyleName, io_rLevel);
    } else {
        // no style given, but has a parent => reuse parent numrules!
        ClampLevel(rNPList.back().second, io_rLevel);
    }
    if (static_cast<sal_uInt16>(io_rLevel) + 1U > rNPList.size()) {
        // new level: need to enlarge
        for (size_t i = rNPList.size();
                i < static_cast<size_t>(io_rLevel); ++i)
        {
            NumParaList_t::value_type const rule(rNPList.back());
            rNPList.push_back(rule);
        }
        NumParaList_t::value_type const rule(rNPList.back());
        rNPList.push_back(xNumRules.is()
            ? ::std::make_pair(i_StyleName, xNumRules)
            : rule);
    } else {
        // old level: no need to enlarge; possibly shrink
        if (xNumRules.is()) {
            rNPList[io_rLevel] = std::make_pair(i_StyleName, xNumRules);
        }
        if (static_cast<sal_uInt16>(io_rLevel) + 1U < rNPList.size()) {
            rNPList.erase(rNPList.begin() + io_rLevel + 1, rNPList.end());
        }
    }
    // remember the list id
    if (mLastNumberedParagraphs.size() <= static_cast<size_t>(io_rLevel)) {
        mLastNumberedParagraphs.resize(io_rLevel+1);
    }
    mLastNumberedParagraphs[io_rLevel] = std::make_pair(i_StyleName, i_ListId);
    return rNPList.back().second;
}

/** extracted from the XMLTextListBlockContext constructor */
uno::Reference<container::XIndexReplace>
XMLTextListsHelper::MakeNumRule(
    SvXMLImport & i_rImport,
    const uno::Reference<container::XIndexReplace>& i_rNumRule,
    const OUString& i_ParentStyleName,
    const OUString& i_StyleName,
    sal_Int16 & io_rLevel,
    bool* o_pRestartNumbering,
    bool* io_pSetDefaults)
{
    static const char s_NumberingRules[] = "NumberingRules";
    uno::Reference<container::XIndexReplace> xNumRules(i_rNumRule);
    if ( !i_StyleName.isEmpty() && i_StyleName != i_ParentStyleName )
    {
        const OUString sDisplayStyleName(
            i_rImport.GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_LIST,
                                             i_StyleName) );
        const uno::Reference < container::XNameContainer >& rNumStyles(
                            i_rImport.GetTextImport()->GetNumberingStyles() );
        if( rNumStyles.is() && rNumStyles->hasByName( sDisplayStyleName ) )
        {
            uno::Reference < style::XStyle > xStyle;
            uno::Any any = rNumStyles->getByName( sDisplayStyleName );
            any >>= xStyle;

            uno::Reference< beans::XPropertySet > xPropSet( xStyle,
                uno::UNO_QUERY );
            any = xPropSet->getPropertyValue(s_NumberingRules);
            any >>= xNumRules;
        }
        else
        {
            const SvxXMLListStyleContext *pListStyle(
                i_rImport.GetTextImport()->FindAutoListStyle( i_StyleName ) );
            if( pListStyle )
            {
                xNumRules = pListStyle->GetNumRules();
                if( !xNumRules.is() )
                {
                    pListStyle->CreateAndInsertAuto();
                    xNumRules = pListStyle->GetNumRules();
                }
            }
        }
    }

    bool bSetDefaults(io_pSetDefaults && *io_pSetDefaults);
    if ( !xNumRules.is() )
    {
        // If no style name has been specified for this style and for any
        // parent or if no num rule with the specified name exists,
        // create a new one.

        xNumRules =
            SvxXMLListStyleContext::CreateNumRule( i_rImport.GetModel() );
        assert(xNumRules.is());
        if ( !xNumRules.is() )
            return xNumRules;

        // Because it is a new num rule, numbering must not be restarted.
        if (o_pRestartNumbering) *o_pRestartNumbering = false;
        bSetDefaults = true;
        if (io_pSetDefaults) *io_pSetDefaults = bSetDefaults;
    }

    ClampLevel(xNumRules, io_rLevel);

    if ( bSetDefaults )
    {
        // Because there is no list style sheet for this style, a default
        // format must be set for any level of this num rule.
        SvxXMLListStyleContext::SetDefaultStyle( xNumRules, io_rLevel,
            false );
    }

    return xNumRules;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
