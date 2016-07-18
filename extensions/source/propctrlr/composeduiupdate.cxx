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

#include "composeduiupdate.hxx"

#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>

#include <algorithm>


namespace pcr
{


    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::inspection::XPropertyHandler;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XObjectInspectorUI;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::NoSupportException;
    using ::com::sun::star::inspection::XPropertyControlObserver;

    namespace PropertyLineElement = ::com::sun::star::inspection::PropertyLineElement;

    namespace
    {
        struct HandlerLess : public ::std::binary_function  <   Reference< XPropertyHandler >
                                                            ,   Reference< XPropertyHandler >
                                                            ,   bool
                                                            >
        {
            bool operator()( const Reference< XPropertyHandler >& lhs, const Reference< XPropertyHandler >& rhs) const
            {
                return lhs.get() < rhs.get();
            }
        };


        typedef ::std::set< OUString >       StringBag;
        typedef ::std::map< sal_Int16, StringBag >  MapIntToStringBag;
    }


    // callbacks for CachedInspectorUI

    typedef void (ComposedPropertyUIUpdate::*FNotifySingleUIChange)();

    typedef ::cppu::WeakImplHelper <   css::inspection::XObjectInspectorUI
                                    >   CachedInspectorUI_Base;
    struct CachedInspectorUI : public CachedInspectorUI_Base
    {
    private:
        ::osl::Mutex            m_aMutex;
        bool                    m_bDisposed;
        ComposedPropertyUIUpdate&
                                m_rMaster;
        FNotifySingleUIChange   m_pUIChangeNotification;

        // enablePropertyUI cache
        StringBag               aEnabledProperties;
        StringBag               aDisabledProperties;

        // show/hidePropertyUI cache
        StringBag               aShownProperties;
        StringBag               aHiddenProperties;

        // rebuildPropertyUI cache
        StringBag               aRebuiltProperties;

        // showCategory cache
        StringBag               aShownCategories;
        StringBag               aHiddenCategories;

        // enablePropertyUIElements cache
        MapIntToStringBag       aEnabledElements;
        MapIntToStringBag       aDisabledElements;

    public:
        typedef StringBag& (CachedInspectorUI::*FGetStringBag)();

        // enablePropertyUI cache
        StringBag&  getEnabledProperties()          { return aEnabledProperties; }
        StringBag&  getDisabledProperties()         { return aDisabledProperties; }

        // show/hidePropertyUI cache
        StringBag&  getShownProperties()            { return aShownProperties; }
        StringBag&  getHiddenProperties()           { return aHiddenProperties; }

        // rebuildPropertyUI cache
        StringBag&  getRebuiltProperties()          { return aRebuiltProperties; }

        // showCategory cache
        StringBag&  getShownCategories()            { return aShownCategories; }
        StringBag&  getHiddenCategories()           { return aHiddenCategories; }

        // enablePropertyUIElements
        StringBag&  getEnabledInputControls()       { return aEnabledElements[ PropertyLineElement::InputControl ]; }
        StringBag&  getDisabledInputControls()      { return aDisabledElements[ PropertyLineElement::InputControl ]; }
        StringBag&  getEnabledPrimaryButtons()      { return aEnabledElements[ PropertyLineElement::PrimaryButton ]; }
        StringBag&  getDisabledPrimaryButtons()     { return aDisabledElements[ PropertyLineElement::PrimaryButton ]; }
        StringBag&  getEnabledSecondaryButtons()    { return aEnabledElements[ PropertyLineElement::SecondaryButton ]; }
        StringBag&  getDisabledSecondaryButtons()   { return aDisabledElements[ PropertyLineElement::SecondaryButton ]; }

    public:
        CachedInspectorUI( ComposedPropertyUIUpdate& _rMaster, FNotifySingleUIChange _pUIChangeNotification );
        CachedInspectorUI(const CachedInspectorUI&) = delete;
        CachedInspectorUI& operator=(const CachedInspectorUI&) = delete;

        /// disposes the instance
        void dispose();

