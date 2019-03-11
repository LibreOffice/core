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

#ifdef UNX
#include <pwd.h>
#include <sys/types.h>
#endif

#include <svtools/inettbc.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XCommandProcessor2.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/SortedDynamicResultSetFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <salhelper/thread.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <unotools/historyoptions.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>
#include <svtools/asynclink.hxx>
#include <svl/urlfilter.hxx>

#include <vector>
#include <algorithm>

using namespace ::ucbhelper;
using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;

class SvtURLBox_Impl
{
public:
    std::vector<OUString>      aURLs;
    std::vector<OUString>      aCompletions;
    std::vector<WildCard>      m_aFilters;

    static bool TildeParsing( OUString& aText, OUString& aBaseUrl );

    SvtURLBox_Impl( )
    {
        FilterMatch::createWildCardFilterList(OUString(),m_aFilters);
    }
};

class SvtMatchContext_Impl: public salhelper::Thread
{
    static ::osl::Mutex*            pDirMutex;

    std::vector<OUString>           aPickList;
    std::vector<OUString>           aCompletions;
    std::vector<OUString>           aURLs;
    svtools::AsynchronLink          aLink;
    OUString const                  aBaseURL;
    OUString const                  aText;
    VclPtr<SvtURLBox>               pBox;
    bool const                      bOnlyDirectories;
    bool const                      bNoSelection;

    osl::Mutex mutex_;
    bool stopped_;
    css::uno::Reference< css::ucb::XCommandProcessor > processor_;
    sal_Int32 commandId_;

    DECL_LINK(                Select_Impl, void*, void );

    virtual                         ~SvtMatchContext_Impl() override;
    virtual void                    execute() override;
    void                            doExecute();
    void                            Insert( const OUString& rCompletion, const OUString& rURL, bool bForce = false);
    void                            ReadFolder( const OUString& rURL, const OUString& rMatch, bool bSmart );
    static void                     FillPicklist(std::vector<OUString>& rPickList);

public:
                                    SvtMatchContext_Impl( SvtURLBox* pBoxP, const OUString& rText );
    void                            Stop();
};

class MatchContext_Impl: public salhelper::Thread
{
    static ::osl::Mutex*            pDirMutex;

    std::vector<OUString>           aPickList;
    std::vector<OUString>           aCompletions;
    std::vector<OUString>           aURLs;
    svtools::AsynchronLink          aLink;
    OUString const                  aText;
    URLBox*                         pBox;

    osl::Mutex mutex_;
    bool stopped_;
    css::uno::Reference< css::ucb::XCommandProcessor > processor_;
    sal_Int32 commandId_;

    DECL_LINK(                Select_Impl, void*, void );

    virtual                         ~MatchContext_Impl() override;
    virtual void                    execute() override;
    void                            doExecute();
    void                            Insert( const OUString& rCompletion, const OUString& rURL, bool bForce = false);
    void                            ReadFolder( const OUString& rURL, const OUString& rMatch, bool bSmart );
    static void                     FillPicklist(std::vector<OUString>& rPickList);

public:
                                    MatchContext_Impl( URLBox* pBoxP, const OUString& rText );
    void                            Stop();
};


namespace
{
    struct theSvtMatchContextMutex
        : public rtl::Static< ::osl::Mutex, theSvtMatchContextMutex > {};
}

SvtMatchContext_Impl::SvtMatchContext_Impl(
    SvtURLBox* pBoxP, const OUString& rText )
    : Thread( "SvtMatchContext_Impl" )
    , aLink( LINK( this, SvtMatchContext_Impl, Select_Impl ) )
    , aBaseURL( pBoxP->aBaseURL )
    , aText( rText )
    , pBox( pBoxP )
    , bOnlyDirectories( pBoxP->bOnlyDirectories )
    , bNoSelection( pBoxP->bNoSelection )
    , stopped_(false)
    , commandId_(0)
{
    aLink.CreateMutex();

    FillPicklist( aPickList );
}

SvtMatchContext_Impl::~SvtMatchContext_Impl()
{
    aLink.ClearPendingCall();
}

void SvtMatchContext_Impl::FillPicklist(std::vector<OUString>& rPickList)
{
    // Read the history of picks
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( ePICKLIST );
    sal_uInt32 nCount = seqPicklist.getLength();

    for( sal_uInt32 nItem=0; nItem < nCount; nItem++ )
    {
        Sequence< PropertyValue > seqPropertySet = seqPicklist[ nItem ];

        OUString sTitle;
        INetURLObject aURL;

        sal_uInt32 nPropertyCount = seqPropertySet.getLength();

        for( sal_uInt32 nProperty=0; nProperty < nPropertyCount; nProperty++ )
        {
            if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_TITLE )
            {
                seqPropertySet[nProperty].Value >>= sTitle;
                aURL.SetURL( sTitle );
                rPickList.insert(rPickList.begin() + nItem, aURL.GetMainURL(INetURLObject::DecodeMechanism::WithCharset));
                break;
            }
        }
    }
}

void SvtMatchContext_Impl::Stop()
{
    css::uno::Reference< css::ucb::XCommandProcessor > proc;
    sal_Int32 id(0);
    {
        osl::MutexGuard g(mutex_);
        if (!stopped_) {
            stopped_ = true;
            proc = processor_;
            id = commandId_;
        }
    }
    if (proc.is()) {
        proc->abort(id);
    }
    terminate();
}

void SvtMatchContext_Impl::execute( )
{
    doExecute();
    aLink.Call( this );
}


// This method is called via AsynchronLink, so it has the SolarMutex and
// calling solar code ( VCL ... ) is safe. It is called when the thread is
// terminated ( finished work or stopped ). Cancelling the thread via
// Cancellable does not discard the information gained so far, it
// inserts all collected completions into the listbox.

IMPL_LINK_NOARG( SvtMatchContext_Impl, Select_Impl, void*, void )
{
    // avoid recursion through cancel button
    {
        osl::MutexGuard g(mutex_);
        if (stopped_) {
            // Completion was stopped, no display:
            return;
        }
    }

    pBox->bAutoCompleteMode = true;

    // did we filter completions which otherwise would have been valid?
    // (to be filled below)
    bool bValidCompletionsFiltered = false;

    // insert all completed strings into the listbox
    pBox->Clear();

    for (auto const& completion : aCompletions)
    {
        // convert the file into an URL
        OUString sURL;
        osl::FileBase::getFileURLFromSystemPath(completion, sURL);
            // note: if this doesn't work, we're not interested in: we're checking the
            // untouched sCompletion then

        if ( !sURL.isEmpty() && !sURL.endsWith("/") )
        {
            OUString sUpperURL( sURL.toAsciiUpperCase() );

            if ( ::std::none_of( pBox->pImpl->m_aFilters.begin(),
                                 pBox->pImpl->m_aFilters.end(),
                                 FilterMatch( sUpperURL ) ) )
            {   // this URL is not allowed
                bValidCompletionsFiltered = true;
                continue;
            }
        }

        pBox->InsertEntry(completion);
    }

    if( !bNoSelection && !aCompletions.empty() && !bValidCompletionsFiltered )
    {
        // select the first one
        OUString aTmp( pBox->GetEntry(0) );
        pBox->SetText( aTmp );
        pBox->SetSelection( Selection( aText.getLength(), aTmp.getLength() ) );
    }

    // transfer string lists to listbox and forget them
    pBox->pImpl->aURLs = aURLs;
    pBox->pImpl->aCompletions = aCompletions;
    aURLs.clear();
    aCompletions.clear();

    // force listbox to resize ( it may be open )
    pBox->Resize();

    // the box has this control as a member so we have to set that member
    // to zero before deleting ourself.
    pBox->pCtx.clear();
}


void SvtMatchContext_Impl::Insert( const OUString& rCompletion,
                                   const OUString& rURL,
                                   bool bForce )
{
    if( !bForce )
    {
        // avoid doubles
        if(find(aCompletions.begin(), aCompletions.end(), rCompletion) != aCompletions.end())
            return;
    }

    aCompletions.push_back(rCompletion);
    aURLs.push_back(rURL);
}


