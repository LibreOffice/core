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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/fileview.hxx>
#include <svtools/svtdata.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/svtools.hrc>
#include "fileview.hrc"
#include "contentenumeration.hxx"
#include <svtools/AccessibleBrowseBoxObjType.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <vcl/waitobj.hxx>
#include <com/sun/star/io/XPersist.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>
#include <memory>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/msgbox.hxx>
#ifndef INCLUDED_RTL_MATH_H
#include <rtl/math.hxx>
#endif
#include <tools/config.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <vos/timer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sound.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svl/urlfilter.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using ::svt::SortingData_Impl;
using ::svt::FolderDescriptor;
using ::vos::TTimeValue;
using ::rtl::OUString;

#define ALL_FILES_FILTER    "*.*"

#define COLUMN_TITLE        1
#define COLUMN_TYPE         2
#define COLUMN_SIZE         3
#define COLUMN_DATE         4

DECLARE_LIST( StringList_Impl, OUString* )

#define ROW_HEIGHT                17    // the height of a row has to be a little higher than the bitmap
#define QUICK_SEARCH_TIMEOUT    1500    // time in mSec before the quicksearch string will be reseted

namespace
{
    //====================================================================
    //= ReleaseSolarMutex
    //====================================================================
    struct ReleaseSolarMutex
    {
    private:
        sal_uLong   m_nCount;

    public:
        inline ReleaseSolarMutex()
        {
            m_nCount = Application::ReleaseSolarMutex();
        }
        inline ~ReleaseSolarMutex()
        {
            Application::AcquireSolarMutex( m_nCount );
        }
    };

    //====================================================================
    //= ITimeoutHandler
    //====================================================================
    class CallbackTimer;
    class ITimeoutHandler
    {
    public:
        virtual void onTimeout( CallbackTimer* _pInstigator ) = 0;
    };

    //====================================================================
    //= CallbackTimer
    //====================================================================
    class CallbackTimer : public ::vos::OTimer
    {
    protected:
        ITimeoutHandler* m_pTimeoutHandler;

    public:
        CallbackTimer( ITimeoutHandler* _pHandler ) : m_pTimeoutHandler( _pHandler ) { }

    protected:
        virtual void SAL_CALL onShot();
    };

    //--------------------------------------------------------------------
    void SAL_CALL CallbackTimer::onShot()
    {
        OSL_ENSURE( m_pTimeoutHandler, "CallbackTimer::onShot: nobody interested in?" );
        ITimeoutHandler* pHandler( m_pTimeoutHandler );
        if ( pHandler )
            pHandler->onTimeout( this );
    }

}

// -----------------------------------------------------------------------

static sal_Bool isHighContrast( const Window* _pView )
{
    return _pView->GetSettings().GetStyleSettings().GetHighContrastMode();
}

// -----------------------------------------------------------------------

void FilterMatch::createWildCardFilterList(const String& _rFilterList,::std::vector< WildCard >& _rFilters)
{
    if( _rFilterList.Len() )
    {// filter is given
        xub_StrLen nCount = _rFilterList.GetTokenCount();
        _rFilters.reserve( nCount );
        xub_StrLen nIndex = 0;
        OUString sToken;
        do
        {
            sToken = _rFilterList.GetToken( 0, ';', nIndex );
            if ( sToken.getLength() )
            {
                _rFilters.push_back( WildCard( sToken.toAsciiUpperCase() ) );
            }
        }
        while ( nIndex != STRING_NOTFOUND );
    }
    else
        // no filter is given -> match all
        _rFilters.push_back( WildCard( String::CreateFromAscii( "*" ) ) );
}
// class ViewTabListBox_Impl ---------------------------------------------

class ViewTabListBox_Impl : public SvHeaderTabListBox
{
private:
    Reference< XCommandEnvironment >    mxCmdEnv;

    ::osl::Mutex            maMutex;
    HeaderBar*              mpHeaderBar;
    SvtFileView_Impl*       mpParent;
    Timer                   maResetQuickSearch;
    OUString                maQuickSearchText;
    String                  msAccessibleDescText;
    String                  msFolder;
    String                  msFile;
    sal_uInt32              mnSearchIndex;
    sal_Bool                mbResizeDisabled        : 1;
    sal_Bool                mbAutoResize            : 1;
    sal_Bool                mbEnableDelete          : 1;
    sal_Bool                mbEnableRename          : 1;

    void            DeleteEntries();
    void            DoQuickSearch( const xub_Unicode& rChar );
    sal_Bool        Kill( const OUString& rURL );

protected:
    virtual sal_Bool            DoubleClickHdl();
    virtual ::rtl::OUString GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const;

public:
    ViewTabListBox_Impl( Window* pParentWin, SvtFileView_Impl* pParent, sal_Int16 nFlags );
   ~ViewTabListBox_Impl();

    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );

    void            ClearAll();
    HeaderBar*      GetHeaderBar() const { return mpHeaderBar; }

    void            EnableAutoResize() { mbAutoResize = sal_True; }
    void            EnableDelete( sal_Bool bEnable ) { mbEnableDelete = bEnable; }
    void            EnableRename( sal_Bool bEnable ) { mbEnableRename = bEnable; }
    sal_Bool        IsDeleteOrContextMenuEnabled() { return mbEnableDelete || IsContextMenuHandlingEnabled(); }

    Reference< XCommandEnvironment >    GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK( ResetQuickSearch_Impl, Timer * );

    virtual PopupMenu*  CreateContextMenu( void );
    virtual void        ExcecuteContextMenuAction( sal_uInt16 nSelectedPopentry );
};

// class HashedEntry --------------------------------------------------

class HashedEntry
{   // just a special String which can be compared on equality much faster
protected:
    OUString                maName;
    sal_Int32               mnHashCode;
public:
    inline                  HashedEntry( const OUString& rName );
    inline                  HashedEntry( const INetURLObject& rURL );
    inline                  HashedEntry( const HashedEntry& rCopy );
    virtual                 ~HashedEntry();

    inline sal_Bool operator    ==( const HashedEntry& rRef ) const;
    inline sal_Bool operator    !=( const HashedEntry& rRef ) const;

    inline const OUString&  GetName() const;
};

inline HashedEntry::HashedEntry( const OUString& rName ): maName( rName ), mnHashCode( rName.hashCode() )
{
}

inline HashedEntry::HashedEntry( const INetURLObject& rURL ):
    maName( rURL.GetMainURL( INetURLObject::NO_DECODE ) ),
    mnHashCode( maName.hashCode() )
{
}

inline HashedEntry::HashedEntry( const HashedEntry& r ): maName( r.maName ), mnHashCode( r.mnHashCode )
{
}

HashedEntry::~HashedEntry()
{
}

inline sal_Bool HashedEntry::operator ==( const HashedEntry& rRef ) const
{
    return mnHashCode == rRef.mnHashCode && maName.reverseCompareTo( rRef.maName ) == 0;
}

inline sal_Bool HashedEntry::operator !=( const HashedEntry& rRef ) const
{
    return mnHashCode != rRef.mnHashCode || maName.reverseCompareTo( rRef.maName ) != 0;
}

inline const OUString& HashedEntry::GetName() const
{
    return maName;
}

// class HashedEntryList ----------------------------------------------

class HashedEntryList : protected List
{// provides a list of _unique_ Entries
protected:
    inline HashedEntry*         First();
    inline HashedEntry*         Next();
    inline void                 Append( HashedEntry* pNewEntry );
public:
    virtual                     ~HashedEntryList();

    const HashedEntry*      Find( const OUString& rNameToSearchFor );
    const HashedEntry*      Find( const HashedEntry& rToSearchFor );
                                // not const, because First()/Next() is used
    using List::Insert;
    const HashedEntry&      Insert( HashedEntry* pInsertOrDelete );
                                // don't care about pInsertOrDelete after this any more and handle it as invalid!
                                // returns the Entry, which is effectively inserted

    void                    Clear();
};

inline HashedEntry* HashedEntryList::First()
{
    return ( HashedEntry* ) List::First();
}

inline HashedEntry* HashedEntryList::Next()
{
    return ( HashedEntry* ) List::Next();
}

inline void HashedEntryList::Append( HashedEntry* pNew )
{
    List::Insert( pNew, LIST_APPEND );
}

HashedEntryList::~HashedEntryList()
{
    Clear();
}

const HashedEntry* HashedEntryList::Find( const OUString& rRefName )
{   // simple linear search, which should be fast enough for this purpose
    HashedEntry  aRef( rRefName );
    HashedEntry* pIter = First();
    while( pIter && *pIter != aRef )
        pIter = Next();

    return pIter;
}

const HashedEntry* HashedEntryList::Find( const HashedEntry& rRef )
{   // simple linear search, which should be fast enough for this purpose
    HashedEntry* pIter = First();
    while( pIter && *pIter != rRef )
        pIter = Next();

    return pIter;
}

const HashedEntry& HashedEntryList::Insert( HashedEntry* pNew )
{   // inserts (appends) only, if entry doesn't already exists
    // if it already exists, pNew is deleted, because the caller must not worry about pNew any more

    DBG_ASSERT( pNew, "HashedEntryList::Insert(): NULL-pointer can't be inserted" );

    const HashedEntry* pSearch = Find( *pNew );
    if( pSearch )
    {
        delete pNew;
        return *pSearch;
    }

    Append( pNew );

    return *pNew;
}

void HashedEntryList::Clear()
{
    HashedEntry* p = First();
    while( p )
    {
        delete p;
        p = Next();
    }
}

// class NameTranslationEntry -----------------------------------------

class NameTranslationEntry : public HashedEntry
{// a fast compareble String and another String, which is used to get a substitution for a given String
protected:
    OUString                maTranslatedName;
public:
    inline                  NameTranslationEntry( const OUString& rOriginalName, const OUString& rTranslatedName );
    inline                  NameTranslationEntry( const ByteString& rOriginalName, const ByteString& rTranslatedName );

    inline const OUString&  GetTranslation() const;
};

inline NameTranslationEntry::NameTranslationEntry( const OUString& rOrg, const OUString& rTrans ):
    HashedEntry( rOrg ),
    maTranslatedName( rTrans )
{
}

inline NameTranslationEntry::NameTranslationEntry( const ByteString& rOrg, const ByteString& rTrans ):
    HashedEntry( OUString( rOrg.GetBuffer(), rOrg.Len(), RTL_TEXTENCODING_ASCII_US ) ),
    maTranslatedName( OUString( rTrans.GetBuffer(), rTrans.Len(), RTL_TEXTENCODING_UTF8 ) )
{
}

