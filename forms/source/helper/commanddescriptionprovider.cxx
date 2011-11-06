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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "commanddescriptionprovider.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
/** === end UNO includes === **/

#include <comphelper/namedvaluecollection.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::beans::PropertyValue;
    /** === end UNO using === **/

    //====================================================================
    //= DefaultCommandDescriptionProvider
    //====================================================================
    class DefaultCommandDescriptionProvider : public ICommandDescriptionProvider
    {
    public:
        DefaultCommandDescriptionProvider( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
        {
            impl_init_nothrow( _rContext, _rxDocument );
        }

        ~DefaultCommandDescriptionProvider()
        {
        }

        // ICommandDescriptionProvider
        virtual ::rtl::OUString getCommandDescription( const ::rtl::OUString& _rCommandURL ) const;

    private:
        void    impl_init_nothrow( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument );

    private:
        Reference< XNameAccess >    m_xCommandAccess;
    };


    //--------------------------------------------------------------------
    void DefaultCommandDescriptionProvider::impl_init_nothrow( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
    {
        OSL_ENSURE( _rxDocument.is(), "DefaultCommandDescriptionProvider::impl_init_nothrow: no document => no command descriptions!" );
        if ( !_rxDocument.is() )
            return;

        try
        {
            Reference< XModuleManager > xModuleManager( _rContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
            ::rtl::OUString sModuleID = xModuleManager->identify( _rxDocument );

            Reference< XNameAccess > xUICommandDescriptions( _rContext.createComponent( "com.sun.star.frame.UICommandDescription" ), UNO_QUERY_THROW );
            m_xCommandAccess.set( xUICommandDescriptions->getByName( sModuleID ), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString DefaultCommandDescriptionProvider::getCommandDescription( const ::rtl::OUString& _rCommandURL ) const
    {
        if ( !m_xCommandAccess.is() )
            return ::rtl::OUString();

        try
        {
            ::comphelper::NamedValueCollection aCommandProperties( m_xCommandAccess->getByName( _rCommandURL ) );
            return aCommandProperties.getOrDefault( "Name", ::rtl::OUString() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return ::rtl::OUString();
    }

    //--------------------------------------------------------------------
    PCommandDescriptionProvider createDocumentCommandDescriptionProvider(
        const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
    {
        PCommandDescriptionProvider pDescriptionProvider( new DefaultCommandDescriptionProvider( _rContext, _rxDocument ) );
        return pDescriptionProvider;
    }

//........................................................................
} // namespace frm
//........................................................................
