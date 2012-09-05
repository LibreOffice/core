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


#include "commanddescriptionprovider.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/UICommandDescription.hpp>

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
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::frame::ModuleManager;
    using ::com::sun::star::frame::XModuleManager2;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::ui::UICommandDescription;
    /** === end UNO using === **/

    //====================================================================
    //= DefaultCommandDescriptionProvider
    //====================================================================
    class DefaultCommandDescriptionProvider : public ICommandDescriptionProvider
    {
    public:
        DefaultCommandDescriptionProvider( const Reference<XComponentContext>& _rxContext, const Reference< XModel >& _rxDocument )
        {
            impl_init_nothrow( _rxContext, _rxDocument );
        }

        ~DefaultCommandDescriptionProvider()
        {
        }

        // ICommandDescriptionProvider
        virtual ::rtl::OUString getCommandDescription( const ::rtl::OUString& _rCommandURL ) const;

    private:
        void    impl_init_nothrow( const Reference<XComponentContext>& _rxContext, const Reference< XModel >& _rxDocument );

    private:
        Reference< XNameAccess >    m_xCommandAccess;
    };


    //--------------------------------------------------------------------
    void DefaultCommandDescriptionProvider::impl_init_nothrow( const Reference<XComponentContext>& _rxContext, const Reference< XModel >& _rxDocument )
    {
        OSL_ENSURE( _rxDocument.is(), "DefaultCommandDescriptionProvider::impl_init_nothrow: no document => no command descriptions!" );
        if ( !_rxDocument.is() )
            return;

        try
        {
            Reference< XModuleManager2 > xModuleManager( ModuleManager::create(_rxContext) );
            ::rtl::OUString sModuleID = xModuleManager->identify( _rxDocument );

            Reference< XNameAccess > xUICommandDescriptions( UICommandDescription::create(_rxContext) );
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
        const Reference<XComponentContext>& _rxContext, const Reference< XModel >& _rxDocument )
    {
        PCommandDescriptionProvider pDescriptionProvider( new DefaultCommandDescriptionProvider( _rxContext, _rxDocument ) );
        return pDescriptionProvider;
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
