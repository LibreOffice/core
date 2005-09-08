/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertycontainer.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:35:40 $
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

#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#define _COMPHELPER_PROPERTYCONTAINER_HXX_

#ifndef COMPHELPER_PROPERTYCONTAINERHELPER_HXX
#include <comphelper/propertycontainerhelper.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

//==========================================================================
//= OPropertyContainer
//==========================================================================
typedef ::cppu::OPropertySetHelper OPropertyContainer_Base;
/** a OPropertySetHelper implementation which is just a simple container for properties represented
    by class members, usually in a derived class.
    <BR>
    A restriction of this class is that no value conversions are made on a setPropertyValue call. Though
    the base class supports this with the convertFastPropertyValue method, the OPropertyContainer accepts only
    values which already have the correct type, it's unable to convert, for instance, a long to a short.
*/
class COMPHELPER_DLLPUBLIC OPropertyContainer
            :public OPropertyContainer_Base
            ,public OPropertyContainerHelper
{
public:
    // this dtor is needed otherwise we can get a wrong delete operator
    virtual ~OPropertyContainer();

protected:
    OPropertyContainer(::cppu::OBroadcastHelper& _rBHelper);

    /// for scripting : the types of the interfaces supported by this class
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

// OPropertySetHelper overridables
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException);

    virtual void SAL_CALL   setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                            )
                            throw (::com::sun::star::uno::Exception);

    virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const;

    // disambiguate a base class method (XFastPropertySet)
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // still waiting to be overridden
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException) = 0;
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTYCONTAINER_HXX_


