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
#ifndef CHART_CHARACTERPROPERTYITEMCONVERTER_HXX
#define CHART_CHARACTERPROPERTYITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <memory>

namespace chart
{
namespace wrapper
{

class CharacterPropertyItemConverter :
        public ::comphelper::ItemConverter
{
public:
    CharacterPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    CharacterPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize,
        const OUString & rRefSizePropertyName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rRefSizePropSet =
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~CharacterPropertyItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >  GetRefSizePropertySet() const;

private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::com::sun::star::awt::Size > m_pRefSize;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    OUString                                m_aRefSizePropertyName;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >    m_xRefSizePropSet;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARACTERPROPERTYITEMCONVERTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
