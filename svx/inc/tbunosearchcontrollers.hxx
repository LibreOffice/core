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

#ifndef INCLUDED_SVX_INC_TBUNOSEARCHCONTROLLERS_HXX
#define INCLUDED_SVX_INC_TBUNOSEARCHCONTROLLERS_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <cppuhelper/weak.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/window.hxx>

#include <map>
#include <vector>

namespace {

class FindTextFieldControl : public ComboBox
{
public:
    FindTextFieldControl( vcl::Window* pParent, WinBits nStyle,
        css::uno::Reference< css::frame::XFrame >& xFrame,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual bool PreNotify( NotifyEvent& rNEvt ) override;

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

    typedef ::std::vector< css::beans::PropertyValue > SearchToolbarControllersVec;
    typedef ::std::map< css::uno::Reference< css::frame::XFrame >, SearchToolbarControllersVec > SearchToolbarControllersMap;
    SearchToolbarControllersMap aSearchToolbarControllersMap;
    std::vector<OUString> m_aSearchStrings;

};

class FindTextToolbarController : public svt::ToolboxController,
                                  public css::lang::XServiceInfo
{
public:

    FindTextToolbarController( const css::uno::Reference< css::uno::XComponentContext > & rxContext );
    virtual ~FindTextToolbarController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);

private:

    VclPtr<FindTextFieldControl> m_pFindTextFieldControl;

    sal_uInt16 m_nDownSearchId; // item position of findbar
    sal_uInt16 m_nUpSearchId;   // item position of findbar

};

class ExitSearchToolboxController   : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    ExitSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ExitSearchToolboxController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
};

class UpDownSearchToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    enum Type { UP, DOWN };

    UpDownSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext, Type eType );
    virtual ~UpDownSearchToolboxController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

private:
    Type meType;
};

class MatchCaseToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    MatchCaseToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~MatchCaseToolboxController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

private:
    VclPtr<CheckBox> m_pMatchCaseControl;
};

class SearchFormattedToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    SearchFormattedToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~SearchFormattedToolboxController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

private:
    VclPtr<CheckBox> m_pSearchFormattedControl;
};

class FindAllToolboxController   : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    FindAllToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~FindAllToolboxController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
};

class SearchLabelToolboxController : public svt::ToolboxController,
                                     public css::lang::XServiceInfo
{
public:
    SearchLabelToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~SearchLabelToolboxController();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
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
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& sTargetFrameName , sal_Int32 nSearchFlags ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException, std::exception ) override;

    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) throw( css::uno::RuntimeException, std::exception ) override;

private:

    css::uno::Reference< css::frame::XFrame > m_xFrame;

};

}

#endif // INCLUDED_SVX_INC_TBUNOSEARCHCONTROLLERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
