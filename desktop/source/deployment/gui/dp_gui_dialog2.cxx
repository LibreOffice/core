/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui_dialog2.cxx,v $
 *
 * $Revision: 1.8.4.8 $
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
#include "precompiled_desktop.hxx"

#include "dp_gui.hrc"
#include "svtools/controldims.hrc"
#include "svtools/svtools.hrc"

#include "dp_gui_dialog2.hxx"
#include "dp_gui_shared.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_misc.h"

#include "vcl/ctrl.hxx"
#include "vcl/menu.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/svapp.hxx"

#include "vos/mutex.hxx"

#include "svtools/extensionlistbox.hxx"

#include "sfx2/sfxdlg.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include "comphelper/processfactory.hxx"
#include "ucbhelper/content.hxx"
#include "unotools/collatorwrapper.hxx"

#include "com/sun/star/beans/StringPair.hpp"

#include "com/sun/star/i18n/CollatorOptions.hpp"

#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/system/XSystemShellExecute.hpp"

#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

using namespace ::com::sun::star;
using namespace ::com::sun::star::system;

using ::rtl::OUString;


namespace dp_gui {

#define ICON_OFFSET         50
#define ICON_HEIGHT         42
#define SMALL_ICON_SIZE     16
#define RIGHT_ICON_OFFSET    5
#define TOP_OFFSET           3
#define SPACE_BETWEEN        3
#define LINE_SIZE            4
#define PROGRESS_WIDTH      60
#define PROGRESS_HEIGHT     14

//------------------------------------------------------------------------------
struct StrAllFiles : public rtl::StaticWithInit< const OUString, StrAllFiles >
{
    const OUString operator () () {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        ::std::auto_ptr< ResMgr > const resmgr( ResMgr::CreateResMgr( "fps_office" ) );
        OSL_ASSERT( resmgr.get() != 0 );
        String ret( ResId( STR_FILTERNAME_ALL, *resmgr.get() ) );
        return ret;
    }
};

//------------------------------------------------------------------------------
//                          struct Entry_Impl
//------------------------------------------------------------------------------
struct Entry_Impl;

typedef ::boost::shared_ptr< Entry_Impl > TEntry_Impl;

struct Entry_Impl
{
    bool            m_bActive;
    bool            m_bLocked;
    bool            m_bHasOptions;
    bool            m_bShared;
    bool            m_bNew;
    bool            m_bChecked;
    PackageState    m_eState;
    String          m_sTitle;
    String          m_sVersion;
    String          m_sDescription;
    String          m_sPublisher;
    String          m_sPublisherURL;
    String          m_sErrorText;
    Image           m_aIcon;
    Image           m_aIconHC;
    svt::FixedHyperlink *m_pPublisher;

    uno::Reference< deployment::XPackage> m_xPackage;
    uno::Reference< deployment::XPackageManager> m_xPackageManager;

    Entry_Impl( const uno::Reference< deployment::XPackage > &xPackage,
                const uno::Reference< deployment::XPackageManager > &xPackageManager,
                PackageState eState );
   ~Entry_Impl();

    StringCompare CompareTo( const CollatorWrapper *pCollator, const TEntry_Impl pEntry ) const;
};

//------------------------------------------------------------------------------
Entry_Impl::Entry_Impl( const uno::Reference< deployment::XPackage > &xPackage,
                        const uno::Reference< deployment::XPackageManager > &xPackageManager,
                        PackageState eState ) :
    m_bActive( false ),
    m_bLocked( false ),
    m_bHasOptions( false ),
    m_bShared( false ),
    m_bNew( false ),
    m_bChecked( false ),
    m_eState( eState ),
    m_pPublisher( NULL ),
    m_xPackage( xPackage ),
    m_xPackageManager( xPackageManager )
{
    m_sTitle = xPackage->getDisplayName();
    m_sVersion = xPackage->getVersion();
    m_sDescription = xPackage->getDescription();

    beans::StringPair aInfo( m_xPackage->getPublisherInfo() );
    m_sPublisher = aInfo.First;
    m_sPublisherURL = aInfo.Second;

    // get the icons for the package if there are any
    uno::Reference< graphic::XGraphic > xGraphic = xPackage->getIcon( false );
    if ( xGraphic.is() )
        m_aIcon = Image( xGraphic );

    xGraphic = xPackage->getIcon( true );
    if ( xGraphic.is() )
        m_aIconHC = Image( xGraphic );
    else
        m_aIconHC = m_aIcon;

    m_bLocked = m_xPackageManager->isReadOnly();

    if ( eState == AMBIGUOUS )
        m_sErrorText = ExtMgrDialog::getResourceString( RID_STR_ERROR_UNKNOWN_STATUS );
}

//------------------------------------------------------------------------------
Entry_Impl::~Entry_Impl()
{
    delete m_pPublisher;
}

//------------------------------------------------------------------------------
StringCompare Entry_Impl::CompareTo( const CollatorWrapper *pCollator, const TEntry_Impl pEntry ) const
{
    StringCompare eCompare = (StringCompare) pCollator->compareString( m_sTitle, pEntry->m_sTitle );
    if ( eCompare == COMPARE_EQUAL )
    {
        eCompare = m_sVersion.CompareTo( pEntry->m_sVersion );
        if ( eCompare == COMPARE_EQUAL )
        {
            if ( m_xPackageManager != pEntry->m_xPackageManager )
            {
                sal_Int32 nCompare = m_xPackageManager->getContext().compareTo( pEntry->m_xPackageManager->getContext() );
                if ( nCompare < 0 )
                    eCompare = COMPARE_LESS;
                else if ( nCompare > 0 )
                    eCompare = COMPARE_GREATER;
            }
        }
    }
    return eCompare;
}

//------------------------------------------------------------------------------
//                            ExtensionBox_Impl
//------------------------------------------------------------------------------

enum MENU_COMMAND
{
    CMD_NONE    = 0,
    CMD_REMOVE  = 1,
    CMD_ENABLE,
    CMD_DISABLE,
    CMD_UPDATE
};

class ExtensionBox_Impl : public ::svt::IExtensionListBox
{
    bool            m_bHasScrollBar;
    bool            m_bHasActive;
    bool            m_bNeedsRecalc;
    bool            m_bHasNew;
    bool            m_bInCheckMode;
    bool            m_bInterfaceLocked;
    //Must be guarded together with m_vEntries to ensure a valid index at all times.
    //Use m_entriesMutex as guard.
    long            m_nActive;
    long            m_nTopIndex;
    long            m_nStdHeight;
    long            m_nActiveHeight;
    long            m_nBtnHeight;
    Size            m_aOutputSize;
    Image           m_aLockedImage;
    Image           m_aLockedImageHC;
    Image           m_aWarningImage;
    Image           m_aWarningImageHC;
    Image           m_aDefaultImage;
    Image           m_aDefaultImageHC;

    PushButton     *m_pOptionsBtn;
    PushButton     *m_pEnableBtn;
    PushButton     *m_pRemoveBtn;
    ScrollBar      *m_pScrollBar;

    ExtMgrDialog             *m_pParent;
    TheExtensionManager      *m_pManager;
    //This mutex is used for synchronizing access to m_vEntries.
    //Currently it is used to synchronize adding, removing entries and
    //functions like getItemName, getItemDescription, etc. to prevent
    //that m_vEntries is accessed at an invalid index.
    //ToDo: There are many more places where m_vEntries is read and which may
    //fail. For example the Paint method is probable called from the main thread
    //while new entries are added / removed in a separate thread.
    mutable ::osl::Mutex    m_entriesMutex;
    std::vector< TEntry_Impl > m_vEntries;

    lang::Locale    *m_pLocale;
    CollatorWrapper *m_pCollator;

    void            CalcActiveHeight( const long nPos );
    long            GetTotalHeight() const;
    Rectangle       GetEntryRect( const long nPos ) const;
    void            SetButtonPos( const Rectangle& rRect );
    void            SetButtonStatus( const TEntry_Impl pEntry );
    void            SetupScrollBar();
    void            DrawRow( const Rectangle& rRect, const TEntry_Impl pEntry );
    bool            HandleTabKey( bool bReverse );
    bool            HandleCursorKey( USHORT nKeyCode );
    MENU_COMMAND    ShowPopupMenu( const Point &rPos, const long nPos );
    void            RecalcAll();
    bool            FindEntryPos( const TEntry_Impl pEntry, long nStart, long nEnd, long &nFound );
    bool            isHCMode();

    //-----------------
    DECL_DLLPRIVATE_LINK( ScrollHdl, ScrollBar * );

    DECL_DLLPRIVATE_LINK( HandleOptionsBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleEnableBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleRemoveBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleHyperlink, svt::FixedHyperlink * );

    //Index starts with 1.
    //Throws an com::sun::star::lang::IllegalArgumentException, when the index is invalid.
    void checkIndex(sal_Int32 pos) const;


public:
                    ExtensionBox_Impl( ExtMgrDialog* pParent, TheExtensionManager *pManager );
                   ~ExtensionBox_Impl();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle &rPaintRect );
    virtual void    Resize();
    virtual long    Notify( NotifyEvent& rNEvt );

    const Size      GetMinOutputSizePixel() const;

