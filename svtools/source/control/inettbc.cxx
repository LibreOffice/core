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

#ifdef UNX
#include <pwd.h>
#include <sys/types.h>
#endif

#include <svtools/inettbc.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <rtl/instance.hxx>
#include <vcl/toolbox.hxx>
#include <salhelper/thread.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unotools/historyoptions.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include "svl/urihelper.hxx"
#include <unotools/pathoptions.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include "iodlg.hrc"
#include <svtools/asynclink.hxx>
#include <svl/urlfilter.hxx>

#include <vector>
#include <algorithm>

using namespace ::rtl;
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
    std::vector<rtl::OUString>      aURLs;
    std::vector<rtl::OUString>      aCompletions;
    const IUrlFilter*               pUrlFilter;
    ::std::vector< WildCard >       m_aFilters;

    static sal_Bool TildeParsing( String& aText, String& aBaseUrl );

    inline SvtURLBox_Impl( )
        :pUrlFilter( NULL )
    {
        FilterMatch::createWildCardFilterList(String(),m_aFilters);
    }
};

class SvtMatchContext_Impl: public salhelper::Thread
{
    static ::osl::Mutex*            pDirMutex;

    std::vector<rtl::OUString>      aPickList;
    std::vector<rtl::OUString>      aCompletions;
    std::vector<rtl::OUString>      aURLs;
    svtools::AsynchronLink          aLink;
    String                          aBaseURL;
    String                          aText;
    SvtURLBox*                      pBox;
    sal_Bool                            bStop;
    sal_Bool                            bOnlyDirectories;
    sal_Bool                            bNoSelection;

    DECL_STATIC_LINK(               SvtMatchContext_Impl, Select_Impl, void* );

    virtual                         ~SvtMatchContext_Impl();
    virtual void                    execute();
    void                            doExecute();
    void                            Insert( const String& rCompletion, const String& rURL, sal_Bool bForce = sal_False);
    void                            ReadFolder( const String& rURL, const String& rMatch, sal_Bool bSmart );
    void                            FillPicklist(std::vector<rtl::OUString>& rPickList);

public:
                                    SvtMatchContext_Impl( SvtURLBox* pBoxP, const String& rText );
    void                            Stop();
};

namespace
{
    struct theSvtMatchContextMutex
        : public rtl::Static< ::osl::Mutex, theSvtMatchContextMutex > {};
}

SvtMatchContext_Impl::SvtMatchContext_Impl(
    SvtURLBox* pBoxP, const String& rText )
    : Thread( "SvtMatchContext_Impl" )
    , aLink( STATIC_LINK( this, SvtMatchContext_Impl, Select_Impl ) )
    , aBaseURL( pBoxP->aBaseURL )
    , aText( rText )
    , pBox( pBoxP )
    , bStop( sal_False )
    , bOnlyDirectories( pBoxP->bOnlyDirectories )
    , bNoSelection( pBoxP->bNoSelection )
{
    aLink.CreateMutex();

    FillPicklist( aPickList );
}

SvtMatchContext_Impl::~SvtMatchContext_Impl()
{
    aLink.ClearPendingCall();
}

void SvtMatchContext_Impl::FillPicklist(std::vector<rtl::OUString>& rPickList)
{
    // Einlesung der Historypickliste
    Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( eHISTORY );
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
                rPickList.insert(rPickList.begin() + nItem, aURL.GetMainURL(INetURLObject::DECODE_WITH_CHARSET));
                break;
            }
        }
    }
}

void SvtMatchContext_Impl::Stop()
{
    bStop = sal_True;
    terminate();
}

void SvtMatchContext_Impl::execute( )
{
    doExecute();
    aLink.Call( this );
}

//-------------------------------------------------------------------------
// This method is called via AsynchronLink, so it has the SolarMutex and
// calling solar code ( VCL ... ) is safe. It is called when the thread is
// terminated ( finished work or stopped ). Cancelling the thread via
// Cancellable does not not discard the information gained so far, it
// inserts all collected completions into the listbox.

