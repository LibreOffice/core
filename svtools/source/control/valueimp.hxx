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

#ifndef INCLUDED_SVTOOLS_SOURCE_CONTROL_VALUEIMP_HXX
#define INCLUDED_SVTOOLS_SOURCE_CONTROL_VALUEIMP_HXX

#include <osl/mutex.hxx>
#include <tools/color.hxx>
#include <vcl/image.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <vector>

#define VALUESET_ITEM_NONEITEM      0xFFFE

enum ValueSetItemType
{
    VALUESETITEM_NONE,
    VALUESETITEM_IMAGE,
    VALUESETITEM_IMAGE_AND_TEXT,
    VALUESETITEM_COLOR,
    VALUESETITEM_USERDRAW
};

class ValueSet;

struct ValueSetItem
{
    ValueSet&           mrParent;
    sal_uInt16          mnId;
    sal_uInt8           meType;
    bool                mbVisible;
    Image               maImage;
    Color               maColor;
    OUString            maText;
    void*               mpData;
    css::uno::Reference< css::accessibility::XAccessible > mxAcc;

    explicit ValueSetItem( ValueSet& rParent );
    ~ValueSetItem();

    css::uno::Reference< css::accessibility::XAccessible > const &
                        GetAccessible( bool bIsTransientChildrenDisabled );
};

typedef ::cppu::WeakComponentImplHelper<
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleEventBroadcaster,
    css::accessibility::XAccessibleContext,
    css::accessibility::XAccessibleComponent,
    css::accessibility::XAccessibleSelection,
    css::lang::XUnoTunnel >
    ValueSetAccComponentBase;

class ValueSetAcc :
    public ::cppu::BaseMutex,
    public ValueSetAccComponentBase
{
public:

    ValueSetAcc( ValueSet* pParent, bool bIsTransientChildrenDisabled );
    virtual ~ValueSetAcc();

    void                FireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );
    bool                HasAccessibleListeners() const { return( mxEventListeners.size() > 0 ); }

    static ValueSetAcc* getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData ) throw();

public:

    /** Called by the corresponding ValueSet when it gets the focus.
        Stores the new focus state and broadcasts a state change event.
    */
    void GetFocus();

    /** Called by the corresponding ValueSet when it loses the focus.
        Stores the new focus state and broadcasts a state change event.
    */
    void LoseFocus();

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw( css::uno::RuntimeException, std::exception ) override;

private:
    ::std::vector< css::uno::Reference<
        css::accessibility::XAccessibleEventListener > >                mxEventListeners;
    VclPtr<ValueSet>                                                    mpParent;
    bool                                                                mbIsTransientChildrenDisabled;
    /// The current FOCUSED state.
    bool mbIsFocused;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    /** Tell all listeners that the object is dying.  This callback is
        usually called from the WeakComponentImplHelper class.
    */
    virtual void SAL_CALL disposing() override;

    /** Return the number of items.  This takes the None-Item into account.
    */
    sal_uInt16 getItemCount() const;

    /** Return the item associated with the given index.  The None-Item is
        taken into account which, when present, is taken to be the first
        (with index 0) item.
        @param nIndex
            Index of the item to return.  The index 0 denotes the None-Item
            when present.
        @return
            Returns NULL when the given index is out of range.
    */
    ValueSetItem* getItem (sal_uInt16 nIndex) const;

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.
    */
    void ThrowIfDisposed()
        throw (css::lang::DisposedException);

    /** Check whether the value set has a 'none' field, i.e. a field (button)
        that deselects any items (selects none of them).
        @return
            Returns <true/> if there is a 'none' field and <false/> if it is
            missing.
    */
    bool HasNoneField() const;
};

class ValueItemAcc : public ::cppu::WeakImplHelper< css::accessibility::XAccessible,
                                                     css::accessibility::XAccessibleEventBroadcaster,
                                                     css::accessibility::XAccessibleContext,
                                                     css::accessibility::XAccessibleComponent,
                                                     css::lang::XUnoTunnel >
{
private:

    ::std::vector< css::uno::Reference<
        css::accessibility::XAccessibleEventListener > >                mxEventListeners;
    ::osl::Mutex                                                        maMutex;
    ValueSetItem*                                                       mpParent;
    bool                                                                mbIsTransientChildrenDisabled;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

public:

    ValueItemAcc( ValueSetItem* pParent, bool bIsTransientChildrenDisabled );
    virtual ~ValueItemAcc();

    void    ParentDestroyed();

    void    FireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );

    static ValueItemAcc* getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData ) throw();

public:

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw (css::accessibility::IllegalAccessibleComponentStateException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw( css::uno::RuntimeException, std::exception ) override;
};

#endif // INCLUDED_SVTOOLS_SOURCE_CONTROL_VALUEIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