inline const OUString& NameTranslationEntry::GetTranslation() const
{
    return maTranslatedName;
}

// class NameTranslationList -----------------------------------------

class NameTranslationList : protected HashedEntryList
{   // contains a list of substitutes of strings for a given folder (as URL)
    // explanation of the circumstances see in remarks for Init();
protected:
    INetURLObject               maTransFile;    // URL of file with translation entries
    HashedEntry                 maHashedURL;    // for future purposes when dealing with a set of cached
                                                //  NameTranslationLists
private:
    const String            maTransFileName;
    void                    Init();             // reads the translation file and fills the (internal) list

public:
                            NameTranslationList( const INetURLObject& rBaseURL );
                                            // rBaseURL: path to folder for which the translation of the entries
                                            //  should be done

    using List::operator==;
    inline sal_Bool operator    ==( const HashedEntry& rRef ) const;
    using List::operator!=;
    inline sal_Bool operator    !=( const HashedEntry& rRef ) const;

    const OUString*             Translate( const OUString& rName ) const;
                                            // returns NULL, if rName can't be found

    inline void                 Update();   // clears list and init

    inline const String&        GetTransTableFileName() const;
                                            // returns the name for the file, which contains the translation strings
};

inline const String& NameTranslationList::GetTransTableFileName() const
{
    return maTransFileName;
}

void NameTranslationList::Init()
{
// Tries to read the file ".nametranslation.table" in the base folder. Complete path/name is in maTransFile.
// Further on, the found entries in the section "TRANSLATIONNAMES" are used to replace names in the
// base folder by translated ones. The translation must be given in UTF8
// See examples of such a files in the samples-folder of an Office installation

    try
    {
        ::ucbhelper::Content aTestContent( maTransFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );

        if( aTestContent.isDocument() )
        {// ... also tests the existence of maTransFile by throwing an Exception
            const sal_Char*     pSection = "TRANSLATIONNAMES";
            String          aFsysName( maTransFile.getFSysPath( INetURLObject::FSYS_DETECT ) );
            Config          aConfig( aFsysName );

            aConfig.SetGroup( ByteString( pSection ) );

            sal_uInt16          nKeyCnt = aConfig.GetKeyCount();

            for( sal_uInt16 nCnt = 0 ; nCnt < nKeyCnt ; ++nCnt )
                Insert( new NameTranslationEntry( aConfig.GetKeyName( nCnt ), aConfig.ReadKey( nCnt ) ) );
        }
    }
    catch( Exception const & ) {}
}

NameTranslationList::NameTranslationList( const INetURLObject& rBaseURL ):
    maTransFile( rBaseURL ),
    maHashedURL( rBaseURL ),
    maTransFileName( String::CreateFromAscii( ".nametranslation.table" ) )
{
    maTransFile.insertName( maTransFileName );
    Init();
}

inline sal_Bool NameTranslationList::operator ==( const HashedEntry& rRef ) const
{
    return maHashedURL == rRef;
}

inline sal_Bool NameTranslationList::operator !=( const HashedEntry& rRef ) const
{
    return maHashedURL != rRef;
}

const OUString* NameTranslationList::Translate( const OUString& rName ) const
{
    const NameTranslationEntry* pSearch = static_cast< const NameTranslationEntry* >(
                                        ( const_cast< NameTranslationList* >( this ) )->Find( rName ) );

    return pSearch? &pSearch->GetTranslation() : NULL;
}

inline void NameTranslationList::Update()
{
    Clear();
    Init();
}

// class NameTranslator_Impl ------------------------------------------

// enables the user to get string substitutions (translations for the content) for a given folder
// see more explanations above in the description for NameTranslationList
class NameTranslator_Impl : public ::svt::IContentTitleTranslation
{
private:
    NameTranslationList*    mpActFolder;
public:
                            NameTranslator_Impl( void );
                            NameTranslator_Impl( const INetURLObject& rActualFolder );
                            virtual ~NameTranslator_Impl();

     // IContentTitleTranslation
    virtual sal_Bool        GetTranslation( const OUString& rOriginalName, OUString& rTranslatedName ) const;

    void                    UpdateTranslationTable(); // reads the translation file again

    void                    SetActualFolder( const INetURLObject& rActualFolder );
    const String*           GetTransTableFileName() const;
                                            // returns the name for the file, which contains the translation strings
};

//====================================================================
//= SvtFileView_Impl
//====================================================================

class SvtFileView_Impl  :public ::svt::IEnumerationResultHandler
                        ,public ITimeoutHandler
{
protected:
    SvtFileView*                mpAntiImpl;
    Link                        m_aSelectHandler;

    ::rtl::Reference< ::svt::FileViewContentEnumerator >
                                        m_pContentEnumerator;
    Link                                m_aCurrentAsyncActionHandler;
    ::osl::Condition                    m_aAsyncActionFinished;
    ::rtl::Reference< ::vos::OTimer >   m_pCancelAsyncTimer;
    ::svt::EnumerationResult            m_eAsyncActionResult;
    bool                                m_bRunningAsyncAction;
    bool                                m_bAsyncActionCancelled;


public:

    ::std::vector< SortingData_Impl* >  maContent;
    ::osl::Mutex                        maMutex;

    ViewTabListBox_Impl*    mpView;
    NameTranslator_Impl*    mpNameTrans;
    const IUrlFilter*       mpUrlFilter;
    sal_uInt16              mnSortColumn;
    sal_Bool                mbAscending     : 1;
    sal_Bool                mbOnlyFolder    : 1;
    sal_Bool                mbReplaceNames  : 1;    // translate folder names or display doc-title instead of file name
    sal_Int16               mnSuspendSelectCallback : 1;
    sal_Bool                mbIsFirstResort : 1;

    IntlWrapper             aIntlWrapper;

    String                  maViewURL;
    String                  maAllFilter;
    String                  maCurrentFilter;
    Image                   maFolderImage;
    Link                    maOpenDoneLink;
    Reference< XCommandEnvironment >    mxCmdEnv;

    SvtFileView_Impl( SvtFileView* pAntiImpl, Reference < XCommandEnvironment > xEnv,
                                              sal_Int16 nFlags,
                                              sal_Bool bOnlyFolder );
    virtual                ~SvtFileView_Impl();

    void                    Clear();

    FileViewResult          GetFolderContent_Impl(
        const String& rFolder,
        const FileViewAsyncAction* pAsyncDescriptor,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList = ::com::sun::star::uno::Sequence< ::rtl::OUString >() );

    FileViewResult          GetFolderContent_Impl(
        const FolderDescriptor& _rFolder,
        const FileViewAsyncAction* pAsyncDescriptor,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList = ::com::sun::star::uno::Sequence< ::rtl::OUString >());
    void                    FilterFolderContent_Impl( const OUString &rFilter );
    void                    CancelRunningAsyncAction();

    void                    OpenFolder_Impl();
    // #83004# -------
    void                        ReplaceTabWithString( OUString& aValue );
    void                    CreateDisplayText_Impl();
    void                    CreateVector_Impl( const Sequence < OUString > &rList );
    void                    SortFolderContent_Impl();

    void                    EntryRemoved( const OUString& rURL );
    void                    EntryRenamed( OUString& rURL,
                                          const OUString& rName );
    String                  FolderInserted( const OUString& rURL,
                                            const OUString& rTitle );

    sal_uLong                   GetEntryPos( const OUString& rURL );

    inline void             EnableContextMenu( sal_Bool bEnable );
    inline void             EnableDelete( sal_Bool bEnable );

    void                    Resort_Impl( sal_Int16 nColumn, sal_Bool bAscending );
    sal_Bool                SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             sal_Bool bWrapAround );

    inline sal_Bool         EnableNameReplacing( sal_Bool bEnable = sal_True ); // returns false, if action wasn't possible
    void                    SetActualFolder( const INetURLObject& rActualFolder );

    sal_Bool                GetDocTitle( const OUString& rTargetURL, OUString& rDocTitle ) const;

    void                    SetSelectHandler( const Link& _rHdl );

    void                    InitSelection();
    void                    ResetCursor();

    inline void             EndEditing( bool _bCancel );

protected:
    DECL_LINK( SelectionMultiplexer, void* );

protected:
    // IEnumerationResultHandler overridables
    virtual void        enumerationDone( ::svt::EnumerationResult _eResult );
            void        implEnumerationSuccess();

    // ITimeoutHandler
    virtual void onTimeout( CallbackTimer* _pInstigator );
};

inline void SvtFileView_Impl::EnableContextMenu( sal_Bool bEnable )
{
    mpView->EnableContextMenuHandling( bEnable );
    if( bEnable )
        mbReplaceNames = sal_False;
}

inline void SvtFileView_Impl::EnableDelete( sal_Bool bEnable )
{
    mpView->EnableDelete( bEnable );
    if( bEnable )
        mbReplaceNames = sal_False;
}

inline sal_Bool SvtFileView_Impl::EnableNameReplacing( sal_Bool bEnable )
{
    mpView->EnableRename( bEnable );

    sal_Bool bRet;
    if( mpView->IsDeleteOrContextMenuEnabled() )
    {
        DBG_ASSERT( !mbReplaceNames, "SvtFileView_Impl::EnableNameReplacing(): state should be not possible!" );
        bRet = !bEnable; // only for enabling this is an unsuccessful result
    }
    else
    {
        mbReplaceNames = bEnable;
        bRet = sal_True;
    }

    return bRet;
}

inline void SvtFileView_Impl::EndEditing( bool _bCancel )
{
    if ( mpView->IsEditingActive() )
        mpView->EndEditing( _bCancel != false );
}

// functions -------------------------------------------------------------

OUString CreateExactSizeText_Impl( sal_Int64 nSize )
{
    double fSize( ( double ) nSize );
    int nDec;

    long nMega = 1024 * 1024;
    long nGiga = nMega * 1024;

    String aUnitStr = ' ';

    if ( nSize < 10000 )
    {
        aUnitStr += String( SvtResId( STR_SVT_BYTES ) );
        nDec = 0;
    }
    else if ( nSize < nMega )
    {
        fSize /= 1024;
        aUnitStr += String( SvtResId( STR_SVT_KB ) );
        nDec = 1;
    }
    else if ( nSize < nGiga )
    {
        fSize /= nMega;
        aUnitStr += String( SvtResId( STR_SVT_MB ) );
        nDec = 2;
    }
    else
    {
        fSize /= nGiga;
        aUnitStr += String( SvtResId( STR_SVT_GB ) );
        nDec = 3;
    }

    OUString aSizeStr( ::rtl::math::doubleToUString( fSize,
                rtl_math_StringFormat_F, nDec,
                SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0)));
    aSizeStr += aUnitStr;

    return aSizeStr;
}

