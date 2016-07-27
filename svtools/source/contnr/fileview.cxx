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

#include <svtools/treelistbox.hxx>
#include <svtools/iconview.hxx>
#include "fileview.hxx"
#include <sal/config.h>
#include <svtools/treelistentry.hxx>
#include <svtools/fileview.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/svtools.hrc>
#include <svtools/viewdataentry.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "fileview.hrc"
#include "contentenumeration.hxx"
#include <svtools/AccessibleBrowseBoxObjType.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
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
#include <vcl/settings.hxx>
#include <com/sun/star/io/XPersist.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/layout.hxx>
#include <rtl/math.hxx>
#include <tools/config.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <salhelper/timer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svl/urlfilter.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

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

#define ALL_FILES_FILTER    "*.*"

#define COLUMN_TITLE        1
#define COLUMN_TYPE         2
#define COLUMN_SIZE         3
#define COLUMN_DATE         4

#define ROW_HEIGHT                17    // the height of a row has to be a little higher than the bitmap
#define QUICK_SEARCH_TIMEOUT    1500    // time in mSec before the quicksearch string will be reseted

enum class FileViewFlags
{
    NONE               = 0x00,
    ONLYFOLDER         = 0x01,
    MULTISELECTION     = 0x02,
    SHOW_TYPE          = 0x04,
    SHOW_ONLYTITLE     = 0x10,
    SHOW_NONE          = 0x20,
};
namespace o3tl
{
    template<> struct typed_flags<FileViewFlags> : is_typed_flags<FileViewFlags, 0x1f> {};
}

namespace
{

    //= CallbackTimer

    class CallbackTimer : public ::salhelper::Timer
    {
    protected:
        SvtFileView_Impl* m_pTimeoutHandler;

    public:
        explicit CallbackTimer( SvtFileView_Impl* _pHandler ) : m_pTimeoutHandler( _pHandler ) { }

    protected:
        virtual void SAL_CALL onShot() override;
    };


}


void FilterMatch::createWildCardFilterList(const OUString& _rFilterList,::std::vector< WildCard >& _rFilters)
{
    if( _rFilterList.getLength() )
    {
        // filter is given
        sal_Int32 nIndex = 0;
        OUString sToken;
        do
        {
            sToken = _rFilterList.getToken( 0, ';', nIndex );
            if ( !sToken.isEmpty() )
            {
                _rFilters.push_back( WildCard( sToken.toAsciiUpperCase() ) );
            }
        }
        while ( nIndex >= 0 );
    }
    else
    {
        // no filter is given -> match all
        _rFilters.push_back( WildCard(OUString("*")) );
    }
}

class ViewTabListBox_Impl : public SvHeaderTabListBox
{
private:
    Reference< XCommandEnvironment >    mxCmdEnv;

    ::osl::Mutex            maMutex;
    VclPtr<HeaderBar>       mpHeaderBar;
    SvtFileView_Impl*       mpParent;
    Timer                   maResetQuickSearch;
    OUString                maQuickSearchText;
    OUString                msAccessibleDescText;
    OUString                msFolder;
    OUString                msFile;
    sal_uInt32              mnSearchIndex;
    bool                    mbResizeDisabled        : 1;
    bool                    mbAutoResize            : 1;
    bool                    mbEnableDelete          : 1;
    bool                    mbShowHeader;

    void            DeleteEntries();
    void            DoQuickSearch( sal_Unicode rChar );
    bool            Kill( const OUString& rURL );

protected:
    virtual bool     DoubleClickHdl() override;
    virtual OUString GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const override;

public:
    ViewTabListBox_Impl( vcl::Window* pParentWin, SvtFileView_Impl* pParent, FileViewFlags nFlags );
    virtual ~ViewTabListBox_Impl();
    virtual void dispose() override;

    virtual void    Resize() override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

    void            ClearAll();
    HeaderBar*      GetHeaderBar() const { return mpHeaderBar; }

    void            EnableAutoResize() { mbAutoResize = true; }
    void            EnableDelete( bool bEnable ) { mbEnableDelete = bEnable; }

    const Reference< XCommandEnvironment >& GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK_TYPED(ResetQuickSearch_Impl, Timer *, void);

    virtual VclPtr<PopupMenu> CreateContextMenu() override;
    virtual void        ExecuteContextMenuAction( sal_uInt16 nSelectedPopentry ) override;
};

// provides a list of _unique_ Entries
class NameTranslationList
{   // contains a list of substitutes of strings for a given folder (as URL)
    // explanation of the circumstances see in remarks for Init();
protected:
    INetURLObject               maTransFile;    // URL of file with translation entries
    /// for future purposes when dealing with a set of cached NameTranslationLists
    OUString m_HashedURL;
private:
    std::unordered_map<OUString, OUString, OUStringHash> m_Translation;
    const OUString              maTransFileName;
    void                        Init();         // reads the translation file and fills the (internal) list

public:
                                explicit NameTranslationList( const INetURLObject& rBaseURL );
                                            // rBaseURL: path to folder for which the translation of the entries
                                            //  should be done

    const OUString*             Translate( const OUString& rName ) const;
                                            // returns NULL, if rName can't be found

    inline const OUString&      GetTransTableFileName() const;
    OUString const& GetHashedURL() { return m_HashedURL; }
                                            // returns the name for the file, which contains the translation strings
};

inline const OUString& NameTranslationList::GetTransTableFileName() const
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
        ::ucbhelper::Content aTestContent( maTransFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );

        if( aTestContent.isDocument() )
        {
            // ... also tests the existence of maTransFile by throwing an Exception
            OUString        aFsysName( maTransFile.getFSysPath( INetURLObject::FSYS_DETECT ) );
            Config          aConfig( aFsysName );

            aConfig.SetGroup( OString("TRANSLATIONNAMES") );

            sal_uInt16          nKeyCnt = aConfig.GetKeyCount();

            for( sal_uInt16 nCnt = 0 ; nCnt < nKeyCnt ; ++nCnt )
            {
                m_Translation.insert(std::make_pair(
                    OStringToOUString(aConfig.GetKeyName(nCnt), RTL_TEXTENCODING_ASCII_US),
                    OStringToOUString(aConfig.ReadKey(nCnt), RTL_TEXTENCODING_UTF8)
                    ));
            }
        }
    }
    catch( Exception const & ) {}
}

NameTranslationList::NameTranslationList( const INetURLObject& rBaseURL ):
    maTransFile( rBaseURL ),
    m_HashedURL(rBaseURL.GetMainURL(INetURLObject::NO_DECODE)),
    maTransFileName( OUString(".nametranslation.table") )
{
    maTransFile.insertName( maTransFileName );
    Init();
}

const OUString* NameTranslationList::Translate( const OUString& rName ) const
{
    auto const iter(m_Translation.find(rName));
    return (iter != m_Translation.end()) ? &iter->second : nullptr;
}

// enables the user to get string substitutions (translations for the content) for a given folder
// see more explanations above in the description for NameTranslationList
class NameTranslator_Impl : public ::svt::IContentTitleTranslation
{
private:
    NameTranslationList*    mpActFolder;
public:
                            explicit NameTranslator_Impl( const INetURLObject& rActualFolder );
                            virtual ~NameTranslator_Impl();

     // IContentTitleTranslation
    virtual bool            GetTranslation( const OUString& rOriginalName, OUString& rTranslatedName ) const override;

    void                    SetActualFolder( const INetURLObject& rActualFolder );
    const OUString*         GetTransTableFileName() const;
                                            // returns the name for the file, which contains the translation strings
};


//= SvtFileView_Impl


class SvtFileView_Impl  :public ::svt::IEnumerationResultHandler
{
protected:
    VclPtr<SvtFileView>                 mpAntiImpl;
    Link<SvTreeListBox*,void>           m_aSelectHandler;

