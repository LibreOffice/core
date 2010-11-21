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

#include "precompiled_xmloff.hxx"

#include <txtlists.hxx>

#include <tools/debug.hxx>
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
   :  mpProcessedLists( 0 ),
      msLastProcessedListId(),
      msListStyleOfLastProcessedList(),
      // Inconsistent behavior regarding lists (#i92811#)
      mpMapListIdToListStyleDefaultListId( 0 ),
      mpContinuingLists( 0 ),
      mpListStack( 0 )
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
//    fprintf(stderr, "PushListContext\n");
    mListStack.push(::boost::make_tuple(i_pListBlock,
        static_cast<XMLTextListItemContext*>(0),
        static_cast<XMLNumberedParaContext*>(0)));
}

void XMLTextListsHelper::PushListContext(
    XMLNumberedParaContext *i_pNumberedParagraph)
{
//    fprintf(stderr, "PushListContext(NP)\n");
    mListStack.push(::boost::make_tuple(
        static_cast<XMLTextListBlockContext*>(0),
        static_cast<XMLTextListItemContext*>(0), i_pNumberedParagraph));
}

void XMLTextListsHelper::PopListContext()
{
    OSL_ENSURE(mListStack.size(),
        "internal error: PopListContext: mListStack empty");
//    fprintf(stderr, "PopListContext\n");
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
        OSL_ENSURE(mListStack.size(),
            "internal error: SetListItem: mListStack empty");
        OSL_ENSURE(mListStack.top().get<0>(),
            "internal error: SetListItem: mListStack has no ListBlock");
        OSL_ENSURE(!mListStack.top().get<1>(),
            "error: SetListItem: list item already exists");
    }
    if ( !mListStack.empty() ) {
        mListStack.top().get<1>() = i_pListItem;
    }
}

// Handling for parameter <sListStyleDefaultListId> (#i92811#)
void XMLTextListsHelper::KeepListAsProcessed( ::rtl::OUString sListId,
                                              ::rtl::OUString sListStyleName,
                                              ::rtl::OUString sContinueListId,
                                              ::rtl::OUString sListStyleDefaultListId )
{
    if ( IsListProcessed( sListId ) )
    {
        DBG_ASSERT( false,
                    "<XMLTextListsHelper::KeepListAsProcessed(..)> - list id already added" );
        return;
    }

    if ( mpProcessedLists == 0 )
    {
        mpProcessedLists = new tMapForLists();
    }

    ::std::pair< ::rtl::OUString, ::rtl::OUString >
                                aListData( sListStyleName, sContinueListId );
    (*mpProcessedLists)[ sListId ] = aListData;

    msLastProcessedListId = sListId;
    msListStyleOfLastProcessedList = sListStyleName;

    // Inconsistent behavior regarding lists (#i92811#)
    if ( sListStyleDefaultListId.getLength() != 0 )
    {
        if ( mpMapListIdToListStyleDefaultListId == 0 )
        {
            mpMapListIdToListStyleDefaultListId = new tMapForLists();
        }

        if ( mpMapListIdToListStyleDefaultListId->find( sListStyleName ) ==
                                mpMapListIdToListStyleDefaultListId->end() )
        {
            ::std::pair< ::rtl::OUString, ::rtl::OUString >
                                aListIdMapData( sListId, sListStyleDefaultListId );
            (*mpMapListIdToListStyleDefaultListId)[ sListStyleName ] =
                                                                aListIdMapData;
        }
    }
}

sal_Bool XMLTextListsHelper::IsListProcessed( const ::rtl::OUString sListId ) const
{
    if ( mpProcessedLists == 0 )
    {
        return sal_False;
    }

    return mpProcessedLists->find( sListId ) != mpProcessedLists->end();
}

::rtl::OUString XMLTextListsHelper::GetListStyleOfProcessedList(
                                            const ::rtl::OUString sListId ) const
{
    if ( mpProcessedLists != 0 )
    {
        tMapForLists::const_iterator aIter = mpProcessedLists->find( sListId );
        if ( aIter != mpProcessedLists->end() )
        {
            return (*aIter).second.first;
        }
    }

    return ::rtl::OUString();
}

