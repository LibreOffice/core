/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysetinfo.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#define _COMPHELPER_PROPERTSETINFO_HXX_

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>
#include "comphelper/comphelperdllapi.h"

//=========================================================================
//= property helper classes
//=========================================================================

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

struct PropertyMapEntry
{
    const sal_Char* mpName;
    sal_uInt16 mnNameLen;
    sal_Int32 mnHandle;
    const com::sun::star::uno::Type* mpType;
    sal_Int16 mnAttributes;
    sal_uInt8 mnMemberId;
};

DECLARE_STL_USTRINGACCESS_MAP( PropertyMapEntry*, PropertyMap );

class PropertyMapImpl;

/** this class implements a XPropertySetInfo that is initialized with arrays of PropertyMapEntry.
    It is used by the class PropertySetHelper.
*/
class COMPHELPER_DLLPUBLIC PropertySetInfo : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
private:
    PropertyMapImpl* mpMap;
public:
    PropertySetInfo() throw();
    PropertySetInfo( PropertyMapEntry* pMap ) throw();
    virtual ~PropertySetInfo() throw();

    /** returns a stl map with all PropertyMapEntry pointer.<p>
        The key is the property name.
    */
    const PropertyMap* getPropertyMap() const throw();

    /** adds an array of PropertyMapEntry to this instance.<p>
        The end is marked with a PropertyMapEntry where mpName equals NULL</p>
    */
    void add( PropertyMapEntry* pMap ) throw();

    /** adds an array of PropertyMapEntry to this instance

        <p>At most the number of entries given will be added, if no terminating entry (<code>mpName == <NULL/></code>) is encountered.</p>

        <p>If <arg>nCount</arg> is less than 0, it is ignored and all entries (up to, but not including, the terminating
        one) are added.</p>
    */
    void add( PropertyMapEntry* pMap, sal_Int32 nCount ) throw();

    /** removes an already added PropertyMapEntry which string in mpName equals to aName */
    void remove( const rtl::OUString& aName ) throw();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _UTL_PROPERTSETINFO_HXX_