IMPL_STATIC_LINK( SvtMatchContext_Impl, Select_Impl, void*, )
{
    // avoid recursion through cancel button
    if( pThis->bStop )
    {
        // completions was stopped, no display
        return 0;
    }

    SvtURLBox* pBox = pThis->pBox;
    pBox->bAutoCompleteMode = sal_True;

    // did we filter completions which otherwise would have been valid?
    // (to be filled below)
    bool bValidCompletionsFiltered = false;

    // insert all completed strings into the listbox
    pBox->Clear();

    for(std::vector<rtl::OUString>::iterator i = pThis->aCompletions.begin(); i != pThis->aCompletions.end(); ++i)
    {
        String sCompletion(*i);

        // convert the file into an URL
        rtl::OUString sURL( sCompletion );
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sCompletion, sURL );
            // note: if this doesn't work, we're not interested in: we're checking the
            // untouched sCompletion then

        if ( pBox->pImp->pUrlFilter )
        {
            if ( !pBox->pImp->pUrlFilter->isUrlAllowed( sURL ) )
            {   // this URL is not allowed
                bValidCompletionsFiltered = true;
                continue;
            }
        }
        if ( !sURL.isEmpty() && ( sURL[sURL.getLength()-1] != '/' ))
        {
            String sUpperURL( sURL );
            sUpperURL.ToUpperAscii();

            ::std::vector< WildCard >::const_iterator aMatchingFilter =
                ::std::find_if(
                    pBox->pImp->m_aFilters.begin(),
                    pBox->pImp->m_aFilters.end(),
                    FilterMatch( sUpperURL )
                );
            if ( aMatchingFilter == pBox->pImp->m_aFilters.end() )

            {   // this URL is not allowed
                bValidCompletionsFiltered = true;
                continue;
            }
        }

        pBox->InsertEntry( sCompletion );
    }

    if( !pThis->bNoSelection && !pThis->aCompletions.empty() && !bValidCompletionsFiltered )
    {
        // select the first one
        String aTmp( pBox->GetEntry(0) );
        pBox->SetText( aTmp );
        pBox->SetSelection( Selection( pThis->aText.Len(), aTmp.Len() ) );
    }

    // transfer string lists to listbox and forget them
    pBox->pImp->aURLs = pThis->aURLs;
    pBox->pImp->aCompletions = pThis->aCompletions;
    pThis->aURLs.clear();
    pThis->aCompletions.clear();

    // force listbox to resize ( it may be open )
    pBox->Resize();

    // the box has this control as a member so we have to set that member
    // to zero before deleting ourself.
    pBox->pCtx.clear();

    return 0;
}

//-------------------------------------------------------------------------
void SvtMatchContext_Impl::Insert( const String& rCompletion,
                                   const String& rURL,
                                   sal_Bool bForce )
{
    if( !bForce )
    {
        // avoid doubles
        if(find(aCompletions.begin(), aCompletions.end(), rtl::OUString(rCompletion)) != aCompletions.end())
            return;
    }

    aCompletions.push_back(rCompletion);
    aURLs.push_back(rURL);
}