    ::rtl::Reference< ::svt::FileViewContentEnumerator >
                                        m_xContentEnumerator;
    Link<void*,void>                    m_aCurrentAsyncActionHandler;
    ::osl::Condition                    m_aAsyncActionFinished;
    ::rtl::Reference< ::salhelper::Timer > m_xCancelAsyncTimer;
    ::svt::EnumerationResult            m_eAsyncActionResult;
    bool                                m_bRunningAsyncAction;
    bool                                m_bAsyncActionCancelled;

    FileViewMode                        m_eViewMode;


public:

    ::std::vector< SortingData_Impl* >  maContent;
    ::osl::Mutex                        maMutex;

    VclPtr<SvTreeListBox>               mpCurView;
    VclPtr<ViewTabListBox_Impl>         mpView;
    VclPtr<IconView>                    mpIconView;
    NameTranslator_Impl*    mpNameTrans;
    sal_uInt16              mnSortColumn;
    bool                    mbAscending     : 1;
    bool                    mbOnlyFolder    : 1;
    bool                    mbReplaceNames  : 1;    // translate folder names or display doc-title instead of file name
    sal_Int16               mnSuspendSelectCallback : 1;
    bool                    mbIsFirstResort : 1;

    IntlWrapper             aIntlWrapper;

    OUString                maViewURL;
    OUString                maAllFilter;
    OUString                maCurrentFilter;
    Image                   maFolderImage;
    Link<SvtFileView*,void> maOpenDoneLink;
    Reference< XCommandEnvironment >    mxCmdEnv;

    SvtFileView_Impl( SvtFileView* pAntiImpl, Reference < XCommandEnvironment > const & xEnv,
                                              FileViewFlags nFlags,
                                              bool bOnlyFolder );
    virtual                ~SvtFileView_Impl();

    void                    Clear();

    FileViewResult          GetFolderContent_Impl(
        const OUString& rFolder,
        const FileViewAsyncAction* pAsyncDescriptor,
        const css::uno::Sequence< OUString >& rBlackList = css::uno::Sequence< OUString >() );

    FileViewResult          GetFolderContent_Impl(
        const FolderDescriptor& _rFolder,
        const FileViewAsyncAction* pAsyncDescriptor,
        const css::uno::Sequence< OUString >& rBlackList );
    void                    FilterFolderContent_Impl( const OUString &rFilter );
    void                    CancelRunningAsyncAction();

    void                    OpenFolder_Impl();
    static void             ReplaceTabWithString( OUString& aValue );
    void                    CreateDisplayText_Impl();
    void                    SortFolderContent_Impl();

    void                    EntryRemoved( const OUString& rURL );
    void                    EntryRenamed( OUString& rURL,
                                          const OUString& rName );
    OUString                FolderInserted( const OUString& rURL,
                                            const OUString& rTitle );

    sal_uLong               GetEntryPos( const OUString& rURL );

    void                    SetViewMode( FileViewMode eMode );

    inline void             EnableDelete( bool bEnable );

    void                    Resort_Impl( sal_Int16 nColumn, bool bAscending );
    bool                    SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             bool bWrapAround );

    void                    SetActualFolder( const INetURLObject& rActualFolder );

    void                    SetSelectHandler( const Link<SvTreeListBox*,void>& _rHdl );

    void                    InitSelection();
    void                    ResetCursor();

    inline void             EndEditing();

    void                    onTimeout();

protected:
    DECL_LINK_TYPED( SelectionMultiplexer, SvTreeListBox*, void );

    // IEnumerationResultHandler overridables
    virtual void        enumerationDone( ::svt::EnumerationResult eResult ) override;
            void        implEnumerationSuccess();
};

inline void SvtFileView_Impl::EnableDelete( bool bEnable )
{
    mpView->EnableDelete( bEnable );
    if( bEnable )
        mbReplaceNames = false;
}

inline void SvtFileView_Impl::EndEditing()
{
    if ( mpCurView->IsEditingActive() )
        mpCurView->EndEditing();
}

// functions -------------------------------------------------------------

OUString CreateExactSizeText( sal_Int64 nSize )
{
    double fSize( ( double ) nSize );
    int nDec;

    long nMega = 1024 * 1024;
    long nGiga = nMega * 1024;

    OUString aUnitStr(' ');

    if ( nSize < 10000 )
    {
        aUnitStr += SVT_RESSTR(STR_SVT_BYTES );
        nDec = 0;
    }
    else if ( nSize < nMega )
    {
        fSize /= 1024;
        aUnitStr += SVT_RESSTR(STR_SVT_KB);
        nDec = 1;
    }
    else if ( nSize < nGiga )
    {
        fSize /= nMega;
        aUnitStr += SVT_RESSTR(STR_SVT_MB);
        nDec = 2;
    }
    else
    {
        fSize /= nGiga;
        aUnitStr += SVT_RESSTR(STR_SVT_GB);
        nDec = 3;
    }

    OUString aSizeStr( ::rtl::math::doubleToUString( fSize,
                rtl_math_StringFormat_F, nDec,
                SvtSysLocale().GetLocaleData().getNumDecimalSep()[0]) );
    aSizeStr += aUnitStr;

    return aSizeStr;
}


ViewTabListBox_Impl::ViewTabListBox_Impl( vcl::Window* pParentWin,
                                          SvtFileView_Impl* pParent,
                                          FileViewFlags nFlags ) :

    SvHeaderTabListBox( pParentWin, WB_TABSTOP ),

    mpHeaderBar         ( nullptr ),
    mpParent            ( pParent ),
    msAccessibleDescText( SVT_RESSTR(STR_SVT_ACC_DESC_FILEVIEW) ),
    msFolder            ( SVT_RESSTR(STR_SVT_ACC_DESC_FOLDER) ),
    msFile              ( SVT_RESSTR(STR_SVT_ACC_DESC_FILE) ),
    mnSearchIndex       ( 0 ),
    mbResizeDisabled    ( false ),
    mbAutoResize        ( false ),
    mbEnableDelete      ( false ),
    mbShowHeader        ( !(nFlags & FileViewFlags::SHOW_NONE) )
{
    Size aBoxSize = pParentWin->GetSizePixel();
    mpHeaderBar = VclPtr<HeaderBar>::Create( pParentWin, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), mpHeaderBar->CalcWindowSizePixel() );

    HeaderBarItemBits nBits = ( HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER | HeaderBarItemBits::CLICKABLE );
    if (nFlags & FileViewFlags::SHOW_ONLYTITLE)
    {
        long pTabs[] = { 2, 20, 600 };
        SetTabs(&pTabs[0], MAP_PIXEL);

        mpHeaderBar->InsertItem(COLUMN_TITLE, SVT_RESSTR(STR_SVT_FILEVIEW_COLUMN_TITLE), 600, nBits | HeaderBarItemBits::UPARROW);
    }
    else
    {
        long pTabs[] = { 5, 20, 180, 320, 400, 600 };
        SetTabs(&pTabs[0], MAP_PIXEL);
        SetTabJustify(2, AdjustRight); // column "Size"

        mpHeaderBar->InsertItem(COLUMN_TITLE, SVT_RESSTR(STR_SVT_FILEVIEW_COLUMN_TITLE), 180, nBits | HeaderBarItemBits::UPARROW);
        if (nFlags & FileViewFlags::SHOW_TYPE)
        {
            mpHeaderBar->InsertItem(COLUMN_TYPE, SVT_RESSTR(STR_SVT_FILEVIEW_COLUMN_TYPE), 140, nBits);
        }
        mpHeaderBar->InsertItem(COLUMN_SIZE, SVT_RESSTR(STR_SVT_FILEVIEW_COLUMN_SIZE), 80, nBits);
        mpHeaderBar->InsertItem(COLUMN_DATE, SVT_RESSTR(STR_SVT_FILEVIEW_COLUMN_DATE), 500, nBits);
    }

    Size aHeadSize = mpHeaderBar->GetSizePixel();
    SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                     Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    InitHeaderBar( mpHeaderBar );
    SetHighlightRange();
    SetEntryHeight( ROW_HEIGHT );
    if (nFlags & FileViewFlags::MULTISELECTION)
        SetSelectionMode( SelectionMode::Multiple );

    Show();
    if( mbShowHeader )
        mpHeaderBar->Show();

    maResetQuickSearch.SetTimeout( QUICK_SEARCH_TIMEOUT );
    maResetQuickSearch.SetTimeoutHdl( LINK( this, ViewTabListBox_Impl, ResetQuickSearch_Impl ) );

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, VCLUnoHelper::GetInterface(GetParentDialog())), UNO_QUERY_THROW );

    mxCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    EnableContextMenuHandling();
}


ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
    disposeOnce();
}

