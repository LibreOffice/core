/*************************************************************************
 *
 *  $RCSfile: inettbc.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mba $ $Date: 2001-02-26 13:12:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "inettbc.hxx"

#pragma hdrstop

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXCANCEL_HXX //autogen
#include <svtools/cancel.hxx>
#endif
#include <vcl/toolbox.hxx>
#ifndef _VOS_THREAD_HXX //autogen
#include <vos/thread.hxx>
#endif
#ifndef _VOS_MUTEX_HXX //autogen
#include <vos/mutex.hxx>
#endif

#include <svtools/itemset.hxx>
#include <svtools/urihelper.hxx>
#include <svtools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>

#include "picklist.hxx"
#include "sfx.hrc"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "objsh.hxx"
#include "referers.hxx"
#include "sfxtypes.hxx"
#include "helper.hxx"
#include "ucbhelp.hxx"

class SfxURLBox_Impl
{
public:
    SvStringsDtor*                  pURLs;
    SvStringsDtor*                  pCompletions;
};

// -----------------------------------------------------------------------
class SfxMatchContext_Impl : public ::vos::OThread
{
    static ::vos::OMutex*           pDirMutex;

    SvStringsDtor                   aPickList;
    SvStringsDtor*                  pCompletions;
    SvStringsDtor*                  pURLs;
    AsynchronLink                   aLink;
    String                          aBaseURL;
    String                          aText;
    SfxURLBox*                      pBox;
    SfxCancellable*                 pCancel;
    BOOL                            bStop;
    BOOL                            bOnlyDirectories;

    DECL_STATIC_LINK(               SfxMatchContext_Impl, Select_Impl, void* );

    virtual void SAL_CALL           onTerminated( );
    virtual void SAL_CALL           run();
    virtual void SAL_CALL           Cancel();
    void                            Insert( const String& rCompletion, const String& rURL, BOOL bForce = FALSE);
    void                            ReadFolder( const String& rURL, const String& rMatch, BOOL bSmart );

public:
    static ::vos::OMutex*           GetMutex();

                                    SfxMatchContext_Impl( SfxURLBox* pBoxP, const String& rText );
                                    ~SfxMatchContext_Impl();
    void                            Stop();
};

::vos::OMutex* SfxMatchContext_Impl::pDirMutex = 0;

::vos::OMutex* SfxMatchContext_Impl::GetMutex()
{
    ::vos::OGuard aGuard( ::vos::OMutex::getGlobalMutex() );
    if( !pDirMutex )
        pDirMutex = new ::vos::OMutex;
    return pDirMutex;
}

SfxMatchContext_Impl::SfxMatchContext_Impl(
    SfxURLBox* pBoxP, const String& rText )
    : aText(  rText )
    , pBox( pBoxP )
    , aLink( STATIC_LINK( this, SfxMatchContext_Impl, Select_Impl ) )
    , bStop( FALSE )
    , pCancel( new SfxCancellable( SFX_APP()->GetCancelManager(), rText ) )
    , bOnlyDirectories( pBoxP->bOnlyDirectories )
    , aBaseURL( pBoxP->aBaseURL )
{
    pURLs = new SvStringsDtor;
    pCompletions = new SvStringsDtor;

    // get copy of picklist because it is not threadsafe
    SfxPickList_Impl& rList = *SfxPickList_Impl::Get();
    USHORT nCount = (USHORT)rList.HistoryPickEntryCount();

    for( USHORT nPos = 0; nPos < nCount; nPos++ )
    {
        const StringPtr pStr = new String(rList.GetHistoryPickEntry( nPos )->aTitle );
        aPickList.Insert(pStr, nPos );
    }

    create();
}

SfxMatchContext_Impl::~SfxMatchContext_Impl()
{
    aLink.ClearPendingCall();
    delete pCancel;
    delete pURLs;
    delete pCompletions;
}

void SAL_CALL SfxMatchContext_Impl::Cancel()
{
    // Cancel button pressed
    terminate();
}

void SfxMatchContext_Impl::Stop()
{
    if( isRunning() )
    {
        bStop = TRUE;
        terminate();
    }
}

void SfxMatchContext_Impl::onTerminated( )
{
    // protect against concurrency
    aLink.CreateMutex();
    aLink.Call( this );
}

// This method is called via AsynchronLink, so it has the SolarMutex and calling solar code ( VCL ... )
// is safe. It is called when the thread is terminated ( finished work or stopped ). Cancelling the
// thread via Cancellable does not not discard the information gained so far, it inserts all collected
// completions into the listbox.
IMPL_STATIC_LINK( SfxMatchContext_Impl, Select_Impl, void*, pArg )
{
    if( pArg )
    {
        // avoid recursion through cancel button
        DELETEZ( pThis->pCancel );
        if( pThis->bStop )
        {
            // completions was stopped, no display
            delete pThis;
            return 0;
        }
    }

    SfxURLBox* pBox = pThis->pBox;
    pBox->bAutoCompleteMode = TRUE;

    // insert all completed strings into the listbox
    pBox->Clear();
    for( USHORT nPos = 0; nPos<pThis->pCompletions->Count(); nPos++ )
        pBox->InsertEntry( *(*pThis->pCompletions)[nPos] );

    if( pThis->pCompletions->Count() )
    {
        // select the first one
        String aTmp( pBox->GetEntry(0) );
        pBox->SetText( aTmp );
        pBox->SetSelection( Selection( pThis->aText.Len(), aTmp.Len() ) );
    }

    // transfer string lists to listbox and forget them
    delete pBox->pImp->pURLs;
    delete pBox->pImp->pCompletions;
    pBox->pImp->pURLs = pThis->pURLs;
    pBox->pImp->pCompletions = pThis->pCompletions;
    pThis->pURLs = NULL;
    pThis->pCompletions = NULL;

    // force listbox to resize ( it may be open )
    pBox->Resize();
    return 0;
}

void SfxMatchContext_Impl::Insert( const String& rCompletion, const String& rURL, BOOL bForce )
{
    if( !bForce )
    {
        // avoid doubles
        for( USHORT nPos = pCompletions->Count(); nPos--; )
            if( *(*pCompletions)[ nPos ] == rCompletion )
                return;
    }

    const StringPtr pCompletion = new String( rCompletion );
    pCompletions->Insert( pCompletion, pCompletions->Count() );
    const StringPtr pURL = new String( rURL );
    pURLs->Insert( pURL, pURLs->Count() );
}

void SfxMatchContext_Impl::ReadFolder( const String& rURL, const String& rMatch, BOOL bSmart )
{
    // check folder to scan
    if( !SfxContentHelper::IsFolder( rURL ) )
        return;

    // string to match with
    INetURLObject aMatchObj( rMatch );
    String aMatchName;

    if ( rURL != aMatchObj.GetMainURL() )
    {
        aMatchName = aMatchObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

        // matching is always done case insensitive, but completion will be case sensitive and case preserving
        aMatchName.ToLowerAscii();
    }

    xub_StrLen nMatchLen = aMatchName.Len();

    // get a sequence of all children of the folder
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aEntries =
                SfxContentHelper::GetFolderContentProperties( rURL, sal_True );

    const ::rtl::OUString* pEntries = aEntries.getConstArray();
    UINT32 nCount = aEntries.getLength();
    for ( UINT32 i=0; schedule() && i < nCount; i++ )
    {
        String aRow( pEntries[i] );
        String aTitle = aRow.GetToken( 0, '\t' );

        // matching is always done case insensitive, but completion will be case sensitive and case preserving
        aTitle.ToLowerAscii();

        String aURL = aRow.GetToken( 3, '\t' );
        sal_Bool bIsFolder = ( '1' == aRow.GetToken( 4, '\t' ).GetChar(0) );

        if( !nMatchLen || aMatchName.CompareTo( aTitle, nMatchLen ) == COMPARE_EQUAL )
        {
            // all names fit if matchstring is empty
            INetURLObject aObj( aURL );
            sal_Unicode aDelimiter = '/';
            if ( bSmart )
                // when parsing is done "smart", the delimiter must be "guessed"
                aObj.getFSysPath( INetURLObject::FSYS_DETECT, &aDelimiter );

            if ( bIsFolder )
                aObj.setFinalSlash();

            // get the last name of the URL
            String aMatch = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            String aInput( aText );
            if ( nMatchLen )
            {
                if ( aText.Len() && aText.GetChar( aText.Len() - 1 ) == '.' )
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

            Insert( aInput, aObj.GetMainURL(), TRUE );
        }
    }
}

String ParseSmart( String aText, String aBaseURL, String aWorkDir )
{
    String aMatch;
    USHORT nPos = aText.Search( ':' );
    if( nPos != STRING_NOTFOUND && ( nPos != 1 || aText.Len() < 3 || aText.GetChar( 2 ) != '\\' ) )
        // cannot be handled correctly by generic URLObject
        return aMatch;

    INetURLObject aURLObject;
    if( aBaseURL.Len() )
    {
        INetProtocol eBaseProt = INetURLObject::CompareProtocolScheme( aBaseURL );

        // if a base URL is set the string may be parsed relative
        if( aText.Search( '/' ) == 0 )
        {
            // text starting with slashes means absolute file URLs
            String aTemp = INetURLObject::GetScheme( eBaseProt );
            aTemp += aText;
            INetURLObject aTmp( aTemp );
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INET_PROT_NOT_VALID )
                aMatch = aTemp;
        }
        else
        {
            String aSmart( aText );
            INetURLObject aObj( aBaseURL );
            if( aText.Search( '\\' ) == 0 )
            {
                // cut to first segment
                String aTmp = INetURLObject::GetScheme( eBaseProt );
                aTmp += '/';
                aTmp += aObj.getName( 0, true, INetURLObject::DECODE_WITH_CHARSET );
                aObj.SetURL( aTmp );
                aSmart.Erase(0,1);
            }

            // base URL must be a directory !
            aObj.setFinalSlash();

            // take base URL and append current input
            bool bWasAbsolute = FALSE;
            INetURLObject aTmp( aObj.smartRel2Abs( aSmart, bWasAbsolute ) );
            if ( aText.GetChar( aText.Len() - 1 ) == '.' )
                // INetURLObject appends a final slash for the directories "." and "..", this is a bug!
                // Remove it as a workaround
                aTmp.removeFinalSlash();
            if ( !aTmp.HasError() && aTmp.GetProtocol() != INET_PROT_NOT_VALID )
                aMatch = aTmp.GetMainURL();
        }
    }
    else
    {
        ::utl::LocalFileHelper::ConvertSystemPathToURL( aText, aWorkDir, aMatch );
    }

    return aMatch;
}


void SfxMatchContext_Impl::run()
{
    ::vos::OGuard aGuard( GetMutex() );

    // Reset match lists
    pCompletions->Remove( 0, pCompletions->Count() );
    pURLs->Remove( 0, pURLs->Count() );

    // check for input
    USHORT nTextLen = aText.Len();
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
    if( eProt == INET_PROT_NOT_VALID || eProt == eSmartProt || eSmartProt == INET_PROT_NOT_VALID && eProt == eBaseProt )
    {
        // not stopped yet ?
        if( schedule() )
        {
            if ( eProt == INET_PROT_NOT_VALID )
                aMatch = ParseSmart( aText, aBaseURL, aWorkDir );
            else
                aMatch = aText;
            if ( aMatch.Len() )
            {
                INetURLObject aURLObject( aMatch );
                String aMainURL( aURLObject.GetMainURL() );
                if ( aMainURL.Len() )
                {
                    // if text input is a directory, it must be part of the match list! Until then it is scanned
                    if ( SfxContentHelper::IsFolder( aMainURL ) && aURLObject.hasFinalSlash() )
                        Insert( aText, aMatch );
                    else
                        // otherwise the parent folder will be taken
                        aURLObject.removeSegment();

                    // scan directory and insert all matches
                    ReadFolder( aURLObject.GetMainURL(), aMatch, eProt == INET_PROT_NOT_VALID );
                }
            }
        }
    }

    if ( bOnlyDirectories )
        // don't scan history picklist if only directories are allowed, picklist contains only files
        return;

    BOOL bFull = FALSE;
    int nCount = aPickList.Count();

    INetURLObject aCurObj;
    String aEmpty, aCurString, aCurMainURL;
    INetURLObject aObj;
    aObj.SetSmartProtocol( eSmartProt == INET_PROT_NOT_VALID ? INET_PROT_HTTP : eSmartProt );
    for( ;; )
    {
        for( USHORT nPos = 0; schedule() && nPos < nCount; nPos++ )
        {
            aCurObj.SetURL( *aPickList.GetObject( nPos ) );
            aCurObj.SetSmartURL( aCurObj.GetURLNoPass());
            aCurMainURL = aCurObj.GetMainURL();
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
                        if( aObj.GetURLPath().Len() > 1 )
                            continue;
                    }

                    aCurString = aCurMainURL;
                    if( eProt == INET_PROT_NOT_VALID )
                    {
                        // try if text matches the scheme
                        String aScheme( INetURLObject::GetScheme( aCurObj.GetProtocol() ) );
                        if ( aText.CompareTo( aScheme, aText.Len() ) == COMPARE_EQUAL && aText.Len() < aScheme.Len() )
                        {
                            if( bFull )
                                aMatch = aCurObj.GetMainURL();
                            else
                            {
                                aCurObj.SetMark( aEmpty );
                                aCurObj.SetParam( aEmpty );
                                aCurObj.SetURLPath( aEmpty );
                                aMatch = aCurObj.GetMainURL();
                            }

                            Insert( aMatch, aMatch );
                        }

                        // now try smart matching
                        aCurString.Erase( 0, aScheme.Len() );
                    }

                    if( aText.CompareTo( aCurString, aText.Len() )== COMPARE_EQUAL )
                    {
                        if( bFull )
                            aMatch = aCurObj.GetMainURL();
                        else
                        {
                            aCurObj.SetMark( aEmpty );
                            aCurObj.SetParam( aEmpty );
                            aCurObj.SetURLPath( aEmpty );
                            aMatch = aCurObj.GetMainURL();
                        }

                        String aURL( aMatch );
                        if( eProt == INET_PROT_NOT_VALID )
                            aMatch.Erase( 0, INetURLObject::GetScheme( aCurObj.GetProtocol() ).Len() );

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
/*                                      // PathToFileName is forbidden for generic parsing !
                    if( eProt == INET_PROT_NOT_VALID )
                    {
                        if( aCurObj.GetProtocol() == INET_PROT_FILE &&
                            aText.CompareTo( aCurObj.PathToFileName(), aText.Len() ) == COMPARE_EQUAL )
                        {
                            if( aText.Len() < aCurObj.PathToFileName().Len() )
                            {
                                String aTmp( aCurObj.PathToFileName() );
                                aTmp.Erase( 0, aText.Len() );
                                Insert( aTmp, aCurMainURL );
                            }
                        }
                    }
*/
                    break;
                }
            }
        }

        if( !bFull )
            bFull = TRUE;
        else
            break;
    }

    return;
}

