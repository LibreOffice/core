/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_comphelper.hxx"

#include "comphelper/ihwrapnofilter.hxx"
#include <com/sun/star/document/NoSuchFilterRequest.hpp>

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star;

    //----------------------------------------------------------------------------------------------------
    OIHWrapNoFilterDialog::OIHWrapNoFilterDialog( uno::Reference< task::XInteractionHandler > xInteraction )
        :m_xInter( xInteraction )
    {
    }

    OIHWrapNoFilterDialog::~OIHWrapNoFilterDialog()
    {
    }

    //----------------------------------------------------------------------------------------------------
    uno::Sequence< ::rtl::OUString > SAL_CALL OIHWrapNoFilterDialog::impl_staticGetSupportedServiceNames()
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.task.InteractionHandlerWrapper");
        return aRet;
    }

    ::rtl::OUString SAL_CALL OIHWrapNoFilterDialog::impl_staticGetImplementationName()
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.task.InteractionHandlerWrapper");
    }

    //----------------------------------------------------------------------------------------------------
    // XInteractionHandler
    //----------------------------------------------------------------------------------------------------
    void SAL_CALL OIHWrapNoFilterDialog::handle( const uno::Reference< task::XInteractionRequest >& xRequest)
            throw( com::sun::star::uno::RuntimeException )
    {
        if( !m_xInter.is() )
            return;

        uno::Any aRequest = xRequest->getRequest();
        document::NoSuchFilterRequest aNoSuchFilterRequest;
        if ( aRequest >>= aNoSuchFilterRequest )
            return;
        else
            m_xInter->handle( xRequest );
    }

    //----------------------------------------------------------------------------------------------------
    // XInitialization
    //----------------------------------------------------------------------------------------------------
    void SAL_CALL OIHWrapNoFilterDialog::initialize( const uno::Sequence< uno::Any >& )
        throw ( uno::Exception,
        uno::RuntimeException,
        frame::DoubleInitializationException )
    {
    }

    //----------------------------------------------------------------------------------------------------
    // XServiceInfo
    //----------------------------------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL OIHWrapNoFilterDialog::getImplementationName()
        throw ( uno::RuntimeException )
    {
        return impl_staticGetImplementationName();
    }

    ::sal_Bool SAL_CALL OIHWrapNoFilterDialog::supportsService( const ::rtl::OUString& ServiceName )
        throw ( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

        for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
            if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
                return sal_True;

        return sal_False;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL OIHWrapNoFilterDialog::getSupportedServiceNames()
        throw ( uno::RuntimeException )
    {
        return impl_staticGetSupportedServiceNames();
    }
}