void ViewTabListBox_Impl::dispose()
{
    maResetQuickSearch.Stop();

    mpHeaderBar.disposeAndClear();
    SvHeaderTabListBox::dispose();
}


IMPL_LINK_NOARG_TYPED(ViewTabListBox_Impl, ResetQuickSearch_Impl, Timer *, void)
{
    ::osl::MutexGuard aGuard( maMutex );

    maQuickSearchText.clear();
    mnSearchIndex = 0;
}


void ViewTabListBox_Impl::Resize()
{
    SvTabListBox::Resize();
    Size aBoxSize = Control::GetParent()->GetOutputSizePixel();

    if ( mbResizeDisabled || !aBoxSize.Width() )
        return;

    Size aBarSize;
    if ( mbShowHeader )
    {
        aBarSize = mpHeaderBar->GetSizePixel();
        aBarSize.Width() = mbAutoResize ? aBoxSize.Width() : GetSizePixel().Width();
        mpHeaderBar->SetSizePixel( aBarSize );
    }

    if ( mbAutoResize )
    {
        mbResizeDisabled = true;
        SetPosSizePixel( Point( 0, aBarSize.Height() ),
                        Size( aBoxSize.Width(), aBoxSize.Height() - aBarSize.Height() ) );
        mbResizeDisabled = false;
    }
}


void ViewTabListBox_Impl::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;

    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if ( 0 == rKeyCode.GetModifier() )
    {
        if ( rKeyCode.GetCode() == KEY_RETURN )
        {
            ResetQuickSearch_Impl( nullptr );
            GetDoubleClickHdl().Call( this );
            bHandled = true;
        }
        else if ( ( rKeyCode.GetCode() == KEY_DELETE ) &&
                  mbEnableDelete )
        {
            ResetQuickSearch_Impl( nullptr );
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
        ResetQuickSearch_Impl( nullptr );
        SvHeaderTabListBox::KeyInput( rKEvt );
    }
}


VclPtr<PopupMenu> ViewTabListBox_Impl::CreateContextMenu()
{
    bool bEnableDelete = mbEnableDelete;
    bool bEnableRename = true;

    if ( bEnableDelete || bEnableRename )
    {
        sal_Int32 nSelectedEntries = GetSelectionCount();
        bEnableDelete &= nSelectedEntries > 0;
        bEnableRename &= nSelectedEntries == 1;
    }

    if ( bEnableDelete || bEnableRename )
    {
        SvTreeListEntry* pEntry = FirstSelected();
        while ( pEntry )
        {
            ::ucbhelper::Content aCnt;
            try
            {
                OUString aURL( static_cast< SvtContentEntry * >(
                    pEntry->GetUserData() )->maURL );
                aCnt = ::ucbhelper::Content( aURL, mxCmdEnv, comphelper::getProcessComponentContext() );
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
                        bEnableDelete = aCommands->hasCommandByName( "delete" );
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
                        Property aProp = aProps->getPropertyByName("Title");
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
                : nullptr;
        }
    }

    if ( bEnableDelete || bEnableRename )
    {
        VclPtrInstance<PopupMenu> pRet(SvtResId( RID_FILEVIEW_CONTEXTMENU ));
        pRet->EnableItem( MID_FILEVIEW_DELETE, bEnableDelete );
        pRet->EnableItem( MID_FILEVIEW_RENAME, bEnableRename );
        pRet->RemoveDisabledEntries( true, true );
        return pRet;
    }

    return nullptr;
}


void ViewTabListBox_Impl::ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
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


void ViewTabListBox_Impl::ClearAll()
{
    for ( sal_uLong i = 0; i < GetEntryCount(); ++i )
        delete static_cast<SvtContentEntry*>(GetEntry(i)->GetUserData());
    Clear();
}


void ViewTabListBox_Impl::DeleteEntries()
{
    short eResult = svtools::QUERYDELETE_YES;
    SvTreeListEntry* pEntry = FirstSelected();
    OUString aURL;

    OString sDialogPosition;
    while ( pEntry && ( eResult != svtools::QUERYDELETE_CANCEL ) )
    {
        SvTreeListEntry *pCurEntry = pEntry;
        pEntry = NextSelected( pEntry );

        if ( pCurEntry->GetUserData() )
            aURL = static_cast<SvtContentEntry*>(pCurEntry->GetUserData())->maURL;

        if ( aURL.isEmpty() )
            continue;

        bool canDelete = true;
        try
        {
            ::ucbhelper::Content aCnt( aURL, mxCmdEnv, comphelper::getProcessComponentContext() );
            Reference< XCommandInfo > aCommands = aCnt.getCommands();
            if ( aCommands.is() )
                canDelete = aCommands->hasCommandByName( "delete" );
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
            ScopedVclPtrInstance< svtools::QueryDeleteDlg_Impl > aDlg(nullptr, aObj.GetName( INetURLObject::DECODE_WITH_CHARSET ) );
            if ( sDialogPosition.getLength() )
                aDlg->SetWindowState( sDialogPosition );

            if ( GetSelectionCount() > 1 )
                aDlg->EnableAllButton();

            eResult = aDlg->Execute();

            sDialogPosition = aDlg->GetWindowState( );
        }

        if ( ( eResult == svtools::QUERYDELETE_ALL ) ||
             ( eResult == svtools::QUERYDELETE_YES ) )
        {
            if ( Kill( aURL ) )
            {
                delete static_cast<SvtContentEntry*>(pCurEntry->GetUserData());
                GetModel()->Remove( pCurEntry );
                mpParent->EntryRemoved( aURL );
            }
        }
    }
}


bool ViewTabListBox_Impl::EditedEntry( SvTreeListEntry* pEntry,
                                 const OUString& rNewText )
{
    bool bRet = false;

    OUString aURL;
    SvtContentEntry* pData = static_cast<SvtContentEntry*>(pEntry->GetUserData());

    if ( pData )
        aURL = pData->maURL;

    if ( aURL.isEmpty() )
        return bRet;

    try
    {
        OUString aPropName( "Title" );
        bool canRename = true;
        ::ucbhelper::Content aContent( aURL, mxCmdEnv, comphelper::getProcessComponentContext() );

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
            aValue <<= rNewText;
            aContent.setPropertyValue( aPropName, aValue );
            mpParent->EntryRenamed( aURL, rNewText );

            if (pData)
                pData->maURL = aURL;

            pEntry->SetUserData( pData );

            bRet = true;
        }
    }
    catch( Exception const & )
    {
    }

    return bRet;
}


void ViewTabListBox_Impl::DoQuickSearch( sal_Unicode rChar )
{
    ::osl::MutexGuard aGuard( maMutex );

    maResetQuickSearch.Stop();

    OUString    aLastText = maQuickSearchText;
    sal_uInt32  aLastPos = mnSearchIndex;

    maQuickSearchText += OUString(rChar).toAsciiLowerCase();

    bool bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, false );

    if ( !bFound && ( aLastText.getLength() == 1 ) &&
         ( aLastText == OUString(rChar) ) )
    {
        mnSearchIndex = aLastPos + 1;
        maQuickSearchText = aLastText;
        bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, true );
    }

    if ( bFound )
    {
        SvTreeListEntry* pEntry = GetEntry( mnSearchIndex );
        if ( pEntry )
        {
            SelectAll( false );
            Select( pEntry );
            SetCurEntry( pEntry );
            MakeVisible( pEntry );
        }
    }

    maResetQuickSearch.Start();
}