void SfxURLBox::TryAutoComplete( BOOL bForward, BOOL bForce )
{
    if( Application::AnyInput( INPUT_KEYBOARD ) ) return;

    String aMatchString;
    String aCurText = GetText();
    Selection aSelection( GetSelection() );
    if( aSelection.Max() != aCurText.Len() && !bForce )
        return;
    USHORT nLen = (USHORT)aSelection.Min();
    aCurText.Erase( nLen );
    if( aCurText.Len() )
        pCtx = new SfxMatchContext_Impl( this, aCurText );
}

SfxURLBox::SfxURLBox( Window* pParent, INetProtocol eSmart )
    :   ComboBox( pParent , WB_DROPDOWN | WB_AUTOSIZE | WB_AUTOHSCROLL ),
        bTryAutoComplete( FALSE ),
        bAutoCompleteMode( FALSE ),
        bOnlyDirectories( FALSE ),
        bCtrlClick( FALSE ),
        pCtx( 0 ),
        eSmartProtocol( eSmart )
{
    pImp = new SfxURLBox_Impl();
    pImp->pURLs = NULL;
    pImp->pCompletions = NULL;
    SetHelpId( SID_OPENURL );
    EnableAutocomplete( FALSE );
    EnableDrop();
    if ( GetDesktopRectPixel().GetWidth() > 800 )
        SetSizePixel( Size( 300, 240 ) );
    else
        SetSizePixel( Size( 225, 240 ) );
    SetText( String() );

    GetSubEdit()->SetAutocompleteHdl( LINK( this, SfxURLBox, AutoCompleteHdl_Impl ) );
    UpdatePicklistForSmartProtocol_Impl();
}

