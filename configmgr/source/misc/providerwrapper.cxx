/*************************************************************************
 *
 *  $RCSfile: providerwrapper.cxx,v $
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

#include "providerwrapper.hxx"

#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#include "bootstrapcontext.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#include <algorithm>

namespace configmgr
{
    //==========================================================================
    namespace uno = com::sun::star::uno;
    namespace lang = com::sun::star::lang;
    using rtl::OUString;
    //==========================================================================
    //= ProviderWrapper
    //==========================================================================

    uno::Reference< uno::XInterface > ProviderWrapper::create( uno::Reference< uno::XInterface > xDelegate, NamedValues const & aPresets)
    {
        Provider xProvDelegate(xDelegate, uno::UNO_QUERY);
        if (!xProvDelegate.is())
        {
            OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("ProviderWrapper: Cannot wrap a NULL provider"));
            throw lang::NullPointerException(sMsg,NULL);
        }

        Provider xResult( new ProviderWrapper(xProvDelegate,aPresets) );

        typedef uno::Reference< lang::XComponent > Comp;
        DisposingForwarder::forward( Comp::query(xProvDelegate),Comp::query(xResult) );
        return xResult;
    }


    ProviderWrapper::ProviderWrapper(Provider const & xDelegate, NamedValues const & aPresets)
    : ProviderWrapper_Base( PWMutexHolder::mutex )
    , m_xDelegate(xDelegate)
    , m_aDefaults(aPresets.getLength())
    {
        OSL_ASSERT(m_xDelegate.is());

        for (sal_Int32 i = 0; i<aPresets.getLength(); ++i)
        {
            m_aDefaults[i] <<= aPresets[i];
        }
    }

    ProviderWrapper::~ProviderWrapper() {}

    void SAL_CALL ProviderWrapper::disposing()
    {
        osl::MutexGuard lock(mutex);
        m_xDelegate.clear();
    }

    ProviderWrapper::Provider ProviderWrapper::getDelegate()
    {
        osl::MutexGuard lock(mutex);
        if (!m_xDelegate.is())
        {
            OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("ProviderWrapper: Delegate Provider has been disposed"));
            throw lang::DisposedException(sMsg,*this);
        }
        return m_xDelegate;
    }


    uno::Reference<lang::XServiceInfo> ProviderWrapper::getDelegateInfo()
    {
        uno::Reference<lang::XServiceInfo> xDelegate( this->getDelegate(), uno::UNO_QUERY );
        if (!xDelegate.is())
        {
            OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("ProviderWrapper: Delegate Provider has no service info"));
            throw uno::RuntimeException(sMsg,*this);
        }
        return xDelegate;
    }

    /// XMultiServiceFactory
    static inline uno::Any const * begin(ProviderWrapper::Arguments const & aArgs)
    { return aArgs.getConstArray(); }
    static inline uno::Any const * end(ProviderWrapper::Arguments const & aArgs)
    { return aArgs.getConstArray() + aArgs.getLength(); }
    static inline uno::Any * begin(ProviderWrapper::Arguments & aArgs)
    { return aArgs.getArray(); }
    static inline uno::Any * end(ProviderWrapper::Arguments & aArgs)
    { return aArgs.getArray() + aArgs.getLength(); }

    ProviderWrapper::Arguments ProviderWrapper::patchArguments(Arguments const & aArgs) const
    {
        // rely on evaluation order front to back
        if (m_aDefaults.getLength() == 0) return aArgs;

        Arguments aResult(m_aDefaults.getLength() + aArgs.getLength());

        uno::Any * pNext = std::copy(begin(m_aDefaults),end(m_aDefaults),begin(aResult));
        pNext = std::copy(begin(aArgs),end(aArgs),pNext);

        OSL_ASSERT(end(aResult) == pNext);

        return aResult;
    }

    uno::Reference< uno::XInterface > SAL_CALL
        ProviderWrapper::createInstance( const OUString& aServiceSpecifier )
            throw(uno::Exception, uno::RuntimeException)
    {
        return getDelegate()->createInstanceWithArguments(aServiceSpecifier,m_aDefaults);
    }

    uno::Reference< uno::XInterface > SAL_CALL
        ProviderWrapper::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
            throw(uno::Exception, uno::RuntimeException)
    {
        return getDelegate()->createInstanceWithArguments(ServiceSpecifier,patchArguments(Arguments));
    }

    uno::Sequence< OUString > SAL_CALL
        ProviderWrapper::getAvailableServiceNames(  )
            throw(uno::RuntimeException)
    {
        return getDelegate()->getAvailableServiceNames(  );
    }

    /// XServiceInfo
    OUString SAL_CALL
        ProviderWrapper::getImplementationName(  )
            throw(uno::RuntimeException)
    {
        return OUString::createFromAscii("com.sun.star.comp.configuration.ConfigurationProviderWrapper");
    }

    sal_Bool SAL_CALL
        ProviderWrapper::supportsService( const ::rtl::OUString& ServiceName )
            throw(uno::RuntimeException)
    {
        return getDelegateInfo()->supportsService( ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL
        ProviderWrapper::getSupportedServiceNames(  )
            throw(uno::RuntimeException)
    {
        return getDelegateInfo()->getSupportedServiceNames(  );
    }


} // namespace configmgr