//-------------------------------------------------------------------------
void SvtMatchContext_Impl::ReadFolder( const String& rURL,
                                       const String& rMatch,
                                       sal_Bool bSmart )
{
    // check folder to scan
    if( !UCBContentHelper::IsFolder( rURL ) )
        return;

    sal_Bool bPureHomePath = sal_False;
#ifdef UNX
    bPureHomePath = aText.Search( '~' ) == 0 && aText.Search( '/' ) == STRING_NOTFOUND;
#endif

    sal_Bool bExectMatch = bPureHomePath
                || aText.CompareToAscii( "." ) == COMPARE_EQUAL
                || (aText.Len() > 1 && aText.Copy( aText.Len() - 2, 2 ).CompareToAscii( "/." ) == COMPARE_EQUAL)
                || (aText.Len() > 2 && aText.Copy( aText.Len() - 3, 3 ).CompareToAscii( "/.." ) == COMPARE_EQUAL);

    // for pure home paths ( ~username ) the '.' at the end of rMatch
    // means that it poits to root catalog
    // this is done only for file contents since home paths parsing is usefull only for them
    if ( bPureHomePath && rMatch.Equals( rtl::OUString("file:///.") ) )
    {
        // a home that refers to /

        String aNewText( aText );
        aNewText += '/';
        Insert( aNewText, rURL, sal_True );

        return;
    }

    // string to match with
    INetURLObject aMatchObj( rMatch );
    String aMatchName;

    if ( rURL != String(aMatchObj.GetMainURL( INetURLObject::NO_DECODE ) ))
    {
        aMatchName = aMatchObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

        // matching is always done case insensitive, but completion will be case sensitive and case preserving
        aMatchName.ToLowerAscii();

        // if the matchstring ends with a slash, we must search for this also
        if ( rMatch.GetChar(rMatch.Len()-1) == '/' )
            aMatchName += '/';
    }

    xub_StrLen nMatchLen = aMatchName.Len();

    INetURLObject aFolderObj( rURL );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        uno::Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();

        Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ),
                      new ::ucbhelper::CommandEnvironment( uno::Reference< XInteractionHandler >(),
                                                     uno::Reference< XProgressHandler >() ),
                      comphelper::getProcessComponentContext() );
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
        pProps[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) );

        try
        {
            uno::Reference< XDynamicResultSet > xDynResultSet;
            ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
            if ( bOnlyDirectories )
                eInclude = INCLUDE_FOLDERS_ONLY;

            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            uno::Reference < XAnyCompareFactory > xCompare;
            uno::Reference < XSortedDynamicResultSetFactory > xSRSFac(
                xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SortedDynamicResultSetFactory") ) ), UNO_QUERY );

            Sequence< NumberedSortingInfo > aSortInfo( 2 );
            NumberedSortingInfo* pInfo = aSortInfo.getArray();
            pInfo[ 0 ].ColumnIndex = 2;
            pInfo[ 0 ].Ascending   = sal_False;
            pInfo[ 1 ].ColumnIndex = 1;
            pInfo[ 1 ].Ascending   = sal_True;

            uno::Reference< XDynamicResultSet > xDynamicResultSet;
            xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xCompare );

            if ( xDynamicResultSet.is() )
            {
                xResultSet = xDynamicResultSet->getStaticResultSet();
            }
        }
        catch( ::com::sun::star::uno::Exception& ) {}

        if ( xResultSet.is() )
        {
            uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );
            uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            try
            {
                while ( schedule() && xResultSet->next() )
                {
                    String   aURL      = xContentAccess->queryContentIdentifierString();
                    String   aTitle    = xRow->getString(1);
                    sal_Bool bIsFolder = xRow->getBoolean(2);

                    // matching is always done case insensitive, but completion will be case sensitive and case preserving
                    aTitle.ToLowerAscii();

                    if (
                        !nMatchLen ||
                        (bExectMatch && aMatchName.Equals(aTitle)) ||
                        (!bExectMatch && aMatchName.CompareTo(aTitle, nMatchLen) == COMPARE_EQUAL)
                       )
                    {
                        // all names fit if matchstring is empty
                        INetURLObject aObj( aURL );
                        sal_Unicode aDelimiter = '/';
                        if ( bSmart )
                            // when parsing is done "smart", the delimiter must be "guessed"
                            aObj.getFSysPath( (INetURLObject::FSysStyle)(INetURLObject::FSYS_DETECT & ~INetURLObject::FSYS_VOS), &aDelimiter );

                        if ( bIsFolder )
                            aObj.setFinalSlash();

                        // get the last name of the URL
                        String aMatch = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                        String aInput( aText );
                        if ( nMatchLen )
                        {
                            if ((aText.Len() && aText.GetChar(aText.Len() - 1) == '.') || bPureHomePath)
                            {
                                // if a "special folder" URL was typed, don't touch the user input
                                aMatch.Erase( 0, nMatchLen );
                            }
                            else
                            {
                                // make the user input case preserving
                                DBG_ASSERT( aInput.Len() >= nMatchLen, "Suspicious Matching!" );
                                aInput.Erase( aInput.Len() - nMatchLen );
                            }
                        }

                        aInput += aMatch;

                        // folders should get a final slash automatically
                        if ( bIsFolder )
                            aInput += aDelimiter;

                        Insert( aInput, aObj.GetMainURL( INetURLObject::NO_DECODE ), sal_True );
                    }
                }
            }
            catch( ::com::sun::star::uno::Exception& )
            {
            }
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }
}