SfxURLBox::SfxURLBox( Window* pParent, const ResId& _rResId, INetProtocol eSmart )
    :   ComboBox( pParent , _rResId ),
        bTryAutoComplete( FALSE ),
        bAutoCompleteMode( FALSE ),
        bOnlyDirectories( FALSE ),
        bCtrlClick( FALSE ),
        pCtx( 0 ),
        eSmartProtocol( eSmart )
{
    pImp = new SfxURLBox_Impl();
    pImp->pURLs = NULL;
    pImp->pCompletions = NULL;
    SetHelpId( SID_OPENURL );
    EnableAutocomplete( FALSE );
    EnableDrop();
    // no automatic sizing if loaded from a resource (the size should be specified therein)
    SetText( String() );

    GetSubEdit()->SetAutocompleteHdl( LINK( this, SfxURLBox, AutoCompleteHdl_Impl ) );
    UpdatePicklistForSmartProtocol_Impl();
}

SfxURLBox::~SfxURLBox()
{
    delete pImp->pURLs;
    delete pImp->pCompletions;
    delete pImp;
}

void SfxURLBox::SetSmartProtocol( INetProtocol eProt )
{
    if ( eSmartProtocol != eProt )
    {
        eSmartProtocol = eProt;
        UpdatePicklistForSmartProtocol_Impl();
    }
}

