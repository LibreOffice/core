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
#endif

#include <svtools/inettbc.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/Property.hpp>
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
#include <salhelper/thread.hxx>
#include <tools/debug.hxx>
#include <o3tl/string_view.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/urlfilter.hxx>

#include <mutex>
#include <utility>
#include <vector>
#include <algorithm>

using namespace ::ucbhelper;
using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
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
        FilterMatch::createWildCardFilterList(u"",m_aFilters);
    }
};

class SvtMatchContext_Impl: public salhelper::Thread
{
    std::vector<OUString>           aPickList;
    std::vector<OUString>           aCompletions;
    std::vector<OUString>           aURLs;
    svtools::AsynchronLink          aLink;
    OUString                        aText;
    SvtURLBox*                      pBox;
    bool                            bOnlyDirectories;
    bool                            bNoSelection;

    std::mutex mutex_;
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
                                    SvtMatchContext_Impl( SvtURLBox* pBoxP, OUString aText );
    void                            Stop();
};


namespace
{
    ::osl::Mutex& theSvtMatchContextMutex()
    {
        static ::osl::Mutex SINGLETON;
        return SINGLETON;
    }
}

SvtMatchContext_Impl::SvtMatchContext_Impl(SvtURLBox* pBoxP, OUString _aText)
    : Thread( "MatchContext_Impl" )
    , aLink( LINK( this, SvtMatchContext_Impl, Select_Impl ) )
    , aText(std::move( _aText ))
    , pBox( pBoxP )
    , bOnlyDirectories( pBoxP->bOnlyDirectories )
    , bNoSelection( pBoxP->bNoSelection )
    , stopped_(false)
    , commandId_(0)
{
    FillPicklist( aPickList );
}

SvtMatchContext_Impl::~SvtMatchContext_Impl()
{
    aLink.ClearPendingCall();
}

void SvtMatchContext_Impl::FillPicklist(std::vector<OUString>& rPickList)
{
    // Read the history of picks
    std::vector< SvtHistoryOptions::HistoryItem > seqPicklist = SvtHistoryOptions::GetList( EHistoryType::PickList );
    sal_uInt32 nCount = seqPicklist.size();

    for( sal_uInt32 nItem=0; nItem < nCount; nItem++ )
    {
        INetURLObject aURL;
        aURL.SetURL( seqPicklist[nItem].sTitle );
        rPickList.insert(rPickList.begin() + nItem, aURL.GetMainURL(INetURLObject::DecodeMechanism::WithCharset));
    }
}

void SvtMatchContext_Impl::Stop()
{
    css::uno::Reference< css::ucb::XCommandProcessor > proc;
    sal_Int32 id(0);
    {
        std::scoped_lock g(mutex_);
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
        std::scoped_lock g(mutex_);
        if (stopped_) {
            // Completion was stopped, no display:
            return;
        }
    }

    // insert all completed strings into the listbox
    pBox->clear();

    for (auto const& completion : aCompletions)
    {
        // convert the file into a URL
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

    pBox->EnableAutocomplete(!bNoSelection);

    // transfer string lists to listbox and forget them
    pBox->pImpl->aURLs = aURLs;
    pBox->pImpl->aCompletions = aCompletions;
    aURLs.clear();
    aCompletions.clear();

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

        OUString aNewText = aText + "/";
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

        try
        {
            ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
            if ( bOnlyDirectories )
                eInclude = INCLUDE_FOLDERS_ONLY;
            uno::Reference< XDynamicResultSet > xDynResultSet = aCnt.createDynamicCursor( { u"Title"_ustr, u"IsFolder"_ustr }, eInclude );

            uno::Reference < XAnyCompareFactory > xCompare;
            uno::Reference < XSortedDynamicResultSetFactory > xSRSFac =
                SortedDynamicResultSetFactory::create( ::comphelper::getProcessComponentContext() );

            uno::Reference< XDynamicResultSet > xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, { { 2, false }, { 1, true } }, xCompare );

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
                            aInput += OUStringChar(aDelimiter);

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

void SvtMatchContext_Impl::doExecute()
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex() );
    {
        // have we been stopped while we were waiting for the mutex?
        std::scoped_lock g(mutex_);
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
                // how SvtMatchContext_Impl is used requires this code to run to
                // completion before further user input is processed, and even
                // SvtMatchContext_Impl::Stop does not guarantee a speedy
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
                            css::uno::Sequence< css::beans::Property > prop{
                                { /* Name       */ u"IsFolder"_ustr,
                                  /* Handle     */ -1,
                                  /* Type       */ cppu::UnoType< bool >::get(),
                                  /* Attributes */ {} }
                            };
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
                                    std::scoped_lock g(mutex_);
                                    processor_ = proc;
                                    commandId_ = id;
                                }
                                res = proc->execute(
                                    css::ucb::Command(
                                        u"getPropertyValues"_ustr, -1,
                                        css::uno::Any(prop)),
                                    id,
                                    css::uno::Reference<
                                        css::ucb::XCommandEnvironment >());
                            } catch (...) {
                                if (proc2.is()) {
                                    try {
                                        proc2->releaseCommandIdentifier(id);
                                    } catch (css::uno::RuntimeException &) {
                                        TOOLS_WARN_EXCEPTION("svtools.control", "ignoring");
                                    }
                                }
                                throw;
                            }
                            if (proc2.is()) {
                                proc2->releaseCommandIdentifier(id);
                            }
                            {
                                std::scoped_lock g(mutex_);
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
                        } catch (css::uno::Exception &) {
                            TOOLS_WARN_EXCEPTION("svtools.control", "ignoring");
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
                                aCurObj.SetMark( u"" );
                                aCurObj.SetParam( u"" );
                                aCurObj.SetURLPath( u"" );
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
                            aCurObj.SetMark( u"" );
                            aCurObj.SetParam( u"" );
                            aCurObj.SetURLPath( u"" );
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
                aParseTilde += aText.subView( 2 );
        }

        aText = aParseTilde;
        aBaseURL.clear(); // tilde provide absolute path
    }
