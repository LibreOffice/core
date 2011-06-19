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

#ifndef _UTL_PROPERTSETINFO_HXX_
#define _UTL_PROPERTSETINFO_HXX_

#include <com/sun/star/uno/Type.h>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>

//=========================================================================
//= property helper classes
//=========================================================================

//... namespace utl .......................................................
namespace utl
{
//.........................................................................

struct PropertyMapEntry
{
    const sal_Char* mpName;
    sal_uInt16 mnNameLen;
    sal_uInt16 mnWhich;
    const com::sun::star::uno::Type* mpType;
    sal_Int16 mnFlags;
    sal_uInt8 mnMemberId;
};

DECLARE_STL_USTRINGACCESS_MAP( PropertyMapEntry*, PropertyMap );

class PropertyMapImpl;

/** this class implements a XPropertySetInfo that is initialized with arrays of PropertyMapEntry.
    It is used by the class PropertySetHelper.
*/
class PropertySetInfo : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
private:
    PropertyMapImpl* mpMap;
public:
    PropertySetInfo() throw();
    virtual ~PropertySetInfo() throw();

    /** returns a stl map with all PropertyMapEntry pointer.<p>
        The key is the property name.
    */
    const PropertyMap* getPropertyMap() const throw();

    /** adds an array of PropertyMapEntry to this instance.<p>
        The end is marked with a PropertyMapEntry where mpName equals NULL
    */
    void add( PropertyMapEntry* pMap ) throw();

    /** removes an already added PropertyMapEntry which string in mpName equals to aName */
    void remove( const rtl::OUString& aName ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//... namespace utl .......................................................

#endif // _UTL_PROPERTSETINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