void SfxURLBox::UpdatePicklistForSmartProtocol_Impl()
{
    Clear();
    SfxPickList_Impl* pPickList = SfxPickList_Impl::Get();
    const ULONG nCount = pPickList->HistoryPickEntryCount();
    INetURLObject aCurObj;
    for( USHORT nPos = 0; nPos < nCount; nPos++ )
    {
        String aEntry = pPickList->GetHistoryPickEntry( nPos )->aTitle;
        if ( eSmartProtocol != INET_PROT_NOT_VALID )
        {
            aCurObj.SetURL( aEntry );
            if( aCurObj.GetProtocol() != eSmartProtocol )
                continue;
        }

        InsertEntry( aEntry );
    }
}

// **************************************************************************

BOOL SfxURLBox::QueryDrop( DropEvent &rEvt )
{
    return SFX_APP()->QueryDrop( rEvt );
}

// **************************************************************************

BOOL SfxURLBox::Drop( const DropEvent &rEvt )
{
    return SFX_APP()->Drop( (DropEvent&) rEvt );
}

// **************************************************************************

void SfxURLBox::OpenURL( const String& rName, BOOL bNew ) const
{
    SfxPickEntry_Impl* pEntry = SfxPickList_Impl::Get()->GetHistoryPickEntryFromTitle( rName );
    String aName;
    String aFilter;
    String aOptions;
    if ( pEntry )
    {
        aName = pEntry->aName;
        String aFilter = pEntry->aFilter;
        USHORT nPos = aFilter.Search( '|' );
        if( nPos != STRING_NOTFOUND )
        {
            aOptions = aFilter.Copy( nPos + 1 );
            aFilter.Erase( nPos + 1 );
        }
    }
    else
    {
        INetURLObject aObj( rName );
        if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
            aName = ParseSmart( rName, aBaseURL, SvtPathOptions().GetWorkPath() );
        else
            aName = rName;
    }

    if ( !aName.Len() )
        return;

    SfxViewFrame *pViewFrame = SfxViewFrame::Current();
    DBG_ASSERT( pViewFrame, "No ViewFrame ?!" );
    if ( pViewFrame )
    {
        pViewFrame = pViewFrame->GetTopViewFrame();
        SfxAllItemSet aSet( pViewFrame->GetPool() );
        aSet.Put( SfxStringItem( SID_FILE_NAME, aName ) );
        aSet.Put( SfxFrameItem( SID_DOCFRAME , pViewFrame ? pViewFrame->GetFrame() : 0 ) );
        aSet.Put( SfxBoolItem( SID_BROWSING, TRUE ) );
        aSet.Put( SfxStringItem( SID_REFERER, DEFINE_CONST_UNICODE(SFX_REFERER_USER) ) );
        if ( bNew )
            aSet.Put( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_blank") ) );

        if ( aFilter.Len() )
        {
            aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilter ) );
            aSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
        }

        SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENURL, SFX_CALLMODE_RECORD, aSet );
    }
}

