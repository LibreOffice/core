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
#ifndef CHART_ITEMCONVERTER_HXX
#define CHART_ITEMCONVERTER_HXX

#include <unotools/eventlisteneradapter.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

// for pair
#include <utility>

namespace comphelper
{

/** This class serves for conversion between properties of an XPropertySet and
    SfxItems in SfxItemSets.

    With this helper classes, you can feed dialogs with XPropertySets and let
    those modify by the dialogs.

    You must implement GetWhichPairs() such that an SfxItemSet created with
    CreateEmptyItemSet() is able to hold all items that may be mapped.

    You also have to implement GetItemProperty(), in order to return the
    property name for a given which-id together with the corresponding member-id
    that has to be used for conversion in QueryValue/PutValue.

    FillSpecialItem and ApplySpecialItem may be used for special handling of
    individual item, e.g. if you need member-ids in QueryValue/PutValue

    A typical use could be the following:

    ::comphelper::ChartTypeItemConverter aItemConverter( xPropertySet, GetItemPool() );
    SfxItemSet aItemSet = aItemConverter.CreateEmptyItemSet();
    aItemConverter.FillItemSet( aItemSet );
    bool bChanged = false;

    MyDialog aDlg( aItemSet );
    if( aDlg.Execute() == RET_OK )
    {
        const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
        if( pOutItemSet )
            bChanged = aItemConverter.ApplyItemSet( *pOutItemSet );
    }

    if( bChanged )
    {
        [ apply model changes to view ]
    }
 */
class ItemConverter :
        public ::utl::OEventListenerAdapter
{
public:
    /** Construct an item converter that uses the given property set for
        reading/writing converted items
     */
    ItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet ,
            SfxItemPool& rItemPool );
    virtual ~ItemConverter();

    typedef sal_uInt16          tWhichIdType;
    typedef OUString tPropertyNameType;
    typedef sal_uInt8            tMemberIdType;

    typedef ::std::pair< tPropertyNameType, tMemberIdType > tPropertyNameWithMemberId;

    /** applies all properties that can be mapped to items into the given item
        set.

        Call this method before opening a dialog.

        @param rOutItemSet
            the SfxItemSet is filled with all items that are a result of a
            conversion from a property of the internal XPropertySet.
     */
    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;

    /** applies all properties that are results of a conversion from all items
        in rItemSet to the internal XPropertySet.

        Call this method after a dialog was closed with OK

        @return true, if any properties have been changed, false otherwise.
     */
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

    /** creates an empty item set using the given pool or a common pool if empty
        (see GetItemPool) and allowing all items given in the ranges returned by
        GetWhichPairs.
     */
    SfxItemSet CreateEmptyItemSet() const;

    /** Invalidates all items in rDestSet, that are set (state SFX_ITEM_SET) in
        both item sets (rDestSet and rSourceSet) and have differing content.
     */
    static void InvalidateUnequalItems( SfxItemSet &rDestSet, const SfxItemSet &rSourceSet );

protected:
    // ________

    /** implement this method to provide an array of which-ranges of the form:

        const sal_uInt16 aMyPairs[] =
        {
            from_1, to_1,
            from_2, to_2,
            ...
            from_n, to_n,
            0
        };
    */
    virtual const sal_uInt16 * GetWhichPairs() const = 0;

    /** implement this method to return a Property object for a given which id.

        @param rOutProperty
            If true is returned, this contains the property name and the
            corresponding Member-Id.

        @return true, if the item can be mapped to a property.
     */
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const = 0;

    /** for items that can not be mapped directly to a property.

        This method is called from FillItemSet(), if GetItemProperty() returns
        false.

        The default implementation does nothing except showing an assertion
     */
    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );

    /** for items that can not be mapped directly to a property.

        This method is called from ApplyItemSet(), if GetItemProperty() returns
        false.

        The default implementation returns just false and shows an assertion

        @return true if the item changed a property, false otherwise.
     */
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

    // ________

    /// Returns the pool
    SfxItemPool & GetItemPool() const;

    /** Returns the XPropertySet that was given in the CTOR and is used to apply
        items in ApplyItemSet().
     */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >  GetPropertySet() const;

    // ____ ::utl::OEventListenerAdapter ____
    virtual void _disposing( const ::com::sun::star::lang::EventObject& rSource );

protected:
    /** sets a new property set, that you get with GetPropertySet().  It should
        not be necessary to use this method.  It is introduced to allow changing
        the regression type of a regression curve which changes the object
        identity.
     */
    void resetPropertySet( const ::com::sun::star::uno::Reference<
                           ::com::sun::star::beans::XPropertySet > & xPropSet );

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >     m_xPropertySet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo > m_xPropertySetInfo;

    SfxItemPool&                                    m_rItemPool;
    bool                                            m_bIsValid;
};

} //  namespace comphelper

// CHART_ITEMCONVERTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
