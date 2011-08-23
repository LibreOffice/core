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
#ifndef _SFX_OBJUNO_HXX
#define _SFX_OBJUNO_HXX

#include <bf_so3/svstor.hxx>

#include <cppuhelper/weakref.hxx>
#include <com/sun/star/document/XDocumentInfo.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/view/XPrintSettingsSupplier.hpp>
#include <com/sun/star/view/XPrintPreview.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/script/XAllListenerAdapterService.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/AllEventObject.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <com/sun/star/io/IOException.hpp>

#include <bf_svtools/itemprop.hxx>
#include <bf_svtools/lstner.hxx>
#include <bf_sfx2/sfxbasemodel.hxx>
#include <tools/datetime.hxx>
#include <bf_sfx2/sfxuno.hxx>
#include <bf_sfx2/objsh.hxx>
namespace binfilter {

class SfxDocumentInfo	;
class SfxMedium			;
class SfxFilter			;

class SfxDocumentInfoObject: public ::com::sun::star::lang::XTypeProvider		,
                             public ::com::sun::star::document::XDocumentInfo		,
                             public ::com::sun::star::lang::XComponent			,
                             public ::com::sun::star::beans::XPropertySet		,
                             public ::com::sun::star::beans::XFastPropertySet	,
                             public ::com::sun::star::beans::XPropertyAccess		,
                             public ::cppu::OWeakObject
{
    SfxItemPropertySet						_aPropSet	;
    sal_Bool								_bStandalone;

protected:
    struct SfxDocumentInfoObject_Impl*		_pImp		;
    SfxDocumentInfo*						_pInfo		;
    const SfxFilter*						_pFilter	;
    ::com::sun::star::uno::WeakReference < ::com::sun::star::frame::XModel > _wModel;

    SfxDocumentInfoObject( sal_Bool bStandalone );
public:
    SfxDocumentInfoObject( SfxObjectShell *pObjSh );
    ~SfxDocumentInfoObject();

    // XInterface, XTypeProvider
    SFX_DECL_XINTERFACE_XTYPEPROVIDER

//ASDBG	virtual void* getImplementation(Reflection *p)
//ASDBG	{ return OWeakObject::getImplementation(p); }

    // XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw( ::com::sun::star::uno::RuntimeException );

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::beans::XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::beans::XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::document::XDocumentInfo
    virtual sal_Int16 SAL_CALL getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldValue(sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::util::DateTime impl_DateTime_Object2Struct( const DateTime& aDateTimeObject );
    static DateTime impl_DateTime_Struct2Object ( const ::com::sun::star::util::DateTime& aDateTimeStruct );
};

class SfxStandaloneDocumentInfoObject: public SfxDocumentInfoObject		,
                                       public ::com::sun::star::lang::XServiceInfo				,
                                       public ::com::sun::star::document::XStandaloneDocumentInfo
{
    SfxMedium*				_pMedium;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > _xFactory;
private:
    SvStorage*				GetStorage_Impl( const String& rName, sal_Bool bWrite );

public:
    SfxStandaloneDocumentInfoObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual ~SfxStandaloneDocumentInfoObject();

    // XInterface, XTypeProvider, XServiceInfo
    SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO

// ÎMPLNAME L"com.sun.star.comp.sfx2.StandaloneDocumentInfo"
//ASDBG	virtual void* getImplementation(Reflection *p)
//ASDBG	{ return Sf::com::sun::star::document::XDocumentInfoObject::getImplementation(p); }

    // ::com::sun::star::document::XDocumentInfo
    virtual sal_Int16 SAL_CALL getUserFieldCount() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldName(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getUserFieldValue(sal_Int16 nIndex) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldName(sal_Int16 nIndex, const ::rtl::OUString& aName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setUserFieldValue(sal_Int16 nIndex, const ::rtl::OUString& aValue ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::document::XStandaloneDocumentInfo
    virtual void SAL_CALL loadFromURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException );
    virtual void SAL_CALL storeIntoURL(const ::rtl::OUString& aURL) throw( ::com::sun::star::io::IOException );
};

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL bf_BinaryDocInfo_createInstance(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw( com::sun::star::uno::Exception );

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