void SvtMatchContext_Impl::ReadFolder( const OUString& rURL,
                                       const OUString& rMatch,
                                       bool bSmart )
{
    // check folder to scan
    if( !UCBContentHelper::IsFolder( rURL ) )
        return;

    bool bPureHomePath = false;
#ifdef UNX
    bPureHomePath = aText.startsWith( "~" ) && aText.indexOf( '/' ) == -1;
#endif

    bool bExectMatch = bPureHomePath
                || aText == "."
                || aText.endsWith("/.")
                || aText.endsWith("/..");

    // for pure home paths ( ~username ) the '.' at the end of rMatch
    // means that it points to root catalog
    // this is done only for file contents since home paths parsing is useful only for them
    if ( bPureHomePath && rMatch == "file:///." )
    {
        // a home that refers to /

        OUString aNewText( aText );
        aNewText += "/";
        Insert( aNewText, rURL, true );

        return;
    }

    // string to match with
    INetURLObject aMatchObj( rMatch );
    OUString aMatchName;

    if ( rURL != aMatchObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) )
    {
        aMatchName = aMatchObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

        // matching is always done case insensitive, but completion will be case sensitive and case preserving
        aMatchName = aMatchName.toAsciiLowerCase();

        // if the matchstring ends with a slash, we must search for this also
        if ( rMatch.endsWith("/") )
            aMatchName += "/";
    }

    sal_Int32 nMatchLen = aMatchName.getLength();

    INetURLObject aFolderObj( rURL );
    DBG_ASSERT( aFolderObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );

    try
    {
        Content aCnt( aFolderObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                      new ::ucbhelper::CommandEnvironment( uno::Reference< XInteractionHandler >(),
                                                     uno::Reference< XProgressHandler >() ),
                      comphelper::getProcessComponentContext() );
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = "Title";
        pProps[1] = "IsFolder";

        try
        {
            uno::Reference< XDynamicResultSet > xDynResultSet;
            ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
            if ( bOnlyDirectories )
                eInclude = INCLUDE_FOLDERS_ONLY;

            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            uno::Reference < XAnyCompareFactory > xCompare;
            uno::Reference < XSortedDynamicResultSetFactory > xSRSFac =
                SortedDynamicResultSetFactory::create( ::comphelper::getProcessComponentContext() );

            Sequence< NumberedSortingInfo > aSortInfo( 2 );
            NumberedSortingInfo* pInfo = aSortInfo.getArray();
            pInfo[ 0 ].ColumnIndex = 2;
            pInfo[ 0 ].Ascending   = false;
            pInfo[ 1 ].ColumnIndex = 1;
            pInfo[ 1 ].Ascending   = true;

            uno::Reference< XDynamicResultSet > xDynamicResultSet;
            xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xCompare );

            if ( xDynamicResultSet.is() )
            {
                xResultSet = xDynamicResultSet->getStaticResultSet();
            }
        }
        catch( css::uno::Exception& ) {}

        if ( xResultSet.is() )
        {
            uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );
            uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            try
            {
                while ( schedule() && xResultSet->next() )
                {
                    OUString   aURL      = xContentAccess->queryContentIdentifierString();
                    OUString   aTitle    = xRow->getString(1);
                    bool   bIsFolder = xRow->getBoolean(2);

                    // matching is always done case insensitive, but completion will be case sensitive and case preserving
                    aTitle = aTitle.toAsciiLowerCase();

                    if (
                        !nMatchLen ||
                        (bExectMatch && aMatchName == aTitle) ||
                        (!bExectMatch && aTitle.startsWith(aMatchName))
                       )
                    {
                        // all names fit if matchstring is empty
                        INetURLObject aObj( aURL );
                        sal_Unicode aDelimiter = '/';
                        if ( bSmart )
                            // when parsing is done "smart", the delimiter must be "guessed"
                            aObj.getFSysPath( static_cast<FSysStyle>(FSysStyle::Detect & ~FSysStyle::Vos), &aDelimiter );

                        if ( bIsFolder )
                            aObj.setFinalSlash();

                        // get the last name of the URL
                        OUString aMatch = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
                        OUString aInput( aText );
                        if ( nMatchLen )
                        {
                            if (aText.endsWith(".") || bPureHomePath)
                            {
                                // if a "special folder" URL was typed, don't touch the user input
                                aMatch = aMatch.copy( nMatchLen );
                            }
                            else
                            {
                                // make the user input case preserving
                                DBG_ASSERT( aInput.getLength() >= nMatchLen, "Suspicious Matching!" );
                                aInput = aInput.copy( 0, aInput.getLength() - nMatchLen );
                            }
                        }

                        aInput += aMatch;

                        // folders should get a final slash automatically
                        if ( bIsFolder )
                            aInput += OUStringLiteral1(aDelimiter);

                        Insert( aInput, aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), true );
                    }
                }
            }
            catch( css::uno::Exception& )
            {
            }
        }
    }
    catch( css::uno::Exception& )
    {
    }
}

MatchContext_Impl::MatchContext_Impl(URLBox* pBoxP, const OUString& rText)
    : Thread( "MatchContext_Impl" )
    , aLink( LINK( this, MatchContext_Impl, Select_Impl ) )
    , aText( rText )
    , pBox( pBoxP )
    , stopped_(false)
    , commandId_(0)
{
    aLink.CreateMutex();

    FillPicklist( aPickList );
}

MatchContext_Impl::~MatchContext_Impl()
{
    aLink.ClearPendingCall();
}

void MatchContext_Impl::FillPicklist(std::vector<OUString>& rPickList)
{
    // Read the history of picks
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( ePICKLIST );
    sal_uInt32 nCount = seqPicklist.getLength();

    for( sal_uInt32 nItem=0; nItem < nCount; nItem++ )
    {
        Sequence< PropertyValue > seqPropertySet = seqPicklist[ nItem ];

        OUString sTitle;
        INetURLObject aURL;

        sal_uInt32 nPropertyCount = seqPropertySet.getLength();

        for( sal_uInt32 nProperty=0; nProperty < nPropertyCount; nProperty++ )
        {
            if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_TITLE )
            {
                seqPropertySet[nProperty].Value >>= sTitle;
                aURL.SetURL( sTitle );
                rPickList.insert(rPickList.begin() + nItem, aURL.GetMainURL(INetURLObject::DecodeMechanism::WithCharset));
                break;
            }
        }
    }
}

void MatchContext_Impl::Stop()
{
    css::uno::Reference< css::ucb::XCommandProcessor > proc;
    sal_Int32 id(0);
    {
        osl::MutexGuard g(mutex_);
        if (!stopped_) {
            stopped_ = true;
            proc = processor_;
            id = commandId_;
        }
    }
    if (proc.is()) {
        proc->abort(id);
    }
    terminate();
}

void MatchContext_Impl::execute( )
{
    doExecute();
    aLink.Call( this );
}


// This method is called via AsynchronLink, so it has the SolarMutex and
// calling solar code ( VCL ... ) is safe. It is called when the thread is
// terminated ( finished work or stopped ). Cancelling the thread via
// Cancellable does not discard the information gained so far, it
// inserts all collected completions into the listbox.

IMPL_LINK_NOARG( MatchContext_Impl, Select_Impl, void*, void )
{
    // avoid recursion through cancel button
    {
        osl::MutexGuard g(mutex_);
        if (stopped_) {
            // Completion was stopped, no display:
            return;
        }
    }

    // insert all completed strings into the listbox
    pBox->Clear();

    for (auto const& completion : aCompletions)
    {
        // convert the file into an URL
        OUString sURL;
        osl::FileBase::getFileURLFromSystemPath(completion, sURL);
            // note: if this doesn't work, we're not interested in: we're checking the
            // untouched sCompletion then

        if ( !sURL.isEmpty() && !sURL.endsWith("/") )
        {
            OUString sUpperURL( sURL.toAsciiUpperCase() );

            if ( ::std::none_of( pBox->pImpl->m_aFilters.begin(),
                                 pBox->pImpl->m_aFilters.end(),
                                 FilterMatch( sUpperURL ) ) )
            {   // this URL is not allowed
                continue;
            }
        }

        pBox->append_text(completion);
    }

    pBox->EnableAutocomplete();

    // transfer string lists to listbox and forget them
    pBox->pImpl->aURLs = aURLs;
    pBox->pImpl->aCompletions = aCompletions;
    aURLs.clear();
    aCompletions.clear();

    // the box has this control as a member so we have to set that member
    // to zero before deleting ourself.
    pBox->pCtx.clear();
}

