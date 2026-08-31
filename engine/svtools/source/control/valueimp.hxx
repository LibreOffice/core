/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <tools/color.hxx>
#include <vcl/image.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/OAccessible.hxx>
#include <comphelper/compbase.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

#include <mutex>
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

class ValueItemAcc;
class ValueSet;

struct ValueSetItem
{
    ValueSet&        mrParent;
    OUString            maText;
    void*               mpData;
    rtl::Reference< ValueItemAcc > mxAcc;
    Image               maImage;
    Color               maColor;
    sal_uInt16          mnId;
    sal_uInt8           meType;
    bool                mbVisible;

    explicit ValueSetItem( ValueSet& rParent );
    ~ValueSetItem();

    const rtl::Reference<ValueItemAcc>& GetAccessible(bool bCreate = true);
};

class ValueSetAcc final
    : public cppu::ImplInheritanceHelper<comphelper::OAccessible,
                                         css::accessibility::XAccessibleSelection>
{
public:

    explicit ValueSetAcc(ValueSet* pValueSet);
    virtual ~ValueSetAcc() override;

    void                FireAccessibleEvent( short nEventId, const cpo::uno::Any& rOldValue, const cpo::uno::Any& rNewValue );
    bool                HasAccessibleListeners() const;

public:

    /** Called by the corresponding ValueSet when it gets the focus.
        Stores the new focus state and broadcasts a state change event.
    */
    void GetFocus();

    /** Called by the corresponding ValueSet when it loses the focus.
        Stores the new focus state and broadcasts a state change event.
    */
    void LoseFocus();

    /** Called by the corresponding ValueSet when it gets destroyed. */
    void Invalidate();

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent(  ) override;
    virtual sal_Int64 getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
    virtual sal_Int64 getAccessibleStateSet(  ) override;
    virtual css::lang::Locale getLocale(  ) override;

    // OAccessible
    virtual css::awt::Rectangle implGetBounds() override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;

    // XAccessibleSelection
    virtual void selectAccessibleChild( sal_Int64 nChildIndex ) override;
    virtual bool isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    virtual void clearAccessibleSelection(  ) override;
    virtual void selectAllAccessibleChildren(  ) override;
    virtual sal_Int64 getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    virtual void deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

private:
    ValueSet* mpValueSet;
    /// The current FOCUSED state.
    bool mbIsFocused;

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
        @throws css::lang::DisposedException
    */
    void ThrowIfDisposed();

    /** Check whether the value set has a 'none' field, i.e. a field (button)
        that deselects any items (selects none of them).
        @return
            Returns <true/> if there is a 'none' field and <false/> if it is
            missing.
    */
    bool HasNoneField() const;
};

class ValueItemAcc : public comphelper::OAccessible
{
private:
    ValueSetItem*                                                       mpValueSetItem;

public:

    ValueItemAcc(ValueSetItem* pValueSetItem);
    virtual ~ValueItemAcc() override;

    void    ValueSetItemDestroyed();

    void    FireAccessibleEvent( short nEventId, const cpo::uno::Any& rOldValue, const cpo::uno::Any& rNewValue );

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleParent(  ) override;
    virtual sal_Int64 getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 getAccessibleRole(  ) override;
    virtual OUString getAccessibleDescription(  ) override;
    virtual OUString getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > getAccessibleRelationSet(  ) override;
    virtual sal_Int64 getAccessibleStateSet(  ) override;
    virtual css::lang::Locale getLocale(  ) override;

    // OAccessible
    virtual css::awt::Rectangle implGetBounds() override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void grabFocus(  ) override;
    virtual sal_Int32 getForeground(  ) override;
    virtual sal_Int32 getBackground(  ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
