/*************************************************************************
 *
 *  $RCSfile: propacc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SFX_PROPBAG_HXX
#define _SFX_PROPBAG_HXX

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HXX_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HXX_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HXX_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HXX_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#define NS_BEANS    ::com::sun::star::beans
#define NS_LANG     ::com::sun::star::lang
#define NS_UNO      ::com::sun::star::uno

typedef NS_BEANS::PropertyValue* SbPropertyValuePtr;
SV_DECL_PTRARR( SbPropertyValueArr_Impl, SbPropertyValuePtr, 4, 4 );

typedef ::cppu::WeakImplHelper2< NS_BEANS::XPropertySet,
                                 NS_BEANS::XPropertyAccess > SbPropertyValuesHelper;


//==========================================================================

class SbPropertyValues:     public SbPropertyValuesHelper
{
    SbPropertyValueArr_Impl _aPropVals;
    NS_UNO::Reference< ::com::sun::star::beans::XPropertySetInfo > _xInfo;

private:
    INT32                   GetIndex_Impl( const ::rtl::OUString &rPropName ) const;

public:
                            SbPropertyValues();
    virtual                 ~SbPropertyValues();

    // XPropertySet
    virtual NS_UNO::Reference< NS_BEANS::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(void) throw( NS_UNO::RuntimeException );
    virtual void SAL_CALL   setPropertyValue(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Any& aValue);
    virtual NS_UNO::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(  NS_BEANS::UnknownPropertyException,
                NS_LANG::WrappedTargetException,
                NS_UNO::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XPropertyChangeListener >& );
    virtual void SAL_CALL   removePropertyChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XPropertyChangeListener >& );
    virtual void SAL_CALL   addVetoableChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XVetoableChangeListener >& );
    virtual void SAL_CALL   removeVetoableChangeListener(
                                const ::rtl::OUString& aPropertyName,
                                const NS_UNO::Reference< NS_BEANS::XVetoableChangeListener >& );

    // XPropertyAccess
    virtual NS_UNO::Sequence< NS_BEANS::PropertyValue > SAL_CALL getPropertyValues(void);
    virtual void SAL_CALL setPropertyValues(const NS_UNO::Sequence< NS_BEANS::PropertyValue >& PropertyValues_);
};

//==========================================================================

typedef ::cppu::WeakImplHelper1< NS_BEANS::XPropertySetInfo > SbPropertySetInfoHelper;

// AB 20.3.2000 Help Class for XPropertySetInfo implementation
class PropertySetInfoImpl
{
    friend class SbPropertySetInfo;
    friend class SbPropertyContainer;

    NS_UNO::Sequence< NS_BEANS::Property > _aProps;

    sal_Int32 GetIndex_Impl( const ::rtl::OUString &rPropName ) const;

public:
    PropertySetInfoImpl();
    PropertySetInfoImpl( NS_UNO::Sequence< NS_BEANS::Property >& rProps );

    // XPropertySetInfo
    NS_UNO::Sequence< NS_BEANS::Property > SAL_CALL getProperties(void);
    NS_BEANS::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
    sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw ( NS_UNO::RuntimeException );
};

class SbPropertySetInfo:    public SbPropertySetInfoHelper
{
    PropertySetInfoImpl aImpl;

public:
                            SbPropertySetInfo();
                            SbPropertySetInfo( const SbPropertyValueArr_Impl &rPropVals );
    virtual                 ~SbPropertySetInfo();

    // XPropertySetInfo
    virtual NS_UNO::Sequence< NS_BEANS::Property > SAL_CALL getProperties(void)
        throw( NS_UNO::RuntimeException );
    virtual NS_BEANS::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
};

//==========================================================================

typedef ::cppu::WeakImplHelper2< NS_BEANS::XPropertySetInfo, NS_BEANS::XPropertyContainer > SbPropertyContainerHelper;

class SbPropertyContainer: public SbPropertyContainerHelper
{
    PropertySetInfoImpl aImpl;

public:
                            SbPropertyContainer();
    virtual                 ~SbPropertyContainer();

    // XPropertyContainer
    virtual void SAL_CALL addProperty(  const ::rtl::OUString& Name,
                                        INT16 Attributes,
                                        const NS_UNO::Any& DefaultValue)
        throw(  NS_BEANS::PropertyExistException, NS_BEANS::IllegalTypeException,
                NS_LANG::IllegalArgumentException, NS_UNO::RuntimeException );
    virtual void SAL_CALL removeProperty(const ::rtl::OUString& Name)
        throw( NS_BEANS::UnknownPropertyException, NS_UNO::RuntimeException );

    // XPropertySetInfo
    virtual NS_UNO::Sequence< NS_BEANS::Property > SAL_CALL getProperties(void);
    virtual NS_BEANS::Property SAL_CALL getPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& Name)
        throw( NS_UNO::RuntimeException );

    // XPropertyAccess
    virtual NS_UNO::Sequence< NS_BEANS::PropertyValue > SAL_CALL getPropertyValues(void);
    virtual void SAL_CALL setPropertyValues(const NS_UNO::Sequence< NS_BEANS::PropertyValue >& PropertyValues_);
};

//=========================================================================

class StarBASIC;
class SbxArray;

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );


#undef NS_BEANS
#undef NS_LANG
#undef NS_UNO



#endif

