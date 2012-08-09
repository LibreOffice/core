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

#ifndef __TBUNOSEARCHCONTROLLERS_HXX_
#define __TBUNOSEARCHCONTROLLERS_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <comphelper/sequenceasvector.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weak.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/combobox.hxx>
#include <vcl/window.hxx>

#include <map>
#include <vector>

namespace css = ::com::sun::star        ;
namespace svx
{

class FindTextFieldControl : public ComboBox
{
public:
    FindTextFieldControl( Window* pParent, WinBits nStyle,
        css::uno::Reference< css::frame::XFrame >& xFrame,
        css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
    virtual ~FindTextFieldControl();

    virtual void Modify();
    virtual long PreNotify( NotifyEvent& rNEvt );

    void InitControls_Impl();
    void Remember_Impl(const String& rStr);

private:

    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceManager;
    sal_Bool m_bToClearTextField;

};

class SearchToolbarControllersManager
{
public:

    SearchToolbarControllersManager();
    ~SearchToolbarControllersManager();

    static SearchToolbarControllersManager& createControllersManager();

    void registryController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const ::rtl::OUString& sCommandURL );
    void freeController ( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const ::rtl::OUString& sCommandURL );
    css::uno::Reference< css::frame::XStatusListener > findController( const css::uno::Reference< css::frame::XFrame >& xFrame, const ::rtl::OUString& sCommandURL );

    void saveSearchHistory(const FindTextFieldControl* m_pFindTextFieldControl);
    void loadSearchHistory(FindTextFieldControl* m_pFindTextFieldControl);

private:

    typedef ::comphelper::SequenceAsVector< css::beans::PropertyValue > SearchToolbarControllersVec;
    typedef ::std::map< css::uno::Reference< css::frame::XFrame >, SearchToolbarControllersVec > SearchToolbarControllersMap;
    SearchToolbarControllersMap aSearchToolbarControllersMap;
    std::vector<rtl::OUString> m_aSearchStrings;

};

class FindTextToolbarController : public svt::ToolboxController,
                                  public css::lang::XServiceInfo
{
public:

    FindTextToolbarController( const css::uno::Reference< css::lang::XMultiServiceFactory > & rServiceManager );
    ~FindTextToolbarController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static ::rtl::OUString getImplementationName_Static() throw()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.svx.FindTextToolboxController" ));
    }

    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException );

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException);
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException );

    DECL_LINK(EditModifyHdl, void*);

private:

    FindTextFieldControl* m_pFindTextFieldControl;

    sal_uInt16 m_nDownSearchId; // item position of findbar
    sal_uInt16 m_nUpSearchId;   // item position of findbar

};

class UpDownSearchToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    enum Type { UP, DOWN };

    UpDownSearchToolboxController( const css::uno::Reference< css::lang::XMultiServiceFactory >& rServiceManager, Type eType );
    ~UpDownSearchToolboxController();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static ::rtl::OUString getImplementationName_Static( Type eType ) throw()
    {
        return eType == UP? ::rtl::OUString( "com.sun.star.svx.UpSearchToolboxController" ) :
                            ::rtl::OUString( "com.sun.star.svx.DownSearchToolboxController" );
    }

    static css::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException);

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException );

private:
    Type meType;
};

// protocol handler for "vnd.sun.star.findbar:*" URLs
// The dispatch object will be used for shortcut commands for findbar
class FindbarDispatcher : public css::lang::XServiceInfo,
                          public css::lang::XInitialization,
                          public css::frame::XDispatchProvider,
                          public css::frame::XDispatch,
                          public ::cppu::OWeakObject
{
public:

    FindbarDispatcher( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
    virtual ~FindbarDispatcher();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static ::rtl::OUString getImplementationName_Static() throw()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.Impl.FindbarDispatcher" ));
    }

    static css::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException );

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const ::rtl::OUString& sTargetFrameName , sal_Int32 nSearchFlags ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException );

    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException );
    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException );

private:

    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
    css::uno::Reference< css::frame::XFrame > m_xFrame;

};

// createInstance
css::uno::Reference< css::uno::XInterface > SAL_CALL FindTextToolbarController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL DownSearchToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL UpSearchToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL FindbarDispatcher_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );

}

#endif // __TBUNOSEARCHCONTROLLERS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
