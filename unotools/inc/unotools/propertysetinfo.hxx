/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertysetinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:36:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UTL_PROPERTSETINFO_HXX_
#define _UTL_PROPERTSETINFO_HXX_

#include <com/sun/star/uno/Type.h>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase1.hxx>

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

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

