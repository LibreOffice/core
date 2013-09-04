/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/window.hxx>

#include <map>
#include <vector>

namespace svx
{

class FindTextFieldControl : public ComboBox
{
public:
    FindTextFieldControl( Window* pParent, WinBits nStyle,
        css::uno::Reference< css::frame::XFrame >& xFrame,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~FindTextFieldControl();

    virtual long PreNotify( NotifyEvent& rNEvt );

    void Remember_Impl(const OUString& rStr);
    void SetTextToSelected_Impl();

private:

    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

class SearchToolbarControllersManager
{
public:

    SearchToolbarControllersManager();
    ~SearchToolbarControllersManager();

    static SearchToolbarControllersManager& createControllersManager();

    void registryController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const OUString& sCommandURL );
    void freeController ( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const OUString& sCommandURL );
    css::uno::Reference< css::frame::XStatusListener > findController( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& sCommandURL );

    void saveSearchHistory(const FindTextFieldControl* m_pFindTextFieldControl);
    void loadSearchHistory(FindTextFieldControl* m_pFindTextFieldControl);

private:

    typedef ::comphelper::SequenceAsVector< css::beans::PropertyValue > SearchToolbarControllersVec;
    typedef ::std::map< css::uno::Reference< css::frame::XFrame >, SearchToolbarControllersVec > SearchToolbarControllersMap;
    SearchToolbarControllersMap aSearchToolbarControllersMap;
    std::vector<OUString> m_aSearchStrings;

};

class FindTextToolbarController : public svt::ToolboxController,
                                  public css::lang::XServiceInfo
{
public:

    FindTextToolbarController( const css::uno::Reference< css::uno::XComponentContext > & rxContext );
    ~FindTextToolbarController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static OUString getImplementationName_Static() throw()
    {
        return OUString("com.sun.star.svx.FindTextToolboxController");
    }

    static ::com::sun::star::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

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

class ExitSearchToolboxController   : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    ExitSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    ~ExitSearchToolboxController();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static OUString getImplementationName_Static( ) throw()
    {
        return OUString( "com.sun.star.svx.ExitFindbarToolboxController" );
    }

    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException);

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException );
};

class UpDownSearchToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    enum Type { UP, DOWN };

    UpDownSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext, Type eType );
    ~UpDownSearchToolboxController();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static OUString getImplementationName_Static( Type eType ) throw()
    {
        return eType == UP? OUString( "com.sun.star.svx.UpSearchToolboxController" ) :
                            OUString( "com.sun.star.svx.DownSearchToolboxController" );
    }

    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

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

class MatchCaseToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    MatchCaseToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    ~MatchCaseToolboxController();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static OUString getImplementationName_Static( ) throw()
    {
        return OUString( "com.sun.star.svx.MatchCaseToolboxController" );
    }

    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException);

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException );

private:
    CheckBox* m_pMatchCaseControl;
};

class FindAllToolboxController   : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    FindAllToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    ~FindAllToolboxController();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static OUString getImplementationName_Static( ) throw()
    {
        return OUString( "com.sun.star.svx.FindAllToolboxController" );
    }

    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException);

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException );

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException );
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

    FindbarDispatcher();
    virtual ~FindbarDispatcher();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    static OUString getImplementationName_Static() throw()
    {
        return OUString("com.sun.star.comp.svx.Impl.FindbarDispatcher");
    }

    static css::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException );

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& sTargetFrameName , sal_Int32 nSearchFlags ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException );

    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException );
    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException );

private:

    css::uno::Reference< css::frame::XFrame > m_xFrame;

};

// createInstance
css::uno::Reference< css::uno::XInterface > SAL_CALL FindTextToolbarController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL DownSearchToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL UpSearchToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL MatchCaseToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL FindAllToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL ExitFindbarToolboxController_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
css::uno::Reference< css::uno::XInterface > SAL_CALL FindbarDispatcher_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );

}

#endif // __TBUNOSEARCHCONTROLLERS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
