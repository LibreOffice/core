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

#ifndef INCLUDED_SVTOOLS_SOURCE_CONTROL_TOOLBARMENUIMP_HXX
#define INCLUDED_SVTOOLS_SOURCE_CONTROL_TOOLBARMENUIMP_HXX

#include <osl/mutex.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <rtl/ref.hxx>

#include <memory>
#include <vector>

namespace svtools {

struct ToolbarMenu_Impl;
class ToolbarMenu;
class ToolbarMenuEntry;

typedef ::std::vector< css::uno::Reference< css::accessibility::XAccessibleEventListener > > EventListenerVector;
typedef std::vector< std::unique_ptr< ToolbarMenuEntry > > ToolbarMenuEntryVector;

const int SEPARATOR_HEIGHT = 4;
const int TITLE_ID = -1;
const int BORDER_X = 0;
const int BORDER_Y = 0;


class ToolbarMenuEntry
{
public:
    ToolbarMenu& mrMenu;

    int mnEntryId;
    MenuItemBits mnBits;
    Size maSize;

    bool mbHasText;
    bool mbHasImage;
    bool mbChecked;
    bool mbEnabled;

    OUString maText;
    Image maImage;
    VclPtr<Control> mpControl;
    tools::Rectangle maRect;

    css::uno::Reference< css::accessibility::XAccessibleContext > mxAccContext;

public:
    ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const OUString& rText, MenuItemBits nBits );
    ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const Image& rImage, const OUString& rText, MenuItemBits nBits );
    ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, Control* pControl, MenuItemBits nBits );
    ~ToolbarMenuEntry();

    void init( int nEntryId, MenuItemBits nBits );

    const css::uno::Reference< css::accessibility::XAccessibleContext >& GetAccessible();

    /// @throws css::uno::RuntimeException
    sal_Int32 getAccessibleChildCount();
    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int32 index );

    bool HasCheck() const
    {
        return mbChecked || ( mnBits & ( MenuItemBits::RADIOCHECK | MenuItemBits::CHECKABLE | MenuItemBits::AUTOCHECK ) );
    }
};


typedef ::cppu::WeakComponentImplHelper<
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleEventBroadcaster,
    css::accessibility::XAccessibleContext,
    css::accessibility::XAccessibleComponent,
    css::accessibility::XAccessibleSelection >
    ToolbarMenuAccComponentBase;

class ToolbarMenuAcc :
    public ::cppu::BaseMutex,
    public ToolbarMenuAccComponentBase
{
public:

    explicit ToolbarMenuAcc( ToolbarMenu_Impl& rParent );
    virtual ~ToolbarMenuAcc() override;

    void                FireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );
    bool                HasAccessibleListeners() const { return( mxEventListeners.size() > 0 ); }

public:

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    DECL_LINK( WindowEventListener, VclWindowEvent&, void );

private:
    EventListenerVector mxEventListeners;
    ToolbarMenu_Impl* mpParent;
    /// The current FOCUSED state.
    bool mbIsFocused;

    /** Tell all listeners that the object is dying.  This callback is
        usually called from the WeakComponentImplHelper class.
    */
    virtual void SAL_CALL disposing() override;

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.

        @throws css::lang::DisposedException
    */
    void ThrowIfDisposed();
};


typedef ::cppu::WeakComponentImplHelper< css::accessibility::XAccessible,
                                                     css::accessibility::XAccessibleEventBroadcaster,
                                                     css::accessibility::XAccessibleContext,
                                                     css::accessibility::XAccessibleComponent > ToolbarMenuEntryAccBase;

class ToolbarMenuEntryAcc : public ::cppu::BaseMutex,
                            public ToolbarMenuEntryAccBase
{
public:
    explicit ToolbarMenuEntryAcc( ToolbarMenuEntry* pParent );
    virtual ~ToolbarMenuEntryAcc() override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

private:
    EventListenerVector    mxEventListeners;
    ::osl::Mutex           maMutex;
    ToolbarMenuEntry*      mpParent;

    /** Tell all listeners that the object is dying.  This callback is
        usually called from the WeakComponentImplHelper class.
    */
    virtual void SAL_CALL disposing() override;
};


struct ToolbarMenu_Impl
{
    ToolbarMenu& mrMenu;

    rtl::Reference< ToolbarMenuAcc >                       mxAccessible;
    css::uno::Reference< css::accessibility::XAccessible > mxOldSelection;

    ToolbarMenuEntryVector  maEntryVector;

    int mnCheckPos;
    int mnImagePos;
    int mnTextPos;

    int mnHighlightedEntry;
    int mnSelectedEntry;
    int mnLastColumn;

    Size maSize;

    Link<ToolbarMenu*,void>          maSelectHdl;

    explicit ToolbarMenu_Impl( ToolbarMenu& rMenu );
    ~ToolbarMenu_Impl();

    void setAccessible( ToolbarMenuAcc* pAccessible );

    void fireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );

    /// @throws css::uno::RuntimeException
    sal_Int32 getAccessibleChildCount();
    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int32 index );
    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( Control const * pControl, sal_Int32 childIndex );

    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    void selectAccessibleChild( sal_Int32 nChildIndex );
    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    bool isAccessibleChildSelected( sal_Int32 nChildIndex );
    void clearAccessibleSelection();

    ToolbarMenuEntry* implGetEntry( int nEntry ) const;
    void notifyHighlightedEntry();

    void implHighlightControl( sal_uInt16 nCode, Control* pControl );
};

}

#endif // INCLUDED_SVTOOLS_SOURCE_CONTROL_TOOLBARMENUIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
