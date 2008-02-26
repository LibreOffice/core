/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objuno.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:58:37 $
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
#ifndef _SFX_OBJUNO_HXX
#define _SFX_OBJUNO_HXX

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTANDALONEDOCUMENTINFO_HPP_
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTPROPERTIESSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#include <com/sun/star/io/IOException.hpp>

#include <svtools/itemprop.hxx>
#include <cppuhelper/implbase10.hxx>

#include "sfxuno.hxx"


// this is now just a wrapper around a XDocumentProperties instance

class SAL_DLLPRIVATE SfxDocumentInfoObject: public ::cppu::WeakImplHelper10<
        ::com::sun::star::document::XDocumentInfo,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XFastPropertySet,
        ::com::sun::star::beans::XPropertyAccess,
        ::com::sun::star::beans::XPropertyContainer,
        ::com::sun::star::document::XDocumentPropertiesSupplier,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::util::XCloneable>
{
protected:
    struct SfxDocumentInfoObject_Impl* _pImp;

public:
    SfxDocumentInfoObject();
    ~SfxDocumentInfoObject();

    // XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( ::com::sun::star::uno::RuntimeException );

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::beans::PropertyVetoException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);

    // ::com::sun::star::beans::XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::beans::PropertyVetoException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::WrappedTargetException);
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw(
        ::com::sun::star::uno::RuntimeException,
        ::com::sun::star::beans::UnknownPropertyException,
        ::com::sun::star::lang::WrappedTargetException);

    // ::com::sun::star::beans::XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::beans::XPropertyContainer
    virtual void SAL_CALL addProperty( const ::rtl::OUString& Name, ::sal_Int16 Attributes, const ::com::sun::star::uno::Any& DefaultValue ) throw (::com::sun::star::beans::PropertyExistException, ::com::sun::star::beans::IllegalTypeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeProperty( const ::rtl::OUString& Name ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::NotRemoveableException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::document::XDocumentInfo
    virtual sal_Int16 SAL_CALL getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldValue(sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::document::XDocumentPropertiesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties >
        SAL_CALL getDocumentProperties()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifiable
    virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const com::sun::star::uno::Reference< com::sun::star::util::XModifyListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException ) ;
    virtual void SAL_CALL removeModifyListener( const com::sun::star::uno::Reference< com::sun::star::util::XModifyListener > & xListener) throw( ::com::sun::star::uno::RuntimeException ) ;

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments)
        throw (com::sun::star::uno::RuntimeException,
               com::sun::star::uno::Exception);

    // ::com::sun::star::util::XCloneable:
    virtual com::sun::star::uno::Reference<com::sun::star::util::XCloneable> SAL_CALL createClone()
        throw (com::sun::star::uno::RuntimeException);

    const SfxDocumentInfoObject& operator=( const SfxDocumentInfoObject & rOther);
};

class SfxStandaloneDocumentInfoObject: public SfxDocumentInfoObject,
                                       public ::com::sun::star::lang::XServiceInfo,
                                       public ::com::sun::star::document::XStandaloneDocumentInfo
{
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > _xFactory;

public:
    SfxStandaloneDocumentInfoObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual ~SfxStandaloneDocumentInfoObject();

    void Clear();

    // XInterface, XTypeProvider, XServiceInfo
    SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO

    // ::com::sun::star::document::XDocumentInfo
    virtual sal_Int16 SAL_CALL getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldValue(sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::document::XStandaloneDocumentInfo
    virtual void SAL_CALL loadFromURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL storeIntoURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException );
};

#endif
