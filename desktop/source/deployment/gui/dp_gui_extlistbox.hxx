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

#include "rtl/ustring.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/fixed.hxx"
#include "vcl/dialog.hxx"

#include "svtools/extensionlistbox.hxx"
#include "svtools/fixedhyper.hxx"
#include "cppuhelper/implbase1.hxx"
#include "unotools/collatorwrapper.hxx"

#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/deployment/XPackage.hpp"

#include <boost/shared_ptr.hpp>

namespace dp_gui {

#define SMALL_ICON_SIZE     16
#define TOP_OFFSET           5
#define ICON_HEIGHT         42
#define ICON_WIDTH          47
#define ICON_OFFSET         72
#define RIGHT_ICON_OFFSET    5
#define SPACE_BETWEEN        3

class TheExtensionManager;

typedef ::boost::shared_ptr< svt::FixedHyperlink > TFixedHyperlink;

//------------------------------------------------------------------------------
//                          struct Entry_Impl
//------------------------------------------------------------------------------
struct Entry_Impl;

typedef ::boost::shared_ptr< Entry_Impl > TEntry_Impl;

struct Entry_Impl
{
    bool            m_bActive       :1;
    bool            m_bLocked       :1;
    bool            m_bHasOptions   :1;
    bool            m_bUser         :1;
    bool            m_bShared       :1;
    bool            m_bNew          :1;
    bool            m_bChecked      :1;
    bool            m_bMissingDeps  :1;
    bool            m_bHasButtons   :1;
    bool            m_bMissingLic   :1;
    PackageState    m_eState;
    String          m_sTitle;
    String          m_sVersion;
    String          m_sDescription;
    String          m_sPublisher;
    String          m_sPublisherURL;
    String          m_sErrorText;
    String          m_sLicenseText;
    Image           m_aIcon;
    Image           m_aIconHC;
    svt::FixedHyperlink *m_pPublisher;

    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> m_xPackage;

    Entry_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                const PackageState eState, const bool bReadOnly );
   ~Entry_Impl();

    StringCompare CompareTo( const CollatorWrapper *pCollator, const TEntry_Impl pEntry ) const;
    void          checkDependencies();
};

//------------------------------------------------------------------------------
//                          class ExtensionBox_Impl
//------------------------------------------------------------------------------

class ExtensionBox_Impl;

//------------------------------------------------------------------------------
class ExtensionRemovedListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
    ExtensionBox_Impl   *m_pParent;

public:

    ExtensionRemovedListener( ExtensionBox_Impl *pParent ) { m_pParent = pParent; }
   ~ExtensionRemovedListener();

    //===================================================================================
    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);
};

//------------------------------------------------------------------------------
class ExtensionBox_Impl : public ::svt::IExtensionListBox
{
    bool            m_bHasScrollBar;
    bool            m_bHasActive;
    bool            m_bNeedsRecalc;
    bool            m_bInCheckMode;
    bool            m_bAdjustActive;
    bool            m_bInDelete;
    //Must be guarded together with m_vEntries to ensure a valid index at all times.
    //Use m_entriesMutex as guard.
    long            m_nActive;
    long            m_nTopIndex;
    long            m_nStdHeight;
    long            m_nActiveHeight;
    long            m_nExtraHeight;
    Size            m_aOutputSize;
    Image           m_aSharedImage;
    Image           m_aLockedImage;
    Image           m_aWarningImage;
    Image           m_aDefaultImage;
    Link            m_aClickHdl;

    ScrollBar      *m_pScrollBar;

    com::sun::star::uno::Reference< ExtensionRemovedListener > m_xRemoveListener;

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
    std::vector< TEntry_Impl > m_vRemovedEntries;

    ::com::sun::star::lang::Locale    *m_pLocale;
    CollatorWrapper *m_pCollator;

    void            CalcActiveHeight( const long nPos );
    long            GetTotalHeight() const;
    void            SetupScrollBar();
    void            DrawRow( const Rectangle& rRect, const TEntry_Impl pEntry );
    bool            HandleTabKey( bool bReverse );
    bool            HandleCursorKey( sal_uInt16 nKeyCode );
    bool            FindEntryPos( const TEntry_Impl pEntry, long nStart, long nEnd, long &nFound );
    void            DeleteRemoved();

    //-----------------
    DECL_DLLPRIVATE_LINK( ScrollHdl, ScrollBar * );

    //Index starts with 1.
    //Throws an com::sun::star::lang::IllegalArgumentException, when the index is invalid.
    void checkIndex(sal_Int32 pos) const;


public:
                    ExtensionBox_Impl( Dialog* pParent, TheExtensionManager *pManager );
                   ~ExtensionBox_Impl();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle &rPaintRect );
    virtual void    Resize();
    virtual long    Notify( NotifyEvent& rNEvt );

    const Size      GetMinOutputSizePixel() const;
    void            SetExtraSize( long nSize ) { m_nExtraHeight = nSize; }
    TEntry_Impl     GetEntryData( long nPos ) { return m_vEntries[ nPos ]; }
    long            GetEntryCount() { return (long) m_vEntries.size(); }
    Rectangle       GetEntryRect( const long nPos ) const;
    bool            HasActive() { return m_bHasActive; }
    long            PointToPos( const Point& rPos );
    void            SetScrollHdl( const Link& rLink );
    void            DoScroll( long nDelta );
    void            SetHyperlinkHdl( const Link& rLink ){ m_aClickHdl = rLink; }
    virtual void    RecalcAll();
    void            RemoveUnlocked();

    //-----------------
    virtual void    selectEntry( const long nPos );
    long            addEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                              bool bLicenseMissing = false );
    void            updateEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    void            removeEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    void            prepareChecking();
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
    virtual ::rtl::OUString getItemName( sal_Int32 index ) const;

    /** @return  The version string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemVersion( sal_Int32 index ) const;

    /** @return  The description string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemDescription( sal_Int32 index ) const;

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
    virtual void select( const ::rtl::OUString & sName );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