void MatchContext_Impl::Insert( const OUString& rCompletion,
                                   const OUString& rURL,
                                   bool bForce )
{
    if( !bForce )
    {
        // avoid doubles
        if(find(aCompletions.begin(), aCompletions.end(), rCompletion) != aCompletions.end())
            return;
    }

    aCompletions.push_back(rCompletion);
    aURLs.push_back(rURL);
}


void MatchContext_Impl::ReadFolder( const OUString& rURL,
                                       const OUString& rMatch,
                                       bool bSmart )
{
    // check folder to scan
    if( !UCBContentHelper::IsFolder( rURL ) )
        return;

    bool bPureHomePath = false;
#ifdef UNX
    bPureHomePath = aText.startsWith( "~" ) && aText.indexOf( '/' ) == -1;
#endif

    bool bExectMatch = bPureHomePath
                || aText == "."
                || aText.endsWith("/.")
                || aText.endsWith("/..");

    // for pure home paths ( ~username ) the '.' at the end of rMatch
    // means that it points to root catalog
    // this is done only for file contents since home paths parsing is useful only for them
    if ( bPureHomePath && rMatch == "file:///." )
    {
        // a home that refers to /

        OUString aNewText( aText );
        aNewText += "/";
        Insert( aNewText, rURL, true );

        return;
    }

    // string to match with
    INetURLObject aMatchObj( rMatch );
    OUString aMatchName;

    if ( rURL != aMatchObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) )
    {
        aMatchName = aMatchObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

        // matching is always done case insensitive, but completion will be case sensitive and case preserving
        aMatchName = aMatchName.toAsciiLowerCase();

        // if the matchstring ends with a slash, we must search for this also
        if ( rMatch.endsWith("/") )
            aMatchName += "/";
    }

    sal_Int32 nMatchLen = aMatchName.getLength();

    INetURLObject aFolderObj( rURL );
    DBG_ASSERT( aFolderObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );

    try
    {
        Content aCnt( aFolderObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                      new ::ucbhelper::CommandEnvironment( uno::Reference< XInteractionHandler >(),
                                                     uno::Reference< XProgressHandler >() ),
                      comphelper::getProcessComponentContext() );
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = "Title";
        pProps[1] = "IsFolder";

        try
        {
            uno::Reference< XDynamicResultSet > xDynResultSet;

            xDynResultSet = aCnt.createDynamicCursor( aProps, INCLUDE_FOLDERS_AND_DOCUMENTS );

            uno::Reference < XAnyCompareFactory > xCompare;
            uno::Reference < XSortedDynamicResultSetFactory > xSRSFac =
                SortedDynamicResultSetFactory::create( ::comphelper::getProcessComponentContext() );

            Sequence< NumberedSortingInfo > aSortInfo( 2 );
            NumberedSortingInfo* pInfo = aSortInfo.getArray();
            pInfo[ 0 ].ColumnIndex = 2;
            pInfo[ 0 ].Ascending   = false;
            pInfo[ 1 ].ColumnIndex = 1;
            pInfo[ 1 ].Ascending   = true;

            uno::Reference< XDynamicResultSet > xDynamicResultSet;
            xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xCompare );

            if ( xDynamicResultSet.is() )
            {
                xResultSet = xDynamicResultSet->getStaticResultSet();
            }
        }
        catch( css::uno::Exception& ) {}

        if ( xResultSet.is() )
        {
            uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );
            uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            try
            {
                while ( schedule() && xResultSet->next() )
                {
                    OUString   aURL      = xContentAccess->queryContentIdentifierString();
                    OUString   aTitle    = xRow->getString(1);
                    bool   bIsFolder = xRow->getBoolean(2);

                    // matching is always done case insensitive, but completion will be case sensitive and case preserving
                    aTitle = aTitle.toAsciiLowerCase();

                    if (
                        !nMatchLen ||
                        (bExectMatch && aMatchName == aTitle) ||
                        (!bExectMatch && aTitle.startsWith(aMatchName))
                       )
                    {
                        // all names fit if matchstring is empty
                        INetURLObject aObj( aURL );
                        sal_Unicode aDelimiter = '/';
                        if ( bSmart )
                            // when parsing is done "smart", the delimiter must be "guessed"
                            aObj.getFSysPath( static_cast<FSysStyle>(FSysStyle::Detect & ~FSysStyle::Vos), &aDelimiter );

                        if ( bIsFolder )
                            aObj.setFinalSlash();

                        // get the last name of the URL
                        OUString aMatch = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
                        OUString aInput( aText );
                        if ( nMatchLen )
                        {
                            if (aText.endsWith(".") || bPureHomePath)
                            {
                                // if a "special folder" URL was typed, don't touch the user input
                                aMatch = aMatch.copy( nMatchLen );
                            }
                            else
                            {
                                // make the user input case preserving
                                DBG_ASSERT( aInput.getLength() >= nMatchLen, "Suspicious Matching!" );
                                aInput = aInput.copy( 0, aInput.getLength() - nMatchLen );
                            }
                        }

                        aInput += aMatch;

                        // folders should get a final slash automatically
                        if ( bIsFolder )
                            aInput += OUStringLiteral1(aDelimiter);

                        Insert( aInput, aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), true );
                    }
                }
            }
            catch( css::uno::Exception& )
            {
            }
        }
    }
    catch( css::uno::Exception& )
    {
    }
}

OUString SvtURLBox::ParseSmart( const OUString& _aText, const OUString& _aBaseURL )
{
    OUString aMatch;
    OUString aText = _aText;
    OUString aBaseURL = _aBaseURL;

    // parse ~ for Unix systems
    // does nothing for Windows
    if( !SvtURLBox_Impl::TildeParsing( aText, aBaseURL ) )
        return OUString();

    if( !aBaseURL.isEmpty() )
    {
        INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( aBaseURL );

        // if a base URL is set the string may be parsed relative
        if( aText.startsWith( "/" ) )
        {
            // text starting with slashes means absolute file URLs
            OUString aTemp = INetURLObject::GetScheme( eBaseProt );

            // file URL must be correctly encoded!
            OUString aTextURL = INetURLObject::encode( aText, INetURLObject::PART_FPATH,
                                                     INetURLObject::EncodeMechanism::All );
            aTemp += aTextURL;

            INetURLObject aTmp( aTemp );
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INetProtocol::NotValid )
                aMatch = aTmp.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }
        else
        {
            OUString aSmart( aText );
            INetURLObject aObj( aBaseURL );

            // HRO: I suppose this hack should only be done for Windows !!!???
#ifdef _WIN32
            // HRO: INetURLObject::smatRel2Abs does not recognize '\\' as a relative path
            //      but in case of "\\\\" INetURLObject is right - this is an absolute path !

            if( aText.startsWith("\\") && (aText.getLength() < 2 || aText[ 1 ] != '\\') )
            {
                // cut to first segment
                OUString aTmp = INetURLObject::GetScheme( eBaseProt );
                aTmp += "/";
                aTmp += aObj.getName( 0, true, INetURLObject::DecodeMechanism::WithCharset );
                aObj.SetURL( aTmp );

                aSmart = aSmart.copy(1);
            }
#endif
            // base URL must be a directory !
            aObj.setFinalSlash();

            // take base URL and append current input
            bool bWasAbsolute = false;
#ifdef UNX
            // encode file URL correctly
            aSmart = INetURLObject::encode( aSmart, INetURLObject::PART_FPATH, INetURLObject::EncodeMechanism::All );
#endif
            INetURLObject aTmp( aObj.smartRel2Abs( aSmart, bWasAbsolute ) );

            if ( aText.endsWith(".") )
                // INetURLObject appends a final slash for the directories "." and "..", this is a bug!
                // Remove it as a workaround
                aTmp.removeFinalSlash();
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INetProtocol::NotValid )
                aMatch = aTmp.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }
    }
    else
    {
        OUString aTmpMatch;
        osl::FileBase::getFileURLFromSystemPath( aText, aTmpMatch );
        aMatch = aTmpMatch;
    }

    return aMatch;
}