//-------------------------------------------------------------------------
String SvtURLBox::ParseSmart( String aText, String aBaseURL, String aWorkDir )
{
    String aMatch;

    // parse ~ for Unix systems
    // does nothing for Windows
    if( !SvtURLBox_Impl::TildeParsing( aText, aBaseURL ) )
        return String();

    INetURLObject aURLObject;
    if( aBaseURL.Len() )
    {
        INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( aBaseURL );

        // if a base URL is set the string may be parsed relative
        if( aText.Search( '/' ) == 0 )
        {
            // text starting with slashes means absolute file URLs
            String aTemp = INetURLObject::GetScheme( eBaseProt );

            // file URL must be correctly encoded!
            String aTextURL = INetURLObject::encode( aText, INetURLObject::PART_FPATH,
                                                     '%', INetURLObject::ENCODE_ALL );
            aTemp += aTextURL;

            INetURLObject aTmp( aTemp );
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INET_PROT_NOT_VALID )
                aMatch = aTmp.GetMainURL( INetURLObject::NO_DECODE );
        }
        else
        {
            String aSmart( aText );
            INetURLObject aObj( aBaseURL );

            // HRO: I suppose this hack should only be done for Windows !!!???
#ifdef WNT
            // HRO: INetURLObject::smatRel2Abs does not recognize '\\' as a relative path
            //      but in case of "\\\\" INetURLObject is right - this is an absolute path !

            if( aText.Search( '\\' ) == 0 && (aText.Len() < 2 || aText.GetChar( 1 ) != '\\') )
            {
                // cut to first segment
                String aTmp = INetURLObject::GetScheme( eBaseProt );
                aTmp += '/';
                aTmp += String(aObj.getName( 0, true, INetURLObject::DECODE_WITH_CHARSET ));
                aObj.SetURL( aTmp );

                aSmart.Erase(0,1);
            }
#endif
            // base URL must be a directory !
            aObj.setFinalSlash();

            // take base URL and append current input
            bool bWasAbsolute = sal_False;
#ifdef UNX
            // don't support FSYS_MAC under Unix, because here ':' is a valid character for a filename
            INetURLObject::FSysStyle eStyle = static_cast< INetURLObject::FSysStyle >( INetURLObject::FSYS_VOS | INetURLObject::FSYS_UNX | INetURLObject::FSYS_DOS );
            // encode file URL correctly
            aSmart = INetURLObject::encode( aSmart, INetURLObject::PART_FPATH, '%', INetURLObject::ENCODE_ALL );
            INetURLObject aTmp( aObj.smartRel2Abs(
                aSmart, bWasAbsolute, false, INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8, false, eStyle ) );
#else
            INetURLObject aTmp( aObj.smartRel2Abs( aSmart, bWasAbsolute ) );
#endif

            if ( aText.GetChar( aText.Len() - 1 ) == '.' )
                // INetURLObject appends a final slash for the directories "." and "..", this is a bug!
                // Remove it as a workaround
                aTmp.removeFinalSlash();
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INET_PROT_NOT_VALID )
                aMatch = aTmp.GetMainURL( INetURLObject::NO_DECODE );
        }
    }
    else
    {
        ::utl::LocalFileHelper::ConvertSystemPathToURL( aText, aWorkDir, aMatch );
    }

    return aMatch;
}

