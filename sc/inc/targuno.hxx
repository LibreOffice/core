/*************************************************************************
 *
 *  $RCSfile: targuno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:50 $
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

#ifndef SC_TARGUNO_HXX
#define SC_TARGUNO_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XLINKTARGETSUPPLIER_HPP_
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

class ScDocShell;


#define SC_LINKTARGETTYPE_SHEET     0
#define SC_LINKTARGETTYPE_RANGENAME 1
#define SC_LINKTARGETTYPE_DBAREA    2

#define SC_LINKTARGETTYPE_COUNT     3

#define SCLINKTARGET_SERVICE        "com.sun.star.document.LinkTarget"


//! Graphic / OleObject (need separate collections!)


class ScLinkTargetTypesObj : public ::cppu::WeakImplHelper2<
                                ::com::sun::star::container::XNameAccess,
                                ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    String                  aNames[SC_LINKTARGETTYPE_COUNT];

public:
                            ScLinkTargetTypesObj(ScDocShell* pDocSh);
    virtual                 ~ScLinkTargetTypesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL     getByName(const ::rtl::OUString& aName)
                                throw(  ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL        getElementType(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasElements(void) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL            getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};


class ScLinkTargetTypeObj : public ::cppu::WeakImplHelper3<
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::document::XLinkTargetSupplier,
                                ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    sal_uInt16              nType;
    String                  aName;

public:
                            ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT);
    virtual                 ~ScLinkTargetTypeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    static void             SetLinkTargetBitmap( ::com::sun::star::uno::Any& rRet, sal_uInt16 nType );

                            // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL  getPropertySetInfo(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   setPropertyValue(const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::beans::PropertyVetoException,
                                         ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL     getPropertyValue(const ::rtl::OUString& PropertyName)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           addPropertyChangeListener(const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & xListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           removePropertyChangeListener(const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           addVetoableChangeListener(const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           removeVetoableChangeListener(const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::document::XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL  getLinks(void) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL            getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};


class ScLinkTargetsObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >            xCollection;

public:
                            ScLinkTargetsObj( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & rColl );
    virtual                 ~ScLinkTargetsObj();

                            // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL         getByName(const ::rtl::OUString& aName)
                                throw(  ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                         ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL        getElementType(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasElements(void) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL            getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};


#endif