#endif

    return true;
}

//--

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
                OUString aTmp = INetURLObject::GetScheme( eBaseProt ) + "/";
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

IMPL_LINK_NOARG(SvtURLBox, TryAutoComplete, Timer *, void)
{
    OUString aCurText = m_xWidget->get_active_text();
    int nStartPos, nEndPos;
    m_xWidget->get_entry_selection_bounds(nStartPos, nEndPos);
    if (std::max(nStartPos, nEndPos) != aCurText.getLength())
        return;

    auto nLen = std::min(nStartPos, nEndPos);
    aCurText = aCurText.copy( 0, nLen );
    if (!aCurText.isEmpty())
    {
        if (pCtx.is())
        {
            pCtx->Stop();
            pCtx->join();
            pCtx.clear();
        }
        pCtx = new SvtMatchContext_Impl(this, aCurText);
        pCtx->launch();
    }
    else
        m_xWidget->clear();
}

SvtURLBox::SvtURLBox(std::unique_ptr<weld::ComboBox> pWidget)
    : aChangedIdle("svtools::URLBox aChangedIdle")
    , eSmartProtocol(INetProtocol::NotValid)
    , bOnlyDirectories( false )
    , bHistoryDisabled( false )
    , bNoSelection( false )
    , m_xWidget(std::move(pWidget))
{
    //don't grow to fix mega-long urls
    Size aSize(m_xWidget->get_preferred_size());
    m_xWidget->set_size_request(aSize.Width(), -1);

    Init();

    m_xWidget->connect_focus_in(LINK(this, SvtURLBox, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, SvtURLBox, FocusOutHdl));
    m_xWidget->connect_changed(LINK(this, SvtURLBox, ChangedHdl));

    aChangedIdle.SetInvokeHandler(LINK(this, SvtURLBox, TryAutoComplete));
}

void SvtURLBox::Init()
{
    pImpl.reset( new SvtURLBox_Impl );

    m_xWidget->set_entry_completion(false);

    UpdatePicklistForSmartProtocol_Impl();
}

SvtURLBox::~SvtURLBox()
{
    if (pCtx.is())
    {
        pCtx->Stop();
        pCtx->join();
    }
}

void SvtURLBox::SetSmartProtocol(INetProtocol eProt)
{
    if ( eSmartProtocol != eProt )
    {
        eSmartProtocol = eProt;
        UpdatePicklistForSmartProtocol_Impl();
    }
}