void SvtMatchContext_Impl::doExecute()
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );
    {
        // have we been stopped while we were waiting for the mutex?
        osl::MutexGuard g(mutex_);
        if (stopped_) {
            return;
        }
    }

    // Reset match lists
    aCompletions.clear();
    aURLs.clear();

    // check for input
    if ( aText.isEmpty() )
        return;

    if( aText.indexOf( '*' ) != -1 || aText.indexOf( '?' ) != -1 )
        // no autocompletion for wildcards
        return;

    OUString aMatch;
    INetProtocol eProt = INetURLObject::CompareProtocolScheme( aText );
    INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( aBaseURL );
    if ( aBaseURL.isEmpty() )
        eBaseProt = INetURLObject::CompareProtocolScheme( SvtPathOptions().GetWorkPath() );
    INetProtocol eSmartProt = pBox->GetSmartProtocol();

    // if the user input is a valid URL, go on with it
    // otherwise it could be parsed smart with a predefined smart protocol
    // ( or if this is not set with the protocol of a predefined base URL )
    if( eProt == INetProtocol::NotValid || eProt == eSmartProt || (eSmartProt == INetProtocol::NotValid && eProt == eBaseProt) )
    {
        // not stopped yet ?
        if( schedule() )
        {
            if ( eProt == INetProtocol::NotValid )
                aMatch = SvtURLBox::ParseSmart( aText, aBaseURL );
            else
                aMatch = aText;
            if ( !aMatch.isEmpty() )
            {
                INetURLObject aURLObject( aMatch );
                OUString aMainURL( aURLObject.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                // Disable autocompletion for anything but the (local) file
                // system (for which access is hopefully fast), as the logic of
                // how MatchContext_Impl is used requires this code to run to
                // completion before further user input is processed, and even
                // MatchContext_Impl::Stop does not guarantee a speedy
                // return:
                if ( !aMainURL.isEmpty()
                     && aURLObject.GetProtocol() == INetProtocol::File )
                {
                    // if text input is a directory, it must be part of the match list! Until then it is scanned
                    bool folder = false;
                    if (aURLObject.hasFinalSlash()) {
                        try {
                            css::uno::Reference< css::uno::XComponentContext >
                                ctx(comphelper::getProcessComponentContext());
                            css::uno::Reference<
                                css::ucb::XUniversalContentBroker > ucb(
                                    css::ucb::UniversalContentBroker::create(
                                        ctx));
                            css::uno::Sequence< css::beans::Property > prop(1);
                            prop[0].Name = "IsFolder";
                            prop[0].Handle = -1;
                            prop[0].Type = cppu::UnoType< bool >::get();
                            css::uno::Any res;
                            css::uno::Reference< css::ucb::XCommandProcessor >
                                proc(
                                    ucb->queryContent(
                                        ucb->createContentIdentifier(aMainURL)),
                                    css::uno::UNO_QUERY_THROW);
                            css::uno::Reference< css::ucb::XCommandProcessor2 >
                                proc2(proc, css::uno::UNO_QUERY);
                            sal_Int32 id = proc->createCommandIdentifier();
                            try {
                                {
                                    osl::MutexGuard g(mutex_);
                                    processor_ = proc;
                                    commandId_ = id;
                                }
                                res = proc->execute(
                                    css::ucb::Command(
                                        "getPropertyValues", -1,
                                        css::uno::makeAny(prop)),
                                    id,
                                    css::uno::Reference<
                                        css::ucb::XCommandEnvironment >());
                            } catch (...) {
                                if (proc2.is()) {
                                    try {
                                        proc2->releaseCommandIdentifier(id);
                                    } catch (css::uno::RuntimeException & e) {
                                        SAL_WARN("svtools.control", "ignoring " << e);
                                    }
                                }
                                throw;
                            }
                            if (proc2.is()) {
                                proc2->releaseCommandIdentifier(id);
                            }
                            {
                                osl::MutexGuard g(mutex_);
                                processor_.clear();
                                // At least the neon-based WebDAV UCP does not
                                // properly support aborting commands, so return
                                // anyway now if an abort request had been
                                // ignored and the command execution only
                                // returned "successfully" after some timeout:
                                if (stopped_) {
                                    return;
                                }
                            }
                            css::uno::Reference< css::sdbc::XRow > row(
                                res, css::uno::UNO_QUERY_THROW);
                            folder = row->getBoolean(1) && !row->wasNull();
                        } catch (css::uno::Exception & e) {
                            SAL_WARN("svtools.control", "ignoring " << e);
                            return;
                        }
                    }
                    if ( folder )
                        Insert( aText, aMatch );
                    else
                        // otherwise the parent folder will be taken
                        aURLObject.removeSegment();

                    // scan directory and insert all matches
                    ReadFolder( aURLObject.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aMatch, eProt == INetProtocol::NotValid );
                }
            }
        }
    }

    if ( bOnlyDirectories )
        // don't scan history picklist if only directories are allowed, picklist contains only files
        return;

    bool bFull = false;

    INetURLObject aCurObj;
    OUString aCurString, aCurMainURL;
    INetURLObject aObj;
    aObj.SetSmartProtocol( eSmartProt == INetProtocol::NotValid ? INetProtocol::Http : eSmartProt );
    for( ;; )
    {
        for(const auto& rPick : aPickList)
        {
            if (!schedule())
                break;

            aCurObj.SetURL(rPick);
            aCurObj.SetSmartURL( aCurObj.GetURLNoPass());
            aCurMainURL = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            if( eProt != INetProtocol::NotValid && aCurObj.GetProtocol() != eProt )
                continue;

            if( eSmartProt != INetProtocol::NotValid && aCurObj.GetProtocol() != eSmartProt )
                continue;

            switch( aCurObj.GetProtocol() )
            {
                case INetProtocol::Http:
                case INetProtocol::Https:
                case INetProtocol::Ftp:
                {
                    if( eProt == INetProtocol::NotValid && !bFull )
                    {
                        aObj.SetSmartURL( aText );
                        if( aObj.GetURLPath().getLength() > 1 )
                            continue;
                    }

                    aCurString = aCurMainURL;
                    if( eProt == INetProtocol::NotValid )
                    {
                        // try if text matches the scheme
                        OUString aScheme( INetURLObject::GetScheme( aCurObj.GetProtocol() ) );
                        if ( aScheme.startsWithIgnoreAsciiCase( aText ) && aText.getLength() < aScheme.getLength() )
                        {
                            if( bFull )
                                aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                            else
                            {
                                aCurObj.SetMark( "" );
                                aCurObj.SetParam( "" );
                                aCurObj.SetURLPath( "" );
                                aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                            }

                            Insert( aMatch, aMatch );
                        }

                        // now try smart matching
                        aCurString = aCurString.copy( aScheme.getLength() );
                    }

                    if( aCurString.startsWithIgnoreAsciiCase( aText ) )
                    {
                        if( bFull )
                            aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                        else
                        {
                            aCurObj.SetMark( "" );
                            aCurObj.SetParam( "" );
                            aCurObj.SetURLPath( "" );
                            aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                        }

                        OUString aURL( aMatch );
                        if( eProt == INetProtocol::NotValid )
                            aMatch = aMatch.copy( INetURLObject::GetScheme( aCurObj.GetProtocol() ).getLength() );

                        if( aText.getLength() < aMatch.getLength() )
                            Insert( aMatch, aURL );

                        continue;
                    }
                    break;
                }
                default:
                {
                    if( bFull )
                        continue;

                    if( aCurMainURL.startsWith(aText) )
                    {
                        if( aText.getLength() < aCurMainURL.getLength() )
                            Insert( aCurMainURL, aCurMainURL );

                        continue;
                    }
                    break;
                }
            }
        }

        if( !bFull )
            bFull = true;
        else
            break;
    }
}