bool ViewTabListBox_Impl::DoubleClickHdl()
{
    SvHeaderTabListBox::DoubleClickHdl();
    return false;
        // this means "do no additional handling". Especially this means that the SvImpLBox does not
        // recognize that the entry at the double click position change after the handler call (which is
        // the case if in the handler, our content was replaced)
        // If it _would_ recognize this change, it would take this as a reason to select the entry, again
        // - which is not what in the case of content replace
        // (I really doubt that this behaviour of the SvImpLBox does make any sense at all, but
        // who knows ...)
}

OUString ViewTabListBox_Impl::GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString sRet = SvHeaderTabListBox::GetAccessibleObjectDescription( _eType, _nPos );
    if ( ::svt::BBTYPE_TABLECELL == _eType )
    {
        sal_Int32 nRow = -1;
        const sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
            nRow = _nPos / nColumnCount;
        SvTreeListEntry* pEntry = GetEntry( nRow );
        if ( pEntry )
        {
            SvtContentEntry* pData = static_cast<SvtContentEntry*>(pEntry->GetUserData());
            if ( pData )
            {
                const OUString sVar1( "%1" );
                const OUString sVar2( "%2" );
                OUString aText( msAccessibleDescText );
                aText = aText.replaceAll( sVar1, pData->mbIsFolder ? msFolder : msFile );
                aText = aText.replaceAll( sVar2, pData->maURL );
                sRet += aText;
            }
        }
    }

    return sRet;
}


bool ViewTabListBox_Impl::Kill( const OUString& rContent )
{
    bool bRet = true;

    try
    {
        ::ucbhelper::Content aCnt( rContent, mxCmdEnv, comphelper::getProcessComponentContext() );
        aCnt.executeCommand( "delete", makeAny( true ) );
    }
    catch( css::ucb::CommandAbortedException const & )
    {
        SAL_INFO( "svtools", "CommandAbortedException" );
        bRet = false;
    }
    catch( Exception const & )
    {
        SAL_INFO( "svtools", "Any other exception" );
        bRet = false;
    }

    return bRet;
}

SvtFileView::SvtFileView( vcl::Window* pParent, WinBits nBits,
                          bool bOnlyFolder, bool bMultiSelection, bool bShowType ) :

    Control( pParent, nBits )
{
    FileViewFlags nFlags = FileViewFlags::NONE;
    if ( bOnlyFolder )
        nFlags |= FileViewFlags::ONLYFOLDER;
    if ( bMultiSelection )
        nFlags |= FileViewFlags::MULTISELECTION;
    if ( bShowType )
        nFlags |= FileViewFlags::SHOW_TYPE;

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, VCLUnoHelper::GetInterface(GetParentDialog())), UNO_QUERY_THROW );
    Reference < XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    mpImpl.reset( new SvtFileView_Impl( this, xCmdEnv, nFlags, bOnlyFolder ) );
    mpImpl->mpView->ForbidEmptyText();
    SetSortColumn( true );

    HeaderBar* pHeaderBar = mpImpl->mpView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, SvtFileView, HeaderEndDrag_Impl ) );
}

SvtFileView::~SvtFileView()
{
    disposeOnce();
}

void SvtFileView::dispose()
{
    mpImpl.reset();
    Control::dispose();
}

VCL_BUILDER_DECL_FACTORY(SvtFileView)
{
    WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    rRet = VclPtr<SvtFileView>::Create(pParent, nBits, true, true);
}

Size SvtFileView::GetOptimalSize() const
{
    return LogicToPixel(Size(208, 50), MAP_APPFONT);
}

void SvtFileView::SetViewMode( FileViewMode eMode )
{
    mpImpl->SetViewMode( eMode );
}

OUString SvtFileView::GetURL( SvTreeListEntry* pEntry )
{
    OUString aURL;
    if ( pEntry && pEntry->GetUserData() )
        aURL = static_cast<SvtContentEntry*>(pEntry->GetUserData())->maURL;
    return aURL;
}


OUString SvtFileView::GetCurrentURL() const
{
    OUString aURL;
    SvTreeListEntry* pEntry = mpImpl->mpCurView->FirstSelected();
    if ( pEntry && pEntry->GetUserData() )
        aURL = static_cast<SvtContentEntry*>(pEntry->GetUserData())->maURL;
    return aURL;
}


void SvtFileView::CreatedFolder( const OUString& rUrl, const OUString& rNewFolder )
{
    OUString sEntry = mpImpl->FolderInserted( rUrl, rNewFolder );

    SvTreeListEntry* pEntry = mpImpl->mpView->InsertEntry( sEntry, mpImpl->maFolderImage, mpImpl->maFolderImage );
    SvtContentEntry* pUserData = new SvtContentEntry( rUrl, true );
    pEntry->SetUserData( pUserData );
    mpImpl->mpView->MakeVisible( pEntry );

    SvTreeListEntry* pEntry2 = mpImpl->mpIconView->InsertEntry( sEntry.getToken( 0, '\t' ), mpImpl->maFolderImage, mpImpl->maFolderImage );
    SvtContentEntry* pUserData2 = new SvtContentEntry( rUrl, true );
    pEntry2->SetUserData( pUserData2 );
    mpImpl->mpIconView->MakeVisible( pEntry2 );
}


FileViewResult SvtFileView::PreviousLevel( const FileViewAsyncAction* pAsyncDescriptor )
{
    FileViewResult eResult = eFailure;

    OUString sParentURL;
    if ( GetParentURL( sParentURL ) )
        eResult = Initialize( sParentURL, mpImpl->maCurrentFilter, pAsyncDescriptor, mpBlackList );

    return eResult;
}


bool SvtFileView::GetParentURL( OUString& rParentURL ) const
{
    bool bRet = false;
    try
    {
        ::ucbhelper::Content aCnt( mpImpl->maViewURL, mpImpl->mxCmdEnv, comphelper::getProcessComponentContext() );
        Reference< XContent > xContent( aCnt.get() );
        Reference< css::container::XChild > xChild( xContent, UNO_QUERY );
        if ( xChild.is() )
        {
            Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
            if ( xParent.is() )
            {
                rParentURL = xParent->getIdentifier()->getContentIdentifier();
                bRet = !rParentURL.isEmpty() && rParentURL != mpImpl->maViewURL;
            }
        }
    }
    catch( Exception const & )
    {
        // perhaps an unknown url protocol (e.g. "private:newdoc")
    }

    return bRet;
}


const OString& SvtFileView::GetHelpId( ) const
{
    return mpImpl->mpView->GetHelpId( );
}


void SvtFileView::SetHelpId( const OString& rHelpId )
{
    mpImpl->mpView->SetHelpId( rHelpId );
}


void SvtFileView::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    mpImpl->mpView->SetSizePixel( rNewSize );
    mpImpl->mpIconView->SetSizePixel( rNewSize );
}


void SvtFileView::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}


bool SvtFileView::Initialize( const css::uno::Reference< css::ucb::XContent>& _xContent, const OUString& rFilter  )
{
    WaitObject aWaitCursor( this );

    mpImpl->Clear();
    ::ucbhelper::Content aContent(_xContent, mpImpl->mxCmdEnv, comphelper::getProcessComponentContext() );
    FileViewResult eResult = mpImpl->GetFolderContent_Impl( FolderDescriptor( aContent ), nullptr, css::uno::Sequence< OUString >() );
    OSL_ENSURE( eResult != eStillRunning, "SvtFileView::Initialize: this was expected to be synchronous!" );
    if ( eResult != eSuccess )
        return false;

    mpImpl->FilterFolderContent_Impl( rFilter );

    mpImpl->SortFolderContent_Impl(); // possibly not necessary!!!!!!!!!!
    mpImpl->CreateDisplayText_Impl();
    mpImpl->OpenFolder_Impl();

    mpImpl->maOpenDoneLink.Call( this );
    return true;
}