// -----------------------------------------------------------------------
// class ViewTabListBox_Impl ---------------------------------------------
// -----------------------------------------------------------------------

ViewTabListBox_Impl::ViewTabListBox_Impl( Window* pParentWin,
                                          SvtFileView_Impl* pParent,
                                          sal_Int16 nFlags ) :

    SvHeaderTabListBox( pParentWin, WB_TABSTOP ),

    mpHeaderBar         ( NULL ),
    mpParent            ( pParent ),
    msAccessibleDescText( SvtResId( STR_SVT_ACC_DESC_FILEVIEW ) ),
    msFolder            ( SvtResId( STR_SVT_ACC_DESC_FOLDER ) ),
    msFile              ( SvtResId( STR_SVT_ACC_DESC_FILE ) ),
    mnSearchIndex       ( 0 ),
    mbResizeDisabled    ( sal_False ),
    mbAutoResize        ( sal_False ),
    mbEnableDelete      ( sal_True ),
    mbEnableRename      ( sal_True )

{
    Size aBoxSize = pParentWin->GetSizePixel();
    mpHeaderBar = new HeaderBar( pParentWin, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), mpHeaderBar->CalcWindowSizePixel() );

    HeaderBarItemBits nBits = ( HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE );
    if ( ( nFlags & FILEVIEW_SHOW_ALL ) == FILEVIEW_SHOW_ALL )
    {
        mpHeaderBar->InsertItem( COLUMN_TITLE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TITLE ) ), 180, nBits | HIB_UPARROW );
        mpHeaderBar->InsertItem( COLUMN_TYPE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TYPE ) ), 140, nBits );
        mpHeaderBar->InsertItem( COLUMN_SIZE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_SIZE ) ), 80, nBits );
        mpHeaderBar->InsertItem( COLUMN_DATE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_DATE ) ), 500, nBits );
    }
    else
        mpHeaderBar->InsertItem( COLUMN_TITLE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TITLE ) ), 600, nBits );

    Size aHeadSize = mpHeaderBar->GetSizePixel();
    SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                     Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    InitHeaderBar( mpHeaderBar );
    SetHighlightRange();
    SetEntryHeight( ROW_HEIGHT );

    Show();
    mpHeaderBar->Show();

    maResetQuickSearch.SetTimeout( QUICK_SEARCH_TIMEOUT );
    maResetQuickSearch.SetTimeoutHdl( LINK( this, ViewTabListBox_Impl, ResetQuickSearch_Impl ) );

    Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
               xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );

    mxCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    EnableContextMenuHandling();
}

// -----------------------------------------------------------------------

ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
    maResetQuickSearch.Stop();

    delete mpHeaderBar;
}

// -----------------------------------------------------------------------

IMPL_LINK( ViewTabListBox_Impl, ResetQuickSearch_Impl, Timer*, EMPTYARG )
{
    ::osl::MutexGuard aGuard( maMutex );

    maQuickSearchText = OUString();
    mnSearchIndex = 0;

    return 0;
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::Resize()
{
    SvTabListBox::Resize();
    Size aBoxSize = Control::GetParent()->GetOutputSizePixel();

    if ( mbResizeDisabled || !aBoxSize.Width() )
        return;

    Size aBarSize = mpHeaderBar->GetSizePixel();
    aBarSize.Width() = mbAutoResize ? aBoxSize.Width() : GetSizePixel().Width();
    mpHeaderBar->SetSizePixel( aBarSize );

    if ( mbAutoResize )
    {
        mbResizeDisabled = sal_True;
        Point aPos = GetPosPixel();
        SetPosSizePixel( Point( 0, aBarSize.Height() ),
                        Size( aBoxSize.Width(), aBoxSize.Height() - aBarSize.Height() ) );
        mbResizeDisabled = sal_False;
    }
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;

    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if ( 0 == rKeyCode.GetModifier() )
    {
        if ( rKeyCode.GetCode() == KEY_RETURN )
        {
            ResetQuickSearch_Impl( NULL );
            GetDoubleClickHdl().Call( this );
            bHandled = true;
        }
        else if ( ( rKeyCode.GetCode() == KEY_DELETE ) &&
                  mbEnableDelete )
        {
            ResetQuickSearch_Impl( NULL );
            DeleteEntries();
            bHandled = true;
        }
        else if ( ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_NUM ) ||
                  ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_ALPHA ) )
        {
            DoQuickSearch( rKEvt.GetCharCode() );
            bHandled = true;
        }
    }

    if ( !bHandled )
    {
        ResetQuickSearch_Impl( NULL );
        SvHeaderTabListBox::KeyInput( rKEvt );
    }
}

// -----------------------------------------------------------------------

PopupMenu* ViewTabListBox_Impl::CreateContextMenu( void )
{
    bool bEnableDelete = mbEnableDelete;
    bool bEnableRename = mbEnableRename;

    if ( bEnableDelete || bEnableRename )
    {
        sal_Int32 nSelectedEntries = GetSelectionCount();
        bEnableDelete &= nSelectedEntries > 0;
        bEnableRename &= nSelectedEntries == 1;
    }

    if ( bEnableDelete || bEnableRename )
    {
        SvLBoxEntry* pEntry = FirstSelected();
        while ( pEntry )
        {
            ::ucbhelper::Content aCnt;
            try
            {
                OUString aURL( static_cast< SvtContentEntry * >(
                    pEntry->GetUserData() )->maURL );
                aCnt = ::ucbhelper::Content( aURL, mxCmdEnv );
            }
            catch( Exception const & )
            {
                bEnableDelete = bEnableRename = false;
            }

            if ( bEnableDelete )
            {
                try
                {
                    Reference< XCommandInfo > aCommands = aCnt.getCommands();
                    if ( aCommands.is() )
                        bEnableDelete
                            = aCommands->hasCommandByName(
                                OUString::createFromAscii( "delete" ) );
                    else
                        bEnableDelete = false;
                }
                catch( Exception const & )
                {
                    bEnableDelete = false;
                }
            }

            if ( bEnableRename )
            {
                try
                {
                    Reference< XPropertySetInfo > aProps = aCnt.getProperties();
                    if ( aProps.is() )
                    {
                        Property aProp
                            = aProps->getPropertyByName(
                                OUString::createFromAscii( "Title" ) );
                        bEnableRename
                            = !( aProp.Attributes & PropertyAttribute::READONLY );
                    }
                    else
                        bEnableRename = false;
                }
                catch( Exception const & )
                {
                    bEnableRename = false;
                }
            }

            pEntry = ( bEnableDelete || bEnableRename )
                ? NextSelected( pEntry )
                : 0;
        }
    }

    if ( bEnableDelete || bEnableRename )
    {
        PopupMenu * pRet
            = new PopupMenu( SvtResId( RID_FILEVIEW_CONTEXTMENU ) );
        pRet->EnableItem( MID_FILEVIEW_DELETE, bEnableDelete );
        pRet->EnableItem( MID_FILEVIEW_RENAME, bEnableRename );
        pRet->RemoveDisabledEntries( sal_True, sal_True );
        return pRet;
    }

    return NULL;
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    switch ( nSelectedPopupEntry )
    {
        case MID_FILEVIEW_DELETE :
            DeleteEntries();
            break;

        case MID_FILEVIEW_RENAME :
            EditEntry( FirstSelected() );
            break;
    }
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::ClearAll()
{
    for ( sal_uInt16 i = 0; i < GetEntryCount(); ++i )
        delete (SvtContentEntry*)GetEntry(i)->GetUserData();
    Clear();
}

// -----------------------------------------------------------------------
void ViewTabListBox_Impl::DeleteEntries()
{
    svtools::QueryDeleteResult_Impl eResult = svtools::QUERYDELETE_YES;
    SvLBoxEntry* pEntry = FirstSelected();
    String aURL;

    ByteString sDialogPosition;
    while ( pEntry && ( eResult != svtools::QUERYDELETE_CANCEL ) )
    {
        SvLBoxEntry *pCurEntry = pEntry;
        pEntry = NextSelected( pEntry );

        if ( pCurEntry->GetUserData() )
            aURL = ( (SvtContentEntry*)pCurEntry->GetUserData() )->maURL;

        if ( !aURL.Len() )
            continue;

        bool canDelete = true;
        try
        {
            ::ucbhelper::Content aCnt( aURL, mxCmdEnv );
            Reference< XCommandInfo > aCommands = aCnt.getCommands();
            if ( aCommands.is() )
                canDelete
                    = aCommands->hasCommandByName(
                        OUString::createFromAscii( "delete" ) );
            else
                canDelete = false;
        }
        catch( Exception const & )
        {
            canDelete = false;
        }

        if (!canDelete)
            continue; // process next entry

        if ( eResult != svtools::QUERYDELETE_ALL )
        {
            INetURLObject aObj( aURL );
            svtools::QueryDeleteDlg_Impl aDlg( NULL, aObj.GetName( INetURLObject::DECODE_WITH_CHARSET ) );
            if ( sDialogPosition.Len() )
                aDlg.SetWindowState( sDialogPosition );

            if ( GetSelectionCount() > 1 )
                aDlg.EnableAllButton();

            if ( aDlg.Execute() == RET_OK )
                eResult = aDlg.GetResult();
            else
                eResult = svtools::QUERYDELETE_CANCEL;

            sDialogPosition = aDlg.GetWindowState( );
        }

        if ( ( eResult == svtools::QUERYDELETE_ALL ) ||
             ( eResult == svtools::QUERYDELETE_YES ) )
        {
            if ( Kill( aURL ) )
            {
                delete (SvtContentEntry*)pCurEntry->GetUserData();
                GetModel()->Remove( pCurEntry );
                mpParent->EntryRemoved( aURL );
            }
        }
    }
}

// -----------------------------------------------------------------------
sal_Bool ViewTabListBox_Impl::EditedEntry( SvLBoxEntry* pEntry,
                                 const XubString& rNewText )
{
    sal_Bool bRet = sal_False;

    OUString aURL;
    SvtContentEntry* pData = (SvtContentEntry*)pEntry->GetUserData();

    if ( pData )
        aURL = OUString( pData->maURL );

    if ( ! aURL.getLength() )
        return bRet;

    try
    {
        OUString aPropName = OUString::createFromAscii( "Title" );
        bool canRename = true;
        ::ucbhelper::Content aContent( aURL, mxCmdEnv );

        try
        {
            Reference< XPropertySetInfo > aProps = aContent.getProperties();
            if ( aProps.is() )
            {
                Property aProp = aProps->getPropertyByName( aPropName );
                canRename = !( aProp.Attributes & PropertyAttribute::READONLY );
            }
            else
            {
                canRename = false;
            }
        }
        catch ( Exception const & )
        {
            canRename = false;
        }

        if ( canRename )
        {
            Any aValue;
            aValue <<= OUString( rNewText );
            aContent.setPropertyValue( aPropName, aValue );
            mpParent->EntryRenamed( aURL, rNewText );

            pData->maURL = aURL;
            pEntry->SetUserData( pData );

            bRet = sal_True;
        }
    }
    catch( Exception const & )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------
void ViewTabListBox_Impl::DoQuickSearch( const xub_Unicode& rChar )
{
    ::osl::MutexGuard aGuard( maMutex );

    maResetQuickSearch.Stop();

    OUString    aLastText = maQuickSearchText;
    sal_uInt32  aLastPos = mnSearchIndex;
    sal_Bool    bFound = sal_False;

    maQuickSearchText += OUString( String( rChar ) ).toAsciiLowerCase();

    bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, sal_False );

    if ( !bFound && ( aLastText.getLength() == 1 ) &&
         ( aLastText == OUString( String( rChar ) ) ) )
    {
        mnSearchIndex = aLastPos + 1;
        maQuickSearchText = aLastText;
        bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, sal_True );
    }

    if ( bFound )
    {
        SvLBoxEntry* pEntry = GetEntry( mnSearchIndex );
        if ( pEntry )
        {
            SelectAll( sal_False );
            Select( pEntry );
            SetCurEntry( pEntry );
            MakeVisible( pEntry );
        }
        else
            bFound = sal_False;
    }

    if ( !bFound )
        Sound::Beep();

    maResetQuickSearch.Start();
}