// **************************************************************************

BOOL SfxURLBox::ProcessKey( const KeyCode& rKey )
{
    // every key input stops the current matching thread
    if( pCtx )
    {
        pCtx->Stop();
        pCtx = 0;
    }

    KeyCode aCode( rKey.GetCode() );
    if ( aCode == KEY_RETURN && GetText().Len() )
    {
        // wait for completion of matching thread
        ::vos::OGuard aGuard( SfxMatchContext_Impl::GetMutex() );

        if ( bAutoCompleteMode )
        {
            // reset picklist
            bAutoCompleteMode = FALSE;
            Selection aSelection( GetSelection() );
            SetSelection( Selection( aSelection.Min(), aSelection.Min() ) );
            if ( bOnlyDirectories )
                Clear();
            else
                UpdatePicklistForSmartProtocol_Impl();
            Resize();
        }

        bCtrlClick = rKey.IsMod1();
        BOOL bHandled = FALSE;
        if ( GetOpenHdl().IsSet() )
        {
            bHandled = TRUE;
            GetOpenHdl().Call(this);
        }
        else if ( GetSelectHdl().IsSet() )
        {
            bHandled = TRUE;
            GetSelectHdl().Call(this);
        }

        bCtrlClick = FALSE;

        ClearModifyFlag();
        return bHandled;
    }
    else if ( aCode == KEY_RETURN && !GetText().Len() && GetOpenHdl().IsSet() )
    {
        // for file dialog
        bAutoCompleteMode = FALSE;
        GetOpenHdl().Call(this);
        return TRUE;
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
        else if ( Application::IsInModalMode() )
        {
            return FALSE;
        }
        else
        {
            SfxViewFrame* pFrm = SfxViewFrame::Current();
            if( pFrm )
                pFrm->GetTopViewFrame()->GetFrame()->GrabFocusOnComponent_Impl();
        }

        bAutoCompleteMode = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// **************************************************************************

void SfxURLBox::Modify()
{
    ComboBox::Modify();
}

// **************************************************************************

long SfxURLBox::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetWindow() == GetSubEdit() && rNEvt.GetType() == EVENT_KEYINPUT )
    {

        const KeyEvent& rEvent = *rNEvt.GetKeyEvent();
        const KeyCode& rKey = rEvent.GetKeyCode();
        KeyCode aCode( rKey.GetCode() );
        if( ProcessKey( rKey ) )
        {
            return TRUE;
        }
        else if( ( aCode == KEY_UP || aCode == KEY_DOWN ) && !rKey.IsMod2() )
        {
            Selection aSelection( GetSelection() );
            USHORT nLen = (USHORT)aSelection.Min();
            GetSubEdit()->KeyInput( rEvent );
            SetSelection( Selection( nLen, GetText().Len() ) );
            return TRUE;
        }
    }

    return ComboBox::PreNotify( rNEvt );
}