FileViewResult SvtFileView::Initialize(
    const OUString& rURL,
    const OUString& rFilter,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList  )
{
    WaitObject aWaitCursor( this );
    mpBlackList = rBlackList;

    OUString sPushURL( mpImpl->maViewURL );

    mpImpl->maViewURL = rURL;
    FileViewResult eResult = ExecuteFilter( rFilter, pAsyncDescriptor );
    switch ( eResult )
    {
    case eFailure:
    case eTimeout:
        mpImpl->maViewURL = sPushURL;
        return eResult;

    case eStillRunning:
        OSL_ENSURE( pAsyncDescriptor, "SvtFileView::Initialize: we told it to read synchronously!" );
        SAL_FALLTHROUGH;
    case eSuccess:
        return eResult;
    }

    OSL_FAIL( "SvtFileView::Initialize: unreachable!" );
    return eFailure;
}

FileViewResult SvtFileView::ExecuteFilter( const OUString& rFilter, const FileViewAsyncAction* pAsyncDescriptor )
{
    mpImpl->maCurrentFilter = rFilter.toAsciiLowerCase();

    mpImpl->Clear();
    FileViewResult eResult = mpImpl->GetFolderContent_Impl( mpImpl->maViewURL, pAsyncDescriptor, mpBlackList );
    OSL_ENSURE( ( eResult != eStillRunning ) || pAsyncDescriptor, "SvtFileView::ExecuteFilter: we told it to read synchronously!" );
    return eResult;
}

void SvtFileView::CancelRunningAsyncAction()
{
    mpImpl->CancelRunningAsyncAction();
}

void SvtFileView::SetNoSelection()
{
    mpImpl->mpCurView->SelectAll( false );
}


void SvtFileView::GetFocus()
{
    Control::GetFocus();
    if ( mpImpl && mpImpl->mpCurView )
        mpImpl->mpCurView->GrabFocus();
}


void SvtFileView::SetSelectHdl( const Link<SvTreeListBox*,void>& rHdl )
{
    mpImpl->SetSelectHandler( rHdl );
}


void SvtFileView::SetDoubleClickHdl( const Link<SvTreeListBox*,bool>& rHdl )
{
    mpImpl->mpView->SetDoubleClickHdl( rHdl );
    mpImpl->mpIconView->SetDoubleClickHdl( rHdl );
}


sal_uLong SvtFileView::GetSelectionCount() const
{
    return mpImpl->mpCurView->GetSelectionCount();
}


SvTreeListEntry* SvtFileView::FirstSelected() const
{
    return mpImpl->mpCurView->FirstSelected();
}


SvTreeListEntry* SvtFileView::NextSelected( SvTreeListEntry* pEntry ) const
{
    return mpImpl->mpCurView->NextSelected( pEntry );
}

void SvtFileView::EnableAutoResize()
{
    mpImpl->mpView->EnableAutoResize();
}

const OUString& SvtFileView::GetViewURL() const
{
    return mpImpl->maViewURL;
}

void SvtFileView::SetOpenDoneHdl( const Link<SvtFileView*,void>& rHdl )
{
    mpImpl->maOpenDoneLink = rHdl;
}

void SvtFileView::EnableDelete( bool bEnable )
{
    mpImpl->EnableDelete( bEnable );
}

void SvtFileView::EndInplaceEditing()
{
    return mpImpl->EndEditing();
}

IMPL_LINK_TYPED( SvtFileView, HeaderSelect_Impl, HeaderBar*, pBar, void )
{
    DBG_ASSERT( pBar, "no headerbar" );
    sal_uInt16 nItemID = pBar->GetCurItemId();

    HeaderBarItemBits nBits;

    // clear the arrow of the recently used column
    if ( nItemID != mpImpl->mnSortColumn )
    {
        if ( !nItemID )
        {
            // first call -> remove arrow from title column,
            // because another column is the sort column
            nItemID = mpImpl->mnSortColumn;
            mpImpl->mnSortColumn = COLUMN_TITLE;
        }
        nBits = pBar->GetItemBits( mpImpl->mnSortColumn );
        nBits &= ~HeaderBarItemBits( HeaderBarItemBits::UPARROW | HeaderBarItemBits::DOWNARROW );
        pBar->SetItemBits( mpImpl->mnSortColumn, nBits );
    }

    nBits = pBar->GetItemBits( nItemID );

    bool bUp = ( ( nBits & HeaderBarItemBits::UPARROW ) == HeaderBarItemBits::UPARROW );

    if ( bUp )
    {
        nBits &= ~HeaderBarItemBits::UPARROW;
        nBits |= HeaderBarItemBits::DOWNARROW;
    }
    else
    {
        nBits &= ~HeaderBarItemBits::DOWNARROW;
        nBits |= HeaderBarItemBits::UPARROW;
    }

    pBar->SetItemBits( nItemID, nBits );
    mpImpl->Resort_Impl( nItemID, !bUp );
}


IMPL_LINK_TYPED( SvtFileView, HeaderEndDrag_Impl, HeaderBar*, pBar, void )
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
            mpImpl->mpView->SetTab( i, aSize.Width(), MAP_PIXEL );
        }
    }
}


OUString SvtFileView::GetConfigString() const
{
    OUString sRet;
    HeaderBar* pBar = mpImpl->mpView->GetHeaderBar();
    DBG_ASSERT( pBar, "invalid headerbar" );

    // sort order
    sRet += OUString::number( mpImpl->mnSortColumn );
    sRet += ";";
    HeaderBarItemBits nBits = pBar->GetItemBits( mpImpl->mnSortColumn );
    bool bUp = ( ( nBits & HeaderBarItemBits::UPARROW ) == HeaderBarItemBits::UPARROW );
    sRet += bUp ? OUString("1") : OUString("0");
    sRet += ";";

    sal_uInt16 nCount = pBar->GetItemCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        sal_uInt16 nId = pBar->GetItemId(i);
        sRet += OUString::number( nId );
        sRet += ";";
        sRet += OUString::number( pBar->GetItemSize( nId ) );
        sRet += ";";
    }

    sRet = comphelper::string::stripEnd(sRet, ';');
    return sRet;
}

::std::vector< SvtContentEntry > SvtFileView::GetContent()
{
    ::std::vector< SvtContentEntry > aContent;

    for(SortingData_Impl* i : mpImpl->maContent)
    {
        SvtContentEntry aEntry( i->maTargetURL, i->mbIsFolder );
        aContent.push_back( aEntry );
    }

    return aContent;
}

void SvtFileView::SetConfigString( const OUString& rCfgStr )
{
    HeaderBar* pBar = mpImpl->mpView->GetHeaderBar();
    DBG_ASSERT( pBar, "invalid headerbar" );

    sal_Int32 nIdx = 0;
    mpImpl->mnSortColumn = (sal_uInt16)rCfgStr.getToken( 0, ';', nIdx ).toInt32();
    bool bUp = (bool)(sal_uInt16)rCfgStr.getToken( 0, ';', nIdx ).toInt32();
    HeaderBarItemBits nBits = pBar->GetItemBits( mpImpl->mnSortColumn );

    if ( bUp )
    {
        nBits &= ~HeaderBarItemBits::UPARROW;
        nBits |= HeaderBarItemBits::DOWNARROW;
    }
    else
    {
        nBits &= ~HeaderBarItemBits::DOWNARROW;
        nBits |= HeaderBarItemBits::UPARROW;
    }
    pBar->SetItemBits( mpImpl->mnSortColumn, nBits );

    while ( nIdx != -1 )
    {
        sal_uInt16 nItemId = (sal_uInt16)rCfgStr.getToken( 0, ';', nIdx ).toInt32();
        pBar->SetItemSize( nItemId, rCfgStr.getToken( 0, ';', nIdx ).toInt32() );
    }

    HeaderSelect_Impl( pBar );
    HeaderEndDrag_Impl( pBar );
}