// -----------------------------------------------------------------------
sal_Bool ViewTabListBox_Impl::DoubleClickHdl()
{
    SvHeaderTabListBox::DoubleClickHdl();
    return sal_False;
        // this means "do no additional handling". Especially this means that the SvImpLBox does not
        // recognize that the entry at the double click position change after the handler call (which is
        // the case if in the handler, our content was replaced)
        // If it _would_ recognize this change, it would take this as a reason to select the entry, again
        // - which is not what in the case of content replace
        // (I really doubt that this behaviour of the SvImpLBox does make any sense at all, but
        // who knows ...)
        // 07.12.2001 - 95727 - fs@openoffice.org
}

::rtl::OUString ViewTabListBox_Impl::GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    ::rtl::OUString sRet = SvHeaderTabListBox::GetAccessibleObjectDescription( _eType, _nPos );
    if ( ::svt::BBTYPE_TABLECELL == _eType )
    {
        sal_Int32 nRow = -1;
        const sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
            nRow = _nPos / nColumnCount;
        SvLBoxEntry* pEntry = GetEntry( nRow );
        if ( pEntry )
        {
            SvtContentEntry* pData = (SvtContentEntry*)pEntry->GetUserData();
            if ( pData )
            {
                static const String sVar1( RTL_CONSTASCII_USTRINGPARAM( "%1" ) );
                static const String sVar2( RTL_CONSTASCII_USTRINGPARAM( "%2" ) );
                String aText( msAccessibleDescText );
                aText.SearchAndReplace( sVar1, pData->mbIsFolder ? msFolder : msFile );
                aText.SearchAndReplace( sVar2, pData->maURL );
                sRet += ::rtl::OUString( aText );
            }
        }
    }

    return sRet;
}

// -----------------------------------------------------------------------
sal_Bool ViewTabListBox_Impl::Kill( const OUString& rContent )
{
    sal_Bool bRet = sal_True;

    try
    {
        ::ucbhelper::Content aCnt( rContent, mxCmdEnv );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException const & )
    {
        DBG_WARNING( "CommandAbortedException" );
        bRet = sal_False;
    }
    catch( Exception const & )
    {
        DBG_WARNING( "Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}




// -----------------------------------------------------------------------
// class SvtFileView -----------------------------------------------------
// -----------------------------------------------------------------------

SvtFileView::SvtFileView( Window* pParent, const ResId& rResId,
                          sal_Bool bOnlyFolder, sal_Bool bMultiSelection ) :

    Control( pParent, rResId )
{
    sal_Int8 nFlags = FILEVIEW_SHOW_ALL;
    if ( bOnlyFolder )
        nFlags |= FILEVIEW_ONLYFOLDER;
    if ( bMultiSelection )
        nFlags |= FILEVIEW_MULTISELECTION;

    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
        ::comphelper::getProcessServiceFactory()->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );
    Reference < XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    mpImp = new SvtFileView_Impl( this, xCmdEnv, nFlags, bOnlyFolder );
    mpImp->mpView->ForbidEmptyText();

    long pTabs[] = { 5, 20, 180, 320, 400, 600 };
    mpImp->mpView->SetTabs( &pTabs[0], MAP_PIXEL );
    mpImp->mpView->SetTabJustify( 2, AdjustRight ); // column "Size"

    if ( bMultiSelection )
        mpImp->mpView->SetSelectionMode( MULTIPLE_SELECTION );

    HeaderBar* pHeaderBar = mpImp->mpView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, SvtFileView, HeaderEndDrag_Impl ) );
}

SvtFileView::SvtFileView( Window* pParent, const ResId& rResId, sal_Int8 nFlags ) :

    Control( pParent, rResId )
{
    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
        ::comphelper::getProcessServiceFactory()->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );
    Reference < XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
    mpImp = new SvtFileView_Impl( this, xCmdEnv, nFlags,
                                  ( nFlags & FILEVIEW_ONLYFOLDER ) == FILEVIEW_ONLYFOLDER );

    if ( ( nFlags & FILEVIEW_SHOW_ALL ) == FILEVIEW_SHOW_ALL )
    {
        long pTabs[] = { 5, 20, 180, 320, 400, 600 };
        mpImp->mpView->SetTabs( &pTabs[0], MAP_PIXEL );
        mpImp->mpView->SetTabJustify( 2, AdjustRight ); // column "Size"
    }
    else
    {
        // show only title
        long pTabs[] = { 2, 20, 600 };
        mpImp->mpView->SetTabs( &pTabs[0], MAP_PIXEL );
    }

    if ( ( nFlags & FILEVIEW_MULTISELECTION ) == FILEVIEW_MULTISELECTION )
        mpImp->mpView->SetSelectionMode( MULTIPLE_SELECTION );

    HeaderBar *pHeaderBar = mpImp->mpView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, SvtFileView, HeaderEndDrag_Impl ) );
}

// -----------------------------------------------------------------------

SvtFileView::~SvtFileView()
{
    // use temp pointer to prevent access of deleted member (GetFocus())
    SvtFileView_Impl* pTemp = mpImp;
    mpImp = NULL;
    delete pTemp;
}

// -----------------------------------------------------------------------

void SvtFileView::OpenFolder( const Sequence< OUString >& aContents )
{
    mpImp->mpView->ClearAll();
    const OUString* pFileProperties  = aContents.getConstArray();
    sal_uInt32 i, nCount = aContents.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String aRow( pFileProperties[i] );
        // extract columns
        // the columns are: title, type, size, date, target url, is folder, image url
        String aTitle, aType, aSize, aDate, aURL, aImageURL;
        xub_StrLen nIdx = 0;
        aTitle = aRow.GetToken( 0, '\t', nIdx );
        aType = aRow.GetToken( 0, '\t', nIdx );
        aSize = aRow.GetToken( 0, '\t', nIdx );
        aDate = aRow.GetToken( 0, '\t', nIdx );
        aURL = aRow.GetToken( 0, '\t', nIdx );
        sal_Unicode cFolder = aRow.GetToken( 0, '\t', nIdx ).GetChar(0);
        sal_Bool bIsFolder = ( '1' == cFolder );
        if ( nIdx != STRING_NOTFOUND )
            aImageURL = aRow.GetToken( 0, '\t', nIdx );

        if ( mpImp->mbOnlyFolder && !bIsFolder )
            continue;

        // build new row
        String aNewRow = aTitle;
        aNewRow += '\t';
        aNewRow += aType;
        aNewRow += '\t';
        aNewRow += aSize;
        aNewRow += '\t';
        aNewRow += aDate;
        // detect image
        sal_Bool bDoInsert = sal_True;
        INetURLObject aObj( aImageURL.Len() > 0 ? aImageURL : aURL );
        Image aImage = SvFileInformationManager::GetImage( aObj, sal_False, isHighContrast( this ) );

        if ( bDoInsert )
        {
            // insert entry and set user data
            SvLBoxEntry* pEntry = mpImp->mpView->InsertEntry( aNewRow, aImage, aImage, NULL );
            SvtContentEntry* pUserData = new SvtContentEntry( aURL, bIsFolder );
            pEntry->SetUserData( pUserData );
        }
    }

    mpImp->InitSelection();
    mpImp->ResetCursor();
}

// -----------------------------------------------------------------------

String SvtFileView::GetURL( SvLBoxEntry* pEntry ) const
{
    String aURL;
    if ( pEntry && pEntry->GetUserData() )
        aURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;
    return aURL;
}

// -----------------------------------------------------------------------

String SvtFileView::GetCurrentURL() const
{
    String aURL;
    SvLBoxEntry* pEntry = mpImp->mpView->FirstSelected();
    if ( pEntry && pEntry->GetUserData() )
        aURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;
    return aURL;
}
// -----------------------------------------------------------------------------

sal_Bool SvtFileView::CreateNewFolder( const String& rNewFolder )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( mpImp->maViewURL );
    aObj.insertName( rNewFolder, false, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
    String sURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
    if ( ::utl::UCBContentHelper::MakeFolder( sURL, sal_True ) )
    {
        String sTitle = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
        String sEntry = mpImp->FolderInserted( sURL, sTitle );
        SvLBoxEntry* pEntry = mpImp->mpView->InsertEntry( sEntry, mpImp->maFolderImage, mpImp->maFolderImage );
        SvtContentEntry* pUserData = new SvtContentEntry( sURL, sal_True );
        pEntry->SetUserData( pUserData );
        mpImp->mpView->MakeVisible( pEntry );
        bRet = sal_True;
    }
    return bRet;
}

