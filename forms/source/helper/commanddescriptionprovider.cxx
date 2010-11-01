/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