IMPL_LINK( SfxURLBox, AutoCompleteHdl_Impl, void*, pVoid )
{
    if ( GetSubEdit()->GetAutocompleteAction() == AUTOCOMPLETE_KEYINPUT )
    {
        TryAutoComplete( TRUE, FALSE );
        return 1L;
    }

    return 0L;
}

// **************************************************************************

long SfxURLBox::Notify( NotifyEvent &rEvt )
{
    if ( EVENT_GETFOCUS == rEvt.GetType() )
    {
        SetSelection( Selection( 0, GetText().Len() ) );
    }
    else if ( EVENT_LOSEFOCUS == rEvt.GetType() )
    {
        if( !GetText().Len() )
            ClearModifyFlag();
        if ( pCtx )
        {
            pCtx->Stop();
            pCtx = 0;
        }
    }

    return ComboBox::Notify( rEvt );
}

// **************************************************************************

void SfxURLBox::Select()
{
    ComboBox::Select();
    ClearModifyFlag();
}

// **************************************************************************

void SfxURLBox::SetOnlyDirectories( BOOL bDir )
{
    bOnlyDirectories = bDir;
    if ( bOnlyDirectories )
        Clear();
}

//***************************************************************************
// SfxURLToolBoxControl_Impl
//***************************************************************************

