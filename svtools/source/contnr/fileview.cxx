/*************************************************************************
 *
 *  $RCSfile: fileview.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 15:01:07 $
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

#include <functional>
#include "fileview.hxx"
#include "svtdata.hxx"
#include "imagemgr.hxx"
#include "headbar.hxx"
#include "svtabbx.hxx"

#include "svtools.hrc"
#include "fileview.hrc"

#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXOBJTYPE_HXX
#include "AccessibleBrowseBoxObjType.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_CONTENTCREATIONEXCEPTION_HPP_
#include <com/sun/star/ucb/ContentCreationException.hpp>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSIST_HPP_
#include <com/sun/star/io/XPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _ALGORITHM_
#include <algorithm>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_H
#include <rtl/math.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif

#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#include <unotools/intlwrapper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include "syslocale.hxx"
#endif
#ifndef SVTOOLS_URL_FILTER_HXX
#include "urlfilter.hxx"
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using namespace ::rtl;
using namespace ::ucb;

#define ALL_FILES_FILTER    "*.*"

#define COLUMN_TITLE        1
#define COLUMN_TYPE         2
#define COLUMN_SIZE         3
#define COLUMN_DATE         4

#define ROW_TITLE           1
#define ROW_SIZE            2
#define ROW_DATE_MOD        3
#define ROW_DATE_CREATE     4
#define ROW_IS_FOLDER       5
#define ROW_TARGET_URL      6
#define ROW_IS_HIDDEN       7
#define ROW_IS_VOLUME       8
#define ROW_IS_REMOTE       9
#define ROW_IS_REMOVEABLE   10
#define ROW_IS_FLOPPY       11
#define ROW_IS_COMPACTDISC  12

DECLARE_LIST( StringList_Impl, OUString* );

#define ROW_HEIGHT  17  // the height of a row has to be a little higher than the bitmap
#define QUICK_SEARCH_TIMEOUT    1500    // time in mSec before the quicksearch string will be reseted

// -----------------------------------------------------------------------

static sal_Bool isHighContrast( const Window* _pView )
{
    return _pView->GetDisplayBackground().GetColor().IsDark();
}

// -----------------------------------------------------------------------

// structs   -------------------------------------------------------------

struct SortingData_Impl
{
private:
    OUString    maFilename;     // only filename in upper case - for compare purposes
    OUString    maTitle;        //  -> be carefull when changing maTitle to update maFilename only when new
    OUString    maLowerTitle;

public:
    OUString    maType;
    OUString    maTargetURL;
    OUString    maImageURL;
    OUString    maDisplayText;
    DateTime    maModDate;
    Image       maImage;
    sal_Int64   maSize;
    sal_Bool    mbIsFolder;
    sal_Bool    mbIsVolume;
    sal_Bool    mbIsRemote;
    sal_Bool    mbIsRemoveable;
    sal_Bool    mbIsFloppy;
    sal_Bool    mbIsCompactDisc;

    inline                  SortingData_Impl();
    inline const OUString&  GetTitle() const;
    inline const OUString&  GetLowerTitle() const;
    inline const OUString&  GetFileName() const;
    inline void             SetNewTitle( const OUString& rNewTitle );       // new maTitle is set -> maFilename is set to same!
    inline void             ChangeTitle( const OUString& rChangedTitle );   // maTitle is changed, maFilename is unchanged!

private:
    void                    SetTitles( const OUString& rNewTitle );
};

inline SortingData_Impl::SortingData_Impl() :
    maSize          ( 0 ),
    mbIsFolder      ( sal_False ),
    mbIsVolume      ( sal_False ),
    mbIsRemote      ( sal_False ),
    mbIsRemoveable  ( sal_False ),
    mbIsFloppy      ( sal_False ),
    mbIsCompactDisc ( sal_False )
{
}

inline const OUString& SortingData_Impl::GetTitle() const
{
    return maTitle;
}

inline const OUString& SortingData_Impl::GetLowerTitle() const
{
    return maLowerTitle;
}

inline const OUString& SortingData_Impl::GetFileName() const
{
    return maFilename;
}

inline void SortingData_Impl::SetNewTitle( const OUString& rNewTitle )
{
    SetTitles( rNewTitle );
    maFilename = rNewTitle.toAsciiUpperCase();
}

inline void SortingData_Impl::ChangeTitle( const OUString& rChangedTitle )
{
    SetTitles( rChangedTitle );
}

void SortingData_Impl::SetTitles( const OUString& rNewTitle )
{
    maTitle = rNewTitle;
    maLowerTitle = rNewTitle.toAsciiLowerCase();
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

    void            DeleteEntries();
    void            DoQuickSearch( const xub_Unicode& rChar );
    sal_Bool        Kill( const OUString& rURL );

protected:
    virtual BOOL            DoubleClickHdl();
    virtual ::rtl::OUString GetAccessibleDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const;

public:
    ViewTabListBox_Impl( Window* pParentWin, SvtFileView_Impl* pParent, sal_Int16 nFlags );
   ~ViewTabListBox_Impl();

    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual BOOL    EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );

    void            ClearAll();
    HeaderBar*      GetHeaderBar() const { return mpHeaderBar; }

    void            EnableAutoResize() { mbAutoResize = sal_True; }
    void            EnableDelete( sal_Bool bEnable ) { mbEnableDelete = bEnable; }
    sal_Bool        IsDeleteOrContextMenuEnabled() { return mbEnableDelete || IsContextMenuHandlingEnabled(); }

    Reference< XCommandEnvironment >    GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK( ResetQuickSearch_Impl, Timer * );

    virtual PopupMenu*  CreateContextMenu( void );
    virtual void        ExcecuteContextMenuAction( USHORT nSelectedPopentry );
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
    inline HashedEntry*     First();
    inline HashedEntry*     Next();
    inline void             Append( HashedEntry* pNewEntry );
public:
    virtual                 ~HashedEntryList();

    const HashedEntry*      Find( const OUString& rNameToSearchFor );
    const HashedEntry*      Find( const HashedEntry& rToSearchFor );
                                // not const, because First()/Next() is used
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
    HashedEntry aRef( rRefName );
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
    INetURLObject           maTransFile;    // URL of file with translation entries
    HashedEntry             maHashedURL;    // for future purposes when dealing with a set of cached
                                            //  NameTranslationLists
private:
    const String            maTransFileName;
    void                    Init();         // reads the translation file and fills the (internal) list

public:
                            NameTranslationList( const INetURLObject& rBaseURL );
                                            // rBaseURL: path to folder for which the translation of the entries
                                            //  should be done

    inline sal_Bool operator    ==( const HashedEntry& rRef ) const;
    inline sal_Bool operator    !=( const HashedEntry& rRef ) const;

    const OUString*         Translate( const OUString& rName ) const;
                                            // returns NULL, if rName can't be found

    inline void             Update();       // clears list and init

    inline const String&    GetTransTableFileName() const;
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
        Content aTestContent( maTransFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );

        if( aTestContent.isDocument() )
        {// ... also tests the existence of maTransFile by throwing an Exception
            const sal_Char* pSection = "TRANSLATIONNAMES";
            String          aFsysName( maTransFile.getFSysPath( INetURLObject::FSYS_DETECT ) );
            Config          aConfig( aFsysName );

            aConfig.SetGroup( ByteString( pSection ) );

            USHORT          nKeyCnt = aConfig.GetKeyCount();

            for( USHORT nCnt = 0 ; nCnt < nKeyCnt ; ++nCnt )
                Insert( new NameTranslationEntry( aConfig.GetKeyName( nCnt ), aConfig.ReadKey( nCnt ) ) );
        }
    }
    catch( Exception& ) {}
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

class NameTranslator_Impl
{   // enables the user to get string substitutions (translations for the content) for a given folder
    // see more explanations above in the description for NameTranslationList
private:
    NameTranslationList*    mpActFolder;
public:
                            NameTranslator_Impl( void );
                            NameTranslator_Impl( const INetURLObject& rActualFolder );
                            ~NameTranslator_Impl();

    void                    UpdateTranslationTable();   // reads the translation file again

    void                    SetActualFolder( const INetURLObject& rActualFolder );
    sal_Bool                GetTranslation( const OUString& rOriginalName, OUString& rTranslatedName ) const;
                                // does nothing with rTranslatedName, when translation is not possible
    const String*           GetTransTableFileName() const;
                                            // returns the name for the file, which contains the translation strings
};

// class SvtFileView_Impl ---------------------------------------------

class SvtFileView_Impl
{
protected:
    Link                    m_aSelectHandler;

public:

    ::std::vector< SortingData_Impl* >  maContent;
    ::osl::Mutex                        maMutex;

    ViewTabListBox_Impl*    mpView;
    NameTranslator_Impl*    mpNameTrans;
    const IUrlFilter*       mpUrlFilter;
    Reference< XPersist >   xDocInfo;
    sal_uInt16              mnSortColumn;
    sal_Bool                mbAscending     : 1;
    sal_Bool                mbOnlyFolder    : 1;
    sal_Bool                mbReplaceNames  : 1;    // translate folder names or display doc-title instead of file name
    sal_Bool                mbSuspendSelectCallback : 1;

    IntlWrapper             aIntlWrapper;

    String                  maViewURL;
    String                  maAllFilter;
    String                  maCurrentFilter;
    Image                   maFolderImage;
    Link                    maOpenDoneLink;

                            SvtFileView_Impl( Window* pParent,
                                              sal_Int16 nFlags,
                                              sal_Bool bOnlyFolder );
                           ~SvtFileView_Impl();

    void                    Clear();

    sal_Bool                GetFolderContent_Impl( const String& rFolder );
    void                    FilterFolderContent_Impl( const OUString &rFilter );

    void                    OpenFolder_Impl();
    // #83004# -------
    void                    ReplaceTabWithString( OUString& aValue );
    void                    CreateDisplayText_Impl();
    void                    CreateVector_Impl( const Sequence < OUString > &rList );
    void                    SortFolderContent_Impl();

    void                    EntryRemoved( const OUString& rURL );
    void                    EntryRenamed( OUString& rURL,
                                          const OUString& rName );
    String                  FolderInserted( const OUString& rURL,
                                            const OUString& rTitle );

    ULONG                   GetEntryPos( const OUString& rURL );

    inline void             EnableContextMenu( sal_Bool bEnable );
    inline void             EnableDelete( sal_Bool bEnable );

    void                    Resort_Impl( sal_Int16 nColumn, sal_Bool bAscending );
    sal_Bool                SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             sal_Bool bWrapAround );

    inline sal_Bool         EnableNameReplacing( sal_Bool bEnable = sal_True ); // returns false, if action wasn't possible
    void                    SetActualFolder( const INetURLObject& rActualFolder );
    sal_Bool                GetTranslatedName( const OUString& rName, OUString& rTranslatedName ) const;

    sal_Bool                GetDocTitle( const OUString& rTargetURL, OUString& rDocTitle ) const;

    void                    SetSelectHandler( const Link& _rHdl );

    void                    InitSelection();
    void                    ResetCursor();

protected:
    DECL_LINK( SelectionMultiplexer, void* );
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
    sal_Bool bRet;
    if( mpView->IsDeleteOrContextMenuEnabled() )
    {
        DBG_ASSERT( !mbReplaceNames, "SvtFileView_Impl::EnableNameReplacing(): state should be not possible!" );
        bRet = !bEnable;    // only for enabling this is an unsuccessful result
    }
    else
    {
        mbReplaceNames = bEnable;
        bRet = sal_True;
    }

    return bRet;
}
// functions -------------------------------------------------------------

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                         Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

void AppendDateTime_Impl( const ::com::sun::star::util::DateTime& rDT,
                          const Locale& rLocale, String& rRow )
{
    DateTime aDT;
    CONVERT_DATETIME( rDT, aDT );
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
    String aDateStr = rLocaleData.getDate( aDT );
    aDateStr += String::CreateFromAscii( ", " );
    aDateStr += rLocaleData.getTime( aDT );
    rRow += aDateStr;
}

OUString CreateExactSizeText_Impl( sal_Int64 nSize )
{
    double fSize( ( double ) nSize );
    int nDec;

    ULONG nMega = 1024 * 1024;
    ULONG nGiga = nMega * 1024;

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
    mbEnableDelete      ( sal_True )

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

    mxCmdEnv = new CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    EnableContextMenuHandling();
}

// -----------------------------------------------------------------------

ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
    maResetQuickSearch.Stop();

    delete mpHeaderBar;
}

// -----------------------------------------------------------------------

IMPL_LINK( ViewTabListBox_Impl, ResetQuickSearch_Impl, Timer*, pTimer )
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
    PopupMenu* pRet;
    sal_Int32 nSelectedEntries = GetSelectionCount();

    if ( nSelectedEntries )
    {
        pRet = new PopupMenu( SvtResId( RID_FILEVIEW_CONTEXTMENU ) );
        pRet->EnableItem( MID_FILEVIEW_DELETE, 0 < nSelectedEntries );
        pRet->EnableItem( MID_FILEVIEW_RENAME, 1 == nSelectedEntries );
        pRet->RemoveDisabledEntries( sal_True, sal_True );
    }
    else
        pRet = NULL;

    return pRet;
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::ExcecuteContextMenuAction( USHORT nSelectedPopupEntry )
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
    for ( USHORT i = 0; i < GetEntryCount(); ++i )
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
            return;

        INetURLObject aObj( aURL );

        if ( eResult != svtools::QUERYDELETE_ALL )
        {
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
BOOL ViewTabListBox_Impl::EditedEntry( SvLBoxEntry* pEntry,
                                 const XubString& rNewText )
{
    BOOL bRet = FALSE;

    OUString aURL;
    SvtContentEntry* pData = (SvtContentEntry*)pEntry->GetUserData();

    if ( pData )
        aURL = OUString( pData->maURL );

    if ( ! aURL.getLength() )
        return bRet;

    try
    {
        Content aContent( aURL, mxCmdEnv );

        OUString aPropName = OUString::createFromAscii( "Title" );
        Any aValue;
        aValue <<= OUString( rNewText );
        aContent.setPropertyValue( aPropName, aValue );
        mpParent->EntryRenamed( aURL, rNewText );

        pData->maURL = aURL;
        pEntry->SetUserData( pData );

        bRet = TRUE;
    }
    catch( ::com::sun::star::ucb::ContentCreationException ) {}
    catch( ::com::sun::star::ucb::CommandAbortedException ) {}
    catch( ::com::sun::star::uno::Exception ) {}

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

    maQuickSearchText += OUString( rChar ).toAsciiLowerCase();

    bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, sal_False );

    if ( !bFound && ( aLastText.getLength() == 1 ) &&
         ( aLastText == OUString( rChar ) ) )
    {
        mnSearchIndex = aLastPos + 1;
        maQuickSearchText = aLastText;
        bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, sal_True );
    }

    if ( bFound )
    {
        SvLBoxEntry* pEntry = GetEntry( mnSearchIndex );
        SelectAll( FALSE );
        Select( pEntry );
        SetCurEntry( pEntry );
        MakeVisible( pEntry );
    }
    else
        Sound::Beep();

    maResetQuickSearch.Start();
}

// -----------------------------------------------------------------------
BOOL ViewTabListBox_Impl::DoubleClickHdl()
{
    SvHeaderTabListBox::DoubleClickHdl();
    return FALSE;
        // this means "do no additional handling". Especially this means that the SvImpLBox does not
        // recognize that the entry at the double click position change after the handler call (which is
        // the case if in the handler, our content was replaced)
        // If it _would_ recognize this change, it would take this as a reason to select the entry, again
        // - which is not what in the case of content replace
        // (I really doubt that this behaviour of the SvImpLBox does make any sense at all, but
        // who knows ...)
        // 07.12.2001 - 95727 - fs@openoffice.org
}

::rtl::OUString ViewTabListBox_Impl::GetAccessibleDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    ::rtl::OUString sRet = SvHeaderTabListBox::GetAccessibleDescription( _eType, _nPos );
    if ( ::svt::BBTYPE_TABLECELL == _eType )
    {
        sal_Int32 nRow = _nPos / GetColumnCount();
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
        Content aCnt( rContent, mxCmdEnv );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ::com::sun::star::uno::Exception& )
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

    mpImp = new SvtFileView_Impl( this, nFlags, bOnlyFolder );

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
    mpImp = new SvtFileView_Impl( this, nFlags,
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
    UINT32 i, nCount = aContents.getLength();
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
        Image aImage = SvFileInformationManager::GetImage( aObj, FALSE, isHighContrast( this ) );

        if ( bDoInsert )
        {
            // insert entry and set user data
            SvLBoxEntry* pEntry = mpImp->mpView->InsertEntry( aNewRow, aImage, aImage );
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

// -----------------------------------------------------------------------

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
        SvtContentEntry* pUserData = new SvtContentEntry( sURL, TRUE );
        pEntry->SetUserData( pUserData );
        mpImp->mpView->MakeVisible( pEntry );
        bRet = sal_True;
    }
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::HasPreviousLevel( String& rParentURL ) const
{
    sal_Bool bRet = sal_False;
    try
    {
        Content aCnt( mpImp->maViewURL, Reference< XCommandEnvironment > () );
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
    catch( ::com::sun::star::uno::Exception )
    {
        // perhaps an unkown url protocol (e.g. "private:newdoc")
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::PreviousLevel( String& rNewURL )
{
    sal_Bool bRet = sal_False;
    if ( HasPreviousLevel( rNewURL ) )
        bRet = Initialize( rNewURL, mpImp->maCurrentFilter );

    return bRet;
}

// -----------------------------------------------------------------------

sal_uInt32 SvtFileView::GetHelpId( ) const
{
    return mpImp->mpView->GetHelpId( );
}

// -----------------------------------------------------------------------

void SvtFileView::SetHelpId( sal_uInt32 nHelpId )
{
    mpImp->mpView->SetHelpId( nHelpId );
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

// -----------------------------------------------------------------------

sal_Bool SvtFileView::Initialize( const String& rURL, const String& rFilter )
{
    WaitObject aWaitCursor( this );

    String sPushURL( mpImp->maViewURL );

    mpImp->maViewURL = rURL;
    if ( !ExecuteFilter( rFilter ) )
    {
        mpImp->maViewURL = sPushURL;
        return sal_False;
    }

    mpImp->maOpenDoneLink.Call( this );
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::Initialize( const String& rURL, const Sequence< OUString >& aContents )
{
    WaitObject aWaitCursor( this );

    mpImp->maViewURL = rURL;
    mpImp->maCurrentFilter = mpImp->maAllFilter;

    mpImp->Clear();
    mpImp->CreateVector_Impl( aContents );
    mpImp->SortFolderContent_Impl();

    mpImp->OpenFolder_Impl();

    mpImp->maOpenDoneLink.Call( this );

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::ExecuteFilter( const String& rFilter )
{
    mpImp->maCurrentFilter = rFilter;
    mpImp->maCurrentFilter.ToLowerAscii();

    mpImp->Clear();
    if ( !mpImp->GetFolderContent_Impl( mpImp->maViewURL ) )
        return sal_False;

    mpImp->FilterFolderContent_Impl( rFilter );

    mpImp->SortFolderContent_Impl();    // possibly not necessary!!!!!!!!!!
    mpImp->CreateDisplayText_Impl();
    mpImp->OpenFolder_Impl();
    return sal_True;
}

// -----------------------------------------------------------------------

void SvtFileView::SetNoSelection()
{
    mpImp->mpView->SelectAll( FALSE );
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

ULONG SvtFileView::GetSelectionCount() const
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
IMPL_LINK( SvtFileView, HeaderSelect_Impl, HeaderBar*, pBar )
{
    DBG_ASSERT( pBar, "no headerbar" );
    USHORT nItemID = pBar->GetCurItemId();

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

    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );

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
        USHORT nTabs = pBar->GetItemCount();
        long nTmpSize = 0;

        for ( USHORT i = 1; i <= nTabs; ++i )
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
    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    sRet += bUp ? '1' : '0';
    sRet += ';';

    USHORT nCount = pBar->GetItemCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        USHORT nId = pBar->GetItemId(i);
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

    USHORT nTokenCount = rCfgStr.GetTokenCount();
    DBG_ASSERT( pBar->GetItemCount() == ( nTokenCount / 2 - 1 ), "invalid config string" );

    USHORT nIdx = 0;
    mpImp->mnSortColumn = (USHORT)rCfgStr.GetToken( 0, ';', nIdx ).ToInt32();
    BOOL bUp = (BOOL)(USHORT)rCfgStr.GetToken( 0, ';', nIdx ).ToInt32();
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
        USHORT nItemId = (USHORT)rCfgStr.GetToken( 0, ';', nIdx ).ToInt32();
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

SvtFileView_Impl::SvtFileView_Impl( Window* pParent, sal_Int16 nFlags, sal_Bool bOnlyFolder )

    :mnSortColumn               ( COLUMN_TITLE )
    ,mbAscending                ( sal_True )
    ,mbOnlyFolder               ( bOnlyFolder )
    ,mbReplaceNames             ( sal_False )
    ,aIntlWrapper               ( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() )
    ,maFolderImage              ( SvtResId( IMG_SVT_FOLDER ) )
    ,mpNameTrans                ( NULL )
    ,mbSuspendSelectCallback    ( sal_False )
    ,mpUrlFilter                ( NULL )

{
    maAllFilter = String::CreateFromAscii( "*.*" );
    mpView = new ViewTabListBox_Impl( pParent, this, nFlags );
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
sal_Bool SvtFileView_Impl::GetFolderContent_Impl( const String& rFolder )
{
    ::osl::MutexGuard aGuard( maMutex );

    SortingData_Impl* pData;

    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    sal_Bool bSuccess = sal_False;

    try
    {
        // prepare name translation
        SetActualFolder( aFolderObj );

        Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), mpView->GetCommandEnvironment() );
        Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(12);

        aProps[0] = OUString::createFromAscii( "Title" );
        aProps[1] = OUString::createFromAscii( "Size" );
        aProps[2] = OUString::createFromAscii( "DateModified" );
        aProps[3] = OUString::createFromAscii( "DateCreated" );
        aProps[4] = OUString::createFromAscii( "IsFolder" );
        aProps[5] = OUString::createFromAscii( "TargetURL" );
        aProps[6] = OUString::createFromAscii( "IsHidden" );
        aProps[7] = OUString::createFromAscii( "IsVolume" );
        aProps[8] = OUString::createFromAscii( "IsRemote" );
        aProps[9] = OUString::createFromAscii( "IsRemoveable" );
        aProps[10] = OUString::createFromAscii( "IsFloppy" );
        aProps[11] = OUString::createFromAscii( "IsCompactDisc" );

        try
        {
            Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
            ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( ::com::sun::star::uno::Exception& e )
        {
            e; // make compiler happy
        }

        if ( xResultSet.is() )
        {
            Reference< com::sun::star::sdbc::XRow > xRow( xResultSet, UNO_QUERY );
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            try
            {
                ::com::sun::star::util::DateTime aDT;

                while ( xResultSet->next() )
                {
                    sal_Bool bIsHidden = xRow->getBoolean( ROW_IS_HIDDEN );
                    // don't show hidden files
                    if ( !bIsHidden )
                    {
                        pData = NULL;

                        aDT = xRow->getTimestamp( ROW_DATE_MOD );
                        if ( xRow->wasNull() )
                            aDT = xRow->getTimestamp( ROW_DATE_CREATE );

                        OUString aContentURL = xContentAccess->queryContentIdentifierString();
                        OUString aTargetURL = xRow->getString( ROW_TARGET_URL );
                        sal_Bool bHasTargetURL = aTargetURL.getLength() > 0;

                        OUString sRealURL = bHasTargetURL ? aTargetURL : aContentURL;

                        // check for restrictions
                        if ( mpUrlFilter && !mpUrlFilter->isUrlAllowed( sRealURL ) )
                            continue;

                        pData = new SortingData_Impl;
                        pData->maTargetURL = sRealURL;

                        pData->mbIsFolder = xRow->getBoolean( ROW_IS_FOLDER );
                        pData->mbIsVolume = xRow->getBoolean( ROW_IS_VOLUME );
                        pData->mbIsRemote = xRow->getBoolean( ROW_IS_REMOTE );
                        pData->mbIsRemoveable = xRow->getBoolean( ROW_IS_REMOVEABLE );
                        pData->mbIsFloppy = xRow->getBoolean( ROW_IS_FLOPPY );
                        pData->mbIsCompactDisc = xRow->getBoolean( ROW_IS_COMPACTDISC );
                        pData->SetNewTitle( xRow->getString( ROW_TITLE ) );
                        pData->maSize = xRow->getLong( ROW_SIZE );

                        if ( bHasTargetURL &&
                             INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                        {
                            Content aCnt( aTargetURL, Reference< XCommandEnvironment > () );
                            aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= pData->maSize;
                            aCnt.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDT;
                        }

                        CONVERT_DATETIME( aDT, pData->maModDate );

                        if ( pData->mbIsFolder )
                        {
                            ::svtools::VolumeInfo aVolInfo( pData->mbIsVolume, pData->mbIsRemote,
                                                            pData->mbIsRemoveable, pData->mbIsFloppy,
                                                            pData->mbIsCompactDisc );
                            pData->maType = SvFileInformationManager::GetFolderDescription( aVolInfo );
                        }
                        else
                            pData->maType = SvFileInformationManager::GetFileDescription(
                                INetURLObject( pData->maTargetURL ) );

                        // replace names on demand
                        if( mbReplaceNames )
                        {
                            OUString aNewTitle;
                            sal_Bool bTranslated;

                            if( pData->mbIsFolder )
                                bTranslated = GetTranslatedName( pData->GetTitle(), aNewTitle );
                            else
                                bTranslated = GetDocTitle( pData->maTargetURL, aNewTitle );

                            if( bTranslated )
                                pData->ChangeTitle( aNewTitle );
                        }

                        maContent.push_back( pData );
                    }
                }
                bSuccess = sal_True;
            }
            catch( CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "GetFolderContents: CommandAbortedException" );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }

    if ( !bSuccess )
        // clear any "intermediate" and unfinished result
        maContent.clear();

    return bSuccess;
}
// -----------------------------------------------------------------------
namespace
{
    struct FilterMatch : public ::std::unary_function< bool, WildCard >
    {
    private:
        const String&   m_rCompareString;
    public:
        FilterMatch( const String& _rCompareString ) : m_rCompareString( _rCompareString ) { }

        bool operator()( const WildCard& _rMatcher )
        {
            return _rMatcher.Matches( m_rCompareString ) ? true : false;
        }
    };
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

    if ( maContent.size() == 0 )
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
    if( rFilter.getLength() )
    {// filter is given
        aFilters.reserve( nTokens );
        sal_Int32 nIndex = 0;
        OUString sToken;
        do
        {
            sToken = rFilter.getToken( 0, ';', nIndex );
            if ( sToken.getLength() )
            {
                aFilters.push_back( WildCard( sToken.toAsciiUpperCase() ) );
            }
        }
        while ( nIndex >= 0 );
    }
    else
        // no filter is given -> match all
        aFilters.push_back( WildCard( String::CreateFromAscii( "*" ) ) );

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
            sCompareString = (*aContentLoop)->GetFileName();    // filter works on file name, not on title!
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
    return mbSuspendSelectCallback ? 0L : m_aSelectHandler.Call( _pSource );
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

    mpView->SetUpdateMode( FALSE );
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

    mbSuspendSelectCallback = sal_True;
    mpView->SetUpdateMode( TRUE );
    mbSuspendSelectCallback = sal_False;

    ResetCursor();
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::ResetCursor()
{
    // deselect
    SvLBoxEntry* pEntry = mpView->FirstSelected();
    if ( pEntry )
        mpView->Select( pEntry, FALSE );
    // set cursor to the first entry
    mpView->SetCursor( mpView->First(), TRUE );
    mpView->Update();
}

// #83004# -------
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
            (*aIt)->maImage = SvFileInformationManager::GetFolderImage( aVolInfo, FALSE, isHighContrast( mpView ) );
        }
        else
            (*aIt)->maImage = SvFileInformationManager::GetFileImage( INetURLObject( (*aIt)->maTargetURL ), FALSE, isHighContrast( mpView ));
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
        pEntry->maImage = SvFileInformationManager::GetImage( aObj, FALSE, isHighContrast( mpView ) );

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

    if ( aEntryURL.Len() )
    {
        ULONG nPos = GetEntryPos( aEntryURL );
        if ( nPos < mpView->GetEntryCount() )
        {
            pEntry = mpView->GetEntry( nPos );
            mpView->SetCurEntry( pEntry );
        }
    }
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
    sal_Bool    bRet;
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
        }
    }

    // when the two elements are equal, we must not return TRUE (which would
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
    pData->maImage = SvFileInformationManager::GetFolderImage( aVolInfo, FALSE, isHighContrast( mpView ) );

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
ULONG SvtFileView_Impl::GetEntryPos( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;
    ULONG   nPos = 0;

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

// -----------------------------------------------------------------------
sal_Bool SvtFileView_Impl::GetTranslatedName( const OUString& rName, OUString& rRet ) const
{
    sal_Bool bRet;

    if( mbReplaceNames && mpNameTrans )
        bRet = mpNameTrans->GetTranslation( rName, rRet );
    else
        bRet = sal_False;

    return bRet;
}

// -----------------------------------------------------------------------
sal_Bool SvtFileView_Impl::GetDocTitle( const OUString& rTargetURL, OUString& rRet ) const
{
    SvtFileView_Impl* p = const_cast< SvtFileView_Impl* >( this );
    sal_Bool bRet = sal_False;

    if( !xDocInfo.is() )
        p->xDocInfo = Reference< XPersist > (
               ::comphelper::getProcessServiceFactory()->createInstance(
               String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.DocumentProperties") ) ), UNO_QUERY );

    DBG_ASSERT( xDocInfo.is(), "SvtFileView_Impl::GetDocTitle(): I don't understand the world any more!" );

    try
    {
        p->xDocInfo->read( rTargetURL );
        Reference< XPropertySet > xPropSet( xDocInfo, UNO_QUERY );

        Any aAny = xPropSet->getPropertyValue( OUString::createFromAscii( "Title" ) );

        OUString aTitle;
        if( aAny >>= aTitle )
        {
            rRet = aTitle;
            bRet = sal_True;
        }
    }
    catch ( IOException& ) {}
    catch ( UnknownPropertyException& ) {}
    catch ( Exception& ) {}

    return bRet;
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

    _aEntryLabel    ( this, ResId( TXT_ENTRY ) ),
    _aEntry     ( this, ResId( TXT_ENTRYNAME ) ),
    _aQueryMsg  ( this, ResId( TXT_QUERYMSG ) ),
    _aYesButton ( this, ResId( BTN_YES ) ),
    _aAllButton ( this, ResId( BTN_ALL ) ),
    _aNoButton  ( this, ResId( BTN_NO ) ),
    _aCancelButton( this, ResId( BTN_CANCEL ) )

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

/*  [Beschreibung]

    Die Methode wertet das Resultat der Abfrage aus.
*/

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

