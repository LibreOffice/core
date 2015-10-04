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

#ifndef INCLUDED_FORMS_SOURCE_INC_FORMNAVIGATION_HXX
#define INCLUDED_FORMS_SOURCE_INC_FORMNAVIGATION_HXX

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/implbase.hxx>
#include "featuredispatcher.hxx"
#include <vector>
#include <map>
#include <memory>


namespace frm
{


    class UrlTransformer;
    class ControlFeatureInterception;


    //= OFormNavigationHelper

    typedef ::cppu::ImplHelper  <   css::frame::XDispatchProviderInterception
                                ,   css::frame::XStatusListener
                                >   OFormNavigationHelper_Base;

    class OFormNavigationHelper
                        :public OFormNavigationHelper_Base
                        ,public IFeatureDispatcher
    {
    private:
        struct FeatureInfo
        {
            css::util::URL                                             aURL;
            css::uno::Reference< css::frame::XDispatch >               xDispatcher;
            bool                                                       bCachedState;
            css::uno::Any                                              aCachedAdditionalState;

            FeatureInfo() : bCachedState( false ) { }
        };
        typedef ::std::map< sal_Int16, FeatureInfo >    FeatureMap;

    private:
        css::uno::Reference< css::uno::XComponentContext >
                            m_xORB;
        ::std::unique_ptr< ControlFeatureInterception >
                            m_pFeatureInterception;

        // all supported features
        FeatureMap          m_aSupportedFeatures;
        // all features which we have an external dispatcher for
        sal_Int32           m_nConnectedFeatures;

    protected:
        OFormNavigationHelper( const css::uno::Reference< css::uno::XComponentContext >& _rxORB );
        virtual ~OFormNavigationHelper();

        // XComponent
        void SAL_CALL dispose(  ) throw( css::uno::RuntimeException );

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& State ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

        // IFeatureDispatcher
        virtual void            dispatch( sal_Int16 _nFeatureId ) const override;
        virtual void            dispatchWithArgument( sal_Int16 _nFeatureId, const sal_Char* _pParamName, const css::uno::Any& _rParamValue ) const override;
        virtual bool            isEnabled( sal_Int16 _nFeatureId ) const override;
        virtual bool            getBooleanState( sal_Int16 _nFeatureId ) const override;
        virtual OUString getStringState( sal_Int16 _nFeatureId ) const override;
        virtual sal_Int32       getIntegerState( sal_Int16 _nFeatureId ) const override;

        // own overridables
        /** is called when the interceptors have.
            <p>The default implementations simply calls <member>updateDispatches</member>,
            derived classes can prevent this in certain cases, or do additional handling.</p>
        */
        virtual void    interceptorsChanged( );

        /** called when the status of a feature changed

            <p>The default implementation does nothing.</p>

            <p>If the feature in question does support more state information that just the
            enabled/disabled state, then this additional information is to be retrieved in
            a separate call.</p>

            @param _nFeatureId
                the id of the feature
            @param _bEnabled
                determines if the features is enabled or disabled
            @see getBooleanState
        */
        virtual void    featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled );

        /** notification for (potential) changes in the state of all features
            <p>The base class implementation does nothing. Derived classes could force
            their peer to update it's state, depending on the result of calls to
            <member>IFeatureDispatcher::isEnabled</member>.</p>
        */
        virtual void    allFeatureStatesChanged( );

        /** retrieves the list of supported features
            <p>To be overridden by derived classes</p>
            @param _rFeatureIds
                the array of features to support. Out parameter to fill by the derivee's implementation
            @pure
        */
        virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds ) = 0;

    protected:
        /** update all our dispatches which are controlled by our dispatch interceptors
        */
        void    updateDispatches();

        /** connect to the dispatch interceptors
        */
        void    connectDispatchers();

        /** disconnect from the dispatch interceptors
        */
        void    disconnectDispatchers();

        /** queries the interceptor chain for a dispatcher for the given URL
        */
        css::uno::Reference< css::frame::XDispatch >
                queryDispatch( const css::util::URL& _rURL );

        /** invalidates the set of supported features

            <p>This will invalidate all structures which are tied to the set of supported
            features. All dispatches will be disconnected.<br/>
            No automatic re-connection to potential external dispatchers is done, instead,
            you have to call updateDispatches explicitly, if necessary.</p>
        */
        void    invalidateSupportedFeaturesSet();

    private:
        /** initialize m_aSupportedFeatures, if necessary
        */
        void    initializeSupportedFeatures();
    };

    /** helper class mapping between feature ids and feature URLs
    */
    class OFormNavigationMapper
    {
    private:
        ::std::unique_ptr< UrlTransformer >   m_pUrlTransformer;

    public:
        OFormNavigationMapper(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );
        ~OFormNavigationMapper( );

        /** retrieves the ASCII representation of a feature URL belonging to an id

            @complexity O(log n)
            @return NULL if the given id is not a known feature id (which is a valid usage)
        */
        static const char* getFeatureURLAscii( sal_Int16 _nFeatureId );

        /** retrieves the feature URL belonging to an feature id

            @complexity O(log n), with n being the number of all potentially known URLs
            @return
                <TRUE/> if and only if the given id is a known feature id
                (which is a valid usage)
        */
        bool        getFeatureURL( sal_Int16 _nFeatureId, css::util::URL& /* [out] */ _rURL );

        /** retrieves the feature id belonging to an feature URL

            @complexity O(n), with n being the number of all potentially known URLs
            @return
                the id of the feature URL, or -1 if the URl is not known
                (which is a valid usage)
        */
        static sal_Int16 getFeatureId( const OUString& _rCompleteURL );

    private:
        OFormNavigationMapper( const OFormNavigationMapper& ) = delete;
        OFormNavigationMapper& operator=( const OFormNavigationMapper& ) = delete;
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_FORMNAVIGATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