void SvtFileView::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::Enable )
        Invalidate();
    Control::StateChanged( nStateChange );
}


// class NameTranslator_Impl


NameTranslator_Impl::NameTranslator_Impl( const INetURLObject& rActualFolder )
{
    mpActFolder = new NameTranslationList( rActualFolder );
}

NameTranslator_Impl::~NameTranslator_Impl()
{
    delete mpActFolder;
}

void NameTranslator_Impl::SetActualFolder( const INetURLObject& rActualFolder )
{
    if( mpActFolder )
    {
        if (mpActFolder->GetHashedURL() != rActualFolder.GetMainURL(INetURLObject::NO_DECODE))
        {
            delete mpActFolder;
            mpActFolder = new NameTranslationList( rActualFolder );
        }
    }
    else
        mpActFolder = new NameTranslationList( rActualFolder );
}

bool NameTranslator_Impl::GetTranslation( const OUString& rOrg, OUString& rTrans ) const
{
    bool bRet = false;

    if( mpActFolder )
    {
        const OUString* pTrans = mpActFolder->Translate( rOrg );
        if( pTrans )
        {
            rTrans = *pTrans;
            bRet = true;
        }
    }

    return bRet;
}

const OUString* NameTranslator_Impl::GetTransTableFileName() const
{
    return mpActFolder? &mpActFolder->GetTransTableFileName() : nullptr;
}


// class SvtFileView_Impl


SvtFileView_Impl::SvtFileView_Impl( SvtFileView* pAntiImpl, Reference < XCommandEnvironment > const & xEnv, FileViewFlags nFlags, bool bOnlyFolder )

    :mpAntiImpl                 ( pAntiImpl )
    ,m_eAsyncActionResult       ( ::svt::ERROR )
    ,m_bRunningAsyncAction      ( false )
    ,m_bAsyncActionCancelled    ( false )
    ,m_eViewMode                ( eDetailedList )
    ,mpNameTrans                ( nullptr )
    ,mnSortColumn               ( COLUMN_TITLE )
    ,mbAscending                ( true )
    ,mbOnlyFolder               ( bOnlyFolder )
    ,mbReplaceNames             ( false )
    ,mnSuspendSelectCallback    ( 0 )
    ,mbIsFirstResort            ( true )
    ,aIntlWrapper               ( Application::GetSettings().GetLanguageTag() )
    ,maFolderImage              ( SvtResId( IMG_SVT_FOLDER ) )
    ,mxCmdEnv ( xEnv )

{
    maAllFilter = "*.*";
    mpView = VclPtr<ViewTabListBox_Impl>::Create( mpAntiImpl, this, nFlags );
    mpCurView = mpView;
    mpIconView = VclPtr<IconView>::Create( mpAntiImpl, WB_TABSTOP );
    mpIconView->Hide();
    mpView->EnableCellFocus();
}


SvtFileView_Impl::~SvtFileView_Impl()
{
    Clear();
    mpView.disposeAndClear();
    mpCurView.clear();
    mpIconView.disposeAndClear();
    mpAntiImpl.clear();
}


void SvtFileView_Impl::Clear()
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); ++aIt )
        delete (*aIt);

    maContent.clear();

    if( mpNameTrans )
        DELETEZ( mpNameTrans );
}


FileViewResult SvtFileView_Impl::GetFolderContent_Impl(
    const OUString& rFolder,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );

    // prepare name translation
    SetActualFolder( aFolderObj );

    FolderDescriptor aFolder( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ) );

    aGuard.clear();
    return GetFolderContent_Impl( aFolder, pAsyncDescriptor, rBlackList );
}