// -----------------------------------------------------------------------

FileViewResult SvtFileView::PreviousLevel( const FileViewAsyncAction* pAsyncDescriptor )
{
    FileViewResult eResult = eFailure;

    String sParentURL;
    if ( GetParentURL( sParentURL ) )
        eResult = Initialize( sParentURL, mpImp->maCurrentFilter, pAsyncDescriptor, mpBlackList );

    return eResult;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::GetParentURL( String& rParentURL ) const
{
    sal_Bool bRet = sal_False;
    try
    {
        ::ucbhelper::Content aCnt( mpImp->maViewURL, mpImp->mxCmdEnv );
        Reference< XContent > xContent( aCnt.get() );
        Reference< com::sun::star::container::XChild > xChild( xContent, UNO_QUERY );
        if ( xChild.is() )
        {
            Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
            if ( xParent.is() )
            {
                rParentURL = String( xParent->getIdentifier()->getContentIdentifier() );
                bRet = ( rParentURL.Len() > 0 && rParentURL != mpImp->maViewURL );
            }
        }
    }
    catch( Exception const & )
    {
        // perhaps an unkown url protocol (e.g. "private:newdoc")
    }

    return bRet;
}

// -----------------------------------------------------------------------

const rtl::OString& SvtFileView::GetHelpId( ) const
{
    return mpImp->mpView->GetHelpId( );
}

// -----------------------------------------------------------------------

void SvtFileView::SetHelpId( const rtl::OString& rHelpId )
{
    mpImp->mpView->SetHelpId( rHelpId );
}

// -----------------------------------------------------------------------

void SvtFileView::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    mpImp->mpView->SetSizePixel( rNewSize );
}

// -----------------------------------------------------------------------

void SvtFileView::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}

// -----------------------------------------------------------------------------
sal_Bool SvtFileView::Initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent, const String& rFilter  )
{
    WaitObject aWaitCursor( this );

    mpImp->Clear();
    ::ucbhelper::Content aContent(_xContent, mpImp->mxCmdEnv );
    FileViewResult eResult = mpImp->GetFolderContent_Impl( FolderDescriptor( aContent ), NULL );
    OSL_ENSURE( eResult != eStillRunning, "SvtFileView::Initialize: this was expected to be synchronous!" );
    if ( eResult != eSuccess )
        return sal_False;

    mpImp->FilterFolderContent_Impl( rFilter );

    mpImp->SortFolderContent_Impl(); // possibly not necessary!!!!!!!!!!
    mpImp->CreateDisplayText_Impl();
    mpImp->OpenFolder_Impl();

    mpImp->maOpenDoneLink.Call( this );
    return sal_True;
}

// -----------------------------------------------------------------------
FileViewResult SvtFileView::Initialize(
    const String& rURL,
    const String& rFilter,
    const FileViewAsyncAction* pAsyncDescriptor,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList  )
{
    WaitObject aWaitCursor( this );
    mpBlackList = rBlackList;

    String sPushURL( mpImp->maViewURL );

    mpImp->maViewURL = rURL;
    FileViewResult eResult = ExecuteFilter( rFilter, pAsyncDescriptor );
    switch ( eResult )
    {
    case eFailure:
    case eTimeout:
        mpImp->maViewURL = sPushURL;
        return eResult;

    case eStillRunning:
        OSL_ENSURE( pAsyncDescriptor, "SvtFileView::Initialize: we told it to read synchronously!" );
    case eSuccess:
        return eResult;
    }

    OSL_ENSURE( sal_False, "SvtFileView::Initialize: unreachable!" );
    return eFailure;
}

