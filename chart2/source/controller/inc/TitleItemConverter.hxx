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
#ifndef CHART_TITLEITEMCONVERTER_HXX
#define CHART_TITLEITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vector>
#include <memory>

class SdrModel;

namespace chart
{
namespace wrapper
{

class TitleItemConverter :
        public ::comphelper::ItemConverter
{
public:
    TitleItemConverter(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );

    virtual ~TitleItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

private:
    ::std::vector< ItemConverter * >    m_aConverters;
};

} //  namespace wrapper
} //  namespace chart

// CHART_TITLEITEMCONVERTER_HXX
#endif