SFX_IMPL_TOOLBOX_CONTROL(SfxURLToolBoxControl_Impl,SfxStringItem)

SfxURLToolBoxControl_Impl::SfxURLToolBoxControl_Impl( USHORT nId ,
                                                ToolBox& rBox ,
                                                SfxBindings& rBindings )
        : SfxToolBoxControl( nId , rBox , rBindings )
        , aURLForwarder( SID_CURRENT_URL, *this )
{
}

SfxURLBox* SfxURLToolBoxControl_Impl::GetURLBox() const
{
    return (SfxURLBox*) GetToolBox().GetItemWindow(GetId());
}

//***************************************************************************

Window* SfxURLToolBoxControl_Impl::CreateItemWindow( Window* pParent )
{
    SfxURLBox* pURLBox = new SfxURLBox( pParent );
    pURLBox->SetOpenHdl( LINK( this, SfxURLToolBoxControl_Impl, OpenHdl ) );
    pURLBox->SetSelectHdl( LINK( this, SfxURLToolBoxControl_Impl, SelectHdl ) );
    return pURLBox;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, SelectHdl, void*, pVoid )
{
    SfxURLBox* pURLBox = GetURLBox();
    String aName( pURLBox->GetText() );

    if ( !pURLBox->IsTravelSelect() && aName.Len() )
    {
/*
        aName = URIHelper::SmartRelToAbs( aName );
        SfxPickList_Impl*  pPickList = SfxPickList_Impl::Get();
        SfxPickEntry_Impl* pEntry = pPickList->GetHistoryPickEntryFromTitle( aName );
        if ( !pEntry )
            pPickList->SetCurHistoryPos( pURLBox->GetEntryPos( aName ) );
 */
        pURLBox->OpenURL( aName, FALSE );
    }

    return 1L;
}

IMPL_LINK( SfxURLToolBoxControl_Impl, OpenHdl, void*, pVoid )
{
    SfxURLBox* pURLBox = GetURLBox();
    pURLBox->OpenURL( pURLBox->GetText(), pURLBox->IsCtrlOpen() );
    SfxViewFrame* pFrm = SfxViewFrame::Current();
    if( pFrm )
        pFrm->GetFrame()->GrabFocusOnComponent_Impl();
    return 1L;
}

String SfxURLBox::GetURL()
{
    // wait for end of autocompletion
    ::vos::OGuard aGuard( SfxMatchContext_Impl::GetMutex() );

    String aText( GetText() );
    // try to get the right case preserving URL from the list of URLs
    if ( pImp->pCompletions && pImp->pURLs )
    {
        for( USHORT nPos=0; nPos<pImp->pCompletions->Count(); nPos++ )
        {
#ifdef DBG_UTIL
            String aTmp( *(*pImp->pCompletions)[ nPos ] );
#endif
            if( *(*pImp->pCompletions)[ nPos ] == aText )
                return *(*pImp->pURLs)[nPos];
        }
    }

    INetURLObject aObj( aText );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aName = ParseSmart( GetText(), aBaseURL, SvtPathOptions().GetWorkPath() );
        if ( aName.Len() )
            aObj.SetURL( aName );
        bool bSlash = aObj.hasFinalSlash();
        ::com::sun::star::uno::Any aAny = UCB_Helper::GetProperty( aObj.GetMainURL(), WID_TITLE );
        ::rtl::OUString aTitle;
        if ( aAny >>= aTitle )
            aObj.SetName( aTitle );
        if ( bSlash )
            aObj.setFinalSlash();
    }

    return aObj.GetMainURL();
}

//***************************************************************************

