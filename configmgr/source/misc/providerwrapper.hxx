/*************************************************************************
 *
 *  $RCSfile: providerwrapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:24 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CONFIGMGR_API_PROVIDERWRAPPER_HXX_
#define CONFIGMGR_API_PROVIDERWRAPPER_HXX_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

namespace configmgr
{
    //==========================================================================
    namespace uno = com::sun::star::uno;
    namespace lang = com::sun::star::lang;
    using rtl::OUString;
    //==========================================================================
    //= ProviderWrapper
    //==========================================================================
    typedef ::cppu::WeakComponentImplHelper2 <  lang::XMultiServiceFactory,
                                                lang::XServiceInfo
                                            >   ProviderWrapper_Base;

    struct PWMutexHolder { osl::Mutex mutex; }; // ad hoc ...

    class ProviderWrapper : private PWMutexHolder, public ProviderWrapper_Base
    {
    public:
        typedef uno::Reference< lang::XMultiServiceFactory > Provider;
        typedef uno::Sequence< com::sun::star::beans::NamedValue >  NamedValues;
        typedef uno::Sequence< uno::Any >                           Arguments;

    private:
        Provider    m_xDelegate;
        Arguments   m_aDefaults;
    private:
        ProviderWrapper(Provider const & xDelegate, NamedValues const & aPresets);

    public:
        static uno::Reference< uno::XInterface > create( uno::Reference< uno::XInterface > xDelegate, NamedValues const & aPresets);
        ~ProviderWrapper();

        /// XMultiServiceFactory
        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstance( const OUString& aServiceSpecifier )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
            getAvailableServiceNames(  )
                throw(uno::RuntimeException);

        /// XServiceInfo
        virtual OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const ::rtl::OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

    protected:
        virtual void SAL_CALL disposing();
    private:
        Provider getDelegate();
        uno::Reference<lang::XServiceInfo> getDelegateInfo();
        Arguments patchArguments(Arguments const & aArgs) const;
    };


} // namespace configmgr

#endif // CONFIGMGR_API_CONFPROVIDER2_HXX_


