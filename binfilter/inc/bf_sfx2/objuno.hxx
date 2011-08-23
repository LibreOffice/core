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

#ifndef _SVSTOR_HXX //autogen
#include <bf_so3/svstor.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTANDALONEDOCUMENTINFO_HPP_
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_PAPERFORMAT_HPP_
#include <com/sun/star/view/PaperFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XPrintSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTPREVIEW_HPP_
#include <com/sun/star/view/XPrintPreview.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XCONTROLACCESS_HPP_
#include <com/sun/star/view/XControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_DOCUMENTZOOMTYPE_HPP_
#include <com/sun/star/view/DocumentZoomType.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSCREENCURSOR_HPP_
#include <com/sun/star/view/XScreenCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
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
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENERADAPTERSERVICE_HPP_
#include <com/sun/star/script/XAllListenerAdapterService.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENER_HPP_
#include <com/sun/star/script/XAllListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_ALLEVENTOBJECT_HPP_
#include <com/sun/star/script/AllEventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#include <com/sun/star/io/IOException.hpp>

#include <bf_svtools/itemprop.hxx>
#include <bf_svtools/lstner.hxx>
#ifndef _SFX_SFXBASEMODEL_HXX_
#include <bf_sfx2/sfxbasemodel.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
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