void MatchContext_Impl::doExecute()
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );
    {
        // have we been stopped while we were waiting for the mutex?
        osl::MutexGuard g(mutex_);
        if (stopped_) {
            return;
        }
    }

    // Reset match lists
    aCompletions.clear();
    aURLs.clear();

    // check for input
    if ( aText.isEmpty() )
        return;

    if( aText.indexOf( '*' ) != -1 || aText.indexOf( '?' ) != -1 )
        // no autocompletion for wildcards
        return;

    OUString aMatch;
    INetProtocol eProt = INetURLObject::CompareProtocolScheme( aText );
    INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( pBox->aBaseURL );
    if ( pBox->aBaseURL.isEmpty() )
        eBaseProt = INetURLObject::CompareProtocolScheme( SvtPathOptions().GetWorkPath() );
    INetProtocol eSmartProt = pBox->GetSmartProtocol();

    // if the user input is a valid URL, go on with it
    // otherwise it could be parsed smart with a predefined smart protocol
    // ( or if this is not set with the protocol of a predefined base URL )
    if( eProt == INetProtocol::NotValid || eProt == eSmartProt || (eSmartProt == INetProtocol::NotValid && eProt == eBaseProt) )
    {
        // not stopped yet ?
        if( schedule() )
        {
            if ( eProt == INetProtocol::NotValid )
                aMatch = SvtURLBox::ParseSmart( aText, pBox->aBaseURL );
            else
                aMatch = aText;
            if ( !aMatch.isEmpty() )
            {
                INetURLObject aURLObject( aMatch );
                OUString aMainURL( aURLObject.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                // Disable autocompletion for anything but the (local) file
                // system (for which access is hopefully fast), as the logic of
                // how MatchContext_Impl is used requires this code to run to
                // completion before further user input is processed, and even
                // MatchContext_Impl::Stop does not guarantee a speedy
                // return:
                if ( !aMainURL.isEmpty()
                     && aURLObject.GetProtocol() == INetProtocol::File )
                {
                    // if text input is a directory, it must be part of the match list! Until then it is scanned
                    bool folder = false;
                    if (aURLObject.hasFinalSlash()) {
                        try {
                            css::uno::Reference< css::uno::XComponentContext >
                                ctx(comphelper::getProcessComponentContext());
                            css::uno::Reference<
                                css::ucb::XUniversalContentBroker > ucb(
                                    css::ucb::UniversalContentBroker::create(
                                        ctx));
                            css::uno::Sequence< css::beans::Property > prop(1);
                            prop[0].Name = "IsFolder";
                            prop[0].Handle = -1;
                            prop[0].Type = cppu::UnoType< bool >::get();
                            css::uno::Any res;
                            css::uno::Reference< css::ucb::XCommandProcessor >
                                proc(
                                    ucb->queryContent(
                                        ucb->createContentIdentifier(aMainURL)),
                                    css::uno::UNO_QUERY_THROW);
                            css::uno::Reference< css::ucb::XCommandProcessor2 >
                                proc2(proc, css::uno::UNO_QUERY);
                            sal_Int32 id = proc->createCommandIdentifier();
                            try {
                                {
                                    osl::MutexGuard g(mutex_);
                                    processor_ = proc;
                                    commandId_ = id;
                                }
                                res = proc->execute(
                                    css::ucb::Command(
                                        "getPropertyValues", -1,
                                        css::uno::makeAny(prop)),
                                    id,
                                    css::uno::Reference<
                                        css::ucb::XCommandEnvironment >());
                            } catch (...) {
                                if (proc2.is()) {
                                    try {
                                        proc2->releaseCommandIdentifier(id);
                                    } catch (css::uno::RuntimeException & e) {
                                        SAL_WARN("svtools.control", "ignoring " << e);
                                    }
                                }
                                throw;
                            }
                            if (proc2.is()) {
                                proc2->releaseCommandIdentifier(id);
                            }
                            {
                                osl::MutexGuard g(mutex_);
                                processor_.clear();
                                // At least the neon-based WebDAV UCP does not
                                // properly support aborting commands, so return
                                // anyway now if an abort request had been
                                // ignored and the command execution only
                                // returned "successfully" after some timeout:
                                if (stopped_) {
                                    return;
                                }
                            }
                            css::uno::Reference< css::sdbc::XRow > row(
                                res, css::uno::UNO_QUERY_THROW);
                            folder = row->getBoolean(1) && !row->wasNull();
                        } catch (css::uno::Exception & e) {
                            SAL_WARN("svtools.control", "ignoring " << e);
                            return;
                        }
                    }
                    if (folder)
                        Insert( aText, aMatch );
                    else
                        // otherwise the parent folder will be taken
                        aURLObject.removeSegment();

                    // scan directory and insert all matches
                    ReadFolder( aURLObject.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aMatch, eProt == INetProtocol::NotValid );
                }
            }
        }
    }

    bool bFull = false;

    INetURLObject aCurObj;
    OUString aCurString, aCurMainURL;
    INetURLObject aObj;
    aObj.SetSmartProtocol( eSmartProt == INetProtocol::NotValid ? INetProtocol::Http : eSmartProt );
    for( ;; )
    {
        for(const auto& rPick : aPickList)
        {
            if (!schedule())
                break;

            aCurObj.SetURL(rPick);
            aCurObj.SetSmartURL( aCurObj.GetURLNoPass());
            aCurMainURL = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            if( eProt != INetProtocol::NotValid && aCurObj.GetProtocol() != eProt )
                continue;

            if( eSmartProt != INetProtocol::NotValid && aCurObj.GetProtocol() != eSmartProt )
                continue;

            switch( aCurObj.GetProtocol() )
            {
                case INetProtocol::Http:
                case INetProtocol::Https:
                case INetProtocol::Ftp:
                {
                    if( eProt == INetProtocol::NotValid && !bFull )
                    {
                        aObj.SetSmartURL( aText );
                        if( aObj.GetURLPath().getLength() > 1 )
                            continue;
                    }

                    aCurString = aCurMainURL;
                    if( eProt == INetProtocol::NotValid )
                    {
                        // try if text matches the scheme
                        OUString aScheme( INetURLObject::GetScheme( aCurObj.GetProtocol() ) );
                        if ( aScheme.startsWithIgnoreAsciiCase( aText ) && aText.getLength() < aScheme.getLength() )
                        {
                            if( bFull )
                                aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                            else
                            {
                                aCurObj.SetMark( "" );
                                aCurObj.SetParam( "" );
                                aCurObj.SetURLPath( "" );
                                aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                            }

                            Insert( aMatch, aMatch );
                        }

                        // now try smart matching
                        aCurString = aCurString.copy( aScheme.getLength() );
                    }

                    if( aCurString.startsWithIgnoreAsciiCase( aText ) )
                    {
                        if( bFull )
                            aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                        else
                        {
                            aCurObj.SetMark( "" );
                            aCurObj.SetParam( "" );
                            aCurObj.SetURLPath( "" );
                            aMatch = aCurObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                        }

                        OUString aURL( aMatch );
                        if( eProt == INetProtocol::NotValid )
                            aMatch = aMatch.copy( INetURLObject::GetScheme( aCurObj.GetProtocol() ).getLength() );

                        if( aText.getLength() < aMatch.getLength() )
                            Insert( aMatch, aURL );

                        continue;
                    }
                    break;
                }
                default:
                {
                    if( bFull )
                        continue;

                    if( aCurMainURL.startsWith(aText) )
                    {
                        if( aText.getLength() < aCurMainURL.getLength() )
                            Insert( aCurMainURL, aCurMainURL );

                        continue;
                    }
                    break;
                }
            }
        }

        if( !bFull )
            bFull = true;
        else
            break;
    }
}

void SvtURLBox::TryAutoComplete()
{
    if( Application::AnyInput( VclInputFlags::KEYBOARD ) ) return;

    OUString aCurText = GetText();
    Selection aSelection( GetSelection() );
    if( aSelection.Max() != aCurText.getLength() )
        return;
    sal_uInt16 nLen = static_cast<sal_uInt16>(aSelection.Min());
    aCurText = aCurText.copy( 0, nLen );
    if( !aCurText.isEmpty() && bIsAutoCompleteEnabled )
    {
        if ( pCtx.is() )
        {
            pCtx->Stop();
            pCtx->join();
            pCtx.clear();
        }
        pCtx = new SvtMatchContext_Impl( this, aCurText );
        pCtx->launch();
    }
}