//-------------------------------------------------------------------------
void SvtMatchContext_Impl::doExecute()
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );
    if( bStop )
        // have we been stopped while we were waiting for the mutex?
        return;

    // Reset match lists
    aCompletions.clear();
    aURLs.clear();

    // check for input
    sal_uInt16 nTextLen = aText.Len();
    if ( !nTextLen )
        return;

    if( aText.Search( '*' ) != STRING_NOTFOUND || aText.Search( '?' ) != STRING_NOTFOUND )
        // no autocompletion for wildcards
        return;

    String aMatch;
    String aWorkDir( SvtPathOptions().GetWorkPath() );
    INetProtocol eProt = INetURLObject::CompareProtocolScheme( aText );
    INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( aBaseURL );
    if ( !aBaseURL.Len() )
        eBaseProt = INetURLObject::CompareProtocolScheme( aWorkDir );
    INetProtocol eSmartProt = pBox->GetSmartProtocol();

    // if the user input is a valid URL, go on with it
    // otherwise it could be parsed smart with a predefined smart protocol
    // ( or if this is not set with the protocol of a predefined base URL )
    if( eProt == INET_PROT_NOT_VALID || eProt == eSmartProt || (eSmartProt == INET_PROT_NOT_VALID && eProt == eBaseProt) )
    {
        // not stopped yet ?
        if( schedule() )
        {
            if ( eProt == INET_PROT_NOT_VALID )
                aMatch = SvtURLBox::ParseSmart( aText, aBaseURL, aWorkDir );
            else
                aMatch = aText;
            if ( aMatch.Len() )
            {
                INetURLObject aURLObject( aMatch );
                String aMainURL( aURLObject.GetMainURL( INetURLObject::NO_DECODE ) );
                if ( aMainURL.Len() )
                {
                    // if text input is a directory, it must be part of the match list! Until then it is scanned
                    if ( UCBContentHelper::IsFolder( aMainURL ) && aURLObject.hasFinalSlash() )
                            Insert( aText, aMatch );
                    else
                        // otherwise the parent folder will be taken
                        aURLObject.removeSegment();

                    // scan directory and insert all matches
                    ReadFolder( aURLObject.GetMainURL( INetURLObject::NO_DECODE ), aMatch, eProt == INET_PROT_NOT_VALID );
                }
            }
        }
    }

    if ( bOnlyDirectories )
        // don't scan history picklist if only directories are allowed, picklist contains only files
        return;

    sal_Bool bFull = sal_False;

    INetURLObject aCurObj;
    String aEmpty, aCurString, aCurMainURL;
    INetURLObject aObj;
    aObj.SetSmartProtocol( eSmartProt == INET_PROT_NOT_VALID ? INET_PROT_HTTP : eSmartProt );
    for( ;; )
    {
        for(std::vector<rtl::OUString>::iterator i = aPickList.begin(); schedule() && i != aPickList.end(); ++i)
        {
            aCurObj.SetURL(*i);
            aCurObj.SetSmartURL( aCurObj.GetURLNoPass());
            aCurMainURL = aCurObj.GetMainURL( INetURLObject::NO_DECODE );

            if( eProt != INET_PROT_NOT_VALID && aCurObj.GetProtocol() != eProt )
                continue;

            if( eSmartProt != INET_PROT_NOT_VALID && aCurObj.GetProtocol() != eSmartProt )
                continue;

            switch( aCurObj.GetProtocol() )
            {
                case INET_PROT_HTTP:
                case INET_PROT_HTTPS:
                case INET_PROT_FTP:
                {
                    if( eProt == INET_PROT_NOT_VALID && !bFull )
                    {
                        aObj.SetSmartURL( aText );
                        if( aObj.GetURLPath().getLength() > 1 )
                            continue;
                    }

                    aCurString = aCurMainURL;
                    if( eProt == INET_PROT_NOT_VALID )
                    {
                        // try if text matches the scheme
                        String aScheme( INetURLObject::GetScheme( aCurObj.GetProtocol() ) );
                        if ( aText.CompareIgnoreCaseToAscii( aScheme, aText.Len() ) == COMPARE_EQUAL && aText.Len() < aScheme.Len() )
                        {
                            if( bFull )
                                aMatch = aCurObj.GetMainURL( INetURLObject::NO_DECODE );
                            else
                            {
                                aCurObj.SetMark( aEmpty );
                                aCurObj.SetParam( aEmpty );
                                aCurObj.SetURLPath( aEmpty );
                                aMatch = aCurObj.GetMainURL( INetURLObject::NO_DECODE );
                            }

                            Insert( aMatch, aMatch );
                        }

                        // now try smart matching
                        aCurString.Erase( 0, aScheme.Len() );
                    }

                    if( aText.CompareIgnoreCaseToAscii( aCurString, aText.Len() )== COMPARE_EQUAL )
                    {
                        if( bFull )
                            aMatch = aCurObj.GetMainURL( INetURLObject::NO_DECODE );
                        else
                        {
                            aCurObj.SetMark( aEmpty );
                            aCurObj.SetParam( aEmpty );
                            aCurObj.SetURLPath( aEmpty );
                            aMatch = aCurObj.GetMainURL( INetURLObject::NO_DECODE );
                        }

                        String aURL( aMatch );
                        if( eProt == INET_PROT_NOT_VALID )
                            aMatch.Erase( 0, sal::static_int_cast< xub_StrLen >(INetURLObject::GetScheme( aCurObj.GetProtocol() ).getLength()) );

                        if( aText.Len() < aMatch.Len() )
                            Insert( aMatch, aURL );

                        continue;
                    }
                    break;
                }
                default:
                {
                    if( bFull )
                        continue;

                    if( aText.CompareTo( aCurMainURL, aText.Len() ) == COMPARE_EQUAL )
                    {
                        if( aText.Len() < aCurMainURL.Len() )
                            Insert( aCurMainURL, aCurMainURL );

                        continue;
                    }
                    break;
                }
            }
        }

        if( !bFull )
            bFull = sal_True;
        else
            break;
    }

    return;
}

