/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertymap.hxx,v $
 * $Revision: 1.4 $
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

#ifndef OOX_HELPER_PROPERTYMAP_HXX
#define OOX_HELPER_PROPERTYMAP_HXX

#include <map>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include "oox/helper/helper.hxx"

namespace oox {

typedef std::map< ::rtl::OUString, com::sun::star::uno::Any > PropertyMapBase;

class PropertyMap : public PropertyMapBase
{
public:
    bool hasProperty( const ::rtl::OUString& rName ) const;
    const com::sun::star::uno::Any* getPropertyValue( const ::rtl::OUString& rName ) const;

    template< typename Type >
    inline void setProperty( const ::rtl::OUString& rName, const Type& rValue )
        { if( rName.getLength() > 0 ) (*this)[ rName ] <<= rValue; }

    void makeSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rSequence ) const;
    void makeSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rSequence ) const;
    void makeSequence( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames,
                       ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > makePropertySet() const;
    void dump_debug(const char *pMessage = NULL);
};

} // namespace oox

#endif