        // XObjectInspectorUI overridables
        virtual void SAL_CALL enablePropertyUI( const OUString& _rPropertyName, sal_Bool _bEnable ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL enablePropertyUIElements( const OUString& _rPropertyName, ::sal_Int16 _nElements, sal_Bool _bEnable ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL rebuildPropertyUI( const OUString& _rPropertyName ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL showPropertyUI( const OUString& _rPropertyName ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL hidePropertyUI( const OUString& _rPropertyName ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL showCategory( const OUString& _rCategory, sal_Bool _bShow ) throw (RuntimeException, std::exception) override;
        virtual Reference< XPropertyControl > SAL_CALL getPropertyControl( const OUString& _rPropertyName ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL registerControlObserver( const Reference< XPropertyControlObserver >& Observer ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL revokeControlObserver( const Reference< XPropertyControlObserver >& Observer ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL setHelpSectionText( const OUString& HelpText ) throw (NoSupportException, RuntimeException, std::exception) override;

    protected:
        virtual ~CachedInspectorUI();

        /// determines whether the instance is already disposed
        inline bool isDisposed() const { return m_bDisposed; }

        /// throws an exception if the component is already disposed
        void checkDisposed() const;

    private:
        void    impl_markElementEnabledOrDisabled( const OUString& _rPropertyName, sal_Int16 _nElementIdOrZero, bool _bEnable );

        /** calls <member>m_pUIChangeNotification</member> at <member>m_rMaster</member>
        */
        void    impl_notifySingleUIChange() const;

    private:
        class MethodGuard;
        friend class MethodGuard;
        class MethodGuard : public ::osl::MutexGuard
        {
        public:
            explicit MethodGuard( CachedInspectorUI& rInstance )
                : ::osl::MutexGuard( rInstance.m_aMutex )
            {
                rInstance.checkDisposed();
            }
        };
    };


    CachedInspectorUI::CachedInspectorUI( ComposedPropertyUIUpdate& _rMaster, FNotifySingleUIChange _pUIChangeNotification )
        :m_bDisposed( false )
        ,m_rMaster( _rMaster )
        ,m_pUIChangeNotification( _pUIChangeNotification )
    {
    }


    CachedInspectorUI::~CachedInspectorUI()
    {
    }


    void CachedInspectorUI::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bDisposed = true;

        clearContainer( aEnabledProperties );
        clearContainer( aDisabledProperties );
        clearContainer( aRebuiltProperties );
        clearContainer( aShownProperties );
        clearContainer( aHiddenProperties );
        clearContainer( aShownCategories );
        clearContainer( aHiddenCategories );
        clearContainer( aEnabledElements );
        clearContainer( aDisabledElements );
    }


    void CachedInspectorUI::checkDisposed() const
    {
        if ( isDisposed() )
            throw DisposedException();
    }


    namespace
    {
        void lcl_markStringKeyPositiveOrNegative( const OUString& _rKeyName, StringBag& _rPositives, StringBag& _rNegatives, bool _bMarkPositive )
        {
            if ( _bMarkPositive )
            {
                _rPositives.insert( _rKeyName );
                // if the same key has been remember as in the "negative" list before, clear this information, since it's overruled
                _rNegatives.erase( _rKeyName );
            }
            else
                _rNegatives.insert( _rKeyName );
        }
    }


    void CachedInspectorUI::enablePropertyUI( const OUString& _rPropertyName, sal_Bool _bEnable ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if ( !m_rMaster.shouldContinuePropertyHandling( _rPropertyName ) )
            return;

        lcl_markStringKeyPositiveOrNegative( _rPropertyName, aEnabledProperties, aDisabledProperties, _bEnable );
        impl_notifySingleUIChange();
    }


    void CachedInspectorUI::impl_markElementEnabledOrDisabled( const OUString& _rPropertyName, sal_Int16 _nElementIdOrZero, bool _bEnable )
    {
        if ( _nElementIdOrZero == 0 )
            return;

        lcl_markStringKeyPositiveOrNegative(
            _rPropertyName,
            aEnabledElements[ _nElementIdOrZero ],
            aDisabledElements[ _nElementIdOrZero ],
            _bEnable
        );
    }


    void CachedInspectorUI::impl_notifySingleUIChange() const
    {
        (m_rMaster.*m_pUIChangeNotification)();
    }


    void CachedInspectorUI::enablePropertyUIElements( const OUString& _rPropertyName, sal_Int16 _nElements, sal_Bool _bEnable ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if ( !m_rMaster.shouldContinuePropertyHandling( _rPropertyName ) )
            return;

        impl_markElementEnabledOrDisabled( _rPropertyName, _nElements & PropertyLineElement::InputControl,    _bEnable );
        impl_markElementEnabledOrDisabled( _rPropertyName, _nElements & PropertyLineElement::PrimaryButton,   _bEnable );
        impl_markElementEnabledOrDisabled( _rPropertyName, _nElements & PropertyLineElement::SecondaryButton, _bEnable );

        impl_notifySingleUIChange();
    }


    void CachedInspectorUI::rebuildPropertyUI( const OUString& _rPropertyName ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if ( !m_rMaster.shouldContinuePropertyHandling( _rPropertyName ) )
            return;

        aRebuiltProperties.insert( _rPropertyName );

        impl_notifySingleUIChange();
    }


    void CachedInspectorUI::showPropertyUI( const OUString& _rPropertyName ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if ( !m_rMaster.shouldContinuePropertyHandling( _rPropertyName ) )
            return;

        aShownProperties.insert( _rPropertyName );
        // if the same category has been hidden before, clear this information, since it's overruled
        aHiddenProperties.erase( _rPropertyName );

        impl_notifySingleUIChange();
    }


    void CachedInspectorUI::hidePropertyUI( const OUString& _rPropertyName ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if ( !m_rMaster.shouldContinuePropertyHandling( _rPropertyName ) )
            return;

        aHiddenProperties.insert( _rPropertyName );
        impl_notifySingleUIChange();
    }


    void CachedInspectorUI::showCategory( const OUString& _rCategory, sal_Bool _bShow ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );

        lcl_markStringKeyPositiveOrNegative( _rCategory, aShownCategories, aHiddenCategories, _bShow );
        impl_notifySingleUIChange();
    }


    Reference< XPropertyControl > SAL_CALL CachedInspectorUI::getPropertyControl( const OUString& _rPropertyName ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this );
        if ( !m_rMaster.shouldContinuePropertyHandling( _rPropertyName ) )
            return Reference< XPropertyControl >();

        return m_rMaster.getDelegatorUI()->getPropertyControl( _rPropertyName );
    }


    void SAL_CALL CachedInspectorUI::registerControlObserver( const Reference< XPropertyControlObserver >& Observer ) throw (RuntimeException, std::exception)
    {
        OSL_FAIL( "CachedInspectorUI::registerControlObserver: not expected to be called!" );
            // CachedInspectorUI is used as context for the controls, and we don't expect them to
            // register listeners themself
        m_rMaster.getDelegatorUI()->registerControlObserver( Observer );
    }


    void SAL_CALL CachedInspectorUI::revokeControlObserver( const Reference< XPropertyControlObserver >& Observer ) throw (RuntimeException, std::exception)
    {
        OSL_FAIL( "CachedInspectorUI::revokeControlObserver: not expected to be called!" );
            // CachedInspectorUI is used as context for the controls, and we don't expect them to
            // register listeners themself
        m_rMaster.getDelegatorUI()->revokeControlObserver( Observer );
    }


    void SAL_CALL CachedInspectorUI::setHelpSectionText( const OUString& HelpText ) throw (NoSupportException, RuntimeException, std::exception)
    {
        m_rMaster.getDelegatorUI()->setHelpSectionText( HelpText );
    }


    // HandlerMap

    typedef ::std::map  <   Reference< XPropertyHandler >
                        ,   ::rtl::Reference< CachedInspectorUI >
                        ,   HandlerLess
                        >   ImplMapHandlerToUI;
    struct MapHandlerToUI
    {
        ImplMapHandlerToUI aHandlers;
    };

    ComposedPropertyUIUpdate::ComposedPropertyUIUpdate( const Reference< XObjectInspectorUI >& _rxDelegatorUI,
        IPropertyExistenceCheck* _pPropertyCheck )
        :m_pCollectedUIs( new MapHandlerToUI )
        ,m_xDelegatorUI( _rxDelegatorUI )
        ,m_nSuspendCounter( 0 )
        ,m_pPropertyCheck( _pPropertyCheck )
    {
        if ( !m_xDelegatorUI.is() )
            throw NullPointerException();
    }


    ComposedPropertyUIUpdate::~ComposedPropertyUIUpdate( )
    {
    }


    Reference< XObjectInspectorUI > ComposedPropertyUIUpdate::getUIForPropertyHandler( const Reference< XPropertyHandler >& _rxHandler )
    {
        impl_checkDisposed();

        ::rtl::Reference< CachedInspectorUI >& rUI = m_pCollectedUIs->aHandlers[ _rxHandler ];
        if ( !rUI.is() )
            rUI = new CachedInspectorUI( *this, &ComposedPropertyUIUpdate::callback_inspectorUIChanged_throw );
        return rUI.get();
    }


    namespace
    {

        // an STL-compatible structure which collects strings from a CachedInspectorUI instances
        struct StringBagCollector : public ::std::unary_function< ImplMapHandlerToUI::value_type, void >
        {
        private:
            StringBag&                      m_rBag;
            CachedInspectorUI::FGetStringBag  m_pGetter;

        public:
            StringBagCollector( StringBag& _rBag, CachedInspectorUI::FGetStringBag _pGetter ) :m_rBag( _rBag ), m_pGetter( _pGetter ) { }

            void operator()( const ImplMapHandlerToUI::value_type& _rUI )
            {
                StringBag& rBag( ((_rUI.second.get())->*m_pGetter)() );
                m_rBag.insert( rBag.begin(), rBag.end() );
            }

            static void collectAll( StringBag& _rAll, const ImplMapHandlerToUI& _rMap, CachedInspectorUI::FGetStringBag _pGetter )
            {
                ::std::for_each( _rMap.begin(), _rMap.end(), StringBagCollector( _rAll, _pGetter ) );
            }
        };


        // an STL-compatible structure which cleans a certain string bag in a CachedInspectorUI instances
        struct StringBagClearer : public ::std::unary_function< ImplMapHandlerToUI::value_type, void >
        {
        private:
            CachedInspectorUI::FGetStringBag  m_pGetter;

        public:
            explicit StringBagClearer( CachedInspectorUI::FGetStringBag _pGetter ) :m_pGetter( _pGetter ) { }

            void operator()( const ImplMapHandlerToUI::value_type& _rUI )
            {
                clearContainer( ((_rUI.second.get())->*m_pGetter)() );
            }

            static void clearAll( const ImplMapHandlerToUI& _rMap, CachedInspectorUI::FGetStringBag _pGetter )
            {
                ::std::for_each( _rMap.begin(), _rMap.end(), StringBagClearer( _pGetter ) );
            }
        };

        // a typedef for a ->XObjectInspectorUI member function taking a string
        typedef void ( SAL_CALL XObjectInspectorUI::*FPropertyUISetter )( const OUString& );


        // an STL-compatible struct which calls a certain member method (taking a string) at a
        //    given ->XObjectInspectorUI instance
        struct PropertyUIOperator : public ::std::unary_function< OUString, void >
        {
        private:
            Reference< XObjectInspectorUI > m_xUpdater;
            FPropertyUISetter               m_pSetter;

        public:
            PropertyUIOperator( const Reference< XObjectInspectorUI >& _rxInspectorUI, FPropertyUISetter _pSetter )
                :m_xUpdater( _rxInspectorUI )
                ,m_pSetter( _pSetter )
            {
            }

            void operator()( const OUString& _rPropertyName )
            {
                ((m_xUpdater.get())->*m_pSetter)( _rPropertyName );
            }

            static void forEach( const StringBag& _rProperties, const Reference< XObjectInspectorUI >& _rxDelegatorUI, FPropertyUISetter _pSetter )
            {
                ::std::for_each( _rProperties.begin(), _rProperties.end(), PropertyUIOperator( _rxDelegatorUI, _pSetter ) );
            }
        };


        // an interface which encapsulates access to a single aspect of the ->XObjectInspectorUI,
        //    where this aspect is given by a string key, and has a boolean value.
        class IStringKeyBooleanUIUpdate
        {
        public:
            virtual void updateUIForKey( const OUString& _rKey, bool _bFlag ) const = 0;

            virtual ~IStringKeyBooleanUIUpdate() { }
        };


        // FPropertyUIFlagSetter

        /** an implementation of the ->IStringKeyBooleanUIUpdate interface which,
            for a fixed ->XObjectInspectorUI instance and a fixed UI element (->PropertyLineElement),
            updates this element for a given property with a given boolean flag
            (->XObjectInspectorUI::enablePropertyUIElements)
        */
        class EnablePropertyUIElement : public IStringKeyBooleanUIUpdate
        {
        private:
            Reference< XObjectInspectorUI > m_xUIUpdate;
            sal_Int16                       m_nElement;

        public:
            EnablePropertyUIElement( const Reference< XObjectInspectorUI >& _rxUIUpdate, sal_Int16 _nElement )
                :m_xUIUpdate( _rxUIUpdate )
                ,m_nElement( _nElement )
            {
            }
            // IStringKeyBooleanUIUpdate
            virtual void updateUIForKey( const OUString& _rKey, bool _bFlag ) const override;
        };


        void EnablePropertyUIElement::updateUIForKey( const OUString& _rKey, bool _bFlag ) const
        {
            m_xUIUpdate->enablePropertyUIElements( _rKey, m_nElement, _bFlag );
        }


        // a ->XObjectInspectorUI method taking a string and a boolean
        typedef void ( SAL_CALL XObjectInspectorUI::*FPropertyUIFlagSetter )( const OUString&, sal_Bool );


        // an implementaiton of the ->IStringKeyBooleanUIUpdate interface which calls
        // am arbitrary ->XObjectInspectorUI method taking a string and a boolean flag
        class DefaultStringKeyBooleanUIUpdate : public IStringKeyBooleanUIUpdate
        {
        private:
            Reference< XObjectInspectorUI > m_xUIUpdate;
            FPropertyUIFlagSetter           m_pSetter;

        public:
            DefaultStringKeyBooleanUIUpdate( const Reference< XObjectInspectorUI >& _rxUIUpdate, FPropertyUIFlagSetter _pSetter );
            // IStringKeyBooleanUIUpdate
            virtual void updateUIForKey( const OUString& _rKey, bool _bFlag ) const override;
        };


        DefaultStringKeyBooleanUIUpdate::DefaultStringKeyBooleanUIUpdate( const Reference< XObjectInspectorUI >& _rxUIUpdate, FPropertyUIFlagSetter _pSetter )
            :m_xUIUpdate( _rxUIUpdate )
            ,m_pSetter( _pSetter )
        {
        }


        void DefaultStringKeyBooleanUIUpdate::updateUIForKey( const OUString& _rKey, bool _bFlag ) const
        {
            ((m_xUIUpdate.get())->*m_pSetter)( _rKey, _bFlag );
        }


        // an STL-compatible structure which applies a ->IStringKeyBooleanUIUpdate::updateUIForKey
        // operation with a fixed boolean value, for a given string value
        struct BooleanUIAspectUpdate : public ::std::unary_function< OUString, void >
        {
        private:
            const IStringKeyBooleanUIUpdate&    m_rUpdater;
            bool                            m_bFlag;

        public:
            BooleanUIAspectUpdate( const IStringKeyBooleanUIUpdate& _rUpdater, bool _bFlag )
                :m_rUpdater( _rUpdater )
                ,m_bFlag( _bFlag )
            {
            }

            void operator()( const OUString& _rPropertyName )
            {
                m_rUpdater.updateUIForKey( _rPropertyName, m_bFlag );
            }

            static void forEach( const StringBag& _rProperties, const IStringKeyBooleanUIUpdate& _rUpdater, bool _bFlag )
            {
                ::std::for_each( _rProperties.begin(), _rProperties.end(), BooleanUIAspectUpdate( _rUpdater, _bFlag ) );
            }
        };


        // BooleanUIAspectUpdate

        // an STL-compatible structure subtracting a given string from a fixed ->StringBag
        struct StringBagComplement : public ::std::unary_function< OUString, void >
        {
        private:
            StringBag&  m_rMinuend;

        public:
            explicit StringBagComplement( StringBag& _rMinuend ) :m_rMinuend( _rMinuend ) { }

            void operator()( const OUString& _rPropertyToSubtract )
            {
                m_rMinuend.erase( _rPropertyToSubtract );
            }

            static void subtract( StringBag& _rMinuend, const StringBag& _rSubtrahend )
            {
                ::std::for_each( _rSubtrahend.begin(), _rSubtrahend.end(), StringBagComplement( _rMinuend ) );
            }
        };


        // BooleanUIAspectUpdate

        void lcl_fireUIStateFlag(
                const IStringKeyBooleanUIUpdate& _rUIUpdate,
                const ImplMapHandlerToUI& _rHandlerUIs,
                CachedInspectorUI::FGetStringBag _pGetPositives,
                CachedInspectorUI::FGetStringBag _pGetNegatives
            )
        {
            // all strings which are in the "positive" list of one handler
            StringBag aAllPositives;
            StringBagCollector::collectAll( aAllPositives, _rHandlerUIs, _pGetPositives );

            // all strings which are in the "negative" list of one handler
            StringBag aAllNegatives;
            StringBagCollector::collectAll( aAllNegatives, _rHandlerUIs, _pGetNegatives );

            // propagate the "negative" flags to the delegator UI
            BooleanUIAspectUpdate::forEach( aAllNegatives, _rUIUpdate, false );

            // propagate the "positive" flags to the delegator UI, for all elements where _no_
            // "negative" flag exists
            StringBagComplement::subtract( aAllPositives, aAllNegatives );
            BooleanUIAspectUpdate::forEach( aAllPositives, _rUIUpdate, true );

            // the "positive" request can be cleared no, only negative requests
            // (such as "disable a property" or "hide a category") need to be preserved for the next round
            StringBagClearer::clearAll( _rHandlerUIs, _pGetPositives );
        }
    }


    void ComposedPropertyUIUpdate::impl_fireEnablePropertyUI_throw()
    {
        lcl_fireUIStateFlag(
            DefaultStringKeyBooleanUIUpdate( m_xDelegatorUI, &XObjectInspectorUI::enablePropertyUI ),
            m_pCollectedUIs->aHandlers,
            &CachedInspectorUI::getEnabledProperties,
            &CachedInspectorUI::getDisabledProperties
        );
    }


    void ComposedPropertyUIUpdate::impl_fireRebuildPropertyUI_throw()
    {
        // collect all properties for which a rebuild request has been made
        StringBag aAllRebuilt;
        StringBagCollector::collectAll( aAllRebuilt, m_pCollectedUIs->aHandlers, &CachedInspectorUI::getRebuiltProperties );

        // rebuild all those properties
        PropertyUIOperator::forEach( aAllRebuilt, m_xDelegatorUI, &XObjectInspectorUI::rebuildPropertyUI );

        // clear the "properties to rebuild" at all handlers, since the request has been fulfilled now.
        StringBagClearer::clearAll( m_pCollectedUIs->aHandlers, &CachedInspectorUI::getRebuiltProperties );
    }


    void ComposedPropertyUIUpdate::impl_fireShowHidePropertyUI_throw()
    {
        // all properties which have been shown by at least one handler
        StringBag aAllShown;
        StringBagCollector::collectAll( aAllShown, m_pCollectedUIs->aHandlers, &CachedInspectorUI::getShownProperties );
        // all properties which have been hidden by at least one handler
        StringBag aAllHidden;
        StringBagCollector::collectAll( aAllHidden, m_pCollectedUIs->aHandlers, &CachedInspectorUI::getHiddenProperties );

        // hide properties as necessary
        PropertyUIOperator::forEach( aAllHidden, m_xDelegatorUI, &XObjectInspectorUI::hidePropertyUI );

        // for those properties which are hidden, ignore all "show" requests which other handlers might have had
        StringBagComplement::subtract( aAllShown, aAllHidden );

        // show properties
        PropertyUIOperator::forEach( aAllShown, m_xDelegatorUI, &XObjectInspectorUI::showPropertyUI );
    }


    void ComposedPropertyUIUpdate::impl_fireShowCategory_throw()
    {
        lcl_fireUIStateFlag(
            DefaultStringKeyBooleanUIUpdate( m_xDelegatorUI, &XObjectInspectorUI::showCategory ),
            m_pCollectedUIs->aHandlers,
            &CachedInspectorUI::getShownCategories,
            &CachedInspectorUI::getHiddenCategories
        );
    }


    void ComposedPropertyUIUpdate::impl_fireEnablePropertyUIElements_throw()
    {
        lcl_fireUIStateFlag(
            EnablePropertyUIElement( m_xDelegatorUI, PropertyLineElement::InputControl ),
            m_pCollectedUIs->aHandlers,
            &CachedInspectorUI::getEnabledInputControls,
            &CachedInspectorUI::getDisabledInputControls
        );

        lcl_fireUIStateFlag(
            EnablePropertyUIElement( m_xDelegatorUI, PropertyLineElement::PrimaryButton ),
            m_pCollectedUIs->aHandlers,
            &CachedInspectorUI::getEnabledPrimaryButtons,
            &CachedInspectorUI::getDisabledPrimaryButtons
        );

        lcl_fireUIStateFlag(
            EnablePropertyUIElement( m_xDelegatorUI, PropertyLineElement::SecondaryButton ),
            m_pCollectedUIs->aHandlers,
            &CachedInspectorUI::getEnabledSecondaryButtons,
            &CachedInspectorUI::getDisabledSecondaryButtons
        );
    }


    void ComposedPropertyUIUpdate::impl_fireAll_throw()
    {
        OSL_PRECOND( !impl_isDisposed(), "ComposedPropertyUIUpdate::impl_fireAll_throw: already disposed, this will crash!" );

        impl_fireEnablePropertyUI_throw();
        impl_fireShowHidePropertyUI_throw();
        impl_fireRebuildPropertyUI_throw();
        impl_fireShowCategory_throw();
        impl_fireEnablePropertyUIElements_throw();
    }


    void SAL_CALL ComposedPropertyUIUpdate::suspendAutoFire()
    {
        impl_checkDisposed();
        osl_atomic_increment( &m_nSuspendCounter );
    }


    void SAL_CALL ComposedPropertyUIUpdate::resumeAutoFire()
    {
        impl_checkDisposed();
        if ( 0 == osl_atomic_decrement( &m_nSuspendCounter ) )
            impl_fireAll_throw();
    }


    void ComposedPropertyUIUpdate::impl_checkDisposed() const
    {
        if ( impl_isDisposed() )
            throw DisposedException();
    }


    void ComposedPropertyUIUpdate::callback_inspectorUIChanged_throw()
    {
        if ( 0 == m_nSuspendCounter )
            impl_fireAll_throw();
    }


    Reference< XObjectInspectorUI > const & ComposedPropertyUIUpdate::getDelegatorUI() const
    {
        impl_checkDisposed();
        return m_xDelegatorUI;
    }


    void SAL_CALL ComposedPropertyUIUpdate::dispose()
    {
        if ( impl_isDisposed() )
            return;

        OSL_ENSURE( m_nSuspendCounter == 0, "ComposedPropertyUIUpdate::dispose: still suspended, the changes will be lost!" );

        for ( ImplMapHandlerToUI::const_iterator singleUI = m_pCollectedUIs->aHandlers.begin();
              singleUI != m_pCollectedUIs->aHandlers.end();
              ++singleUI
            )
        {
            singleUI->second->dispose();
        }
        m_pCollectedUIs.reset();
        m_xDelegatorUI.set( nullptr );
    }


    bool ComposedPropertyUIUpdate::shouldContinuePropertyHandling( const OUString& _rName ) const
    {
        if ( !m_pPropertyCheck )
            return true;
        if ( m_pPropertyCheck->hasPropertyByName( _rName ) )
            return true;
        return false;
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
