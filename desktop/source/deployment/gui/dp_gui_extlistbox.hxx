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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_EXTLISTBOX_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_EXTLISTBOX_HXX

#include "rtl/ustring.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/fixed.hxx"
#include "vcl/fixedhyper.hxx"
#include "vcl/dialog.hxx"

#include "svtools/extensionlistbox.hxx"
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

typedef ::boost::shared_ptr< FixedHyperlink > TFixedHyperlink;


//                          struct Entry_Impl

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
    OUString        m_sTitle;
    OUString        m_sVersion;
    OUString        m_sDescription;
    OUString        m_sPublisher;
    OUString        m_sPublisherURL;
    OUString        m_sErrorText;
    OUString        m_sLicenseText;
    Image           m_aIcon;
    Image           m_aIconHC;
    FixedHyperlink* m_pPublisher;

    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> m_xPackage;

    Entry_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                const PackageState eState, const bool bReadOnly );
   ~Entry_Impl();

    sal_Int32     CompareTo( const CollatorWrapper *pCollator, const TEntry_Impl pEntry ) const;
    void          checkDependencies();
};


//                          class ExtensionBox_Impl


class ExtensionBox_Impl;


class ExtensionRemovedListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
    ExtensionBox_Impl   *m_pParent;

public:

    ExtensionRemovedListener( ExtensionBox_Impl *pParent ) { m_pParent = pParent; }
   virtual ~ExtensionRemovedListener();


    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


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

    //Holds weak references to extensions to which is we have added an XEventListener
    std::vector< ::com::sun::star::uno::WeakReference<
        ::com::sun::star::deployment::XPackage> > m_vListenerAdded;
    //Removes the dead weak references from m_vListenerAdded
    void cleanVecListenerAdded();
    void addEventListenerOnce( ::com::sun::star::uno::Reference<
                               ::com::sun::star::deployment::XPackage> const & extension);

    void            CalcActiveHeight( const long nPos );
    long            GetTotalHeight() const;
    void            SetupScrollBar();
    void            DrawRow( const Rectangle& rRect, const TEntry_Impl pEntry );
    bool            HandleTabKey( bool bReverse );
    bool            HandleCursorKey( sal_uInt16 nKeyCode );
    bool            FindEntryPos( const TEntry_Impl pEntry, long nStart, long nEnd, long &nFound );
    void            DeleteRemoved();


    DECL_DLLPRIVATE_LINK( ScrollHdl, ScrollBar * );

    //Index starts with 1.
    //Throws an com::sun::star::lang::IllegalArgumentException, when the index is invalid.
    void checkIndex(sal_Int32 pos) const;


    void Init();
public:
                    ExtensionBox_Impl(Window* pParent);
                    ExtensionBox_Impl(Window* pParent, TheExtensionManager *pManager);
                   virtual ~ExtensionBox_Impl();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle &rPaintRect ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

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


    virtual void    selectEntry( const long nPos );
    long            addEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                              bool bLicenseMissing = false );
    void            updateEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    void            removeEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    void            prepareChecking();
    void            checkEntries();

    TheExtensionManager*    getExtensionManager() const { return m_pManager; }
    void setExtensionManager(TheExtensionManager* pManager) { m_pManager = pManager; }


    //These functions are used for automatic testing

    /** @return  The count of the entries in the list box. */
    virtual sal_Int32 getItemCount() const SAL_OVERRIDE;

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns ENTRY_NOTFOUND */
    virtual sal_Int32 getSelIndex() const SAL_OVERRIDE;

    /** @return  The item name of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemName( sal_Int32 index ) const SAL_OVERRIDE;

    /** @return  The version string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemVersion( sal_Int32 index ) const SAL_OVERRIDE;

    /** @return  The description string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemDescription( sal_Int32 index ) const SAL_OVERRIDE;

    /** @return  The publisher string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemPublisher( sal_Int32 index ) const SAL_OVERRIDE;

    /** @return  The link behind the publisher text of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemPublisherLink( sal_Int32 index ) const SAL_OVERRIDE;

    /** The entry at the given position will be selected
        Index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual void select( sal_Int32 pos ) SAL_OVERRIDE;

    /** The first found entry with the given name will be selected
        When there was no entry found with the name, the selection doesn't change.
        Please note that there might be more than one entry with the same
        name, because:
            1. the name is not unique
            2. one extension can be installed as user and shared extension.
    */
    virtual void select( const OUString & sName ) SAL_OVERRIDE;
};

}

#endif // INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_EXTLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