void SvtURLBox::UpdatePicklistForSmartProtocol_Impl()
{
    m_xWidget->clear();
    if ( bHistoryDisabled )
        return;

    if (bHistoryDisabled)
        return;

    // read history pick list
    const std::vector< SvtHistoryOptions::HistoryItem > seqPicklist = SvtHistoryOptions::GetList( EHistoryType::PickList );
    INetURLObject aCurObj;

    for( const SvtHistoryOptions::HistoryItem& rPropertySet : seqPicklist )
    {
        aCurObj.SetURL( rPropertySet.sURL );

        if ( !rPropertySet.sURL.isEmpty() && ( eSmartProtocol != INetProtocol::NotValid ) )
        {
            if( aCurObj.GetProtocol() != eSmartProtocol )
                continue;
        }

        OUString aURL( aCurObj.GetMainURL( INetURLObject::DecodeMechanism::WithCharset ) );

        if ( !aURL.isEmpty() )
        {
            bool bFound = aURL.endsWith("/");
            if ( !bFound )
            {
                OUString aUpperURL = aURL.toAsciiUpperCase();

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
    }
}

IMPL_LINK_NOARG(SvtURLBox, ChangedHdl, weld::ComboBox&, void)
{
    aChangeHdl.Call(*m_xWidget);
    aChangedIdle.Start(); //launch this to happen on idle after cursor position will have been set
}

IMPL_LINK_NOARG(SvtURLBox, FocusInHdl, weld::Widget&, void)
{
#ifndef UNX
    // pb: don't select automatically on unix #93251#
    m_xWidget->select_entry_region(0, -1);
#endif
    aFocusInHdl.Call(*m_xWidget);
}

IMPL_LINK_NOARG(SvtURLBox, FocusOutHdl, weld::Widget&, void)
{
    if (pCtx.is())
    {
        pCtx->Stop();
        pCtx->join();
        pCtx.clear();
    }
    aFocusOutHdl.Call(*m_xWidget);
}

void SvtURLBox::SetOnlyDirectories( bool bDir )
{
    bOnlyDirectories = bDir;
    if ( bOnlyDirectories )
        m_xWidget->clear();
}

void SvtURLBox::SetNoURLSelection( bool bSet )
{
    bNoSelection = bSet;
}

OUString SvtURLBox::GetURL()
{
    // wait for end of autocompletion
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex() );

    OUString aText(m_xWidget->get_active_text());
    if (MatchesPlaceHolder(aText))
        return aPlaceHolder;

    // try to get the right case preserving URL from the list of URLs
    for(std::vector<OUString>::iterator i = pImpl->aCompletions.begin(), j = pImpl->aURLs.begin(); i != pImpl->aCompletions.end() && j != pImpl->aURLs.end(); ++i, ++j)
    {
        if((*i) == aText)
            return *j;
    }

#ifdef _WIN32
    // erase trailing spaces on Windows since they are invalid on this OS and
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
            OUString aFileURL;

            Any aAny = UCBContentHelper::GetProperty(aURL, u"CasePreservingURL"_ustr);
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
                    aObj.setName( aTitle );
                    if ( bSlash )
                        aObj.setFinalSlash();
            }
        }
    }

    return aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

void SvtURLBox::SetBaseURL( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex() );

    // Reset match lists
    pImpl->aCompletions.clear();
    pImpl->aURLs.clear();

    aBaseURL = rURL;
}

void SvtURLBox::DisableHistory()
{
    bHistoryDisabled = true;
    UpdatePicklistForSmartProtocol_Impl();
}

void SvtURLBox::SetFilter(std::u16string_view _sFilter)
{
    pImpl->m_aFilters.clear();
    FilterMatch::createWildCardFilterList(_sFilter,pImpl->m_aFilters);
}

void FilterMatch::createWildCardFilterList(std::u16string_view _rFilterList,::std::vector< WildCard >& _rFilters)
{
    if( !_rFilterList.empty() )
    {
        // filter is given
        sal_Int32 nIndex = 0;
        OUString sToken;
        do
        {
            sToken = o3tl::getToken(_rFilterList, 0, ';', nIndex );
            if ( !sToken.isEmpty() )
            {
                _rFilters.emplace_back( sToken.toAsciiUpperCase() );
            }
        }
        while ( nIndex >= 0 );
    }
    else
    {
        // no filter is given -> match all
        _rFilters.emplace_back(u"*" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