::rtl::OUString XMLTextListsHelper::GetContinueListIdOfProcessedList(
                                            const ::rtl::OUString sListId ) const
{
    if ( mpProcessedLists != 0 )
    {
        tMapForLists::const_iterator aIter = mpProcessedLists->find( sListId );
        if ( aIter != mpProcessedLists->end() )
        {
            return (*aIter).second.second;
        }
    }

    return ::rtl::OUString();
}

const ::rtl::OUString& XMLTextListsHelper::GetLastProcessedListId() const
{
    return msLastProcessedListId;
}

const ::rtl::OUString& XMLTextListsHelper::GetListStyleOfLastProcessedList() const
{
    return msListStyleOfLastProcessedList;
}

::rtl::OUString XMLTextListsHelper::GenerateNewListId() const
{
    // Value of xml:id in element <text:list> has to be a valid ID type (#i92478#)
    ::rtl::OUString sTmpStr( RTL_CONSTASCII_USTRINGPARAM( "list" ) );
    sal_Int64 n = Time().GetTime();
    n += Date().GetDate();
    n += rand();
    // Value of xml:id in element <text:list> has to be a valid ID type (#i92478#)
    sTmpStr += ::rtl::OUString::valueOf( n );

    long nHitCount = 0;
    ::rtl::OUString sNewListId( sTmpStr );
    if ( mpProcessedLists != 0 )
    {
        while ( mpProcessedLists->find( sNewListId ) != mpProcessedLists->end() )
        {
            ++nHitCount;
            sNewListId = sTmpStr;
            sNewListId += ::rtl::OUString::valueOf( nHitCount );
        }
    }

    return sNewListId;
}