    //-----------------
    long            addEntry( const uno::Reference< deployment::XPackage > &xPackage,
                              const uno::Reference< deployment::XPackageManager > &xPackageManager );
    void            selectEntry( const long nPos );
    void            enableButtons( bool bEnable );

    void            updateEntry( const uno::Reference< deployment::XPackage > &xPackage );
    void            removeEntry( const uno::Reference< deployment::XPackage > &xPackage );

    void            prepareChecking( const uno::Reference< deployment::XPackageManager > &xPackageMgr );
    void            checkEntries();

    TheExtensionManager*    getExtensionManager() const { return m_pManager; }

    //===================================================================================
    //These functions are used for automatic testing

    /** @return  The count of the entries in the list box. */
    virtual sal_Int32 getItemCount() const;

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns EXTENSION_LISTBOX_ENTRY_NOTFOUND */
    virtual sal_Int32 getSelIndex() const;

    /** @return  The item name of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemName( sal_Int32 index ) const;

    /** @return  The version string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemVersion( sal_Int32 index ) const;

    /** @return  The description string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemDescription( sal_Int32 index ) const;

    /** @return  The publisher string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemPublisher( sal_Int32 index ) const;

    /** @return  The link behind the publisher text of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemPublisherLink( sal_Int32 index ) const;

    /** The entry at the given position will be selected
        Index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual void select( sal_Int32 pos );

    /** The first found entry with the given name will be selected
        When there was no entry found with the name, the selection doesn't change.
        Please note that there might be more than one entry with the same
        name, because:
            1. the name is not unique
            2. one extension can be installed as user and shared extension.
    */
    virtual void select( const OUString & sName );
    //===================================================================================
};

//------------------------------------------------------------------------------
ExtensionBox_Impl::ExtensionBox_Impl( ExtMgrDialog* pParent, TheExtensionManager *pManager ) :
    IExtensionListBox( pParent, WB_BORDER | WB_TABSTOP ),
    m_bHasScrollBar( false ),
    m_bHasActive( false ),
    m_bNeedsRecalc( true ),
    m_bHasNew( false ),
    m_bInCheckMode( false ),
    m_bInterfaceLocked( false ),
    m_nActive( 0 ),
    m_nTopIndex( 0 ),
    m_nActiveHeight( 0 ),
    m_aLockedImage( ExtMgrDialog::getResId( RID_IMG_LOCKED ) ),
    m_aLockedImageHC( ExtMgrDialog::getResId( RID_IMG_LOCKED_HC ) ),
    m_aWarningImage( ExtMgrDialog::getResId( RID_IMG_WARNING ) ),
    m_aWarningImageHC( ExtMgrDialog::getResId( RID_IMG_WARNING_HC ) ),
    m_aDefaultImage( ExtMgrDialog::getResId( RID_IMG_EXTENSION ) ),
    m_aDefaultImageHC( ExtMgrDialog::getResId( RID_IMG_EXTENSION_HC ) ),
    m_pOptionsBtn( NULL ),
    m_pEnableBtn( NULL ),
    m_pRemoveBtn( NULL ),
    m_pScrollBar( NULL ),
    m_pParent( pParent ),
    m_pManager( pManager )
{
    m_pOptionsBtn = new PushButton( this, WB_TABSTOP );
    m_pEnableBtn = new PushButton( this, WB_TABSTOP );
    m_pRemoveBtn = new PushButton( this, WB_TABSTOP );

    SetHelpId( HID_EXTENSION_MANAGER_LISTBOX );
    m_pOptionsBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_OPTIONS );
    m_pEnableBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_DISABLE );
    m_pRemoveBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_REMOVE );

    m_pOptionsBtn->SetClickHdl( LINK( this, ExtensionBox_Impl, HandleOptionsBtn ) );
    m_pEnableBtn->SetClickHdl( LINK( this, ExtensionBox_Impl, HandleEnableBtn ) );
    m_pRemoveBtn->SetClickHdl( LINK( this, ExtensionBox_Impl, HandleRemoveBtn ) );

    m_pScrollBar = new ScrollBar( this, WB_VERT );
    m_pScrollBar->SetScrollHdl( LINK( this, ExtensionBox_Impl, ScrollHdl ) );
    m_pScrollBar->EnableDrag();

    m_pOptionsBtn->SetText( ExtMgrDialog::getResourceString( RID_CTX_ITEM_OPTIONS ) );
    m_pEnableBtn->SetText( ExtMgrDialog::getResourceString( RID_CTX_ITEM_DISABLE ) );
    m_pRemoveBtn->SetText( ExtMgrDialog::getResourceString( RID_CTX_ITEM_REMOVE ) );

    Size aSize = LogicToPixel( Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ),
                               MapMode( MAP_APPFONT ) );
    m_pOptionsBtn->SetSizePixel( aSize );
    m_pEnableBtn->SetSizePixel( aSize );
    m_pRemoveBtn->SetSizePixel( aSize );

    SetPaintTransparent( true );
    SetPosPixel( Point( RSC_SP_DLG_INNERBORDER_LEFT, RSC_SP_DLG_INNERBORDER_TOP ) );
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        m_nStdHeight = nTitleHeight;
    else
        m_nStdHeight = nIconHeight;
    m_nStdHeight += GetTextHeight() + TOP_OFFSET;

    nIconHeight = ICON_HEIGHT + 2*TOP_OFFSET + 1;
    if ( m_nStdHeight < nIconHeight )
        m_nStdHeight = nIconHeight;

    m_nActiveHeight = m_nStdHeight;
    m_nBtnHeight = aSize.Height() + 2 * TOP_OFFSET;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( rStyleSettings.GetFieldColor() );

    m_pLocale = new lang::Locale( Application::GetSettings().GetLocale() );
    m_pCollator = new CollatorWrapper( ::comphelper::getProcessServiceFactory() );
    m_pCollator->loadDefaultCollator( *m_pLocale, i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );

    Show();
}

//------------------------------------------------------------------------------
ExtensionBox_Impl::~ExtensionBox_Impl()
{
    delete m_pOptionsBtn;
    delete m_pEnableBtn;
    delete m_pRemoveBtn;
    delete m_pScrollBar;

    delete m_pLocale;
    delete m_pCollator;
}

//------------------------------------------------------------------------------
sal_Int32 ExtensionBox_Impl::getItemCount() const
{
    return static_cast< sal_Int32 >( m_vEntries.size() );
}

