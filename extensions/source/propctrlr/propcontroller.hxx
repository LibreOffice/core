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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPCONTROLLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPCONTROLLER_HXX

#include "composeduiupdate.hxx"
#include "formbrowsertools.hxx"
#include "formmetadata.hxx"
#include "proplinelistener.hxx"
#include "propcontrolobserver.hxx"
#include "browserview.hxx"
#include "modulepcr.hxx"
#include "propertyinfo.hxx"

#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/inspection/XPropertyControlFactory.hpp>
#include <com/sun/star/inspection/XObjectInspector.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace vcl { class Window; }


namespace pcr
{


    class OPropertyEditor;
    struct OLineDescriptor;

    typedef ::cppu::WeakImplHelper <   css::lang::XServiceInfo
                                    ,   css::awt::XFocusListener
                                    ,   css::awt::XLayoutConstrains
                                    ,   css::beans::XPropertyChangeListener
                                    ,   css::inspection::XPropertyControlFactory
                                    ,   css::inspection::XObjectInspector
                                    ,   css::lang::XInitialization
                                    >   OPropertyBrowserController_Base;

    class OPropertyBrowserController
                :public ::comphelper::OMutexAndBroadcastHelper
                ,public OPropertyBrowserController_Base
                ,public css::inspection::XObjectInspectorUI
                    // that's intentionally *not* part of the OPropertyBrowserController_Base
                    // We do not want this to be available in queryInterface, getTypes, and the like.
                ,public IPropertyLineListener
                ,public IPropertyControlObserver
                ,public IPropertyExistenceCheck
    {
    private:
        typedef ::std::multimap< sal_Int32, css::beans::Property >  OrderedPropertyMap;
        typedef ::std::vector< css::uno::Reference< css::uno::XInterface > >
                                                                            InterfaceArray;

    protected:
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

    private:
        css::uno::Reference< css::frame::XFrame > m_xFrame;
        css::uno::Reference< css::awt::XWindow >  m_xView;

        ::cppu::OInterfaceContainerHelper   m_aDisposeListeners;
        ::cppu::OInterfaceContainerHelper   m_aControlObservers;
        // meta data about the properties
        VclPtr<OPropertyBrowserView>        m_pView;

        OUString                     m_sPageSelection;
        OUString                     m_sLastValidPageSelection;

        typedef css::uno::Reference< css::inspection::XPropertyHandler >
                                                        PropertyHandlerRef;
        typedef ::std::vector< PropertyHandlerRef >     PropertyHandlerArray;
        typedef std::unordered_map< OUString, PropertyHandlerRef, OUStringHash >
                                                        PropertyHandlerRepository;
        typedef std::unordered_multimap< OUString, PropertyHandlerRef, OUStringHash >
                                                        PropertyHandlerMultiRepository;
        PropertyHandlerRepository                       m_aPropertyHandlers;
        PropertyHandlerMultiRepository                  m_aDependencyHandlers;
        PropertyHandlerRef                              m_xInteractiveHandler;

        ::std::unique_ptr< ComposedPropertyUIUpdate >   m_pUIRequestComposer;

        /// our InspectorModel
        css::uno::Reference< css::inspection::XObjectInspectorModel >
                                                        m_xModel;
        /// the object(s) we're currently inspecting
        InterfaceArray                                  m_aInspectedObjects;
        /// the properties of the currently inspected object(s)
        OrderedPropertyMap                              m_aProperties;
        /// the property we're just committing
        OUString                                        m_sCommittingProperty;

        typedef std::unordered_map< OUString, sal_uInt16, OUStringHash >     HashString2Int16;
        HashString2Int16                                m_aPageIds;

        bool        m_bContainerFocusListening;
        bool        m_bSuspendingPropertyHandlers;
        bool        m_bConstructed;
        bool        m_bBindingIntrospectee;

    protected:
        DECLARE_XINTERFACE()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        // XController
        virtual void SAL_CALL attachFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL attachModel( const css::uno::Reference< css::frame::XModel >& xModel ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL suspend( sal_Bool bSuspend ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getViewData(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL restoreViewData( const css::uno::Any& Data ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame(  ) throw(css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

        // XFocusListener
        virtual void SAL_CALL focusGained( const css::awt::FocusEvent& _rSource ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL focusLost( const css::awt::FocusEvent& _rSource ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        // XLayoutConstrains
        virtual css::awt::Size SAL_CALL getMinimumSize(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Size SAL_CALL getPreferredSize(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw (css::uno::RuntimeException, std::exception) override;

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& _rEvent ) throw (css::uno::RuntimeException, std::exception) override;

        /** XPropertyControlFactory
        */
        virtual css::uno::Reference< css::inspection::XPropertyControl > SAL_CALL createPropertyControl( ::sal_Int16 ControlType, sal_Bool CreateReadOnly ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    public:
        explicit OPropertyBrowserController(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

    protected:
        virtual ~OPropertyBrowserController();

    public:
        // XServiceInfo - static versions
        static OUString getImplementationName_static(  ) throw(css::uno::RuntimeException);
        static css::uno::Sequence< OUString > getSupportedServiceNames_static(  ) throw(css::uno::RuntimeException);
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
                        Create(const css::uno::Reference< css::uno::XComponentContext >&);

    protected:
        // IPropertyLineListener
        virtual void    Clicked(    const OUString& _rName, bool _bPrimary ) override;
        virtual void    Commit(     const OUString& _rName, const css::uno::Any& _rVal ) override;

        // IPropertyControlObserver
        virtual void    focusGained( const css::uno::Reference< css::inspection::XPropertyControl >& _Control ) override;
        virtual void    valueChanged( const css::uno::Reference< css::inspection::XPropertyControl >& _Control ) override;

        // IPropertyExistenceCheck
        virtual bool SAL_CALL hasPropertyByName( const OUString& _rName ) throw (css::uno::RuntimeException) override;

        // XObjectInspectorUI
        virtual void SAL_CALL enablePropertyUI( const OUString& _rPropertyName, sal_Bool _bEnable ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL enablePropertyUIElements( const OUString& _rPropertyName, ::sal_Int16 _nElements, sal_Bool _bEnable ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL rebuildPropertyUI( const OUString& _rPropertyName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL showPropertyUI( const OUString& _rPropertyName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL hidePropertyUI( const OUString& _rPropertyName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL showCategory( const OUString& _rCategory, sal_Bool _bShow ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::inspection::XPropertyControl > SAL_CALL getPropertyControl( const OUString& _rPropertyName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL registerControlObserver( const css::uno::Reference< css::inspection::XPropertyControlObserver >& _Observer ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL revokeControlObserver( const css::uno::Reference< css::inspection::XPropertyControlObserver >& _Observer ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHelpSectionText( const OUString& HelpText ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // XObjectInspector
        virtual css::uno::Reference< css::inspection::XObjectInspectorModel > SAL_CALL getInspectorModel() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInspectorModel( const css::uno::Reference< css::inspection::XObjectInspectorModel >& _inspectormodel ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::inspection::XObjectInspectorUI > SAL_CALL getInspectorUI() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL inspect( const css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >& Objects ) throw (css::util::VetoException, css::uno::RuntimeException, std::exception) override;

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& URL, const OUString& TargetFrameName, ::sal_Int32 SearchFlags ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& Requests ) throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    private:
        void UpdateUI();

        void startContainerWindowListening();
        void stopContainerWindowListening();

        // stop the inspection
        void stopInspection( bool _bCommitModified );

        bool haveView() const { return nullptr != m_pView; }
        OPropertyEditor&    getPropertyBox() { return m_pView->getPropertyBox(); }

        // does the inspection of the objects as indicated by our model
        void doInspection();

        // bind the browser to m_xIntrospecteeAsProperty
        void    impl_rebindToInspectee_nothrow( const InterfaceArray& _rObjects );

        /** retrieves special property handlers for our introspectee
        */
        void    getPropertyHandlers( const InterfaceArray& _rObjects, PropertyHandlerArray& _rHandlers );

        /** called when a property changed, to broadcast any handlers which might have
            registered for this property

            @param _bFirstTimeInit
                if set to <FALSE/>, this is a real change in the property value, not just a call
                for purposes of initialization.
        */
        void    impl_broadcastPropertyChange_nothrow( const OUString& _rPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, bool _bFirstTimeInit ) const;

        /** determines whether the given property is an actuating property, that is, at least one
            handler expressed interest in changes to this property's value.
        */
        inline bool impl_isActuatingProperty_nothrow( const OUString& _rPropertyName ) const
        {
            return ( m_aDependencyHandlers.find( _rPropertyName ) != m_aDependencyHandlers.end() );
        }

        /** retrieves the value of the given property, by asking the appropriate XPropertyHandler
            @param  _rPropertyName
                the name whose handler is to be obtained. Must be the name of a property
                for which a handler is registered.
            @throws
                RuntimeException if there is no handler for the given property
            @return
                the value of this property
        */
        css::uno::Any
                        impl_getPropertyValue_throw( const OUString& _rPropertyName );

        /// calls XPropertyHandler::suspend for all our property handlers
        bool    suspendPropertyHandlers_nothrow( bool _bSuspend );

        /// suspends the complete inspector
        bool    suspendAll_nothrow();

        /** selects a page according to our current view data
        */
        void selectPageFromViewData();

        /** updates our view data from the currently active page
        */
        void updateViewDataFromActivePage();

        /// describes the UI for the given property
        void describePropertyLine( const css::beans::Property& _rPropertyName, OLineDescriptor& _rDescriptor );

        /** retrieves the position of the property given by name in m_aProperties
            @return
                <TRUE/> if and only if the property could be found. In this case, <arg>_pProperty</arg> (if
                not <NULL/> contains the iterator pointing to this property.
        */
        bool impl_findObjectProperty_nothrow( const OUString& _rName, OrderedPropertyMap::const_iterator* _pProperty = NULL );

        bool Construct(vcl::Window* _pParentWin);

        /** retrieves the property handler for a given property name
            @param  _rPropertyName
                the name whose handler is to be obtained. Must be the name of a property
                for which a handler is registered.
            @throws
                RuntimeException if there is no handler for the given property
            @return
                the handler which is responsible for the given property
        */
        PropertyHandlerRef
            impl_getHandlerForProperty_throw( const OUString& _rPropertyName ) const;

        /** determines whether we have a handler for the given property
            @param _rPropertyName
                the name of the property for which the existence of a handler should be checked
        */
        bool
            impl_hasPropertyHandlerFor_nothrow( const OUString& _rPropertyName ) const;

        /** builds up m_aPageIds from InspectorModel::describeCategories, and insert all the
            respective tab pages into our view
            @precond
                m_aPageIds is empty
            @throws css::uno::RuntimeException
                if one of the callees of this method throws this exception
        */
        void
            impl_buildCategories_throw();

        /** retrieves the id of the tab page which represents a given category.
            @param  _rCategoryName
                the programmatic name of a category.
            @return
                the id of the tab page, or <code>(sal_uInt16)-1</code> if there
                is no tab page for the given category
        */
        sal_uInt16
            impl_getPageIdForCategory_nothrow( const OUString& _rCategoryName ) const;

        /** adds or removes ourself as XEventListener to/from all our inspectees
        */
        void    impl_toggleInspecteeListening_nothrow( bool _bOn );

        /** binds the instance to a new model
        */
        void    impl_bindToNewModel_nothrow( const css::uno::Reference< css::inspection::XObjectInspectorModel >& _rxInspectorModel );

        /** initializes our view, as indicated by the model's view-relevant properties

            It's allowed to call this method when no model exists, yet. In this case, nothing
            happens.
        */
        void    impl_initializeView_nothrow();

        /** determines whether the view should be readonly.

            Effectively, this means that the method simply checks the IsReadOnly attribute of the model.
            If there is no model, <FALSE/> is returned.

            @throws css::uno::RuntimeException
                in case asking the model for its IsReadOnly attribute throws a css::uno::RuntimeException
                itself.
        */
        bool    impl_isReadOnlyModel_throw() const;

        /** updates our view so that it is read-only, as indicated by the model property
            @see impl_isReadOnlyModel_throw
        */
        void    impl_updateReadOnlyView_nothrow();

        /** starts or stops listening at the model
        */
        void    impl_startOrStopModelListening_nothrow( bool _bDoListen ) const;

    private:
        DECL_LINK_TYPED(OnPageActivation, LinkParamNone*, void);

    private:
        // constructors
        void    createDefault();
        void    createWithModel( const css::uno::Reference< css::inspection::XObjectInspectorModel >& _rxModel );
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