FileViewResult SvtFileView_Impl::GetFolderContent_Impl(
    const FolderDescriptor& _rFolder,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList )
{
    DBG_TESTSOLARMUTEX();
    ::osl::ClearableMutexGuard aGuard( maMutex );

    OSL_ENSURE( !m_xContentEnumerator.is(), "SvtFileView_Impl::GetFolderContent_Impl: still running another enumeration!" );
    m_xContentEnumerator.set(new ::svt::FileViewContentEnumerator(
        mpView->GetCommandEnvironment(), maContent, maMutex, mbReplaceNames ? mpNameTrans : nullptr));
        // TODO: should we cache and re-use this thread?

    if ( !pAsyncDescriptor )
    {
        ::svt::EnumerationResult eResult = m_xContentEnumerator->enumerateFolderContentSync( _rFolder, rBlackList );
        if ( ::svt::SUCCESS == eResult )
        {
            implEnumerationSuccess();
            m_xContentEnumerator.clear();
            return eSuccess;
        }
        m_xContentEnumerator.clear();
        return eFailure;
    }

    m_bRunningAsyncAction = true;
    m_bAsyncActionCancelled = false;
    m_eAsyncActionResult = ::svt::ERROR;
    m_aAsyncActionFinished.reset();

    // don't (yet) set m_aCurrentAsyncActionHandler to pTimeout->aFinishHandler.
    // By definition, this handler *only* get's called when the result cannot be obtained
    // during the minimum wait time, so it is only set below, when needed.
    m_aCurrentAsyncActionHandler = Link<void*,void>();

    // minimum time to wait
    std::unique_ptr< TimeValue > pTimeout( new TimeValue );
    sal_Int32 nMinTimeout = pAsyncDescriptor->nMinTimeout;
    OSL_ENSURE( nMinTimeout > 0, "SvtFileView_Impl::GetFolderContent_Impl: invalid minimum timeout!" );
    if ( nMinTimeout <= 0 )
        nMinTimeout = sal_Int32( 1000L );
    pTimeout->Seconds = nMinTimeout / 1000L;
    pTimeout->Nanosec = ( nMinTimeout % 1000L ) * 1000000L;

    m_xContentEnumerator->enumerateFolderContent( _rFolder, this );

    // wait until the enumeration is finished
    // for this, release our own mutex (which is used by the enumerator thread)
    aGuard.clear();

    ::osl::Condition::Result eResult = ::osl::Condition::result_ok;
    {
        // also release the SolarMutex. Not all code which is needed during the enumeration
        // is Solar-Thread-Safe, in particular there is some code which needs to access
        // string resources (and our resource system relies on the SolarMutex :()
        SolarMutexReleaser aSolarRelease;

        // now wait. Note that if we didn't get an pAsyncDescriptor, then this is an infinite wait.
        eResult = m_aAsyncActionFinished.wait( pTimeout.get() );
    }

    ::osl::MutexGuard aGuard2( maMutex );
    if ( ::osl::Condition::result_timeout == eResult )
    {
        // maximum time to wait
        OSL_ENSURE( !m_xCancelAsyncTimer.get(), "SvtFileView_Impl::GetFolderContent_Impl: there's still a previous timer!" );
        m_xCancelAsyncTimer.set(new CallbackTimer(this));
        sal_Int32 nMaxTimeout = pAsyncDescriptor->nMaxTimeout;
        OSL_ENSURE( nMaxTimeout > nMinTimeout,
            "SvtFileView_Impl::GetFolderContent_Impl: invalid maximum timeout!" );
        if ( nMaxTimeout <= nMinTimeout )
            nMaxTimeout = nMinTimeout + 5000;
        m_xCancelAsyncTimer->setRemainingTime( salhelper::TTimeValue( nMaxTimeout - nMinTimeout ) );
            // we already waited for nMinTimeout milliseconds, so take this into account
        m_xCancelAsyncTimer->start();

        m_aCurrentAsyncActionHandler = pAsyncDescriptor->aFinishHandler;
        DBG_ASSERT( m_aCurrentAsyncActionHandler.IsSet(), "SvtFileView_Impl::GetFolderContent_Impl: nobody interested when it's finished?" );
        mpView->ClearAll();
        mpIconView->ClearAll();
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

    SAL_WARN( "svtools.contnr", "SvtFileView_Impl::GetFolderContent_Impl: unreachable!" );
    return eFailure;
}


void SvtFileView_Impl::FilterFolderContent_Impl( const OUString &rFilter )
{
    bool bHideTransFile = mbReplaceNames && mpNameTrans;

    OUString sHideEntry;
    if( bHideTransFile )
    {
        const OUString* pTransTableFileName = mpNameTrans->GetTransTableFileName();
        if( pTransTableFileName )
        {
            sHideEntry = *pTransTableFileName;
            sHideEntry = sHideEntry.toAsciiUpperCase();
        }
        else
            bHideTransFile = false;
    }

    if ( !bHideTransFile &&
        ( rFilter.isEmpty() || ( rFilter == ALL_FILES_FILTER ) ) )
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
    OUString sCompareString;
    do
    {
        if ( (*aContentLoop)->mbIsFolder )
            ++aContentLoop;
        else
        {
            // normalize the content title (we always match case-insensitive)
            // 91872 - 11.09.2001 - frank.schoenheit@sun.com
            sCompareString = (*aContentLoop)->GetFileName(); // filter works on file name, not on title!
            bool bDelete;

            if( bHideTransFile && sCompareString == sHideEntry )
                bDelete = true;
            else
            {
                bDelete = ::std::none_of( aFilters.begin(), aFilters.end(),
                                          FilterMatch( sCompareString ) );
            }

            if( bDelete )
            {
                // none of the filters did match
                delete *aContentLoop;

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


IMPL_LINK_TYPED( SvtFileView_Impl, SelectionMultiplexer, SvTreeListBox*, _pSource, void )
{
    if (!mnSuspendSelectCallback)
        m_aSelectHandler.Call( _pSource );
}


void SvtFileView_Impl::SetSelectHandler( const Link<SvTreeListBox*,void>& _rHdl )
{
    m_aSelectHandler = _rHdl;

    Link<SvTreeListBox*,void> aMasterHandler;
    if ( m_aSelectHandler.IsSet() )
        aMasterHandler = LINK( this, SvtFileView_Impl, SelectionMultiplexer );

    mpView->SetSelectHdl( aMasterHandler );
    mpIconView->SetSelectHdl( aMasterHandler );
}


void SvtFileView_Impl::InitSelection()
{
    mpCurView->SelectAll( false );
    SvTreeListEntry* pFirst = mpCurView->First();
    if ( pFirst )
        mpCurView->SetCursor( pFirst, true );
}


void SvtFileView_Impl::OpenFolder_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    mpView->SetUpdateMode( false );
    mpIconView->SetUpdateMode( false );
    mpView->ClearAll();
    mpIconView->ClearAll();

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); ++aIt )
    {
        if ( mbOnlyFolder && ! (*aIt)->mbIsFolder )
            continue;

        // insert entry and set user data
        SvTreeListEntry* pEntry = mpView->InsertEntry( (*aIt)->maDisplayText,
                                                   (*aIt)->maImage,
                                                   (*aIt)->maImage );

        SvTreeListEntry* pEntry2 = mpIconView->InsertEntry( (*aIt)->maDisplayText.getToken( 0, '\t' ),
                                                   (*aIt)->maImage, (*aIt)->maImage );

        SvtContentEntry* pUserData = new SvtContentEntry( (*aIt)->maTargetURL,
                                                          (*aIt)->mbIsFolder );
        SvtContentEntry* pUserData2 = new SvtContentEntry( (*aIt)->maTargetURL,
                                                          (*aIt)->mbIsFolder );

        pEntry->SetUserData( pUserData );
        pEntry2->SetUserData( pUserData2 );
    }

    InitSelection();

    ++mnSuspendSelectCallback;
    mpView->SetUpdateMode( true );
    mpIconView->SetUpdateMode( true );
    --mnSuspendSelectCallback;

    ResetCursor();
}


void SvtFileView_Impl::ResetCursor()
{
    // deselect
    SvTreeListEntry* pEntry = mpCurView->FirstSelected();
    if ( pEntry )
        mpCurView->Select( pEntry, false );
    // set cursor to the first entry
    mpCurView->SetCursor( mpCurView->First(), true );
    mpCurView->Update();
}


void SvtFileView_Impl::CancelRunningAsyncAction()
{
    DBG_TESTSOLARMUTEX();
    ::osl::MutexGuard aGuard( maMutex );
    if ( !m_xContentEnumerator.is() )
        return;

    m_bAsyncActionCancelled = true;
    m_xContentEnumerator->cancel();
    m_bRunningAsyncAction = false;

    m_xContentEnumerator.clear();
    if ( m_xCancelAsyncTimer.is() && m_xCancelAsyncTimer->isTicking() )
        m_xCancelAsyncTimer->stop();
    m_xCancelAsyncTimer.clear();
}


void SvtFileView_Impl::onTimeout()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( maMutex );
    if ( !m_bRunningAsyncAction )
        // there might have been a race condition while we waited for the mutex
        return;

    CancelRunningAsyncAction();

    if ( m_aCurrentAsyncActionHandler.IsSet() )
    {
        Application::PostUserEvent( m_aCurrentAsyncActionHandler, reinterpret_cast< void* >( eTimeout ) );
        m_aCurrentAsyncActionHandler = Link<void*,void>();
    }
}


void SvtFileView_Impl::enumerationDone( ::svt::EnumerationResult eResult )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( maMutex );

    m_xContentEnumerator.clear();
    if ( m_xCancelAsyncTimer.is() && m_xCancelAsyncTimer->isTicking() )
        m_xCancelAsyncTimer->stop();
    m_xCancelAsyncTimer.clear();

    if ( m_bAsyncActionCancelled )
        // this is to prevent race conditions
        return;

    m_eAsyncActionResult = eResult;
    m_bRunningAsyncAction = false;

    m_aAsyncActionFinished.set();

    if ( svt::SUCCESS == eResult )
        implEnumerationSuccess();

    if ( m_aCurrentAsyncActionHandler.IsSet() )
    {
        Application::PostUserEvent( m_aCurrentAsyncActionHandler, reinterpret_cast< void* >( m_eAsyncActionResult ) );
        m_aCurrentAsyncActionHandler = Link<void*,void>();
    }
}


void SvtFileView_Impl::implEnumerationSuccess()
{
    FilterFolderContent_Impl( maCurrentFilter );
    SortFolderContent_Impl();
    CreateDisplayText_Impl();
    OpenFolder_Impl();
    maOpenDoneLink.Call( mpAntiImpl );
}


void SvtFileView_Impl::ReplaceTabWithString( OUString& aValue )
{
    OUString aTab( "\t" );
    OUString aTabString( "%09" );
    sal_Int32 iPos;

    while ( ( iPos = aValue.indexOf( aTab ) ) >= 0 )
       aValue = aValue.replaceAt( iPos, 1, aTabString );
}


