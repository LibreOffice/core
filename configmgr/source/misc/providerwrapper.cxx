/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerwrapper.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "providerwrapper.hxx"
#include "bootstrap.hxx"
#include "bootstrapcontext.hxx"


#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <algorithm>


namespace configmgr
{
    //==========================================================================
    namespace uno = com::sun::star::uno;
    namespace lang = com::sun::star::lang;
    //==========================================================================
    //= ProviderWrapper
    //==========================================================================

    uno::Reference< uno::XInterface > ProviderWrapper::create( uno::Reference< uno::XInterface > xDelegate, uno::Sequence< com::sun::star::beans::NamedValue > const & aPresets)
    {
        uno::Reference< lang::XMultiServiceFactory > xProvDelegate(xDelegate, uno::UNO_QUERY);
        if (!xProvDelegate.is())
        {
            rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("ProviderWrapper: Cannot wrap a NULL provider"));
            throw lang::NullPointerException(sMsg,NULL);
        }
        //Strip prefixes
        uno::Sequence< com::sun::star::beans::NamedValue > aStrippedPresets = aPresets;

        for (sal_Int32 i = 0; i < aPresets.getLength(); ++i)
        {
            if(aPresets[i].Name.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_PREFIX_ )))
            {
                aStrippedPresets[i].Name = aPresets[i].Name.copy(RTL_CONSTASCII_LENGTH(CONTEXT_ITEM_PREFIX_ ));
            }
        }

        uno::Reference< lang::XMultiServiceFactory > xResult( new ProviderWrapper(xProvDelegate,aStrippedPresets) );

        DisposingForwarder::forward( uno::Reference< lang::XComponent >::query(xProvDelegate),uno::Reference< lang::XComponent >::query(xResult) );
        return uno::Reference< uno::XInterface >( xResult, uno::UNO_QUERY );
    }


    ProviderWrapper::ProviderWrapper(uno::Reference< lang::XMultiServiceFactory > const & xDelegate, uno::Sequence< com::sun::star::beans::NamedValue > const & aPresets)
    : cppu::WeakComponentImplHelper2< lang::XMultiServiceFactory, lang::XServiceInfo >( PWMutexHolder::mutex )
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

    uno::Reference< lang::XMultiServiceFactory > ProviderWrapper::getDelegate()
    {
        osl::MutexGuard lock(mutex);
        if (!m_xDelegate.is())
        {
            rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("ProviderWrapper: Delegate Provider has been disposed"));
            throw lang::DisposedException(sMsg,*this);
        }
        return m_xDelegate;
    }


    uno::Reference<lang::XServiceInfo> ProviderWrapper::getDelegateInfo()
    {
        uno::Reference<lang::XServiceInfo> xDelegate( this->getDelegate(), uno::UNO_QUERY );
        if (!xDelegate.is())
        {
            rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("ProviderWrapper: Delegate Provider has no service info"));
            throw uno::RuntimeException(sMsg,*this);
        }
        return xDelegate;
    }

    /// XMultiServiceFactory
    static inline uno::Any const * begin(uno::Sequence< uno::Any > const & aArgs)
    { return aArgs.getConstArray(); }
    static inline uno::Any const * end(uno::Sequence< uno::Any > const & aArgs)
    { return aArgs.getConstArray() + aArgs.getLength(); }
    static inline uno::Any * begin(uno::Sequence< uno::Any > & aArgs)
    { return aArgs.getArray(); }
    static inline uno::Any * end(uno::Sequence< uno::Any > & aArgs)
    { return aArgs.getArray() + aArgs.getLength(); }

    uno::Sequence< uno::Any > ProviderWrapper::patchArguments(uno::Sequence< uno::Any > const & aArgs) const
    {
        // rely on evaluation order front to back
        if (m_aDefaults.getLength() == 0) return aArgs;

        uno::Sequence< uno::Any > aResult(m_aDefaults.getLength() + aArgs.getLength());

        uno::Any * pNext = std::copy(begin(m_aDefaults),end(m_aDefaults),begin(aResult));
        pNext = std::copy(begin(aArgs),end(aArgs),pNext);

        OSL_ASSERT(end(aResult) == pNext);

        return aResult;
    }

    uno::Reference< uno::XInterface > SAL_CALL
        ProviderWrapper::createInstance( const rtl::OUString& aServiceSpecifier )
            throw(uno::Exception, uno::RuntimeException)
    {
        return getDelegate()->createInstanceWithArguments(aServiceSpecifier,m_aDefaults);
    }

    uno::Reference< uno::XInterface > SAL_CALL
        ProviderWrapper::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& rArguments )
            throw(uno::Exception, uno::RuntimeException)
    {
        return getDelegate()->createInstanceWithArguments(ServiceSpecifier,patchArguments(rArguments));
    }

    uno::Sequence< rtl::OUString > SAL_CALL
        ProviderWrapper::getAvailableServiceNames(  )
            throw(uno::RuntimeException)
    {
        return getDelegate()->getAvailableServiceNames(  );
    }

    /// XServiceInfo
    rtl::OUString SAL_CALL
        ProviderWrapper::getImplementationName(  )
            throw(uno::RuntimeException)
    {
        return rtl::OUString::createFromAscii("com.sun.star.comp.configuration.ConfigurationProviderWrapper");
    }

    sal_Bool SAL_CALL
        ProviderWrapper::supportsService( const ::rtl::OUString& ServiceName )
            throw(uno::RuntimeException)
    {
        return getDelegateInfo()->supportsService( ServiceName );
    }

    uno::Sequence< rtl::OUString > SAL_CALL
        ProviderWrapper::getSupportedServiceNames(  )
            throw(uno::RuntimeException)
    {
        return getDelegateInfo()->getSupportedServiceNames(  );
    }


} // namespace configmgr



