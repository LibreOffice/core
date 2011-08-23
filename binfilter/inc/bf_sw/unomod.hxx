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
#ifndef _UNOMOD_HXX
#define _UNOMOD_HXX

#ifndef _COM_SUN_STAR_TEXT_XMODULE_HPP_
#include <com/sun/star/text/XModule.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XPrintSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XVIEWSETTINGSSUPPLIER_HPP_
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>	// helper for implementations
#endif
#ifndef _COMPHELPER_CHAINABLEPROPERTYSET_HXX_
#include <comphelper/ChainablePropertySet.hxx>
#endif
#ifndef _COMPHELPER_SETTINGSHELPER_HXX_
#include <comphelper/SettingsHelper.hxx>
#endif
namespace binfilter {

class SwView;
class SwViewOption;
struct SwPrintData;
class SwDoc;

/******************************************************************************
 *
 ******************************************************************************/
/*-----------------15.03.98 13:21-------------------

--------------------------------------------------*/

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SwXModule_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );

class SwXModule : public cppu::WeakImplHelper4
<
    ::com::sun::star::text::XModule,
    ::com::sun::star::view::XViewSettingsSupplier,
    ::com::sun::star::view::XPrintSettingsSupplier,
    ::com::sun::star::lang::XServiceInfo
>
{

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > * 	pxViewSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > * 	pxPrintSettings;

protected:
    virtual ~SwXModule();
public:
    SwXModule();
    

    //XViewSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getViewSettings(void) 
        throw( ::com::sun::star::uno::RuntimeException );

    //XPrintSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getPrintSettings(void) 
        throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) 
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) 
        throw( ::com::sun::star::uno::RuntimeException );
};

enum SwXPrintSettingsType
{
    PRINT_SETTINGS_MODULE,
    PRINT_SETTINGS_WEB,
    PRINT_SETTINGS_DOCUMENT
};

class SwXPrintSettings : public comphelper::ChainableHelperNoState
{
    friend class SwXDocumentSettings;
protected:
    SwXPrintSettingsType meType;
    SwPrintData * mpPrtOpt;
    SwDoc *mpDoc;

    virtual void _preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue ) 
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual void _preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue ) 
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual ~SwXPrintSettings()
        throw();
public:
    SwXPrintSettings( SwXPrintSettingsType eType, SwDoc * pDoc = NULL );
    

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) 
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) 
        throw( ::com::sun::star::uno::RuntimeException );
};
/*-----------------15.03.98 13:21-------------------

--------------------------------------------------*/
class SwXViewSettings : public comphelper::ChainableHelperNoState
{

    friend class SwXDocumentSettings;
protected:
    SwView*						pView;
    SwViewOption* 		mpViewOption;
    const SwViewOption* 		mpConstViewOption;
    sal_Bool					bObjectValid:1, bWeb:1, mbApplyZoom;
    virtual void _preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue ) 
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual void _preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue ) 
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual ~SwXViewSettings()
        throw();
public:
    SwXViewSettings(sal_Bool bWeb, SwView*	pView);
    

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    sal_Bool 	IsValid() const {return bObjectValid;}
    void	Invalidate() {bObjectValid = sal_False;}
};
/* -----------------24.09.98 10:47-------------------
 *
 * --------------------------------------------------*/
/*class SwXTerminateListener : public ::com::sun::star::frame::XTerminateListener,
                                public UsrObject
{
public:
    SwXTerminateListener();
    virtual ~SwXTerminateListener();

    SMART_UNO_DECLARATION( SwXTerminateListener, UsrObject );

    virtual	UString 				getClassName();
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//	virtual ::com::sun::star::uno::XInterface *			queryInterface( ::com::sun::star::uno::Uik aUik);

// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass > >	getIdlClasses(void);


    //XTerminateListener
    virtual void queryTermination(const ::com::sun::star::lang::EventObject& aEvent);
    virtual void notifyTermination(const ::com::sun::star::lang::EventObject& aEvent);

    //XEventListener
    virtual void disposing(const ::com::sun::star::lang::EventObject& Source);
};

*/
} //namespace binfilter
#endif