void SvtFileView_Impl::CreateDisplayText_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString aValue;
    OUString aTab( "\t" );
    OUString aDateSep( ", " );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); ++aIt )
    {
        // title, type, size, date
        aValue = (*aIt)->GetTitle();
        ReplaceTabWithString( aValue );
        aValue += aTab;
        aValue += (*aIt)->maType;
        aValue += aTab;
        // folders don't have a size
        if ( ! (*aIt)->mbIsFolder )
            aValue += CreateExactSizeText( (*aIt)->maSize );
        aValue += aTab;
        // set the date, but volumes have no date
        if ( ! (*aIt)->mbIsFolder || ! (*aIt)->mbIsVolume )
        {
            SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
            aValue += rLocaleData.getDate( (*aIt)->maModDate );
            aValue += aDateSep;
            aValue += rLocaleData.getTime( (*aIt)->maModDate, false );
        }
        (*aIt)->maDisplayText = aValue;

        // detect image
        if ( (*aIt)->mbIsFolder )
        {
            ::svtools::VolumeInfo aVolInfo( (*aIt)->mbIsVolume, (*aIt)->mbIsRemote,
                                            (*aIt)->mbIsRemoveable, (*aIt)->mbIsFloppy,
                                            (*aIt)->mbIsCompactDisc );
            (*aIt)->maImage = SvFileInformationManager::GetFolderImage( aVolInfo );
        }
        else
            (*aIt)->maImage = SvFileInformationManager::GetFileImage( INetURLObject( (*aIt)->maTargetURL ) );
    }
}

void SvtFileView_Impl::Resort_Impl( sal_Int16 nColumn, bool bAscending )
{
    // TODO: IconView ()
    ::osl::MutexGuard aGuard( maMutex );

    if ( ( nColumn == mnSortColumn ) &&
         ( bAscending == mbAscending ) )
         return;

    // reset the quick search index
    mpView->ResetQuickSearch_Impl( nullptr );

    OUString aEntryURL;
    SvTreeListEntry* pEntry = mpView->GetCurEntry();
    if ( pEntry && pEntry->GetUserData() )
        aEntryURL = static_cast<SvtContentEntry*>(pEntry->GetUserData())->maURL;

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

            ++mnSuspendSelectCallback;  // #i15668#
            mpView->SetCurEntry( pEntry );
            --mnSuspendSelectCallback;
        }
    }
    else
        mbIsFirstResort = false;
}


static bool                     gbAscending = true;
static sal_Int16                gnColumn = COLUMN_TITLE;
static const CollatorWrapper*   pCollatorWrapper = nullptr;

/* this function returns true, if aOne is less then aTwo
*/
bool CompareSortingData_Impl( SortingData_Impl* const aOne, SortingData_Impl* const aTwo )
{
    DBG_ASSERT( pCollatorWrapper, "*CompareSortingData_Impl(): Can't work this way!" );

    sal_Int32 nComp;
    bool      bRet = false;
    bool      bEqual = false;

    if ( aOne->mbIsFolder != aTwo->mbIsFolder )
    {
        if ( aOne->mbIsFolder )
            bRet = true;
        else
            bRet = false;

        // !!! pb: #100376# folder always on top
        if ( !gbAscending )
            bRet = !bRet;
    }
    else
    {
        switch ( gnColumn )
        {
            case COLUMN_TITLE:
                // compare case insensitive first
                nComp = pCollatorWrapper->compareString( aOne->GetLowerTitle(), aTwo->GetLowerTitle() );

                if ( nComp == 0 )
                    nComp = pCollatorWrapper->compareString( aOne->GetTitle(), aTwo->GetTitle() );

                if ( nComp < 0 )
                    bRet = true;
                else if ( nComp > 0 )
                    bRet = false;
                else
                    bEqual = true;
                break;
            case COLUMN_TYPE:
                nComp = pCollatorWrapper->compareString( aOne->maType, aTwo->maType );
                if ( nComp < 0 )
                    bRet = true;
                else if ( nComp > 0 )
                    bRet = false;
                else
                    bEqual = true;
                break;
            case COLUMN_SIZE:
                if ( aOne->maSize < aTwo->maSize )
                    bRet = true;
                else if ( aOne->maSize > aTwo->maSize )
                    bRet = false;
                else
                    bEqual = true;
                break;
            case COLUMN_DATE:
                if ( aOne->maModDate < aTwo->maModDate )
                    bRet = true;
                else if ( aOne->maModDate > aTwo->maModDate )
                    bRet = false;
                else
                    bEqual = true;
                break;
            default:
                SAL_INFO( "svtools", "CompareSortingData_Impl: Compare unknown type!" );
                bRet = false;
        }
    }

    // when the two elements are equal, we must not return sal_True (which would
    // happen if we just return ! ( a < b ) when not sorting ascending )
    if ( bEqual )
        return false;

    return gbAscending ? bRet : !bRet;
}


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

        pCollatorWrapper = nullptr;
    }
}


void SvtFileView_Impl::EntryRemoved( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); ++aIt )
    {
        if ( (*aIt)->maTargetURL == rURL )
        {
            maContent.erase( aIt );
            break;
        }
    }
}


void SvtFileView_Impl::EntryRenamed( OUString& rURL,
                                     const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); ++aIt )
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


OUString SvtFileView_Impl::FolderInserted( const OUString& rURL, const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    SortingData_Impl* pData = new SortingData_Impl;

    pData->SetNewTitle( rTitle );
    pData->maSize        = 0;
    pData->mbIsFolder    = true;
    pData->maTargetURL   = rURL;

    ::svtools::VolumeInfo aVolInfo;
    pData->maType = SvFileInformationManager::GetFolderDescription( aVolInfo );
    pData->maImage = SvFileInformationManager::GetFolderImage( aVolInfo );

    OUString aValue;
    OUString aTab( "\t" );
    OUString aDateSep( ", " );

    // title, type, size, date
    aValue = pData->GetTitle();
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

    return aValue;
}


sal_uLong SvtFileView_Impl::GetEntryPos( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;
    sal_uLong   nPos = 0;

    for ( aIt = maContent.begin(); aIt != maContent.end(); ++aIt )
    {
        if ( (*aIt)->maTargetURL == rURL )
            return nPos;
        nPos += 1;
    }

    return nPos;
}


void SvtFileView_Impl::SetViewMode( FileViewMode eMode )
{
    m_eViewMode = eMode;
    switch ( eMode )
    {
        case eDetailedList:
            mpCurView = mpView;
            mpView->Show();
            mpView->GetHeaderBar()->Show();
            mpIconView->Hide();
            break;

        case eIcon:
            mpCurView = mpIconView;
            mpView->Hide();
            mpView->GetHeaderBar()->Hide();
            mpIconView->Show();
            break;

        default:
            mpCurView = mpView;
            mpView->Show();
            mpView->GetHeaderBar()->Show();
            mpIconView->Hide();
    };
}


bool SvtFileView_Impl::SearchNextEntry( sal_uInt32& nIndex, const OUString& rTitle, bool bWrapAround )
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_uInt32 nEnd = maContent.size();
    sal_uInt32 nStart = nIndex;
    while ( nIndex < nEnd )
    {
        SortingData_Impl* pData = maContent[ nIndex ];
        if ( pData->GetLowerTitle().startsWith( rTitle ) )
            return true;
        nIndex += 1;
    }

    if ( bWrapAround )
    {
        nIndex = 0;
        while ( nIndex < nEnd && nIndex <= nStart )
        {
            SortingData_Impl* pData = maContent[ nIndex ];
            if ( pData->GetLowerTitle().startsWith( rTitle ) )
                return true;
            nIndex += 1;
        }
    }

    return false;
}


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

QueryDeleteDlg_Impl::QueryDeleteDlg_Impl(vcl::Window* pParent, const OUString& rName)
    : MessageDialog(pParent, "QueryDeleteDialog", "svt/ui/querydeletedialog.ui")
{
    get(m_pAllButton, "all");

    // display specified texts
    set_secondary_text(get_secondary_text().replaceFirst("%s", rName));
}

QueryDeleteDlg_Impl::~QueryDeleteDlg_Impl()
{
    disposeOnce();
}

void QueryDeleteDlg_Impl::dispose()
{
    m_pAllButton.clear();
    MessageDialog::dispose();
}

}

namespace {
    void SAL_CALL CallbackTimer::onShot()
    {
        OSL_ENSURE( m_pTimeoutHandler, "CallbackTimer::onShot: nobody interested in?" );
        SvtFileView_Impl* pHandler( m_pTimeoutHandler );
        if ( pHandler )
            pHandler->onTimeout();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
