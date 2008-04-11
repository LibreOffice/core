/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlfeatureinterception.hxx,v $
 * $Revision: 1.4 $
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

#ifndef FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX
#define FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace frm
{
//........................................................................

    class UrlTransformer;
    //====================================================================
    //= ControlFeatureInterception
    //====================================================================
    /** helper class for controls which allow some of their features to be intercepted
        by external instances

        For using this class, instantiate it as member, derive yourself from
        <type scope="com::sun::star::frame">XDispatchProviderInterception</type>, and forward all
        respective methods to this member.

        Additionally, don't forget to call <member>dispose</member> when your class is disposed itself.
    */
    class ControlFeatureInterception
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >
                                            m_xFirstDispatchInterceptor;
        ::std::auto_ptr< UrlTransformer >   m_pUrlTransformer;

    public:
        /** retrieves our URL transformer, so our clients may use it, too
        */
        const UrlTransformer& getTransformer() const { return *m_pUrlTransformer; }

    public:
        ControlFeatureInterception( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        // XDispatchProviderInterception
        void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException );
        void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException );

        // XComponent
        void SAL_CALL dispose();

        /** queries the interceptor chain for the given dispatch
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& _rTargetFrameName, ::sal_Int32 _nSearchFlags ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** queries the interceptor chain for the given dispatch, with a blank target frame and no frame search flags
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            queryDispatch( const ::com::sun::star::util::URL& _rURL ) SAL_THROW((::com::sun::star::uno::RuntimeException));

        /** queries the interceptor chain for the URL given as ASCII string,
            with a blank target frame and no frame search flags
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
            queryDispatch( const sal_Char* _pAsciiURL ) SAL_THROW((::com::sun::star::uno::RuntimeException));
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX

