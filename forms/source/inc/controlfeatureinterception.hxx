/*************************************************************************
 *
 *  $RCSfile: controlfeatureinterception.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:44:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX
#define FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
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