// Provide list id for a certain list block for import (#i92811#)
::rtl::OUString XMLTextListsHelper::GetListIdForListBlock( XMLTextListBlockContext& rListBlock )
{
    ::rtl::OUString sListBlockListId( rListBlock.GetContinueListId() );
    if ( sListBlockListId.getLength() == 0 )
    {
        sListBlockListId = rListBlock.GetListId();
    }

    if ( mpMapListIdToListStyleDefaultListId != 0 )
    {
        if ( sListBlockListId.getLength() != 0 )
        {
            const ::rtl::OUString sListStyleName =
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

void XMLTextListsHelper::StoreLastContinuingList( ::rtl::OUString sListId,
                                                  ::rtl::OUString sContinuingListId )
{
    if ( mpContinuingLists == 0 )
    {
        mpContinuingLists = new tMapForContinuingLists();
    }

    (*mpContinuingLists)[ sListId ] = sContinuingListId;
}

::rtl::OUString XMLTextListsHelper::GetLastContinuingListId(
                                                ::rtl::OUString sListId ) const
{
    if ( mpContinuingLists != 0)
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

void XMLTextListsHelper::PushListOnStack( ::rtl::OUString sListId,
                                          ::rtl::OUString sListStyleName )
{
    if ( mpListStack == 0 )
    {
        mpListStack = new tStackForLists();
    }
    ::std::pair< ::rtl::OUString, ::rtl::OUString >
                                aListData( sListId, sListStyleName );
    mpListStack->push_back( aListData );
}
void XMLTextListsHelper::PopListFromStack()
{
    if ( mpListStack != 0 &&
         mpListStack->size() > 0 )
    {
        mpListStack->pop_back();
    }
}

sal_Bool XMLTextListsHelper::EqualsToTopListStyleOnStack( const ::rtl::OUString sListId ) const
{
    return mpListStack != 0
           ? sListId == mpListStack->back().second
           : sal_False;
}

::rtl::OUString
XMLTextListsHelper::GetNumberedParagraphListId(
    const sal_uInt16 i_Level,
    const ::rtl::OUString i_StyleName)
{
    if (!i_StyleName.getLength()) {
        OSL_ENSURE(false, "invalid numbered-paragraph: no style-name");
    }
    if (i_StyleName.getLength()
        && (i_Level < mLastNumberedParagraphs.size())
        && (mLastNumberedParagraphs[i_Level].first == i_StyleName) )
    {
        OSL_ENSURE(mLastNumberedParagraphs[i_Level].second.getLength(),
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
    OSL_ENSURE(i_xNumRules.is(), "internal error: ClampLevel: NumRules null");
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
    const ::rtl::OUString i_ListId,
    sal_Int16 & io_rLevel, const ::rtl::OUString i_StyleName)
{
    OSL_ENSURE(i_ListId.getLength(), "inavlid ListId");
    OSL_ENSURE(io_rLevel >= 0, "inavlid Level");
    NumParaList_t & rNPList( mNPLists[i_ListId] );
    const ::rtl::OUString none; // default
    if ( rNPList.empty() && (0 != io_rLevel)) {
        // create default list style for top level
        sal_Int16 lev(0);
        rNPList.push_back(::std::make_pair(none,
            MakeNumRule(i_rImport, 0, none, none, lev) ));
    }
    // create num rule first because this might clamp the level...
    uno::Reference<container::XIndexReplace> xNumRules;
    if ((0 == io_rLevel) || rNPList.empty() || i_StyleName.getLength()) {
        // no parent to inherit from, or explicit style given => new numrules!
        // index of parent: level - 1, but maybe that does not exist
        const size_t parent( std::min(static_cast<size_t>(io_rLevel),
            rNPList.size()) - 1 );
        xNumRules = MakeNumRule(i_rImport,
            io_rLevel > 0 ? rNPList[parent].second : 0,
            io_rLevel > 0 ? rNPList[parent].first  : none,
            i_StyleName, io_rLevel);
    } else {
        // no style given, but has a parent => reuse parent numrules!
        ClampLevel(rNPList.back().second, io_rLevel);
    }
    if (static_cast<sal_uInt16>(io_rLevel) + 1U > rNPList.size()) {
        // new level: need to enlarge
        for (size_t i = rNPList.size();
                i < static_cast<size_t>(io_rLevel); ++i) {
            rNPList.push_back(rNPList.back());
        }
        rNPList.push_back(xNumRules.is()
            ? ::std::make_pair(i_StyleName, xNumRules)
            : rNPList.back());
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
    const ::rtl::OUString i_ParentStyleName,
    const ::rtl::OUString i_StyleName,
    sal_Int16 & io_rLevel,
    sal_Bool* o_pRestartNumbering,
    sal_Bool* io_pSetDefaults)
{
    static ::rtl::OUString s_NumberingRules(
        RTL_CONSTASCII_USTRINGPARAM("NumberingRules"));
    uno::Reference<container::XIndexReplace> xNumRules(i_rNumRule);
    if ( i_StyleName.getLength() &&
         i_StyleName != i_ParentStyleName )
    {
        const ::rtl::OUString sDisplayStyleName(
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

    sal_Bool bSetDefaults(io_pSetDefaults ? *io_pSetDefaults : sal_False);
    if ( !xNumRules.is() )
    {
        // If no style name has been specified for this style and for any
        // parent or if no num rule with the specified name exists,
        // create a new one.

        xNumRules =
            SvxXMLListStyleContext::CreateNumRule( i_rImport.GetModel() );
        DBG_ASSERT( xNumRules.is(), "got no numbering rule" );
        if ( !xNumRules.is() )
            return xNumRules;

        // Because it is a new num rule, numbering must not be restarted.
        if (o_pRestartNumbering) *o_pRestartNumbering = sal_False;
        bSetDefaults = sal_True;
        if (io_pSetDefaults) *io_pSetDefaults = bSetDefaults;
    }

    ClampLevel(xNumRules, io_rLevel);

    if ( bSetDefaults )
    {
        // Because there is no list style sheet for this style, a default
        // format must be set for any level of this num rule.
        SvxXMLListStyleContext::SetDefaultStyle( xNumRules, io_rLevel,
            sal_False );
    }

    return xNumRules;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
