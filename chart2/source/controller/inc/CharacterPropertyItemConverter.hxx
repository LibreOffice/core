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
    CharacterPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize,
        const ::rtl::OUString & rRefSizePropertyName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rRefSizePropSet =
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >() );
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
    ::std::auto_ptr< ::com::sun::star::awt::Size > m_pRefSize;
    ::rtl::OUString                                m_aRefSizePropertyName;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >    m_xRefSizePropSet;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARACTERPROPERTYITEMCONVERTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