SvtURLBox::SvtURLBox( vcl::Window* pParent, INetProtocol eSmart, bool bSetDefaultHelpID )
    :   ComboBox( pParent , WB_DROPDOWN | WB_AUTOHSCROLL ),
        eSmartProtocol( eSmart ),
        bAutoCompleteMode( false ),
        bOnlyDirectories( false ),
        bHistoryDisabled( false ),
        bNoSelection( false ),
        bIsAutoCompleteEnabled( true )
{
    Init(bSetDefaultHelpID);

    if ( GetDesktopRectPixel().GetWidth() > 800 )
        SetSizePixel( Size( 300, 240 ) );
    else
        SetSizePixel( Size( 225, 240 ) );
}


SvtURLBox::SvtURLBox( vcl::Window* pParent, WinBits _nStyle, INetProtocol eSmart,
    bool bSetDefaultHelpID )
    :   ComboBox( pParent, _nStyle ),
        eSmartProtocol( eSmart ),
        bAutoCompleteMode( false ),
        bOnlyDirectories( false ),
        bHistoryDisabled( false ),
        bNoSelection( false ),
        bIsAutoCompleteEnabled( true )
{
    Init(bSetDefaultHelpID);
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSvtURLBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP|
                       WB_DROPDOWN|WB_AUTOHSCROLL;
    VclPtrInstance<SvtURLBox> pListBox(pParent, nWinBits, INetProtocol::NotValid, false);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}


void SvtURLBox::Init(bool bSetDefaultHelpID)
{
    pImpl.reset( new SvtURLBox_Impl );

    if (bSetDefaultHelpID && GetHelpId().isEmpty())
        SetHelpId( ".uno:OpenURL" );
    EnableAutocomplete( false );

    SetText( OUString() );

    GetSubEdit()->SetAutocompleteHdl(LINK(this, SvtURLBox, AutoCompleteHdl_Impl));
    UpdatePicklistForSmartProtocol_Impl();

    EnableAutoSize(true);
}

SvtURLBox::~SvtURLBox()
{
    disposeOnce();
}

void SvtURLBox::dispose()
{
    if( pCtx.is() )
    {
        pCtx->Stop();
        pCtx->join();
    }

    pImpl.reset();
    ComboBox::dispose();
}

void SvtURLBox::UpdatePickList( )
{
    if( pCtx.is() )
    {
        pCtx->Stop();
        pCtx->join();
        pCtx.clear();
    }

    OUString sText = GetText();
    if ( !sText.isEmpty() && bIsAutoCompleteEnabled )
    {
        pCtx = new SvtMatchContext_Impl( this, sText );
        pCtx->launch();
    }
}

void SvtURLBox::SetSmartProtocol( INetProtocol eProt )
{
    if ( eSmartProtocol != eProt )
    {
        eSmartProtocol = eProt;
        UpdatePicklistForSmartProtocol_Impl();
    }
}

void SvtURLBox::UpdatePicklistForSmartProtocol_Impl()
{
    Clear();
    if ( bHistoryDisabled )
        return;

    // read history pick list
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( ePICKLIST );
    sal_uInt32 nCount = seqPicklist.getLength();
    INetURLObject aCurObj;

    for( sal_uInt32 nItem=0; nItem < nCount; nItem++ )
    {
        Sequence< PropertyValue > seqPropertySet = seqPicklist[ nItem ];

        OUString sURL;

        sal_uInt32 nPropertyCount = seqPropertySet.getLength();

        for( sal_uInt32 nProperty=0; nProperty < nPropertyCount; nProperty++ )
        {
            if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_URL )
            {
                seqPropertySet[nProperty].Value >>= sURL;
                aCurObj.SetURL( sURL );

                if ( !sURL.isEmpty() && ( eSmartProtocol != INetProtocol::NotValid ) )
                {
                    if( aCurObj.GetProtocol() != eSmartProtocol )
                        break;
                }

                OUString aURL( aCurObj.GetMainURL( INetURLObject::DecodeMechanism::WithCharset ) );

                if ( !aURL.isEmpty() )
                {
                    bool bFound = aURL.endsWith("/");
                    if ( !bFound )
                    {
                        OUString aUpperURL( aURL );
                        aUpperURL = aUpperURL.toAsciiUpperCase();

                        bFound = ::std::any_of(pImpl->m_aFilters.begin(),
                                               pImpl->m_aFilters.end(),
                                               FilterMatch( aUpperURL ) );
                    }
                    if ( bFound )
                    {
                        OUString aFile;
                        if (osl::FileBase::getSystemPathFromFileURL(aURL, aFile) == osl::FileBase::E_None)
                            InsertEntry(aFile);
                        else
                            InsertEntry(aURL);
                    }
                }
                break;
            }
        }
    }
}


bool SvtURLBox::ProcessKey( const vcl::KeyCode& rKey )
{
    // every key input stops the current matching thread
    if( pCtx.is() )
    {
        pCtx->Stop();
        pCtx->join();
        pCtx.clear();
    }

    vcl::KeyCode aCode( rKey.GetCode() );
    if ( aCode == KEY_RETURN && !GetText().isEmpty() )
    {
        // wait for completion of matching thread
        ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

        if ( bAutoCompleteMode )
        {
            // reset picklist
            bAutoCompleteMode = false;
            Selection aSelection( GetSelection() );
            SetSelection( Selection( aSelection.Min(), aSelection.Min() ) );
            if ( bOnlyDirectories )
                Clear();
            else
                UpdatePicklistForSmartProtocol_Impl();
            Resize();
        }

        bool bHandled = false;
        if ( GetOpenHdl().IsSet() )
        {
            bHandled = true;
            GetOpenHdl().Call(this);
        }
        else if ( GetSelectHdl().IsSet() )
        {
            bHandled = true;
            GetSelectHdl().Call(*this);
        }

        ClearModifyFlag();
        return bHandled;
    }
    else if ( aCode == KEY_RETURN && GetText().isEmpty() && GetOpenHdl().IsSet() )
    {
        // for file dialog
        bAutoCompleteMode = false;
        GetOpenHdl().Call(this);
        return true;
    }
    else if( aCode == KEY_ESCAPE )
    {
        Selection aSelection( GetSelection() );
        if ( bAutoCompleteMode || aSelection.Min() != aSelection.Max() )
        {
            SetSelection( Selection( aSelection.Min(), aSelection.Min() ) );
            if ( bOnlyDirectories )
                Clear();
            else
                UpdatePicklistForSmartProtocol_Impl();
            Resize();
        }
        else
        {
           return false;
        }

        bAutoCompleteMode = false;
        return true;
    }
    else
    {
        return false;
    }
}


bool SvtURLBox::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetWindow() == GetSubEdit() && rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {

        const KeyEvent& rEvent = *rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKey = rEvent.GetKeyCode();
        vcl::KeyCode aCode( rKey.GetCode() );
        if( ProcessKey( rKey ) )
        {
            return true;
        }
        else if( ( aCode == KEY_UP || aCode == KEY_DOWN ) && !rKey.IsMod2() )
        {
            Selection aSelection( GetSelection() );
            sal_uInt16 nLen = static_cast<sal_uInt16>(aSelection.Min());
            GetSubEdit()->KeyInput( rEvent );
            SetSelection( Selection( nLen, GetText().getLength() ) );
            return true;
        }

        if ( MatchesPlaceHolder( GetText() ) )
        {
            // set the selection so a key stroke will overwrite
            // the placeholder rather than edit it
            SetSelection( Selection( 0, GetText().getLength() ) );
        }
    }

    return ComboBox::PreNotify( rNEvt );
}

IMPL_LINK_NOARG(SvtURLBox, AutoCompleteHdl_Impl, Edit&, void)
{
    TryAutoComplete();
}

bool SvtURLBox::EventNotify( NotifyEvent &rEvt )
{
    if ( MouseNotifyEvent::GETFOCUS == rEvt.GetType() )
    {
#ifndef UNX
        // pb: don't select automatically on unix #93251#
        SetSelection( Selection( 0, GetText().getLength() ) );
#endif
    }
    else if ( MouseNotifyEvent::LOSEFOCUS == rEvt.GetType() )
    {
        if( GetText().isEmpty() )
            ClearModifyFlag();
        if ( pCtx.is() )
        {
            pCtx->Stop();
            pCtx->join();
            pCtx.clear();
        }
    }

    return ComboBox::EventNotify( rEvt );
}