//------------------------------------------------------------------------------
sal_Int32 ExtensionBox_Impl::getSelIndex() const
{
    if ( m_bHasActive )
    {
        OSL_ASSERT( m_nActive >= -1);
        return static_cast< sal_Int32 >( m_nActive );
    }
    else
        return static_cast< sal_Int32 >( EXTENSION_LISTBOX_ENTRY_NOTFOUND );
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::checkIndex( sal_Int32 nIndex ) const
{
    if ( nIndex < 0 )
        throw lang::IllegalArgumentException( OUSTR("The list index starts with 0"),0, 0 );
    if ( static_cast< sal_uInt32 >( nIndex ) >= m_vEntries.size())
        throw lang::IllegalArgumentException( OUSTR("There is no element at the provided position."
        "The position exceeds the number of available list entries"),0, 0 );
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemName( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sTitle;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemVersion( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sVersion;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemDescription( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sDescription;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemPublisher( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sPublisher;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemPublisherLink( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sPublisherURL;
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::select( sal_Int32 nIndex )
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    selectEntry( nIndex );
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::select( const OUString & sName )
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    typedef ::std::vector< TEntry_Impl >::const_iterator It;

    for ( It iIter = m_vEntries.begin(); iIter < m_vEntries.end(); iIter++ )
    {
        if ( sName.equals( (*iIter)->m_sTitle ) )
        {
            long nPos = iIter - m_vEntries.begin();
            selectEntry( nPos );
            break;
        }
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Title + description + buttons height
void ExtensionBox_Impl::CalcActiveHeight( const long nPos )
{
    // get title height
    long aTextHeight;
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // calc description height
    Size aSize = GetOutputSizePixel();
    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    aSize.Width() -= ICON_OFFSET;
    aSize.Height() = 10000;

    Rectangle aRect = GetTextRect( Rectangle( Point(), aSize ),
                                   m_vEntries[ nPos ]->m_sDescription,
                                   TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aTextHeight += aRect.GetHeight();

    if ( aTextHeight < m_nStdHeight )
        aTextHeight = m_nStdHeight;

    m_nActiveHeight = aTextHeight + m_nBtnHeight; // button bar height
}

//------------------------------------------------------------------------------
const Size ExtensionBox_Impl::GetMinOutputSizePixel() const
{
    long nHeight = m_nStdHeight;
    nHeight += m_pOptionsBtn->GetSizePixel().Height();
    nHeight +=  2 * TOP_OFFSET;
    long nWidth = m_pOptionsBtn->GetSizePixel().Width();
    nWidth *= 3;
    nWidth += 5*TOP_OFFSET;
    nWidth += m_pScrollBar->GetSizePixel().Width();

    return Size( nWidth, nHeight );
}

//------------------------------------------------------------------------------
Rectangle ExtensionBox_Impl::GetEntryRect( const long nPos ) const
{
    Size aSize( GetOutputSizePixel() );

    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    if ( m_vEntries[ nPos ]->m_bActive )
        aSize.Height() = m_nActiveHeight;
    else
        aSize.Height() = m_nStdHeight;

    Point aPos( 0, -m_nTopIndex + nPos * m_nStdHeight );
    if ( m_bHasActive && ( nPos < m_nActive ) )
        aPos.Y() += m_nActiveHeight - m_nStdHeight;

    return Rectangle( aPos, aSize );
}

//------------------------------------------------------------------------------
//This function may be called with nPos < 0
void ExtensionBox_Impl::selectEntry( long nPos )
{
    //ToDo whe should not use the guard at such a big scope here.
    //Currently it is used to gard m_vEntries and m_nActive. m_nActive will be
    //modified in this function.
    //It would be probably best to always use a copy of m_vEntries
    //and some other state variables from ExtensionBox_Impl for
    //the whole painting operation. See issue i86993
    ::osl::ClearableMutexGuard guard(m_entriesMutex);

    if ( m_bInCheckMode )
        return;

    if ( m_bHasActive )
    {
        if ( nPos == m_nActive )
            return;

        m_bHasActive = false;
        m_vEntries[ m_nActive ]->m_bActive = false;

        if ( IsReallyVisible() )
            Invalidate();
    }

    if ( ( nPos >= 0 ) && ( nPos < (long) m_vEntries.size() ) )
    {
        m_bHasActive = true;
        m_nActive = nPos;
        m_vEntries[ nPos ]->m_bActive = true;

        if ( IsReallyVisible() )
        {
            Invalidate();
            CalcActiveHeight( nPos );
            SetupScrollBar();
            Rectangle aEntryRect = GetEntryRect( nPos );

            // If the top of the selected entry isn't visible, make it visible
            if ( aEntryRect.Top() < 0 )
            {
                m_nTopIndex += aEntryRect.Top();
                aEntryRect.Move( 0, -aEntryRect.Top() );
            }

            // If the bottom of the selected entry isn't visible, make it visible even if now the top
            // isn't visible any longer ( the buttons are more important )
            Size aOutputSize = GetOutputSizePixel();
            if ( aEntryRect.Bottom() > aOutputSize.Height() )
            {
                m_nTopIndex += ( aEntryRect.Bottom() - aOutputSize.Height() );
                aEntryRect.Move( 0, -( aEntryRect.Bottom() - aOutputSize.Height() ) );
            }

            // If there is unused space below the last entry but all entries don't fit into the box,
            // move the content down to use the whole space
            const long nTotalHeight = GetTotalHeight();
            if ( m_bHasScrollBar && ( aOutputSize.Height() + m_nTopIndex > nTotalHeight ) )
            {
                long nOffset = m_nTopIndex;
                m_nTopIndex = nTotalHeight - aOutputSize.Height();
                nOffset -= m_nTopIndex;
                aEntryRect.Move( 0, nOffset );
            }

            if ( m_bHasScrollBar )
                m_pScrollBar->SetThumbPos( m_nTopIndex );

            SetButtonPos( aEntryRect );
        }
        SetButtonStatus( m_vEntries[ nPos ] ); //dv
    }
    else
    {
        m_pOptionsBtn->Hide();
        m_pEnableBtn->Hide();
        m_pRemoveBtn->Hide();
    }
    guard.clear();

}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::SetButtonPos( const Rectangle& rRect )
{
    Size  aBtnSize( m_pOptionsBtn->GetSizePixel() );
    Point aBtnPos( rRect.Left() + ICON_OFFSET,
                   rRect.Bottom() - TOP_OFFSET - aBtnSize.Height() );

    m_pOptionsBtn->SetPosPixel( aBtnPos );
    aBtnPos.X() = rRect.Right() - TOP_OFFSET - aBtnSize.Width();
    m_pRemoveBtn->SetPosPixel( aBtnPos );
    aBtnPos.X() -= ( TOP_OFFSET + aBtnSize.Width() );
    m_pEnableBtn->SetPosPixel( aBtnPos );
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::SetButtonStatus( const TEntry_Impl pEntry )
{
    if ( ( pEntry->m_eState == REGISTERED ) || ( pEntry->m_eState == NOT_AVAILABLE ) )
    {
        m_pEnableBtn->SetText( ExtMgrDialog::getResourceString( RID_CTX_ITEM_DISABLE ) );
        m_pEnableBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_DISABLE );
    }
    else
    {
        m_pEnableBtn->SetText( ExtMgrDialog::getResourceString( RID_CTX_ITEM_ENABLE ) );
        m_pEnableBtn->SetHelpId( HID_EXTENSION_MANAGER_LISTBOX_ENABLE );
    }

    if ( pEntry->m_eState == NOT_AVAILABLE )
        m_pEnableBtn->Hide();
    else
    {
        m_pEnableBtn->Enable( !pEntry->m_bLocked );
        m_pEnableBtn->Show();
    }

    if ( pEntry->m_bHasOptions )
    {
        m_pOptionsBtn->Enable( pEntry->m_bHasOptions );
        m_pOptionsBtn->Show();
    }
    else
        m_pOptionsBtn->Hide();

    m_pRemoveBtn->Show();
    m_pRemoveBtn->Enable( !pEntry->m_bLocked );
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::DrawRow( const Rectangle& rRect, const TEntry_Impl pEntry )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( pEntry->m_bActive )
        SetTextColor( rStyleSettings.GetHighlightTextColor() );
    else if ( ( pEntry->m_eState != REGISTERED ) && ( pEntry->m_eState != NOT_AVAILABLE ) )
        SetTextColor( rStyleSettings.GetDisableColor() );
    else if ( IsControlForeground() )
        SetTextColor( GetControlForeground() );
    else
        SetTextColor( rStyleSettings.GetFieldTextColor() );

    if ( pEntry->m_bActive )
    {
        SetLineColor();
        SetFillColor( rStyleSettings.GetHighlightColor() );
        DrawRect( rRect );
    }
    else
    {
        if( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetFieldColor() );

        SetTextFillColor();
        Erase( rRect );
    }

    // Draw extension icon
    Point aPos( rRect.TopLeft() );
    aPos += Point( TOP_OFFSET, TOP_OFFSET );
    Image aImage;
    if ( ! pEntry->m_aIcon )
        aImage = isHCMode() ? m_aDefaultImageHC : m_aDefaultImage;
    else
        aImage = isHCMode() ? pEntry->m_aIconHC : pEntry->m_aIcon;
    Size aImageSize = aImage.GetSizePixel();
    if ( ( aImageSize.Width() <= ICON_HEIGHT ) && ( aImageSize.Height() <= ICON_HEIGHT ) )
        DrawImage( Point( aPos.X()+((ICON_HEIGHT-aImageSize.Width())/2), aPos.Y()+((ICON_HEIGHT-aImageSize.Height())/2) ), aImage );
    else
        DrawImage( aPos, Size( ICON_HEIGHT, ICON_HEIGHT ), aImage );

    // Setup fonts
    Font aStdFont( GetFont() );
    Font aBoldFont( aStdFont );
    aBoldFont.SetWeight( WEIGHT_BOLD );
    SetFont( aBoldFont );
    long aTextHeight = GetTextHeight();

    // Init publisher link here
    if ( !pEntry->m_pPublisher && pEntry->m_sPublisher.Len() )
    {
        pEntry->m_pPublisher = new svt::FixedHyperlink( this );
        pEntry->m_pPublisher->SetBackground();
        pEntry->m_pPublisher->SetPaintTransparent( true );
        pEntry->m_pPublisher->SetURL( pEntry->m_sPublisherURL );
        pEntry->m_pPublisher->SetDescription( pEntry->m_sPublisher );
        Size aSize = FixedText::CalcMinimumTextSize( pEntry->m_pPublisher );
        pEntry->m_pPublisher->SetSizePixel( aSize );
        pEntry->m_pPublisher->SetClickHdl( LINK( this, ExtensionBox_Impl, HandleHyperlink ) );
    }

    // Get max title width
    long nMaxTitleWidth = rRect.GetWidth() - ICON_OFFSET;
    nMaxTitleWidth -= ( 2 * SMALL_ICON_SIZE ) + ( 4 * SPACE_BETWEEN );
    if ( pEntry->m_pPublisher )
    {
        nMaxTitleWidth -= pEntry->m_pPublisher->GetSizePixel().Width() + (2*SPACE_BETWEEN);
    }

    long aVersionWidth = GetTextWidth( pEntry->m_sVersion );
    long aTitleWidth = GetTextWidth( pEntry->m_sTitle ) + (aTextHeight / 3);

    aPos = rRect.TopLeft() + Point( ICON_OFFSET, TOP_OFFSET );

    if ( aTitleWidth > nMaxTitleWidth - aVersionWidth )
    {
        aTitleWidth = nMaxTitleWidth - aVersionWidth - (aTextHeight / 3);
        String aShortTitle = GetEllipsisString( pEntry->m_sTitle, aTitleWidth );
        DrawText( aPos, aShortTitle );
        aTitleWidth += (aTextHeight / 3);
    }
    else
        DrawText( aPos, pEntry->m_sTitle );

    SetFont( aStdFont );
    DrawText( Point( aPos.X() + aTitleWidth, aPos.Y() ), pEntry->m_sVersion );

    long nIconHeight = TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // draw description
    String sDescription;
    if ( pEntry->m_sErrorText.Len() )
    {
        if ( pEntry->m_bActive )
            sDescription = pEntry->m_sErrorText + OUSTR("\n") + pEntry->m_sDescription;
        else
            sDescription = pEntry->m_sErrorText;
    }
    else
        sDescription = pEntry->m_sDescription;

    aPos.Y() += aTextHeight;
    if ( pEntry->m_bActive )
    {
        DrawText( Rectangle( aPos.X(), aPos.Y(), rRect.Right(), rRect.Bottom() - m_nBtnHeight ),
                  sDescription, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    }
    else
    {
        const long nWidth = GetTextWidth( sDescription );
        if ( nWidth > rRect.GetWidth() - aPos.X() )
            sDescription = GetEllipsisString( sDescription, rRect.GetWidth() - aPos.X() );
        DrawText( aPos, sDescription );
    }

    // Draw publisher link
    if ( pEntry->m_pPublisher )
    {
        pEntry->m_pPublisher->Show();
        aPos = rRect.TopLeft() + Point( ICON_OFFSET + nMaxTitleWidth + (2*SPACE_BETWEEN), TOP_OFFSET );
        pEntry->m_pPublisher->SetPosPixel( aPos );
    }

    // Draw status icons
    if ( pEntry->m_bShared )
    {
        aPos = rRect.TopRight() + Point( -(RIGHT_ICON_OFFSET + SMALL_ICON_SIZE), TOP_OFFSET );
        DrawImage( aPos, Size( SMALL_ICON_SIZE, SMALL_ICON_SIZE ), isHCMode() ? m_aLockedImageHC : m_aLockedImage );
    }
    if ( pEntry->m_eState == AMBIGUOUS )
    {
        aPos = rRect.TopRight() + Point( -(RIGHT_ICON_OFFSET + SPACE_BETWEEN + 2*SMALL_ICON_SIZE), TOP_OFFSET );
        DrawImage( aPos, Size( SMALL_ICON_SIZE, SMALL_ICON_SIZE ), isHCMode() ? m_aWarningImageHC : m_aWarningImage );
    }

    SetLineColor( Color( COL_LIGHTGRAY ) );
    DrawLine( rRect.BottomLeft(), rRect.BottomRight() );
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::RecalcAll()
{
    if ( m_bHasActive )
        CalcActiveHeight( m_nActive );

    SetupScrollBar();

    if ( m_bHasActive )
        SetButtonPos( GetEntryRect( m_nActive ) );

    m_bNeedsRecalc = false;
}

// -----------------------------------------------------------------------
bool ExtensionBox_Impl::HandleTabKey( bool bReverse )
{
    if ( ! m_bHasActive )
        return false;

    PushButton *pNext = NULL;

    if ( m_pOptionsBtn->HasFocus() ) {
        if ( !bReverse && !m_vEntries[ m_nActive ]->m_bLocked )
            pNext = m_pEnableBtn;
    }
    else if ( m_pEnableBtn->HasFocus() ) {
        if ( !bReverse )
            pNext = m_pRemoveBtn;
        else if ( m_vEntries[ m_nActive ]->m_bHasOptions )
            pNext = m_pOptionsBtn;
    }
    else if ( m_pRemoveBtn->HasFocus() ) {
        if ( bReverse )
            pNext = m_pEnableBtn;
    }
    else {
        if ( !bReverse ) {
            if ( m_vEntries[ m_nActive ]->m_bHasOptions )
                pNext = m_pOptionsBtn;
            else if ( ! m_vEntries[ m_nActive ]->m_bLocked )
                pNext = m_pEnableBtn;
        } else {
            if ( ! m_vEntries[ m_nActive ]->m_bLocked )
                pNext = m_pRemoveBtn;
            else if ( m_vEntries[ m_nActive ]->m_bHasOptions )
                pNext = m_pOptionsBtn;
        }
    }

    if ( pNext )
    {
        pNext->GrabFocus();
        return true;
    }
    else
        return false;
}

// -----------------------------------------------------------------------
bool ExtensionBox_Impl::HandleCursorKey( USHORT nKeyCode )
{
    if ( m_vEntries.empty() )
        return true;

    long nSelect = 0;

    if ( m_bHasActive )
    {
        long nPageSize = GetOutputSizePixel().Height() / m_nStdHeight;
        if ( nPageSize < 2 )
            nPageSize = 2;

        if ( ( nKeyCode == KEY_DOWN ) || ( nKeyCode == KEY_RIGHT ) )
            nSelect = m_nActive + 1;
        else if ( ( nKeyCode == KEY_UP ) || ( nKeyCode == KEY_LEFT ) )
            nSelect = m_nActive - 1;
        else if ( nKeyCode == KEY_HOME )
            nSelect = 0;
        else if ( nKeyCode == KEY_END )
            nSelect = m_vEntries.size() - 1;
        else if ( nKeyCode == KEY_PAGEUP )
            nSelect = m_nActive - nPageSize + 1;
        else if ( nKeyCode == KEY_PAGEDOWN )
            nSelect = m_nActive + nPageSize - 1;
    }
    else // when there is no selected entry, we will select the first or the last.
    {
        if ( ( nKeyCode == KEY_DOWN ) || ( nKeyCode == KEY_PAGEDOWN ) || ( nKeyCode == KEY_HOME ) )
            nSelect = 0;
        else if ( ( nKeyCode == KEY_UP ) || ( nKeyCode == KEY_PAGEUP ) || ( nKeyCode == KEY_END ) )
            nSelect = m_vEntries.size() - 1;
    }

    if ( nSelect < 0 )
        nSelect = 0;
    if ( nSelect >= (long) m_vEntries.size() )
        nSelect = m_vEntries.size() - 1;

    selectEntry( nSelect );

    return true;
}

// -----------------------------------------------------------------------
MENU_COMMAND ExtensionBox_Impl::ShowPopupMenu( const Point & rPos, const long nPos )
{
    if ( nPos >= (long) m_vEntries.size() )
        return CMD_NONE;

    PopupMenu aPopup;

    aPopup.InsertItem( CMD_UPDATE, ExtMgrDialog::getResourceString( RID_CTX_ITEM_CHECK_UPDATE ) );

    if ( ! m_vEntries[ nPos ]->m_bLocked )
    {
        if ( m_vEntries[ nPos ]->m_eState == REGISTERED )
            aPopup.InsertItem( CMD_DISABLE, ExtMgrDialog::getResourceString( RID_CTX_ITEM_DISABLE ) );
        else if ( m_vEntries[ nPos ]->m_eState != NOT_AVAILABLE )
            aPopup.InsertItem( CMD_ENABLE, ExtMgrDialog::getResourceString( RID_CTX_ITEM_ENABLE ) );

        aPopup.InsertItem( CMD_REMOVE, ExtMgrDialog::getResourceString( RID_CTX_ITEM_REMOVE ) );
    }

    return (MENU_COMMAND) aPopup.Execute( this, rPos );
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::Paint( const Rectangle &/*rPaintRect*/ )
{
    if ( m_bNeedsRecalc )
        RecalcAll();

    Point aStart( 0, -m_nTopIndex );
    Size aSize( GetOutputSizePixel() );

    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    typedef std::vector< TEntry_Impl >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        aSize.Height() = (*iIndex)->m_bActive ? m_nActiveHeight : m_nStdHeight;
        Rectangle aEntryRect( aStart, aSize );
//        if ( aEntryRect.IsOver( rPaintRect ) )
            DrawRow( aEntryRect, *iIndex );
        aStart.Y() += aSize.Height();
    }
}

// -----------------------------------------------------------------------
long ExtensionBox_Impl::GetTotalHeight() const
{
    long nHeight = m_vEntries.size() * m_nStdHeight;

    if ( m_bHasActive )
    {
        nHeight += m_nActiveHeight - m_nStdHeight;
    }

    return nHeight;
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::SetupScrollBar()
{
    const Size aSize = GetOutputSizePixel();
    const long nScrBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    const long nTotalHeight = GetTotalHeight();
    const bool bNeedsScrollBar = ( nTotalHeight > aSize.Height() );

    if ( bNeedsScrollBar )
    {
        if ( m_nTopIndex + aSize.Height() > nTotalHeight )
            m_nTopIndex = nTotalHeight - aSize.Height();

        m_pScrollBar->SetPosSizePixel( Point( aSize.Width() - nScrBarSize, 0 ),
                                       Size( nScrBarSize, aSize.Height() ) );
        m_pScrollBar->SetRangeMax( nTotalHeight );
        m_pScrollBar->SetVisibleSize( aSize.Height() );
        m_pScrollBar->SetPageSize( ( aSize.Height() * 4 ) / 5 );
        m_pScrollBar->SetLineSize( m_nStdHeight );
        m_pScrollBar->SetThumbPos( m_nTopIndex );

        if ( !m_bHasScrollBar )
            m_pScrollBar->Show();
    }
    else if ( m_bHasScrollBar )
    {
        m_pScrollBar->Hide();
        m_nTopIndex = 0;
    }

    m_bHasScrollBar = bNeedsScrollBar;
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::Resize()
{
    RecalcAll();
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( m_bInterfaceLocked )
        return;

    const Point aMousePos( rMEvt.GetPosPixel() );
    long nPos = ( aMousePos.Y() + m_nTopIndex ) / m_nStdHeight;
    if ( m_bHasActive && ( nPos > m_nActive ) )
    {
        if ( aMousePos.Y() + m_nTopIndex <= m_nActive*m_nStdHeight + m_nActiveHeight )
            nPos = m_nActive;
        else
            nPos = ( aMousePos.Y() + m_nTopIndex - (m_nActiveHeight - m_nStdHeight) ) / m_nStdHeight;
    }

    if ( rMEvt.IsRight() )
    {
        switch( ShowPopupMenu( aMousePos, nPos ) )
        {
            case CMD_NONE:      break;
            case CMD_ENABLE:    m_pParent->enablePackage( m_vEntries[ nPos ]->m_xPackageManager,
                                                          m_vEntries[ nPos ]->m_xPackage, true );
                                break;
            case CMD_DISABLE:   m_pParent->enablePackage( m_vEntries[ nPos ]->m_xPackageManager,
                                                          m_vEntries[ nPos ]->m_xPackage, false );
                                break;
            case CMD_UPDATE:    m_pParent->updatePackage( m_vEntries[ nPos ]->m_xPackageManager,
                                                          m_vEntries[ nPos ]->m_xPackage );
                                break;
            case CMD_REMOVE:    m_pParent->removePackage( m_vEntries[ nPos ]->m_xPackageManager,
                                                          m_vEntries[ nPos ]->m_xPackage );
                                break;
        }
    }
    else if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.IsMod1() && m_bHasActive )
            selectEntry( m_vEntries.size() );   // Selecting an not existing entry will deselect the current one
        else
            selectEntry( nPos );
    }
}

//------------------------------------------------------------------------------
long ExtensionBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode         aKeyCode = pKEvt->GetKeyCode();
        USHORT          nKeyCode = aKeyCode.GetCode();

        if ( nKeyCode == KEY_TAB )
            bHandled = HandleTabKey( aKeyCode.IsShift() );
        else if ( aKeyCode.GetGroup() == KEYGROUP_CURSOR )
            bHandled = HandleCursorKey( nKeyCode );
    }

    if ( rNEvt.GetType() == EVENT_COMMAND )
    {
        if ( m_bHasScrollBar &&
             ( rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL ) )
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            if ( pData->GetMode() == COMMAND_WHEEL_SCROLL )
            {
                long nThumbPos = m_pScrollBar->GetThumbPos();
                if ( pData->GetDelta() < 0 )
                    m_pScrollBar->DoScroll( nThumbPos + m_nStdHeight );
                else
                    m_pScrollBar->DoScroll( nThumbPos - m_nStdHeight );
                bHandled = true;
            }
        }
    }

    if ( !bHandled )
        return Control::Notify( rNEvt );
    else
        return true;
}

//------------------------------------------------------------------------------
bool ExtensionBox_Impl::FindEntryPos( const TEntry_Impl pEntry, const long nStart,
                                      const long nEnd, long &nPos )
{
    nPos = nStart;
    if ( nStart > nEnd )
        return false;

    StringCompare eCompare;

    if ( nStart == nEnd )
    {
        eCompare = pEntry->CompareTo( m_pCollator, m_vEntries[ nStart ] );
        if ( eCompare == COMPARE_LESS )
            return false;
        else if ( eCompare == COMPARE_EQUAL )
        {
            //Workaround. See i86963.
            if (pEntry->m_xPackage != m_vEntries[nStart]->m_xPackage)
                return false;

            if ( m_bInCheckMode )
                m_vEntries[ nStart ]->m_bChecked = true;
            return true;
        }
        else
        {
            nPos = nStart + 1;
            return false;
        }
    }

    const long nMid = nStart + ( ( nEnd - nStart ) / 2 );
    eCompare = pEntry->CompareTo( m_pCollator, m_vEntries[ nMid ] );

    if ( eCompare == COMPARE_LESS )
        return FindEntryPos( pEntry, nStart, nMid-1, nPos );
    else if ( eCompare == COMPARE_GREATER )
        return FindEntryPos( pEntry, nMid+1, nEnd, nPos );
    else
    {
        //Workaround.See i86963.
        if (pEntry->m_xPackage != m_vEntries[nMid]->m_xPackage)
            return false;

        if ( m_bInCheckMode )
            m_vEntries[ nMid ]->m_bChecked = true;
        nPos = nMid;
        return true;
    }
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::removeEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
    ::osl::ClearableMutexGuard aGuard( m_entriesMutex );
    typedef std::vector< TEntry_Impl >::iterator ITER;

    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        if ( (*iIndex)->m_xPackage == xPackage )
        {
            long nPos = iIndex - m_vEntries.begin();

            m_vEntries.erase( iIndex );

            if ( IsReallyVisible() )
                Invalidate();

            if ( m_bHasActive )
            {
                if ( nPos < m_nActive )
                    m_nActive -= 1;
                else if ( ( nPos == m_nActive ) &&
                          ( nPos == (long) m_vEntries.size() ) )
                    m_nActive -= 1;

                m_bHasActive = false;
                //clear before calling out of this method
                aGuard.clear();
                selectEntry( m_nActive );
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------
long ExtensionBox_Impl::addEntry( const uno::Reference< deployment::XPackage > &xPackage,
                                  const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    long nPos = 0;
    PackageState eState = m_pManager->getPackageState( xPackage );

    TEntry_Impl pEntry( new Entry_Impl( xPackage, xPackageManager, eState ) );

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    if ( m_vEntries.empty() )
    {
        pEntry->m_bHasOptions = m_pManager->supportsOptions( xPackage );
        pEntry->m_bShared     = ( m_pManager->getSharedPkgMgr() == xPackageManager );
        pEntry->m_bNew        = m_bInCheckMode;
        m_vEntries.push_back( pEntry );
    }
    else
    {
        if ( !FindEntryPos( pEntry, 0, m_vEntries.size()-1, nPos ) )
        {
            pEntry->m_bHasOptions = m_pManager->supportsOptions( xPackage );
            pEntry->m_bShared     = ( m_pManager->getSharedPkgMgr() == xPackageManager );
            pEntry->m_bNew        = m_bInCheckMode;
            m_vEntries.insert( m_vEntries.begin()+nPos, pEntry );
        }
        else if ( !m_bInCheckMode )
        {
            OSL_ENSURE( 0, "ExtensionBox_Impl::addEntry(): Will not add duplicate entries"  );
        }
    }
    //access to m_nActive must be guarded
    if ( !m_bInCheckMode && m_bHasActive && ( m_nActive >= nPos ) )
        m_nActive += 1;

    guard.clear();

    if ( IsReallyVisible() )
        Invalidate();

    m_bNeedsRecalc = true;

    return nPos;
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::updateEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
    typedef std::vector< TEntry_Impl >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        if ( (*iIndex)->m_xPackage == xPackage )
        {
            PackageState eState = m_pManager->getPackageState( xPackage );
            (*iIndex)->m_bHasOptions = m_pManager->supportsOptions( xPackage );
            (*iIndex)->m_eState = eState;
            (*iIndex)->m_sTitle = xPackage->getDisplayName();
            (*iIndex)->m_sVersion = xPackage->getVersion();
            (*iIndex)->m_sDescription = xPackage->getDescription();

            if ( eState == AMBIGUOUS )
                (*iIndex)->m_sErrorText = ExtMgrDialog::getResourceString( RID_STR_ERROR_UNKNOWN_STATUS );
            else
                (*iIndex)->m_sErrorText = String();

            if ( IsReallyVisible() )
                Invalidate();
            break;
        }
    }
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::prepareChecking( const uno::Reference< deployment::XPackageManager > &xPackageMgr )
{
    m_bInCheckMode = true;
    typedef std::vector< TEntry_Impl >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        if ( (*iIndex)->m_xPackageManager == xPackageMgr )
            (*iIndex)->m_bChecked = false;
        else
            (*iIndex)->m_bChecked = true;
        (*iIndex)->m_bNew = false;
    }
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::checkEntries()
{
    long nNewPos = -1;
    long nPos = 0;
    bool bNeedsUpdate = false;
    bool bReselectActive = false;

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    typedef std::vector< TEntry_Impl >::iterator ITER;
    ITER iIndex = m_vEntries.begin();
    while ( iIndex < m_vEntries.end() )
    {
        BOOL bNext = true;
        if ( (*iIndex)->m_bChecked == false )
        {
            bNeedsUpdate = true;
            nPos = iIndex-m_vEntries.begin();
            if ( (*iIndex)->m_bNew )
            {
                if ( nNewPos == - 1)
                    nNewPos = nPos;
                if ( nPos <= m_nActive )
                    m_nActive += 1;
            }
            else
            {
                //We reach this point when we updated an extension.
                iIndex = m_vEntries.erase( iIndex );
                bNext = false;

                if ( m_bHasActive )
                {
                    if ( nPos < m_nActive )
                        m_nActive -= 1;
                    else if ( nPos == m_nActive )
                    {
                        m_bHasActive = false;
                        bReselectActive = true;
                    }
                }
            }
        }
        if ( bNext )
            iIndex++;
    }
    guard.clear();

    m_bInCheckMode = false;

    if ( nNewPos != - 1)
        selectEntry( nNewPos );
    else if ( bReselectActive )
    {
        {
            ::osl::MutexGuard guard2(m_entriesMutex);
            if ( m_nActive >= (long) m_vEntries.size() )
                m_nActive = (long) m_vEntries.size() - 1;
        }
        selectEntry( m_nActive );
    }

    if ( bNeedsUpdate )
    {
        m_bNeedsRecalc = true;
        if ( IsReallyVisible() )
            Invalidate();
    }
}
//------------------------------------------------------------------------------
bool ExtensionBox_Impl::isHCMode()
{
    return (bool)GetDisplayBackground().GetColor().IsDark();
}

//------------------------------------------------------------------------------
/*void ExtensionBox_Impl::DataChanged( DataChangedEvent const & evt )
{
    SvTreeListBox::DataChanged( evt );
    if (evt.GetType() == DATACHANGED_SETTINGS &&
        (evt.GetFlags() & SETTINGS_STYLE) != 0 &&
        m_hiContrastMode != (bool)GetDisplayBackground().GetColor().IsDark())
    {
        m_hiContrastMode = ! m_hiContrastMode;

        // Update all images as we changed from/to high contrast mode:
        for ( SvLBoxEntry * entry = First(); entry != 0; entry = Next(entry) )
        {
            NodeImpl * node = NodeImpl::get(entry);
            Image img( node->getIcon() );
            SetExpandedEntryBmp( entry, img );
            SetCollapsedEntryBmp( entry, img );
        }

        // force redraw:
        Invalidate();
    }
}
*/
//------------------------------------------------------------------------------
void ExtensionBox_Impl::enableButtons( bool bEnable )
{
    m_bInterfaceLocked = ! bEnable;

    if ( bEnable )
    {
        if ( m_bHasActive )
            SetButtonStatus( m_vEntries[ m_nActive ] );
    }
    else
    {
        m_pOptionsBtn->Enable( false );
        m_pRemoveBtn->Enable( false );
        m_pEnableBtn->Enable( false );
    }
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtensionBox_Impl, ScrollHdl, ScrollBar*, pScrBar )
{
    long nDelta = pScrBar->GetDelta();

    m_nTopIndex += nDelta;
    Point aNewOptPt( m_pOptionsBtn->GetPosPixel() - Point( 0, nDelta ) );
    Point aNewRemPt( m_pRemoveBtn->GetPosPixel() - Point( 0, nDelta ) );
    Point aNewEnPt( m_pEnableBtn->GetPosPixel() - Point( 0, nDelta ) );
    Point aNewSBPt( m_pScrollBar->GetPosPixel() );

    Rectangle aScrRect( Point(), GetOutputSizePixel() );
    aScrRect.Right() -= pScrBar->GetSizePixel().Width();
    Scroll( 0, -nDelta, aScrRect );

    m_pOptionsBtn->SetPosPixel( aNewOptPt );
    m_pRemoveBtn->SetPosPixel( aNewRemPt );
    m_pEnableBtn->SetPosPixel( aNewEnPt );
    m_pScrollBar->SetPosPixel( aNewSBPt );

    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtensionBox_Impl, HandleOptionsBtn, void*, EMPTYARG )
{
    if ( m_bHasActive )
    {
        SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

        if ( pFact )
        {
            OUString sExtensionId = m_vEntries[ m_nActive ]->m_xPackage->getIdentifier().Value;
            VclAbstractDialog* pDlg = pFact->CreateOptionsDialog( this, sExtensionId, rtl::OUString() );

            pDlg->Execute();

            delete pDlg;
        }
    }

    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtensionBox_Impl, HandleEnableBtn, void*, EMPTYARG )
{
    if ( m_bHasActive )
    {
        const bool bEnable( m_vEntries[ m_nActive ]->m_eState != REGISTERED );

        m_pParent->enablePackage( m_vEntries[ m_nActive ]->m_xPackageManager,
                                  m_vEntries[ m_nActive ]->m_xPackage,
                                  bEnable );
    }

    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtensionBox_Impl, HandleRemoveBtn, void*, EMPTYARG )
{
    if ( m_bHasActive )
    {
         m_pParent->removePackage( m_vEntries[ m_nActive ]->m_xPackageManager,
                                   m_vEntries[ m_nActive ]->m_xPackage );
    }

    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtensionBox_Impl, HandleHyperlink, svt::FixedHyperlink*, pHyperlink )
{
    m_pParent->openWebBrowser( pHyperlink->GetURL() );
    return 1;
}


//------------------------------------------------------------------------------
//                             ExtMgrDialog
//------------------------------------------------------------------------------
ExtMgrDialog::ExtMgrDialog( Window *pParent, TheExtensionManager *pManager ) :
    ModelessDialog( pParent, getResId( RID_DLG_EXTENSION_MANAGER ) ),
    m_aAddBtn( this,        getResId( RID_EM_BTN_ADD ) ),
    m_aUpdateBtn( this,     getResId( RID_EM_BTN_CHECK_UPDATES ) ),
    m_aCloseBtn( this,      getResId( RID_EM_BTN_CLOSE ) ),
    m_aHelpBtn( this,       getResId( RID_EM_BTN_HELP ) ),
    m_aDivider( this ),
    m_aGetExtensions( this, getResId( RID_EM_FT_GET_EXTENSIONS ) ),
    m_aProgressText( this,  getResId( RID_EM_FT_PROGRESS ) ),
    m_aProgressBar( this,   WB_BORDER + WB_3DLOOK ),
    m_aCancelBtn( this,     getResId( RID_EM_BTN_CANCEL ) ),
    m_sAddPackages(         getResourceString( RID_STR_ADD_PACKAGES ) ),
    m_bHasProgress(         false ),
    m_bProgressChanged(     false ),
    m_bStartProgress(       false ),
    m_bStopProgress(        false ),
    m_bUpdateWarning(       false ),
    m_bEnableWarning(       false ),
    m_bDisableWarning(      false ),
    m_bDeleteWarning(       false ),
    m_bIsBusy(              false ),
    m_nProgress(            0 ),
    m_pManager( pManager )
{
    // free local resources (RID < 256):
    FreeResource();

    m_pExtensionBox = new ExtensionBox_Impl( this, pManager );

    m_aAddBtn.SetClickHdl( LINK( this, ExtMgrDialog, HandleAddBtn ) );
    m_aUpdateBtn.SetClickHdl( LINK( this, ExtMgrDialog, HandleUpdateBtn ) );
    m_aGetExtensions.SetClickHdl( LINK( this, ExtMgrDialog, HandleHyperlink ) );
    m_aCancelBtn.SetClickHdl( LINK( this, ExtMgrDialog, HandleCancelBtn ) );

    // resize update button
    Size aBtnSize = m_aUpdateBtn.GetSizePixel();
    String sTitle = m_aUpdateBtn.GetText();
    long nWidth = m_aUpdateBtn.GetCtrlTextWidth( sTitle );
    nWidth += 2 * m_aUpdateBtn.GetTextHeight();
    if ( nWidth > aBtnSize.Width() )
        m_aUpdateBtn.SetSizePixel( Size( nWidth, aBtnSize.Height() ) );

    // minimum size:
    SetMinOutputSizePixel(
        Size( // width:
              (3 * m_aHelpBtn.GetSizePixel().Width()) +
                   m_aUpdateBtn.GetSizePixel().Width() +
              (5 * RSC_SP_DLG_INNERBORDER_LEFT ),
              // height:
              (1 * m_aHelpBtn.GetSizePixel().Height()) +
              (1 * m_aGetExtensions.GetSizePixel().Height()) +
              (1 * m_pExtensionBox->GetMinOutputSizePixel().Height()) +
              (3 * RSC_SP_DLG_INNERBORDER_LEFT) ) );

    m_aDivider.Show();
    m_aProgressBar.Hide();

    m_aUpdateBtn.Enable( false );

    m_aTimeoutTimer.SetTimeout( 500 ); // mSec
    m_aTimeoutTimer.SetTimeoutHdl( LINK( this, ExtMgrDialog, TimeOutHdl ) );
}

//------------------------------------------------------------------------------
ExtMgrDialog::~ExtMgrDialog()
{
    m_aTimeoutTimer.Stop();
    delete m_pExtensionBox;
}

//------------------------------------------------------------------------------
ResId ExtMgrDialog::getResId( USHORT id )
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    return ResId( id, *DeploymentGuiResMgr::get() );
}

//------------------------------------------------------------------------------
String ExtMgrDialog::getResourceString( USHORT id )
{
    // init with non-acquired solar mutex:
    BrandName::get();
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    String ret( ResId( id, *DeploymentGuiResMgr::get() ) );
    if (ret.SearchAscii( "%PRODUCTNAME" ) != STRING_NOTFOUND) {
        ret.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
    }
    return ret;
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::IsSharedPkgMgr( const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    if ( xPackageManager->getContext().equals( OUSTR("shared") ) )
        return true;
    else
        return false;
}

//------------------------------------------------------------------------------
void ExtMgrDialog::selectEntry( long nPos )
{
    m_pExtensionBox->selectEntry( nPos );
}

//------------------------------------------------------------------------------
void ExtMgrDialog::setGetExtensionsURL( const ::rtl::OUString &rURL )
{
    m_aGetExtensions.SetURL( rURL );
}

//------------------------------------------------------------------------------
long ExtMgrDialog::addPackageToList( const uno::Reference< deployment::XPackage > &xPackage,
                                     const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    m_aUpdateBtn.Enable( true );
    return m_pExtensionBox->addEntry( xPackage, xPackageManager );
}

//------------------------------------------------------------------------------
void ExtMgrDialog::prepareChecking( const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    if ( xPackageManager.is() )
        m_pExtensionBox->prepareChecking( xPackageManager );
}

//------------------------------------------------------------------------------
void ExtMgrDialog::checkEntries()
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    m_pExtensionBox->checkEntries();
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::continueOnSharedExtension( const uno::Reference< deployment::XPackageManager > &xPackageManager,
                                              const USHORT nResID,
                                              bool &bHadWarning ) const
{
    if ( IsSharedPkgMgr( xPackageManager ) && !bHadWarning )
    {
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        WarningBox aInfoBox( const_cast< ExtMgrDialog* >(this), getResId( nResID ) );
        String aMsgText = aInfoBox.GetMessText();
        aMsgText.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
        aInfoBox.SetMessText( aMsgText );

        bHadWarning = true;

        if ( RET_OK == aInfoBox.Execute() )
            return true;
        else
            return false;
    }
    else
        return true;
}

//------------------------------------------------------------------------------
void ExtMgrDialog::openWebBrowser( OUString const & sURL ) const
{
    if ( ! sURL.getLength() ) // Nothing to do, when the URL is empty
        return;

    try
    {
        uno::Reference< uno::XComponentContext > xContext = m_pManager->getContext();

        uno::Reference< XSystemShellExecute > xSystemShellExecute(
            xContext->getServiceManager()->createInstanceWithContext( OUSTR( "com.sun.star.system.SystemShellExecute" ), xContext), uno::UNO_QUERY_THROW);
        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
        xSystemShellExecute->execute( sURL, OUString(),  SystemShellExecuteFlags::DEFAULTS );
    }
    catch ( uno::Exception& )
    {
        uno::Any exc( ::cppu::getCaughtException() );
        OUString msg( ::comphelper::anyToString( exc ) );
        const ::vos::OGuard guard( Application::GetSolarMutex() );
        ErrorBox aErrorBox( const_cast< ExtMgrDialog* >(this), WB_OK, msg );
        aErrorBox.SetText( GetText() );
        aErrorBox.Execute();
    }
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::installForAllUsers( bool &bInstallForAll ) const
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    QueryBox aQuery( const_cast< ExtMgrDialog* >(this), getResId( RID_QUERYBOX_INSTALL_FOR_ALL ) );

    String sMsgText = aQuery.GetMessText();
    sMsgText.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
    aQuery.SetMessText( sMsgText );

    USHORT nYesBtnID = aQuery.GetButtonId( 0 );
    USHORT nNoBtnID = aQuery.GetButtonId( 1 );

    if ( nYesBtnID != BUTTONDIALOG_BUTTON_NOTFOUND )
        aQuery.SetButtonText( nYesBtnID, getResourceString( RID_STR_INSTALL_FOR_ME ) );
    if ( nNoBtnID != BUTTONDIALOG_BUTTON_NOTFOUND )
        aQuery.SetButtonText( nNoBtnID, getResourceString( RID_STR_INSTALL_FOR_ALL ) );

    short nRet = aQuery.Execute();

    if ( nRet == RET_CANCEL )
        return false;

    bInstallForAll = ( nRet == RET_NO );
    return true;
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::installExtensionWarn( const OUString &rExtensionName ) const
{
    const ::vos::OGuard guard( Application::GetSolarMutex() );
    WarningBox aInfo( const_cast< ExtMgrDialog* >(this), getResId( RID_WARNINGBOX_INSTALL_EXTENSION ) );

    String sText( aInfo.GetMessText() );
    sText.SearchAndReplaceAllAscii( "%NAME", rExtensionName );
    aInfo.SetMessText( sText );

    return ( RET_OK == aInfo.Execute() );
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::enablePackage( const uno::Reference< deployment::XPackageManager > &xPackageManager,
                                  const uno::Reference< deployment::XPackage > &xPackage,
                                  bool bEnable )
{
    if ( !xPackageManager.is() || !xPackage.is() )
        return false;

    if ( bEnable )
    {
        if ( ! continueOnSharedExtension( xPackageManager, RID_WARNINGBOX_ENABLE_SHARED_EXTENSION, m_bEnableWarning ) )
            return false;
    }
    else
    {
        if ( ! continueOnSharedExtension( xPackageManager, RID_WARNINGBOX_DISABLE_SHARED_EXTENSION, m_bDisableWarning ) )
            return false;
    }

    m_pManager->enablePackage( xPackage, bEnable );

    return true;
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::removePackage( const uno::Reference< deployment::XPackageManager > &xPackageManager,
                                  const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackageManager.is() || !xPackage.is() )
        return false;

    if ( ! continueOnSharedExtension( xPackageManager, RID_WARNINGBOX_REMOVE_SHARED_EXTENSION, m_bDeleteWarning ) )
        return false;

    m_pManager->removePackage( xPackageManager, xPackage );

    return true;
}

//------------------------------------------------------------------------------
bool ExtMgrDialog::updatePackage( const uno::Reference< deployment::XPackageManager > &xPackageManager,
                                  const uno::Reference< deployment::XPackage > &xPackage )
{
    if ( !xPackageManager.is() || !xPackage.is() )
        return false;

    m_pManager->updatePackage( xPackageManager, xPackage );

    return true;
}

//------------------------------------------------------------------------------
uno::Sequence< OUString > ExtMgrDialog::raiseAddPicker( const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    const uno::Any mode( static_cast< sal_Int16 >( ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE ) );
    const uno::Reference< uno::XComponentContext > xContext( m_pManager->getContext() );
    const uno::Reference< ui::dialogs::XFilePicker > xFilePicker(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUSTR("com.sun.star.ui.dialogs.FilePicker"),
            uno::Sequence< uno::Any >( &mode, 1 ), xContext ), uno::UNO_QUERY_THROW );
    xFilePicker->setTitle( m_sAddPackages );

    if ( m_sLastFolderURL.Len() )
        xFilePicker->setDisplayDirectory( m_sLastFolderURL );

    // collect and set filter list:
    typedef ::std::map< OUString, OUString > t_string2string;
    t_string2string title2filter;
    OUString sDefaultFilter( StrAllFiles::get() );

    const uno::Sequence< uno::Reference< deployment::XPackageTypeInfo > > packageTypes( xPackageManager->getSupportedPackageTypes() );

    for ( sal_Int32 pos = 0; pos < packageTypes.getLength(); ++pos )
    {
        uno::Reference< deployment::XPackageTypeInfo > const & xPackageType = packageTypes[ pos ];
        const OUString filter( xPackageType->getFileFilter() );
        if (filter.getLength() > 0)
        {
            const OUString title( xPackageType->getShortDescription() );
            const ::std::pair< t_string2string::iterator, bool > insertion(
                title2filter.insert( t_string2string::value_type( title, filter ) ) );
            if ( ! insertion.second )
            { // already existing, append extensions:
                ::rtl::OUStringBuffer buf;
                buf.append( insertion.first->second );
                buf.append( static_cast<sal_Unicode>(';') );
                buf.append( filter );
                insertion.first->second = buf.makeStringAndClear();
            }
            if ( xPackageType->getMediaType() == OUSTR( "application/vnd.sun.star.package-bundle" ) )
                sDefaultFilter = title;
        }
    }

    const uno::Reference< ui::dialogs::XFilterManager > xFilterManager( xFilePicker, uno::UNO_QUERY_THROW );
    // All files at top:
    xFilterManager->appendFilter( StrAllFiles::get(), OUSTR("*.*") );
    // then supported ones:
    t_string2string::const_iterator iPos( title2filter.begin() );
    const t_string2string::const_iterator iEnd( title2filter.end() );
    for ( ; iPos != iEnd; ++iPos ) {
        try {
            xFilterManager->appendFilter( iPos->first, iPos->second );
        }
        catch (lang::IllegalArgumentException & exc) {
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            (void) exc;
        }
    }
    xFilterManager->setCurrentFilter( sDefaultFilter );

    if ( xFilePicker->execute() != ui::dialogs::ExecutableDialogResults::OK )
        return uno::Sequence<OUString>(); // cancelled

    m_sLastFolderURL = xFilePicker->getDisplayDirectory();
    uno::Sequence< OUString > files( xFilePicker->getFiles() );
    OSL_ASSERT( files.getLength() > 0 );
    return files;
}

//------------------------------------------------------------------------------
IMPL_LINK( ExtMgrDialog, HandleCancelBtn, void*, EMPTYARG )
{
    // m_dialog->m_cmdEnv->m_aborted = true;
    if ( m_xAbortChannel.is() )
    {
        try
        {
            m_xAbortChannel->sendAbort();
        }
        catch ( uno::RuntimeException & )
        {
            OSL_ENSURE( 0, "### unexpected RuntimeException!" );
        }
    }
    return 1;
}

// ------------------------------------------------------------------------------
IMPL_LINK( ExtMgrDialog, startProgress, void*, _bLockInterface )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    bool bLockInterface = (bool) _bLockInterface;

    if ( m_bStartProgress && !m_bHasProgress )
        m_aTimeoutTimer.Start();

    if ( m_bStopProgress )
    {
        if ( m_aProgressBar.IsVisible() )
            m_aProgressBar.SetValue( 100 );
        m_xAbortChannel.clear();
//        bLockInterface = false;
        OSL_TRACE( " startProgress handler: stop\n" );
    }
    else
    {
        OSL_TRACE( " startProgress handler: start\n" );
    }

    m_aCancelBtn.Enable( bLockInterface );
    m_aAddBtn.Enable( !bLockInterface );
    m_aUpdateBtn.Enable( !bLockInterface && m_pExtensionBox->getItemCount() );
    m_pExtensionBox->enableButtons( !bLockInterface );

//    pCond->set();
    return 0;
}

// ------------------------------------------------------------------------------
void ExtMgrDialog::showProgress( bool _bStart )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bStart = _bStart;

    if ( bStart )
    {
        m_nProgress = 0;
        m_bStartProgress = true;
        OSL_TRACE( "showProgress start\n" );
    }
    else
    {
        m_nProgress = 100;
        m_bStopProgress = true;
        OSL_TRACE( "showProgress stop!\n" );
    }

    Application::PostUserEvent( LINK( this, ExtMgrDialog, startProgress ), (void*) bStart );
//    ::osl::Condition cond;
//    Application::PostUserEvent( LINK( this, ExtMgrDialog, startProgress ), &cond );
//    cond.wait();
}

// -----------------------------------------------------------------------
void ExtMgrDialog::updateProgress( const long nProgress )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_nProgress = nProgress;
}

// -----------------------------------------------------------------------
void ExtMgrDialog::updateProgress( const OUString &rText,
                                   const uno::Reference< task::XAbortChannel > &xAbortChannel)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xAbortChannel = xAbortChannel;
    m_sProgressText = rText;
    m_bProgressChanged = true;
}

//------------------------------------------------------------------------------
void ExtMgrDialog::updatePackageInfo( const uno::Reference< deployment::XPackage > &xPackage )
{
    m_pExtensionBox->updateEntry( xPackage );
}

//------------------------------------------------------------------------------
void ExtMgrDialog::removeEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
    const vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pExtensionBox->removeEntry( xPackage );
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtMgrDialog, HandleAddBtn, void*, EMPTYARG )
{
    m_bIsBusy = true;

    uno::Reference< deployment::XPackageManager > xUserPkgMgr = m_pManager->getUserPkgMgr();
    uno::Sequence< OUString > aFileList = raiseAddPicker( xUserPkgMgr );

    if ( aFileList.getLength() )
    {
        m_pManager->installPackage( aFileList[0] );
    }

    m_bIsBusy = false;
    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtMgrDialog, HandleUpdateBtn, void*, EMPTYARG )
{
    m_pManager->checkUpdates( false, true );

    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtMgrDialog, HandleHyperlink, svt::FixedHyperlink*, pHyperlink )
{
    openWebBrowser( pHyperlink->GetURL() );

    return 1;
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtMgrDialog, TimeOutHdl, Timer*, EMPTYARG )
{
    if ( m_bStopProgress )
    {
        m_bHasProgress = false;
        m_bStopProgress = false;
        m_aProgressText.Hide();
        m_aProgressBar.Hide();
        m_aCancelBtn.Hide();
    }
    else
    {
        if ( m_bProgressChanged )
        {
            m_bProgressChanged = false;
            m_aProgressText.SetText( m_sProgressText );
        }

        if ( m_bStartProgress )
        {
            m_bStartProgress = false;
            m_bHasProgress = true;
            m_aProgressBar.Show();
            m_aProgressText.Show();
            m_aCancelBtn.Enable();
            m_aCancelBtn.Show();
        }

        if ( m_aProgressBar.IsVisible() )
            m_aProgressBar.SetValue( (USHORT) m_nProgress );

        m_aTimeoutTimer.Start();
    }

    return 1;
}

//------------------------------------------------------------------------------
// VCL::Window / Dialog
void ExtMgrDialog::Resize()
{
    Size aTotalSize( GetOutputSizePixel() );
    Size aBtnSize( m_aHelpBtn.GetSizePixel() );
    Size aUpdBtnSize( m_aUpdateBtn.GetSizePixel() );

    Point aPos( RSC_SP_DLG_INNERBORDER_LEFT,
                aTotalSize.Height() - RSC_SP_DLG_INNERBORDER_BOTTOM - aBtnSize.Height() );

    m_aHelpBtn.SetPosPixel( aPos );

    aPos.X() = aTotalSize.Width() - RSC_SP_DLG_INNERBORDER_RIGHT - aBtnSize.Width();
    m_aCloseBtn.SetPosPixel( aPos );

    aPos.X() -= ( RSC_SP_CTRL_X + aUpdBtnSize.Width() );
    m_aUpdateBtn.SetPosPixel( aPos );

    aPos.X() -= ( RSC_SP_CTRL_GROUP_Y + aBtnSize.Width() );
    m_aAddBtn.SetPosPixel( aPos );

    Size aDivSize( aTotalSize.Width(), LINE_SIZE );
    aPos = Point( 0, aPos.Y() - LINE_SIZE - RSC_SP_DLG_INNERBORDER_BOTTOM );
    m_aDivider.SetPosSizePixel( aPos, aDivSize );

    Size aFTSize( m_aGetExtensions.CalcMinimumSize() );
    aPos = Point( RSC_SP_DLG_INNERBORDER_LEFT, aPos.Y() - RSC_CD_FIXEDTEXT_HEIGHT - 2*RSC_SP_DLG_INNERBORDER_BOTTOM );

    m_aGetExtensions.SetPosSizePixel( aPos, aFTSize );

    aPos.X() = aTotalSize.Width() - RSC_SP_DLG_INNERBORDER_RIGHT - aBtnSize.Width();
    m_aCancelBtn.SetPosPixel( Point( aPos.X(), aPos.Y() - ((aBtnSize.Height()-aFTSize.Height())/2) ) );

    // Calc progress height
    long nProgressHeight = aFTSize.Height();

    if( IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aValue;
        bool bNativeOK;
        Region aControlRegion( Rectangle( (const Point&)Point(), m_aProgressBar.GetSizePixel() ) );
        Region aNativeControlRegion, aNativeContentRegion;
        if( (bNativeOK = GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                                 CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                                 aNativeControlRegion, aNativeContentRegion ) ) != FALSE )
        {
            nProgressHeight = aNativeControlRegion.GetBoundRect().GetHeight();
        }
    }

    if ( nProgressHeight < PROGRESS_HEIGHT )
        nProgressHeight = PROGRESS_HEIGHT;

    aPos.X() -= ( RSC_SP_CTRL_GROUP_Y + PROGRESS_WIDTH );
    m_aProgressBar.SetPosSizePixel( Point( aPos.X(), aPos.Y() - ((nProgressHeight-aFTSize.Height())/2) ),
                                    Size( PROGRESS_WIDTH, nProgressHeight ) );

    Rectangle aRect1( m_aGetExtensions.GetPosPixel(), m_aGetExtensions.GetSizePixel() );
    Rectangle aRect2( m_aProgressBar.GetPosPixel(), m_aProgressBar.GetSizePixel() );

    aFTSize.Width() = ( aRect2.Left() - aRect1.Right() ) - 2*RSC_SP_DLG_INNERBORDER_LEFT;
    aPos.X() = aRect1.Right() + RSC_SP_DLG_INNERBORDER_LEFT;
    m_aProgressText.SetPosSizePixel( aPos, aFTSize );

    Size aSize( aTotalSize.Width() - RSC_SP_DLG_INNERBORDER_LEFT - RSC_SP_DLG_INNERBORDER_RIGHT,
                aTotalSize.Height() - 2*aBtnSize.Height() - LINE_SIZE -
                RSC_SP_DLG_INNERBORDER_TOP - 3*RSC_SP_DLG_INNERBORDER_BOTTOM );

    m_pExtensionBox->SetSizePixel( aSize );
}
//------------------------------------------------------------------------------
// VCL::Window / Dialog

long ExtMgrDialog::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode         aKeyCode = pKEvt->GetKeyCode();
        USHORT          nKeyCode = aKeyCode.GetCode();

        if ( nKeyCode == KEY_TAB )
        {
            if ( aKeyCode.IsShift() ) {
                if ( m_aAddBtn.HasFocus() ) {
                    m_pExtensionBox->GrabFocus();
                    bHandled = true;
                }
            } else {
                if ( m_aGetExtensions.HasFocus() ) {
                    m_pExtensionBox->GrabFocus();
                    bHandled = true;
                }
            }
        }
        if ( aKeyCode.GetGroup() == KEYGROUP_CURSOR )
            bHandled = m_pExtensionBox->Notify( rNEvt );
    }
// VCLEVENT_WINDOW_CLOSE
    if ( !bHandled )
        return ModelessDialog::Notify( rNEvt );
    else
        return true;
}

//------------------------------------------------------------------------------
BOOL ExtMgrDialog::Close()
{
    bool bRet = m_pManager->queryTermination();
    if ( bRet )
    {
        bRet = ModelessDialog::Close();
        m_pManager->terminateDialog();
    }
    return bRet;
}

SelectedPackage::~SelectedPackage() {}

} //namespace dp_gui