void SvtURLBox::TryAutoComplete( sal_Bool bForce )
{
    if( Application::AnyInput( VCL_INPUT_KEYBOARD ) ) return;

    String aCurText = GetText();
    Selection aSelection( GetSelection() );
    if( aSelection.Max() != aCurText.Len() && !bForce )
        return;
    sal_uInt16 nLen = (sal_uInt16)aSelection.Min();
    aCurText.Erase( nLen );
    if( aCurText.Len() && bIsAutoCompleteEnabled )
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

//-------------------------------------------------------------------------
SvtURLBox::SvtURLBox( Window* pParent, INetProtocol eSmart )
    :   ComboBox( pParent , WB_DROPDOWN | WB_AUTOSIZE | WB_AUTOHSCROLL ),
        eSmartProtocol( eSmart ),
        bAutoCompleteMode( sal_False ),
        bOnlyDirectories( sal_False ),
        bCtrlClick( sal_False ),
        bHistoryDisabled( sal_False ),
        bNoSelection( sal_False ),
        bIsAutoCompleteEnabled( sal_True )
{
    ImplInit();

    if ( GetDesktopRectPixel().GetWidth() > 800 )
        SetSizePixel( Size( 300, 240 ) );
    else
        SetSizePixel( Size( 225, 240 ) );
}

//-------------------------------------------------------------------------
SvtURLBox::SvtURLBox( Window* pParent, WinBits _nStyle, INetProtocol eSmart )
    :   ComboBox( pParent, _nStyle ),
        eSmartProtocol( eSmart ),
        bAutoCompleteMode( sal_False ),
        bOnlyDirectories( sal_False ),
        bCtrlClick( sal_False ),
        bHistoryDisabled( sal_False ),
        bNoSelection( sal_False ),
        bIsAutoCompleteEnabled( sal_True )
{
    ImplInit();
}

//-------------------------------------------------------------------------
SvtURLBox::SvtURLBox( Window* pParent, const ResId& _rResId, INetProtocol eSmart )
    :   ComboBox( pParent , _rResId ),
        eSmartProtocol( eSmart ),
        bAutoCompleteMode( sal_False ),
        bOnlyDirectories( sal_False ),
        bCtrlClick( sal_False ),
        bHistoryDisabled( sal_False ),
        bNoSelection( sal_False ),
        bIsAutoCompleteEnabled( sal_True )
{
    ImplInit();
}

void SvtURLBox::ImplInit()
{
    pImp = new SvtURLBox_Impl();

    if ( GetHelpId().getLength() == 0 )
        SetHelpId( ".uno:OpenURL" );
    EnableAutocomplete( sal_False );

    SetText( String() );

    GetSubEdit()->SetAutocompleteHdl( LINK( this, SvtURLBox, AutoCompleteHdl_Impl ) );
    UpdatePicklistForSmartProtocol_Impl();
}

SvtURLBox::~SvtURLBox()
{
    if( pCtx.is() )
    {
        pCtx->Stop();
        pCtx->join();
    }

    delete pImp;
}

void SvtURLBox::UpdatePickList( )
{
    if( pCtx.is() )
    {
        pCtx->Stop();
        pCtx->join();
        pCtx.clear();
    }

    String sText = GetText();
    if ( sText.Len() && bIsAutoCompleteEnabled )
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
    if ( !bHistoryDisabled )
    {
        // read history pick list
        Sequence< Sequence< PropertyValue > > seqPicklist = SvtHistoryOptions().GetList( eHISTORY );
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

                    if ( !sURL.isEmpty() && ( eSmartProtocol != INET_PROT_NOT_VALID ) )
                    {
                        if( aCurObj.GetProtocol() != eSmartProtocol )
                            break;
                    }

                    String aURL( aCurObj.GetMainURL( INetURLObject::DECODE_WITH_CHARSET ) );

                    if ( aURL.Len() && ( !pImp->pUrlFilter || pImp->pUrlFilter->isUrlAllowed( aURL ) ) )
                    {
                        sal_Bool bFound = (aURL.GetChar(aURL.Len()-1) == '/' );
                        if ( !bFound )
                        {
                            String aUpperURL( aURL );
                            aUpperURL.ToUpperAscii();

                            bFound
                                = (::std::find_if(
                                    pImp->m_aFilters.begin(),
                                    pImp->m_aFilters.end(),
                                    FilterMatch( aUpperURL ) )
                                        != pImp->m_aFilters.end());
                        }
                        if ( bFound )
                        {
                            String aFile;
                            if (::utl::LocalFileHelper::ConvertURLToSystemPath(aURL,aFile))
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
}

//-------------------------------------------------------------------------
sal_Bool SvtURLBox::ProcessKey( const KeyCode& rKey )
{
    // every key input stops the current matching thread
    if( pCtx.is() )
    {
        pCtx->Stop();
        pCtx->join();
        pCtx.clear();
    }

    KeyCode aCode( rKey.GetCode() );
    if ( aCode == KEY_RETURN && GetText().Len() )
    {
        // wait for completion of matching thread
        ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

        if ( bAutoCompleteMode )
        {
            // reset picklist
            bAutoCompleteMode = sal_False;
            Selection aSelection( GetSelection() );
            SetSelection( Selection( aSelection.Min(), aSelection.Min() ) );
            if ( bOnlyDirectories )
                Clear();
            else
                UpdatePicklistForSmartProtocol_Impl();
            Resize();
        }

        bCtrlClick = rKey.IsMod1();
        sal_Bool bHandled = sal_False;
        if ( GetOpenHdl().IsSet() )
        {
            bHandled = sal_True;
            GetOpenHdl().Call(this);
        }
        else if ( GetSelectHdl().IsSet() )
        {
            bHandled = sal_True;
            GetSelectHdl().Call(this);
        }

        bCtrlClick = sal_False;

        ClearModifyFlag();
        return bHandled;
    }
    else if ( aCode == KEY_RETURN && !GetText().Len() && GetOpenHdl().IsSet() )
    {
        // for file dialog
        bAutoCompleteMode = sal_False;
        GetOpenHdl().Call(this);
        return sal_True;
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
           return sal_False;
        }

        bAutoCompleteMode = sal_False;
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

//-------------------------------------------------------------------------
void SvtURLBox::Modify()
{
    ComboBox::Modify();
}

//-------------------------------------------------------------------------
long SvtURLBox::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetWindow() == GetSubEdit() && rNEvt.GetType() == EVENT_KEYINPUT )
    {

        const KeyEvent& rEvent = *rNEvt.GetKeyEvent();
        const KeyCode& rKey = rEvent.GetKeyCode();
        KeyCode aCode( rKey.GetCode() );
        if( ProcessKey( rKey ) )
        {
            return sal_True;
        }
        else if( ( aCode == KEY_UP || aCode == KEY_DOWN ) && !rKey.IsMod2() )
        {
            Selection aSelection( GetSelection() );
            sal_uInt16 nLen = (sal_uInt16)aSelection.Min();
            GetSubEdit()->KeyInput( rEvent );
            SetSelection( Selection( nLen, GetText().Len() ) );
            return sal_True;
        }

        if ( MatchesPlaceHolder( GetText() ) )
        {
            // set the selection so a key stroke will overwrite
            // the placeholder rather than edit it
            SetSelection( Selection( 0, GetText().Len() ) );
        }
    }

    return ComboBox::PreNotify( rNEvt );
}

//-------------------------------------------------------------------------
IMPL_LINK_NOARG(SvtURLBox, AutoCompleteHdl_Impl)
{
    if ( GetSubEdit()->GetAutocompleteAction() == AUTOCOMPLETE_KEYINPUT )
    {
        TryAutoComplete( sal_False );
        return 1L;
    }

    return 0L;
}

//-------------------------------------------------------------------------
long SvtURLBox::Notify( NotifyEvent &rEvt )
{
    if ( EVENT_GETFOCUS == rEvt.GetType() )
    {
#ifndef UNX
        // pb: don't select automatically on unix #93251#
        SetSelection( Selection( 0, GetText().Len() ) );
#endif
    }
    else if ( EVENT_LOSEFOCUS == rEvt.GetType() )
    {
        if( !GetText().Len() )
            ClearModifyFlag();
        if ( pCtx.is() )
        {
            pCtx->Stop();
            pCtx->join();
            pCtx.clear();
        }
    }

    return ComboBox::Notify( rEvt );
}

//-------------------------------------------------------------------------
void SvtURLBox::Select()
{
    ComboBox::Select();
    ClearModifyFlag();
}

//-------------------------------------------------------------------------
void SvtURLBox::SetOnlyDirectories( sal_Bool bDir )
{
    bOnlyDirectories = bDir;
    if ( bOnlyDirectories )
        Clear();
}

//-------------------------------------------------------------------------
void SvtURLBox::SetNoURLSelection( sal_Bool bSet )
{
    bNoSelection = bSet;
}

//-------------------------------------------------------------------------
String SvtURLBox::GetURL()
{
    // wait for end of autocompletion
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

    String aText( GetText() );
    if ( MatchesPlaceHolder( aText ) )
        return aPlaceHolder;

    // try to get the right case preserving URL from the list of URLs
    for(std::vector<rtl::OUString>::iterator i = pImp->aCompletions.begin(), j = pImp->aURLs.begin(); i != pImp->aCompletions.end() && j != pImp->aURLs.end(); ++i, ++j)
    {
        if((*i).equals(aText))
            return *j;
    }

#ifdef WNT
    // erase trailing spaces on Windows since thay are invalid on this OS and
    // most of the time they are inserted by accident via copy / paste
    aText = comphelper::string::stripEnd(aText, ' ');
    if ( !aText.Len() )
        return aText;
    // #i9739#
#endif

    INetURLObject aObj( aText );
    if( aText.Search( '*' ) != STRING_NOTFOUND || aText.Search( '?' ) != STRING_NOTFOUND )
    {
        // no autocompletion for wildcards
        INetURLObject aTempObj;
        if ( eSmartProtocol != INET_PROT_NOT_VALID )
            aTempObj.SetSmartProtocol( eSmartProtocol );
        if ( aTempObj.SetSmartURL( aText ) )
            return aTempObj.GetMainURL( INetURLObject::NO_DECODE );
        else
            return aText;
    }

    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        rtl::OUString aName = ParseSmart( aText, aBaseURL, SvtPathOptions().GetWorkPath() );
        aObj.SetURL(aName);
        ::rtl::OUString aURL( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
        if ( aURL.isEmpty() )
            // aText itself is invalid, and even together with aBaseURL, it could not
            // made valid -> no chance
            return aText;

        bool bSlash = aObj.hasFinalSlash();
        {
            const rtl::OUString aPropName("CasePreservingURL");

            rtl::OUString aFileURL;

            Any aAny = UCBContentHelper::GetProperty(aURL, aPropName);
            sal_Bool success = (aAny >>= aFileURL);
            rtl::OUString aTitle;
            if(success)
                aTitle = String(
                    INetURLObject(aFileURL).getName(
                        INetURLObject::LAST_SEGMENT,
                        true,
                        INetURLObject::DECODE_WITH_CHARSET ));
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

    return aObj.GetMainURL( INetURLObject::NO_DECODE );
}

void SvtURLBox::DisableHistory()
{
    bHistoryDisabled = sal_True;
    UpdatePicklistForSmartProtocol_Impl();
}

void SvtURLBox::SetBaseURL( const String& rURL )
{
    ::osl::MutexGuard aGuard( theSvtMatchContextMutex::get() );

    // Reset match lists
    pImp->aCompletions.clear();
    pImp->aURLs.clear();

    aBaseURL = rURL;
}

/** Parse leading ~ for Unix systems,
    does nothing for Windows
 */
sal_Bool SvtURLBox_Impl::TildeParsing(
    String&
#ifdef UNX
    aText
#endif
    , String&
#ifdef UNX
    aBaseURL
#endif
)
{
#ifdef UNX
    if( aText.Search( '~' ) == 0 )
    {
        String aParseTilde;
        sal_Bool bTrailingSlash = sal_True; // use trailing slash

        if( aText.Len() == 1 || aText.GetChar( 1 ) == '/' )
        {
            // covers "~" or "~/..." cases
            const char* aHomeLocation = getenv( "HOME" );
            if( !aHomeLocation )
                aHomeLocation = "";

            aParseTilde = rtl::OUString::createFromAscii(aHomeLocation);

            // in case the whole path is just "~" then there should
            // be no trailing slash at the end
            if( aText.Len() == 1 )
                bTrailingSlash = sal_False;
        }
        else
        {
            // covers "~username" and "~username/..." cases
            xub_StrLen nNameEnd = aText.Search( '/' );
            rtl::OUString aUserName = aText.Copy( 1, ( nNameEnd != STRING_NOTFOUND ) ? nNameEnd : ( aText.Len() - 1 ) );

            struct passwd* pPasswd = NULL;
#ifdef SOLARIS
            Sequence< sal_Int8 > sBuf( 1024 );
            struct passwd aTmp;
            sal_Int32 nRes = getpwnam_r( OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US ).getStr(),
                                  &aTmp,
                                  (char*)sBuf.getArray(),
                                  1024,
                                  &pPasswd );
            if( !nRes && pPasswd )
                aParseTilde = rtl::OUString::createFromAscii(pPasswd->pw_dir);
            else
                return sal_False; // no such user
#else
            pPasswd = getpwnam( OUStringToOString( aUserName, RTL_TEXTENCODING_ASCII_US ).getStr() );
            if( pPasswd )
                aParseTilde = rtl::OUString::createFromAscii(pPasswd->pw_dir);
            else
                return sal_False; // no such user
#endif

            // in case the path is "~username" then there should
            // be no trailing slash at the end
            if( nNameEnd == STRING_NOTFOUND )
                bTrailingSlash = sal_False;
        }

        if( !bTrailingSlash )
        {
            if( !aParseTilde.Len() || aParseTilde.EqualsAscii( "/" ) )
            {
                // "/" path should be converted to "/."
                aParseTilde = rtl::OUString("/.");
            }
            else
            {
                // "blabla/" path should be converted to "blabla"
                aParseTilde = comphelper::string::stripEnd(aParseTilde, '/');
            }
        }
        else
        {
            if( aParseTilde.GetChar( aParseTilde.Len() - 1 ) != '/' )
                aParseTilde += '/';
            if( aText.Len() > 2 )
                aParseTilde += aText.Copy( 2 );
        }

        aText = aParseTilde;
        aBaseURL = String(); // tilde provide absolute path
    }
#endif

    return sal_True;
}

void SvtURLBox::SetUrlFilter( const IUrlFilter* _pFilter )
{
    pImp->pUrlFilter = _pFilter;
}

void SvtURLBox::SetFilter(const String& _sFilter)
{
    pImp->m_aFilters.clear();
    FilterMatch::createWildCardFilterList(_sFilter,pImp->m_aFilters);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