void SvtURLBox::Select()
{
    ComboBox::Select();
    ClearModifyFlag();
}


void SvtURLBox::SetOnlyDirectories( bool bDir )
{
    bOnlyDirectories = bDir;
    if ( bOnlyDirectories )
        Clear();
}


void SvtURLBox::SetNoURLSelection( bool bSet )
{
    bNoSelection = bSet;
}


OUString SvtURLBox::GetURL()
{
    // wait for end of autocompletion
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

    OUString aText( GetText() );
    if ( MatchesPlaceHolder( aText ) )
        return aPlaceHolder;

    // try to get the right case preserving URL from the list of URLs
    for(std::vector<OUString>::iterator i = pImpl->aCompletions.begin(), j = pImpl->aURLs.begin(); i != pImpl->aCompletions.end() && j != pImpl->aURLs.end(); ++i, ++j)
    {
        if((*i) == aText)
            return *j;
    }

#ifdef _WIN32
    // erase trailing spaces on Windows since thay are invalid on this OS and
    // most of the time they are inserted by accident via copy / paste
    aText = comphelper::string::stripEnd(aText, ' ');
    if ( aText.isEmpty() )
        return aText;
    // #i9739#
#endif

    INetURLObject aObj( aText );
    if( aText.indexOf( '*' ) != -1 || aText.indexOf( '?' ) != -1 )
    {
        // no autocompletion for wildcards
        INetURLObject aTempObj;
        if ( eSmartProtocol != INetProtocol::NotValid )
            aTempObj.SetSmartProtocol( eSmartProtocol );
        if ( aTempObj.SetSmartURL( aText ) )
            return aTempObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        else
            return aText;
    }

    if ( aObj.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aName = ParseSmart( aText, aBaseURL );
        aObj.SetURL(aName);
        OUString aURL( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        if ( aURL.isEmpty() )
            // aText itself is invalid, and even together with aBaseURL, it could not
            // made valid -> no chance
            return aText;

        bool bSlash = aObj.hasFinalSlash();
        {
            const OUString aPropName("CasePreservingURL");

            OUString aFileURL;

            Any aAny = UCBContentHelper::GetProperty(aURL, aPropName);
            bool success = (aAny >>= aFileURL);
            OUString aTitle;
            if(success)
                aTitle = INetURLObject(aFileURL).getName(
                             INetURLObject::LAST_SEGMENT,
                             true,
                             INetURLObject::DecodeMechanism::WithCharset );
            else
                success =
                    UCBContentHelper::GetTitle(aURL,&aTitle);

            if( success && aTitle != "/" && aTitle != "." )
            {
                    aObj.SetName( aTitle );
                    if ( bSlash )
                        aObj.setFinalSlash();
            }
        }
    }

    return aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

void SvtURLBox::DisableHistory()
{
    bHistoryDisabled = true;
    UpdatePicklistForSmartProtocol_Impl();
}

void SvtURLBox::SetBaseURL( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

    // Reset match lists
    pImpl->aCompletions.clear();
    pImpl->aURLs.clear();

    aBaseURL = rURL;
}

/** Parse leading ~ for Unix systems,
    does nothing for Windows
 */
bool SvtURLBox_Impl::TildeParsing(
    OUString&
#ifdef UNX
    aText
#endif
    , OUString&
#ifdef UNX
    aBaseURL
#endif
)
{
#ifdef UNX
    if( aText.startsWith( "~" ) )
    {
        OUString aParseTilde;
        bool bTrailingSlash = true; // use trailing slash

        if( aText.getLength() == 1 || aText[ 1 ] == '/' )
        {
            // covers "~" or "~/..." cases
            const char* aHomeLocation = getenv( "HOME" );
            if( !aHomeLocation )
                aHomeLocation = "";

            aParseTilde = OUString::createFromAscii(aHomeLocation);

            // in case the whole path is just "~" then there should
            // be no trailing slash at the end
            if( aText.getLength() == 1 )
                bTrailingSlash = false;
        }
        else
        {
            // covers "~username" and "~username/..." cases
            sal_Int32 nNameEnd = aText.indexOf( '/' );
            OUString aUserName = aText.copy( 1, ( nNameEnd != -1 ) ? nNameEnd : ( aText.getLength() - 1 ) );

            struct passwd* pPasswd = nullptr;
#ifdef __sun
            Sequence< sal_Int8 > sBuf( 1024 );
            struct passwd aTmp;
            sal_Int32 nRes = getpwnam_r( OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US ).getStr(),
                                  &aTmp,
                                  (char*)sBuf.getArray(),
                                  1024,
                                  &pPasswd );
            if( !nRes && pPasswd )
                aParseTilde = OUString::createFromAscii(pPasswd->pw_dir);
            else
                return false; // no such user
#else
            pPasswd = getpwnam( OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US ).getStr() );
            if( pPasswd )
                aParseTilde = OUString::createFromAscii(pPasswd->pw_dir);
            else
                return false; // no such user
#endif

            // in case the path is "~username" then there should
            // be no trailing slash at the end
            if( nNameEnd == -1 )
                bTrailingSlash = false;
        }

        if( !bTrailingSlash )
        {
            if( aParseTilde.isEmpty() || aParseTilde == "/" )
            {
                // "/" path should be converted to "/."
                aParseTilde = "/.";
            }
            else
            {
                // "blabla/" path should be converted to "blabla"
                aParseTilde = comphelper::string::stripEnd(aParseTilde, '/');
            }
        }
        else
        {
            if( !aParseTilde.endsWith("/") )
                aParseTilde += "/";
            if( aText.getLength() > 2 )
                aParseTilde += aText.copy( 2 );
        }

        aText = aParseTilde;
        aBaseURL.clear(); // tilde provide absolute path
    }
#endif

    return true;
}

void SvtURLBox::SetFilter(const OUString& _sFilter)
{
    pImpl->m_aFilters.clear();
    FilterMatch::createWildCardFilterList(_sFilter,pImpl->m_aFilters);
}

//--

OUString URLBox::ParseSmart( const OUString& _aText, const OUString& _aBaseURL )
{
    OUString aMatch;
    OUString aText = _aText;
    OUString aBaseURL = _aBaseURL;

    // parse ~ for Unix systems
    // does nothing for Windows
    if( !SvtURLBox_Impl::TildeParsing( aText, aBaseURL ) )
        return OUString();

    if( !aBaseURL.isEmpty() )
    {
        INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( aBaseURL );

        // if a base URL is set the string may be parsed relative
        if( aText.startsWith( "/" ) )
        {
            // text starting with slashes means absolute file URLs
            OUString aTemp = INetURLObject::GetScheme( eBaseProt );

            // file URL must be correctly encoded!
            OUString aTextURL = INetURLObject::encode( aText, INetURLObject::PART_FPATH,
                                                     INetURLObject::EncodeMechanism::All );
            aTemp += aTextURL;

            INetURLObject aTmp( aTemp );
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INetProtocol::NotValid )
                aMatch = aTmp.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }
        else
        {
            OUString aSmart( aText );
            INetURLObject aObj( aBaseURL );

            // HRO: I suppose this hack should only be done for Windows !!!???
#ifdef _WIN32
            // HRO: INetURLObject::smatRel2Abs does not recognize '\\' as a relative path
            //      but in case of "\\\\" INetURLObject is right - this is an absolute path !

            if( aText.startsWith("\\") && (aText.getLength() < 2 || aText[ 1 ] != '\\') )
            {
                // cut to first segment
                OUString aTmp = INetURLObject::GetScheme( eBaseProt );
                aTmp += "/";
                aTmp += aObj.getName( 0, true, INetURLObject::DecodeMechanism::WithCharset );
                aObj.SetURL( aTmp );

                aSmart = aSmart.copy(1);
            }
#endif
            // base URL must be a directory !
            aObj.setFinalSlash();

            // take base URL and append current input
            bool bWasAbsolute = false;
#ifdef UNX
            // encode file URL correctly
            aSmart = INetURLObject::encode( aSmart, INetURLObject::PART_FPATH, INetURLObject::EncodeMechanism::All );
#endif
            INetURLObject aTmp( aObj.smartRel2Abs( aSmart, bWasAbsolute ) );

            if ( aText.endsWith(".") )
                // INetURLObject appends a final slash for the directories "." and "..", this is a bug!
                // Remove it as a workaround
                aTmp.removeFinalSlash();
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INetProtocol::NotValid )
                aMatch = aTmp.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }
    }
    else
    {
        OUString aTmpMatch;
        osl::FileBase::getFileURLFromSystemPath( aText, aTmpMatch );
        aMatch = aTmpMatch;
    }

    return aMatch;
}