void SfxURLToolBoxControl_Impl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if( nSID == SID_FOCUSURLBOX )
    {
        if ( GetURLBox()->IsVisible() )
            GetURLBox()->GrabFocus();
    }
    else if ( !GetURLBox()->IsModified() && SFX_ITEM_AVAILABLE == eState )
    {
        SfxURLBox* pURLBox = GetURLBox();

        SfxPickList_Impl* pPickList = SfxPickList_Impl::Get();
        DBG_ASSERT( pPickList , "Pickliste invalid" );
        pURLBox->Clear();
        const ULONG nPickEntryCount = pPickList->HistoryPickEntryCount();
        ULONG nPickEntry;
        for ( nPickEntry = 0; nPickEntry < nPickEntryCount; ++nPickEntry )
        {
            DBG_ASSERT( pPickList->GetHistoryPickEntry( nPickEntry ),
                        "Pickentry ist invalid" );
            pURLBox->InsertEntry(
                        pPickList->GetHistoryPickEntry( nPickEntry )->aTitle );
        }

        const SfxStringItem *pURL = PTR_CAST(SfxStringItem,pState);
        String aRep( pURL->GetValue() );
        INetURLObject aURL( aRep );
        INetProtocol eProt = aURL.GetProtocol();
        pURLBox->SetText( aURL.GetURLNoPass() );
    }
}

//***************************************************************************
// SfxCancelToolBoxControl_Impl
//***************************************************************************

SFX_IMPL_TOOLBOX_CONTROL(SfxCancelToolBoxControl_Impl,SfxBoolItem)

//***************************************************************************

SfxCancelToolBoxControl_Impl::SfxCancelToolBoxControl_Impl
(
    USHORT nId,
    ToolBox& rBox,
    SfxBindings& rBindings
)
:   SfxToolBoxControl( nId, rBox, rBindings )
{
}

//***************************************************************************

SfxPopupWindowType SfxCancelToolBoxControl_Impl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

//***************************************************************************

SfxPopupWindow* SfxCancelToolBoxControl_Impl::CreatePopupWindow()
{
    PopupMenu aMenu;
    BOOL bExecute = FALSE, bSeparator = FALSE;
    USHORT nIndex = 1;
    for ( SfxCancelManager *pCancelMgr = SfxViewFrame::Current()->GetTopViewFrame()->GetCancelManager();
          pCancelMgr;
          pCancelMgr = pCancelMgr->GetParent() )
    {
        for ( USHORT n=0; n<pCancelMgr->GetCancellableCount(); ++n )
        {
            if ( !n && bSeparator )
            {
                aMenu.InsertSeparator();
                bSeparator = FALSE;
            }
            String aItemText = pCancelMgr->GetCancellable(n)->GetTitle();
            if ( aItemText.Len() > 50 )
            {
                aItemText.Erase( 48 );
                aItemText += DEFINE_CONST_UNICODE("...");
            }
            aMenu.InsertItem( nIndex++, aItemText );
            bExecute = TRUE;
            bSeparator = TRUE;
        }
    }

    ToolBox& rToolBox = GetToolBox();
    USHORT nId = bExecute ? nId = aMenu.Execute( &rToolBox, rToolBox.GetPointerPosPixel() ) : 0;
    GetToolBox().EndSelection();
    ClearCache();
    UpdateSlot();
    if ( nId )
    {
        String aSearchText = aMenu.GetItemText(nId);
        for ( SfxCancelManager *pCancelMgr = SfxViewFrame::Current()->GetTopViewFrame()->GetCancelManager();
              pCancelMgr;
              pCancelMgr = pCancelMgr->GetParent() )
        {
            for ( USHORT n = 0; n < pCancelMgr->GetCancellableCount(); ++n )
            {
                SfxCancellable *pCancel = pCancelMgr->GetCancellable(n);
                String aItemText = pCancel->GetTitle();
                if ( aItemText.Len() > 50 )
                {
                    aItemText.Erase( 48 );
                    aItemText += DEFINE_CONST_UNICODE("...");
                }

                if ( aItemText == aSearchText )
                {
                    pCancel->Cancel();
                    return 0;
                }
            }
        }

    }

    return 0;
}

//***************************************************************************

void SfxCancelToolBoxControl_Impl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    SfxVoidItem aVoidItem( nSID );
    //SfxToolBoxControl::StateChanged( nSID, eState, pState ? &aVoidItem : 0 );
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}