// -----------------------------------------------------------------------
FileViewResult SvtFileView::Initialize(
    const String& rURL,
    const String& rFilter,
    const FileViewAsyncAction* pAsyncDescriptor )
{
    return Initialize( rURL, rFilter, pAsyncDescriptor, ::com::sun::star::uno::Sequence< ::rtl::OUString >());
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
sal_Bool SvtFileView::Initialize( const Sequence< OUString >& aContents )
{
    WaitObject aWaitCursor( this );

    mpImp->maViewURL = String();
    mpImp->maCurrentFilter = mpImp->maAllFilter;

    mpImp->Clear();
    mpImp->CreateVector_Impl( aContents );
    mpImp->SortFolderContent_Impl();

    mpImp->OpenFolder_Impl();

    mpImp->maOpenDoneLink.Call( this );

    return sal_True;
}

// -----------------------------------------------------------------------

FileViewResult SvtFileView::ExecuteFilter( const String& rFilter, const FileViewAsyncAction* pAsyncDescriptor )
{
    mpImp->maCurrentFilter = rFilter;
    mpImp->maCurrentFilter.ToLowerAscii();

    mpImp->Clear();
    FileViewResult eResult = mpImp->GetFolderContent_Impl( mpImp->maViewURL, pAsyncDescriptor, mpBlackList );
    OSL_ENSURE( ( eResult != eStillRunning ) || pAsyncDescriptor, "SvtFileView::ExecuteFilter: we told it to read synchronously!" );
    return eResult;
}

// -----------------------------------------------------------------------

void SvtFileView::CancelRunningAsyncAction()
{
    mpImp->CancelRunningAsyncAction();
}

// -----------------------------------------------------------------------

void SvtFileView::SetNoSelection()
{
    mpImp->mpView->SelectAll( sal_False );
}

// -----------------------------------------------------------------------

void SvtFileView::GetFocus()
{
    Control::GetFocus();
    if ( mpImp && mpImp->mpView )
        mpImp->mpView->GrabFocus();
}

// -----------------------------------------------------------------------

void SvtFileView::ResetCursor()
{
    mpImp->ResetCursor();
}

// -----------------------------------------------------------------------

void SvtFileView::SetSelectHdl( const Link& rHdl )
{
    mpImp->SetSelectHandler( rHdl );
}

// -----------------------------------------------------------------------

void SvtFileView::SetDoubleClickHdl( const Link& rHdl )
{
    mpImp->mpView->SetDoubleClickHdl( rHdl );
}

// -----------------------------------------------------------------------

sal_uLong SvtFileView::GetSelectionCount() const
{
    return mpImp->mpView->GetSelectionCount();
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvtFileView::FirstSelected() const
{
    return mpImp->mpView->FirstSelected();
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvtFileView::NextSelected( SvLBoxEntry* pEntry ) const
{
    return mpImp->mpView->NextSelected( pEntry );
}

// -----------------------------------------------------------------------

void SvtFileView::EnableAutoResize()
{
    mpImp->mpView->EnableAutoResize();
}

// -----------------------------------------------------------------------

void SvtFileView::SetFocus()
{
    mpImp->mpView->GrabFocus();
}

// -----------------------------------------------------------------------
const String& SvtFileView::GetViewURL() const
{
    return mpImp->maViewURL;
}

// -----------------------------------------------------------------------
void SvtFileView::SetOpenDoneHdl( const Link& rHdl )
{
    mpImp->maOpenDoneLink = rHdl;
}

// -----------------------------------------------------------------------
void SvtFileView::EnableContextMenu( sal_Bool bEnable )
{
    mpImp->EnableContextMenu( bEnable );
}

// -----------------------------------------------------------------------
void SvtFileView::EnableDelete( sal_Bool bEnable )
{
    mpImp->EnableDelete( bEnable );
}

void SvtFileView::EnableNameReplacing( sal_Bool bEnable )
{
    mpImp->EnableNameReplacing( bEnable );
}

// -----------------------------------------------------------------------
void SvtFileView::EndInplaceEditing( bool _bCancel )
{
    return mpImp->EndEditing( _bCancel );
}

// -----------------------------------------------------------------------
IMPL_LINK( SvtFileView, HeaderSelect_Impl, HeaderBar*, pBar )
{
    DBG_ASSERT( pBar, "no headerbar" );
    sal_uInt16 nItemID = pBar->GetCurItemId();

    HeaderBarItemBits nBits;

    // clear the arrow of the recently used column
    if ( nItemID != mpImp->mnSortColumn )
    {
        if ( !nItemID )
        {
            // first call -> remove arrow from title column,
            // because another column is the sort column
            nItemID = mpImp->mnSortColumn;
            mpImp->mnSortColumn = COLUMN_TITLE;
        }
        nBits = pBar->GetItemBits( mpImp->mnSortColumn );
        nBits &= ~( HIB_UPARROW | HIB_DOWNARROW );
        pBar->SetItemBits( mpImp->mnSortColumn, nBits );
    }

    nBits = pBar->GetItemBits( nItemID );

    sal_Bool bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }

    pBar->SetItemBits( nItemID, nBits );
    mpImp->Resort_Impl( nItemID, !bUp );
    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( SvtFileView, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    if ( !pBar->IsItemMode() )
    {
        Size aSize;
        sal_uInt16 nTabs = pBar->GetItemCount();
        long nTmpSize = 0;

        for ( sal_uInt16 i = 1; i <= nTabs; ++i )
        {
            long nWidth = pBar->GetItemSize(i);
            aSize.Width() =  nWidth + nTmpSize;
            nTmpSize += nWidth;
            mpImp->mpView->SetTab( i, aSize.Width(), MAP_PIXEL );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------
String SvtFileView::GetConfigString() const
{
    String sRet;
    HeaderBar* pBar = mpImp->mpView->GetHeaderBar();
    DBG_ASSERT( pBar, "invalid headerbar" );

    // sort order
    sRet += String::CreateFromInt32( mpImp->mnSortColumn );
    sRet += ';';
    HeaderBarItemBits nBits = pBar->GetItemBits( mpImp->mnSortColumn );
    sal_Bool bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    sRet += bUp ? '1' : '0';
    sRet += ';';

    sal_uInt16 nCount = pBar->GetItemCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        sal_uInt16 nId = pBar->GetItemId(i);
        sRet += String::CreateFromInt32( nId );
        sRet += ';';
        sRet += String::CreateFromInt32( pBar->GetItemSize( nId ) );
        sRet += ';';
    }

    sRet.EraseTrailingChars( ';' );
    return sRet;
}

// -----------------------------------------------------------------------
void SvtFileView::SetConfigString( const String& rCfgStr )
{
    HeaderBar* pBar = mpImp->mpView->GetHeaderBar();
    DBG_ASSERT( pBar, "invalid headerbar" );

    sal_uInt16 nIdx = 0;
    mpImp->mnSortColumn = (sal_uInt16)rCfgStr.GetToken( 0, ';', nIdx ).ToInt32();
    sal_Bool bUp = (sal_Bool)(sal_uInt16)rCfgStr.GetToken( 0, ';', nIdx ).ToInt32();
    HeaderBarItemBits nBits = pBar->GetItemBits( mpImp->mnSortColumn );

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }
    pBar->SetItemBits( mpImp->mnSortColumn, nBits );

    while ( nIdx != STRING_NOTFOUND )
    {
        sal_uInt16 nItemId = (sal_uInt16)rCfgStr.GetToken( 0, ';', nIdx ).ToInt32();
        pBar->SetItemSize( nItemId, rCfgStr.GetToken( 0, ';', nIdx ).ToInt32() );
    }

    HeaderSelect_Impl( pBar );
    HeaderEndDrag_Impl( pBar );
}

// -----------------------------------------------------------------------
void SvtFileView::SetUrlFilter( const IUrlFilter* _pFilter )
{
    mpImp->mpUrlFilter = _pFilter;
}

// -----------------------------------------------------------------------
const IUrlFilter* SvtFileView::GetUrlFilter( ) const
{
    return mpImp->mpUrlFilter;
}

// -----------------------------------------------------------------------
void SvtFileView::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_ENABLE )
        Invalidate();
    Control::StateChanged( nStateChange );
}

// -----------------------------------------------------------------------
// class NameTranslator_Impl
// -----------------------------------------------------------------------

NameTranslator_Impl::NameTranslator_Impl( void ) :
    mpActFolder( NULL )
{
}

NameTranslator_Impl::NameTranslator_Impl( const INetURLObject& rActualFolder )
{
    mpActFolder = new NameTranslationList( rActualFolder );
}

NameTranslator_Impl::~NameTranslator_Impl()
{
    if( mpActFolder )
        delete mpActFolder;
}

void NameTranslator_Impl::UpdateTranslationTable()
{
    if( mpActFolder )
        mpActFolder->Update();
}

void NameTranslator_Impl::SetActualFolder( const INetURLObject& rActualFolder )
{
    HashedEntry aActFolder( rActualFolder );

    if( mpActFolder )
    {
        if( *mpActFolder != aActFolder )
        {
            delete mpActFolder;
            mpActFolder = new NameTranslationList( rActualFolder );
        }
    }
    else
        mpActFolder = new NameTranslationList( rActualFolder );
}

sal_Bool NameTranslator_Impl::GetTranslation( const OUString& rOrg, OUString& rTrans ) const
{
    sal_Bool bRet = sal_False;

    if( mpActFolder )
    {
        const OUString* pTrans = mpActFolder->Translate( rOrg );
        if( pTrans )
        {
            rTrans = *pTrans;
            bRet = sal_True;
        }
    }

    return bRet;
}

const String* NameTranslator_Impl::GetTransTableFileName() const
{
    return mpActFolder? &mpActFolder->GetTransTableFileName() : NULL;
}

// -----------------------------------------------------------------------
// class SvtFileView_Impl
// -----------------------------------------------------------------------

SvtFileView_Impl::SvtFileView_Impl( SvtFileView* pAntiImpl, Reference < XCommandEnvironment > xEnv, sal_Int16 nFlags, sal_Bool bOnlyFolder )

    :mpAntiImpl                 ( pAntiImpl )
    ,m_eAsyncActionResult       ( ::svt::ERROR )
    ,m_bRunningAsyncAction      ( false )
    ,m_bAsyncActionCancelled    ( false )
    ,mpNameTrans                ( NULL )
    ,mpUrlFilter                ( NULL )
    ,mnSortColumn               ( COLUMN_TITLE )
    ,mbAscending                ( sal_True )
    ,mbOnlyFolder               ( bOnlyFolder )
    ,mbReplaceNames             ( sal_False )
    ,mnSuspendSelectCallback    ( 0 )
    ,mbIsFirstResort            ( sal_True )
    ,aIntlWrapper               ( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() )
    ,maFolderImage              ( SvtResId( IMG_SVT_FOLDER ) )
    ,mxCmdEnv ( xEnv )

{
    maAllFilter = String::CreateFromAscii( "*.*" );
    mpView = new ViewTabListBox_Impl( mpAntiImpl, this, nFlags );
    mpView->EnableCellFocus();
}

// -----------------------------------------------------------------------
SvtFileView_Impl::~SvtFileView_Impl()
{
    Clear();

    // use temp pointer to prevent access of deleted member (GetFocus())
    ViewTabListBox_Impl* pTemp = mpView;
    mpView = NULL;
    delete pTemp;
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::Clear()
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
        delete (*aIt);

    maContent.clear();

    if( mpNameTrans )
        DELETEZ( mpNameTrans );
}

// -----------------------------------------------------------------------
FileViewResult SvtFileView_Impl::GetFolderContent_Impl(
    const String& rFolder,
    const FileViewAsyncAction* pAsyncDescriptor,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    // prepare name translation
    SetActualFolder( aFolderObj );

    FolderDescriptor aFolder( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) );

    aGuard.clear();
    return GetFolderContent_Impl( aFolder, pAsyncDescriptor, rBlackList );
}

// -----------------------------------------------------------------------
FileViewResult SvtFileView_Impl::GetFolderContent_Impl(
    const FolderDescriptor& _rFolder,
    const FileViewAsyncAction* pAsyncDescriptor,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList )
{
    DBG_TESTSOLARMUTEX();
    ::osl::ClearableMutexGuard aGuard( maMutex );

    OSL_ENSURE( !m_pContentEnumerator.is(), "SvtFileView_Impl::GetFolderContent_Impl: still running another enumeration!" );
    m_pContentEnumerator = new ::svt::FileViewContentEnumerator(
        mpView->GetCommandEnvironment(), maContent, maMutex, mbReplaceNames ? mpNameTrans : NULL );
        // TODO: should we cache and re-use this thread?

    if ( !pAsyncDescriptor )
    {
        ::svt::EnumerationResult eResult = m_pContentEnumerator->enumerateFolderContentSync( _rFolder, mpUrlFilter, rBlackList );
        if ( ::svt::SUCCESS == eResult )
        {
            implEnumerationSuccess();
            m_pContentEnumerator = NULL;
            return eSuccess;
        }
        m_pContentEnumerator = NULL;
        return eFailure;
    }

    m_bRunningAsyncAction = true;
    m_bAsyncActionCancelled = false;
    m_eAsyncActionResult = ::svt::ERROR;
    m_aAsyncActionFinished.reset();

    // don't (yet) set m_aCurrentAsyncActionHandler to pTimeout->aFinishHandler.
    // By definition, this handler *only* get's called when the result cannot be obtained
    // during the minimum wait time, so it is only set below, when needed.
    m_aCurrentAsyncActionHandler = Link();

    // minimum time to wait
    ::std::auto_ptr< TimeValue > pTimeout( new TimeValue );
    sal_Int32 nMinTimeout = pAsyncDescriptor->nMinTimeout;
    OSL_ENSURE( nMinTimeout > 0, "SvtFileView_Impl::GetFolderContent_Impl: invalid minimum timeout!" );
    if ( nMinTimeout <= 0 )
        nMinTimeout = sal_Int32( 1000L );
    pTimeout->Seconds = nMinTimeout / 1000L;
    pTimeout->Nanosec = ( nMinTimeout % 1000L ) * 1000000L;

    m_pContentEnumerator->enumerateFolderContent( _rFolder, mpUrlFilter, this );

    // wait until the enumeration is finished
    // for this, release our own mutex (which is used by the enumerator thread)
    aGuard.clear();

    ::osl::Condition::Result eResult = ::osl::Condition::result_ok;
    {
        // also release the SolarMutex. Not all code which is needed during the enumeration
        // is Solar-Thread-Safe, in particular there is some code which needs to access
        // string resources (and our resource system relies on the SolarMutex :()
        ReleaseSolarMutex aSolarRelease;

        // now wait. Note that if we didn't get an pAsyncDescriptor, then this is an infinite wait.
        eResult = m_aAsyncActionFinished.wait( pTimeout.get() );
    }

    ::osl::MutexGuard aGuard2( maMutex );
    if ( ::osl::Condition::result_timeout == eResult )
    {
        // maximum time to wait
        OSL_ENSURE( !m_pCancelAsyncTimer.get(), "SvtFileView_Impl::GetFolderContent_Impl: there's still a previous timer!" );
        m_pCancelAsyncTimer = new CallbackTimer( this );
        sal_Int32 nMaxTimeout = pAsyncDescriptor->nMaxTimeout;
        OSL_ENSURE( nMaxTimeout > nMinTimeout,
            "SvtFileView_Impl::GetFolderContent_Impl: invalid maximum timeout!" );
        if ( nMaxTimeout <= nMinTimeout )
            nMaxTimeout = nMinTimeout + 5000;
        m_pCancelAsyncTimer->setRemainingTime( TTimeValue( nMaxTimeout - nMinTimeout ) );
            // we already waited for nMinTimeout milliseconds, so take this into account
        m_pCancelAsyncTimer->start();

        m_aCurrentAsyncActionHandler = pAsyncDescriptor->aFinishHandler;
        DBG_ASSERT( m_aCurrentAsyncActionHandler.IsSet(), "SvtFileView_Impl::GetFolderContent_Impl: nobody interested when it's finished?" );
        mpView->ClearAll();
        return eStillRunning;
    }

    m_bRunningAsyncAction = false;
    switch ( m_eAsyncActionResult )
    {
    case ::svt::SUCCESS:
        return eSuccess;

    case ::svt::ERROR:
        return eFailure;

    case ::svt::RUNNING:
        return eStillRunning;
    }

    DBG_ERRORFILE( "SvtFileView_Impl::GetFolderContent_Impl: unreachable!" );
    return eFailure;
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::FilterFolderContent_Impl( const OUString &rFilter )
{
    sal_Bool bHideTransFile = mbReplaceNames && mpNameTrans;

    String sHideEntry;
    if( bHideTransFile )
    {
        const String* pTransTableFileName = mpNameTrans->GetTransTableFileName();
        if( pTransTableFileName )
        {
            sHideEntry = *pTransTableFileName;
            sHideEntry.ToUpperAscii();
        }
        else
            bHideTransFile = sal_False;
    }

    if ( !bHideTransFile &&
        ( !rFilter.getLength() || ( rFilter.compareToAscii( ALL_FILES_FILTER ) == COMPARE_EQUAL ) ) )
        // when replacing names, there is always something to filter (no view of ".nametranslation.table")
        return;

    ::osl::MutexGuard aGuard( maMutex );

    if ( maContent.empty() )
        return;

    // count (estimate) the number of filter tokens
    sal_Int32 nTokens=0;
    const sal_Unicode* pStart = rFilter.getStr();
    const sal_Unicode* pEnd = pStart + rFilter.getLength();
    while ( pStart != pEnd )
        if ( *pStart++ == ';' )
            ++nTokens;

    // collect the filter tokens
    ::std::vector< WildCard > aFilters;
    FilterMatch::createWildCardFilterList(rFilter,aFilters);


    // do the filtering
    ::std::vector< SortingData_Impl* >::iterator aContentLoop = maContent.begin();
    String sCompareString;
    do
    {
        if ( (*aContentLoop)->mbIsFolder )
            ++aContentLoop;
        else
        {
            // normalize the content title (we always match case-insensitive)
            // 91872 - 11.09.2001 - frank.schoenheit@sun.com
            sCompareString = (*aContentLoop)->GetFileName(); // filter works on file name, not on title!
            sal_Bool bDelete;

            if( bHideTransFile && sCompareString == sHideEntry )
                bDelete = sal_True;
            else
            {
                // search for the first filter which matches
                ::std::vector< WildCard >::const_iterator pMatchingFilter =
                    ::std::find_if(
                        aFilters.begin(),
                        aFilters.end(),
                        FilterMatch( sCompareString )
                    );

                bDelete = aFilters.end() == pMatchingFilter;
            }

            if( bDelete )
            {
                // none of the filters did match
                delete (*aContentLoop);

                if ( maContent.begin() == aContentLoop )
                {
                    maContent.erase( aContentLoop );
                    aContentLoop = maContent.begin();
                }
                else
                {
                    std::vector< SortingData_Impl* >::iterator aDelete = aContentLoop;
                    --aContentLoop; // move the iterator to a position which is not invalidated by the erase
                    maContent.erase( aDelete );
                    ++aContentLoop; // this is now the next one ....
                }
            }
            else
                ++aContentLoop;
        }
    }
    while ( aContentLoop != maContent.end() );
}

// -----------------------------------------------------------------------
IMPL_LINK( SvtFileView_Impl, SelectionMultiplexer, void*, _pSource )
{
    return mnSuspendSelectCallback ? 0L : m_aSelectHandler.Call( _pSource );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::SetSelectHandler( const Link& _rHdl )
{
    m_aSelectHandler = _rHdl;

    Link aMasterHandler;
    if ( m_aSelectHandler.IsSet() )
        aMasterHandler = LINK( this, SvtFileView_Impl, SelectionMultiplexer );

    mpView->SetSelectHdl( aMasterHandler );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::InitSelection()
{
    mpView->SelectAll( sal_False );
    SvLBoxEntry* pFirst = mpView->First();
    if ( pFirst )
        mpView->SetCursor( pFirst, sal_True );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::OpenFolder_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    mpView->SetUpdateMode( sal_False );
    mpView->ClearAll();

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( mbOnlyFolder && ! (*aIt)->mbIsFolder )
            continue;

        // insert entry and set user data
        SvLBoxEntry* pEntry = mpView->InsertEntry( (*aIt)->maDisplayText,
                                                   (*aIt)->maImage,
                                                   (*aIt)->maImage );

        SvtContentEntry* pUserData = new SvtContentEntry( (*aIt)->maTargetURL,
                                                          (*aIt)->mbIsFolder );
        pEntry->SetUserData( pUserData );
    }

    InitSelection();

    ++mnSuspendSelectCallback;
    mpView->SetUpdateMode( sal_True );
    --mnSuspendSelectCallback;

    ResetCursor();
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::ResetCursor()
{
    // deselect
    SvLBoxEntry* pEntry = mpView->FirstSelected();
    if ( pEntry )
        mpView->Select( pEntry, sal_False );
    // set cursor to the first entry
    mpView->SetCursor( mpView->First(), sal_True );
    mpView->Update();
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::CancelRunningAsyncAction()
{
    DBG_TESTSOLARMUTEX();
    ::osl::MutexGuard aGuard( maMutex );
    if ( !m_pContentEnumerator.is() )
        return;

    m_bAsyncActionCancelled = true;
    m_pContentEnumerator->cancel();
    m_bRunningAsyncAction = false;

    m_pContentEnumerator = NULL;
    if ( m_pCancelAsyncTimer.is() && m_pCancelAsyncTimer->isTicking() )
        m_pCancelAsyncTimer->stop();
    m_pCancelAsyncTimer = NULL;
}

//-----------------------------------------------------------------------
void SvtFileView_Impl::onTimeout( CallbackTimer* )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( maMutex );
    if ( !m_bRunningAsyncAction )
        // there might have been a race condition while we waited for the mutex
        return;

    CancelRunningAsyncAction();

    if ( m_aCurrentAsyncActionHandler.IsSet() )
    {
        Application::PostUserEvent( m_aCurrentAsyncActionHandler, reinterpret_cast< void* >( eTimeout ) );
        m_aCurrentAsyncActionHandler = Link();
    }
}

//-----------------------------------------------------------------------
void SvtFileView_Impl::enumerationDone( ::svt::EnumerationResult _eResult )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( maMutex );

    m_pContentEnumerator = NULL;
    if ( m_pCancelAsyncTimer.is() && m_pCancelAsyncTimer->isTicking() )
        m_pCancelAsyncTimer->stop();
    m_pCancelAsyncTimer = NULL;

    if ( m_bAsyncActionCancelled )
        // this is to prevent race conditions
        return;

    m_eAsyncActionResult = _eResult;
    m_bRunningAsyncAction = false;

    m_aAsyncActionFinished.set();

    if ( svt::SUCCESS == _eResult )
        implEnumerationSuccess();

    if ( m_aCurrentAsyncActionHandler.IsSet() )
    {
        Application::PostUserEvent( m_aCurrentAsyncActionHandler, reinterpret_cast< void* >( m_eAsyncActionResult ) );
        m_aCurrentAsyncActionHandler = Link();
    }
}

//-----------------------------------------------------------------------
void SvtFileView_Impl::implEnumerationSuccess()
{
    FilterFolderContent_Impl( maCurrentFilter );
    SortFolderContent_Impl();
    CreateDisplayText_Impl();
    OpenFolder_Impl();
    maOpenDoneLink.Call( mpAntiImpl );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::ReplaceTabWithString( OUString& aValue )
{
    OUString aTab     = OUString::createFromAscii( "\t" );
    OUString aTabString = OUString::createFromAscii( "%09" );
    sal_Int32 iPos;

    while ( ( iPos = aValue.indexOf( aTab ) ) >= 0 )
       aValue = aValue.replaceAt( iPos, 1, aTabString );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::CreateDisplayText_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString aValue;
    OUString aTab     = OUString::createFromAscii( "\t" );
    OUString aDateSep = OUString::createFromAscii( ", " );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        // title, type, size, date
        aValue = (*aIt)->GetTitle();
        // #83004# --------------------
        ReplaceTabWithString( aValue );
        aValue += aTab;
        aValue += (*aIt)->maType;
        aValue += aTab;
        // folders don't have a size
        if ( ! (*aIt)->mbIsFolder )
            aValue += CreateExactSizeText_Impl( (*aIt)->maSize );
        aValue += aTab;
        // set the date, but volumes have no date
        if ( ! (*aIt)->mbIsFolder || ! (*aIt)->mbIsVolume )
        {
            SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
            aValue += rLocaleData.getDate( (*aIt)->maModDate );
            aValue += aDateSep;
            aValue += rLocaleData.getTime( (*aIt)->maModDate );
        }
        (*aIt)->maDisplayText = aValue;

        // detect image
        if ( (*aIt)->mbIsFolder )
        {
            ::svtools::VolumeInfo aVolInfo( (*aIt)->mbIsVolume, (*aIt)->mbIsRemote,
                                            (*aIt)->mbIsRemoveable, (*aIt)->mbIsFloppy,
                                            (*aIt)->mbIsCompactDisc );
            (*aIt)->maImage = SvFileInformationManager::GetFolderImage( aVolInfo, sal_False, isHighContrast( mpView ) );
        }
        else
            (*aIt)->maImage = SvFileInformationManager::GetFileImage( INetURLObject( (*aIt)->maTargetURL ), sal_False, isHighContrast( mpView ));
    }
}

// -----------------------------------------------------------------------
// this function converts the sequence of strings into a vector of SortingData
// the string should have the form :
// title \t type \t size \t date \t target url \t is folder \t image url

void SvtFileView_Impl::CreateVector_Impl( const Sequence < OUString > &rList )
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString aTab     = OUString::createFromAscii( "\t" );

    sal_uInt32 nCount = (sal_uInt32) rList.getLength();

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SortingData_Impl*   pEntry = new SortingData_Impl;
        OUString            aValue = rList[i];
        OUString            aDisplayText;
        sal_Int32           nIndex = 0;

        // get the title
        pEntry->SetNewTitle( aValue.getToken( 0, '\t', nIndex ) );
        aDisplayText = pEntry->GetTitle();
        // #83004# --------------------
        ReplaceTabWithString( aDisplayText );
        aDisplayText += aTab;

        // get the type
        if ( nIndex >= 0 )
        {
            pEntry->maType = aValue.getToken( 0, '\t', nIndex );
            aDisplayText += pEntry->maType;
        }
        aDisplayText += aTab;

        // get the size
        if ( nIndex >= 0 )
        {
            OUString aSize = aValue.getToken( 0, '\t', nIndex );
            aDisplayText += aSize;

            if ( aSize.getLength() )
                pEntry->maSize = aSize.toInt64();
        }
        aDisplayText += aTab;

        // get the date
        if ( nIndex >= 0 )
        {
            OUString aDate = aValue.getToken( 0, '\t', nIndex );
            aDisplayText += aDate;

            if ( aDate.getLength() )
            {
                DBG_ERRORFILE( "Don't know, how to convert date" );
                ;// convert date string to date
            }
        }
        // get the target url
        if ( nIndex >= 0 )
        {
            pEntry->maTargetURL = aValue.getToken( 0, '\t', nIndex );
        }
        // get the size
        if ( nIndex >= 0 )
        {
            OUString aBool = aValue.getToken( 0, '\t', nIndex );
            if ( aBool.getLength() )
                pEntry->mbIsFolder = aBool.toBoolean();
        }
        // get the image url
        if ( nIndex >= 0 )
        {
            pEntry->maImageURL = aValue.getToken( 0, '\t', nIndex );
        }

        // set the display text
        pEntry->maDisplayText = aDisplayText;

        // detect the image
        INetURLObject aObj( pEntry->maImageURL.getLength() ? pEntry->maImageURL : pEntry->maTargetURL );
        pEntry->maImage = SvFileInformationManager::GetImage( aObj, sal_False, isHighContrast( mpView ) );

        maContent.push_back( pEntry );
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::Resort_Impl( sal_Int16 nColumn, sal_Bool bAscending )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( ( nColumn == mnSortColumn ) &&
         ( bAscending == mbAscending ) )
         return;

    // reset the quick search index
    mpView->ResetQuickSearch_Impl( NULL );

    String aEntryURL;
    SvLBoxEntry* pEntry = mpView->GetCurEntry();
    if ( pEntry && pEntry->GetUserData() )
        aEntryURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;

    mnSortColumn = nColumn;
    mbAscending = bAscending;

    SortFolderContent_Impl();
    OpenFolder_Impl();

    if ( !mbIsFirstResort )
    {
        sal_uLong nPos = GetEntryPos( aEntryURL );
        if ( nPos < mpView->GetEntryCount() )
        {
            pEntry = mpView->GetEntry( nPos );

            ++mnSuspendSelectCallback;  // #i15668# - 2004-04-25 - fs@openoffice.org
            mpView->SetCurEntry( pEntry );
            --mnSuspendSelectCallback;
        }
    }
    else
        mbIsFirstResort = sal_False;
}

// -----------------------------------------------------------------------
static sal_Bool                 gbAscending = sal_True;
static sal_Int16                gnColumn = COLUMN_TITLE;
static const CollatorWrapper*   pCollatorWrapper = NULL;

/* this functions returns true, if aOne is less then aTwo
*/
sal_Bool CompareSortingData_Impl( SortingData_Impl* const aOne, SortingData_Impl* const aTwo )
{
    DBG_ASSERT( pCollatorWrapper, "*CompareSortingData_Impl(): Can't work this way!" );

    sal_Int32   nComp;
    sal_Bool    bRet = sal_False;
    sal_Bool    bEqual = sal_False;

    if ( aOne->mbIsFolder != aTwo->mbIsFolder )
    {
        if ( aOne->mbIsFolder )
            bRet = sal_True;
        else
            bRet = sal_False;

        // !!! pb: #100376# folder always on top
        if ( !gbAscending )
            bRet = !bRet;
    }
    else
    {
        switch ( gnColumn )
        {
            case COLUMN_TITLE:
                // compare case insensitiv first
                nComp = pCollatorWrapper->compareString( aOne->GetLowerTitle(), aTwo->GetLowerTitle() );

                if ( nComp == 0 )
                    nComp = pCollatorWrapper->compareString( aOne->GetTitle(), aTwo->GetTitle() );

                if ( nComp < 0 )
                    bRet = sal_True;
                else if ( nComp > 0 )
                    bRet = sal_False;
                else
                    bEqual = sal_True;
                break;
            case COLUMN_TYPE:
                nComp = pCollatorWrapper->compareString( aOne->maType, aTwo->maType );
                if ( nComp < 0 )
                    bRet = sal_True;
                else if ( nComp > 0 )
                    bRet = sal_False;
                else
                    bEqual = sal_True;
                break;
            case COLUMN_SIZE:
                if ( aOne->maSize < aTwo->maSize )
                    bRet = sal_True;
                else if ( aOne->maSize > aTwo->maSize )
                    bRet = sal_False;
                else
                    bEqual = sal_True;
                break;
            case COLUMN_DATE:
                if ( aOne->maModDate < aTwo->maModDate )
                    bRet = sal_True;
                else if ( aOne->maModDate > aTwo->maModDate )
                    bRet = sal_False;
                else
                    bEqual = sal_True;
                break;
            default:
                DBG_WARNING( "CompareSortingData_Impl: Compare unknown type!" );
                bRet = sal_False;
        }
    }

    // when the two elements are equal, we must not return sal_True (which would
    // happen if we just return ! ( a < b ) when not sorting ascending )
    if ( bEqual )
        return sal_False;

    return gbAscending ? bRet : !bRet;
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::SortFolderContent_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_uInt32 nSize = maContent.size();

    if ( nSize > 1 )
    {
        gbAscending = mbAscending;
        gnColumn = mnSortColumn;
        pCollatorWrapper = aIntlWrapper.getCaseCollator();

        std::stable_sort( maContent.begin(), maContent.end(), CompareSortingData_Impl );

        pCollatorWrapper = NULL;
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::EntryRemoved( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( (*aIt)->maTargetURL == rURL )
        {
            maContent.erase( aIt );
            break;
        }
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::EntryRenamed( OUString& rURL,
                                     const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( (*aIt)->maTargetURL == rURL )
        {
            (*aIt)->SetNewTitle( rTitle );
            OUString aDisplayText = (*aIt)->maDisplayText;
            sal_Int32 nIndex = aDisplayText.indexOf( '\t' );

            if ( nIndex > 0 )
                (*aIt)->maDisplayText = aDisplayText.replaceAt( 0, nIndex, rTitle );

            INetURLObject aURLObj( rURL );
            aURLObj.SetName( rTitle, INetURLObject::ENCODE_ALL );

            rURL = aURLObj.GetMainURL( INetURLObject::NO_DECODE );

            (*aIt)->maTargetURL = rURL;
            break;
        }
    }
}

// -----------------------------------------------------------------------
String SvtFileView_Impl::FolderInserted( const OUString& rURL, const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    SortingData_Impl* pData = new SortingData_Impl;

    pData->SetNewTitle( rTitle );
    pData->maSize     = 0;
    pData->mbIsFolder = sal_True;
    pData->maTargetURL   = rURL;

    INetURLObject aURLObj( rURL );

    ::svtools::VolumeInfo aVolInfo;
    pData->maType = SvFileInformationManager::GetFolderDescription( aVolInfo );
    pData->maImage = SvFileInformationManager::GetFolderImage( aVolInfo, sal_False, isHighContrast( mpView ) );

    OUString aValue;
    OUString aTab     = OUString::createFromAscii( "\t" );
    OUString aDateSep = OUString::createFromAscii( ", " );

    // title, type, size, date
    aValue = pData->GetTitle();
    // #83004# --------------------
    ReplaceTabWithString( aValue );
    aValue += aTab;
    aValue += pData->maType;
    aValue += aTab;
    // folders don't have a size
    aValue += aTab;
    // set the date
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
    aValue += rLocaleData.getDate( pData->maModDate );
    aValue += aDateSep;
    aValue += rLocaleData.getTime( pData->maModDate );

    pData->maDisplayText = aValue;
    maContent.push_back( pData );

    return String( aValue );
}

// -----------------------------------------------------------------------
sal_uLong SvtFileView_Impl::GetEntryPos( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;
    sal_uLong   nPos = 0;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( (*aIt)->maTargetURL == rURL )
            return nPos;
        nPos += 1;
    }

    return nPos;
}

// -----------------------------------------------------------------------
sal_Bool SvtFileView_Impl::SearchNextEntry( sal_uInt32& nIndex, const OUString& rTitle, sal_Bool bWrapAround )
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_uInt32 nEnd = maContent.size();
    sal_uInt32 nStart = nIndex;
    while ( nIndex < nEnd )
    {
        SortingData_Impl* pData = maContent[ nIndex ];
        if ( rTitle.compareTo( pData->GetLowerTitle(), rTitle.getLength() ) == 0 )
            return sal_True;
        nIndex += 1;
    }

    if ( bWrapAround )
    {
        nIndex = 0;
        while ( nIndex < nEnd && nIndex <= nStart )
        {
            SortingData_Impl* pData = maContent[ nIndex ];
            if ( rTitle.compareTo( pData->GetLowerTitle(), rTitle.getLength() ) == 0 )
                return sal_True;
            nIndex += 1;
        }
    }

    return sal_False;
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::SetActualFolder( const INetURLObject& rActualFolder )
{
    if( mbReplaceNames )
    {
        if( mpNameTrans )
            mpNameTrans->SetActualFolder( rActualFolder );
        else
            mpNameTrans = new NameTranslator_Impl( rActualFolder );
    }
}

namespace svtools {

// -----------------------------------------------------------------------
// QueryDeleteDlg_Impl
// -----------------------------------------------------------------------

QueryDeleteDlg_Impl::QueryDeleteDlg_Impl
(
    Window* pParent,
    const String& rName      // Eintragsname
) :

    ModalDialog( pParent, SvtResId( DLG_SVT_QUERYDELETE ) ),

    _aEntryLabel  ( this, SvtResId( TXT_ENTRY ) ),
    _aEntry       ( this, SvtResId( TXT_ENTRYNAME ) ),
    _aQueryMsg    ( this, SvtResId( TXT_QUERYMSG ) ),
    _aYesButton   ( this, SvtResId( BTN_YES ) ),
    _aAllButton   ( this, SvtResId( BTN_ALL ) ),
    _aNoButton    ( this, SvtResId( BTN_NO ) ),
    _aCancelButton( this, SvtResId( BTN_CANCEL ) )

{
    FreeResource();

    // Handler
    Link aLink( STATIC_LINK( this, QueryDeleteDlg_Impl, ClickLink ) );
    _aYesButton.SetClickHdl( aLink );
    _aAllButton.SetClickHdl( aLink );
    _aNoButton.SetClickHdl( aLink );

    // Anzeige der spezifizierten Texte

    WinBits nTmpStyle = _aEntry.GetStyle();
    nTmpStyle |= WB_PATHELLIPSIS;
    _aEntry.SetStyle( nTmpStyle );
    _aEntry.SetText( rName );
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( QueryDeleteDlg_Impl, ClickLink, PushButton*, pBtn )
{
    if ( pBtn == &pThis->_aYesButton )
        pThis->_eResult = QUERYDELETE_YES;
    else if ( pBtn == &pThis->_aNoButton )
        pThis->_eResult = QUERYDELETE_NO;
    else if ( pBtn == &pThis->_aAllButton )
        pThis->_eResult = QUERYDELETE_ALL;
    else if ( pBtn == &pThis->_aCancelButton )
        pThis->_eResult = QUERYDELETE_CANCEL;

    pThis->EndDialog( RET_OK );

    return 0;
}

}