IMPL_LINK_NOARG(URLBox, TryAutoComplete, Timer *, void)
{
    OUString aCurText = m_xWidget->get_active_text();
    int nStartPos, nEndPos;
    m_xWidget->get_entry_selection_bounds(nStartPos, nEndPos);
    if (nEndPos != aCurText.getLength())
        return;
    aCurText = aCurText.copy(0, nStartPos);
    if (!aCurText.isEmpty())
    {
        if (pCtx.is())
        {
            pCtx->Stop();
            pCtx->join();
            pCtx.clear();
        }
        pCtx = new MatchContext_Impl(this, aCurText);
        pCtx->launch();
    }
    else
        m_xWidget->clear();
}

URLBox::URLBox(std::unique_ptr<weld::ComboBox> pWidget)
    : eSmartProtocol(INetProtocol::NotValid)
    , bHistoryDisabled(false)
    , m_xWidget(std::move(pWidget))
{
    Init();

    m_xWidget->connect_focus_in(LINK(this, URLBox, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, URLBox, FocusOutHdl));
    m_xWidget->connect_changed(LINK(this, URLBox, ChangedHdl));

    aChangedIdle.SetInvokeHandler(LINK(this, URLBox, TryAutoComplete));
    aChangedIdle.SetDebugName("svtools::URLBox aChangedIdle");
}

void URLBox::Init()
{
    pImpl.reset( new SvtURLBox_Impl );

    m_xWidget->set_entry_completion(false);

    UpdatePicklistForSmartProtocol_Impl();
}

URLBox::~URLBox()
{
    if (pCtx.is())
    {
        pCtx->Stop();
        pCtx->join();
    }
}

void URLBox::SetSmartProtocol(INetProtocol eProt)
{
    if ( eSmartProtocol != eProt )
    {
        eSmartProtocol = eProt;
        UpdatePicklistForSmartProtocol_Impl();
    }
}

void URLBox::UpdatePicklistForSmartProtocol_Impl()
{
    m_xWidget->clear();
    if ( bHistoryDisabled )
        return;

    if (bHistoryDisabled)
        return;

    // read history pick list
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( ePICKLIST );
    sal_uInt32 nCount = seqPicklist.getLength();
    INetURLObject aCurObj;

    for( sal_uInt32 nItem=0; nItem < nCount; nItem++ )
    {
        Sequence< PropertyValue > seqPropertySet = seqPicklist[ nItem ];

        OUString sURL;

        sal_uInt32 nPropertyCount = seqPropertySet.getLength();

        for( sal_uInt32 nProperty=0; nProperty < nPropertyCount; nProperty++ )
        {
            if( seqPropertySet[nProperty].Name == HISTORY_PROPERTYNAME_URL )
            {
                seqPropertySet[nProperty].Value >>= sURL;
                aCurObj.SetURL( sURL );

                if ( !sURL.isEmpty() && ( eSmartProtocol != INetProtocol::NotValid ) )
                {
                    if( aCurObj.GetProtocol() != eSmartProtocol )
                        break;
                }

                OUString aURL( aCurObj.GetMainURL( INetURLObject::DecodeMechanism::WithCharset ) );

                if ( !aURL.isEmpty() )
                {
                    bool bFound = aURL.endsWith("/");
                    if ( !bFound )
                    {
                        OUString aUpperURL( aURL );
                        aUpperURL = aUpperURL.toAsciiUpperCase();

                        bFound = ::std::any_of(pImpl->m_aFilters.begin(),
                                               pImpl->m_aFilters.end(),
                                               FilterMatch( aUpperURL ) );
                    }
                    if ( bFound )
                    {
                        OUString aFile;
                        if (osl::FileBase::getSystemPathFromFileURL(aURL, aFile) == osl::FileBase::E_None)
                            m_xWidget->append_text(aFile);
                        else
                            m_xWidget->append_text(aURL);
                    }
                }
                break;
            }
        }
    }
}

IMPL_LINK_NOARG(URLBox, ChangedHdl, weld::ComboBox&, void)
{
    aChangeHdl.Call(*m_xWidget);
    aChangedIdle.Start(); //launch this to happen on idle after cursor position will have been set
}

IMPL_LINK_NOARG(URLBox, FocusInHdl, weld::Widget&, void)
{
    (void)this; // loplugin:staticmethod
#ifndef UNX
    // pb: don't select automatically on unix #93251#
    m_xWidget->select_entry_region(0, -1);
#endif
}

IMPL_LINK_NOARG(URLBox, FocusOutHdl, weld::Widget&, void)
{
    if (pCtx.is())
    {
        pCtx->Stop();
        pCtx->join();
        pCtx.clear();
    }
}

OUString URLBox::GetURL()
{
    // wait for end of autocompletion
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

    OUString aText(m_xWidget->get_active_text());

    // try to get the right case preserving URL from the list of URLs
    for(std::vector<OUString>::iterator i = pImpl->aCompletions.begin(), j = pImpl->aURLs.begin(); i != pImpl->aCompletions.end() && j != pImpl->aURLs.end(); ++i, ++j)
    {
        if((*i) == aText)
            return *j;
    }

#ifdef _WIN32
    // erase trailing spaces on Windows since thay are invalid on this OS and
    // most of the time they are inserted by accident via copy / paste
    aText = comphelper::string::stripEnd(aText, ' ');
    if ( aText.isEmpty() )
        return aText;
    // #i9739#
#endif

    INetURLObject aObj( aText );
    if( aText.indexOf( '*' ) != -1 || aText.indexOf( '?' ) != -1 )
    {
        // no autocompletion for wildcards
        INetURLObject aTempObj;
        if ( eSmartProtocol != INetProtocol::NotValid )
            aTempObj.SetSmartProtocol( eSmartProtocol );
        if ( aTempObj.SetSmartURL( aText ) )
            return aTempObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        else
            return aText;
    }

    if ( aObj.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aName = ParseSmart( aText, aBaseURL );
        aObj.SetURL(aName);
        OUString aURL( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        if ( aURL.isEmpty() )
            // aText itself is invalid, and even together with aBaseURL, it could not
            // made valid -> no chance
            return aText;

        bool bSlash = aObj.hasFinalSlash();
        {
            const OUString aPropName("CasePreservingURL");

            OUString aFileURL;

            Any aAny = UCBContentHelper::GetProperty(aURL, aPropName);
            bool success = (aAny >>= aFileURL);
            OUString aTitle;
            if(success)
                aTitle = INetURLObject(aFileURL).getName(
                             INetURLObject::LAST_SEGMENT,
                             true,
                             INetURLObject::DecodeMechanism::WithCharset );
            else
                success =
                    UCBContentHelper::GetTitle(aURL,&aTitle);

            if( success && aTitle != "/" && aTitle != "." )
            {
                    aObj.SetName( aTitle );
                    if ( bSlash )
                        aObj.setFinalSlash();
            }
        }
    }

    return aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

void URLBox::SetBaseURL( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

    // Reset match lists
    pImpl->aCompletions.clear();
    pImpl->aURLs.clear();

    aBaseURL = rURL;
}

void URLBox::DisableHistory()
{
    bHistoryDisabled = true;
    UpdatePicklistForSmartProtocol_Impl();
}

void URLBox::SetFilter(const OUString& _sFilter)
{
    pImpl->m_aFilters.clear();
    FilterMatch::createWildCardFilterList(_sFilter,pImpl->m_aFilters);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
