/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#include "precompiled_svx.hxx"

#include "xfm_addcondition.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <cppuhelper/typeprovider.hxx>
#include "datanavi.hxx"
#include <vcl/msgbox.hxx>

//........................................................................
namespace svxform
{
//........................................................................

#define PROPERTY_ID_BINDING             5724
#define PROPERTY_ID_FORM_MODEL          5725
#define PROPERTY_ID_FACET_NAME          5726
#define PROPERTY_ID_CONDITION_VALUE     5727

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xforms;

    //====================================================================
    //= OAddConditionDialog
    //====================================================================
    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAddConditionDialog_Create( const Reference< XMultiServiceFactory > & _rxORB )
    {
        return OAddConditionDialog::Create( _rxORB );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OAddConditionDialog_GetSupportedServiceNames()
    {
        ::comphelper::StringSequence aSupported( 1 );
        aSupported.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xforms.ui.dialogs.AddCondition" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAddConditionDialog_GetImplementationName()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.svx.OAddConditionDialog"));
    }

    //====================================================================
    //= OAddConditionDialog
    //====================================================================
    //--------------------------------------------------------------------
    OAddConditionDialog::OAddConditionDialog( const Reference< XMultiServiceFactory >& _rxORB )
        :OAddConditionDialogBase( _rxORB )
    {
        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Binding" ) ),
            PROPERTY_ID_BINDING,
            PropertyAttribute::TRANSIENT,
            &m_xBinding,
            ::getCppuType( &m_xBinding )
        );

        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FacetName" ) ),
            PROPERTY_ID_FACET_NAME,
            PropertyAttribute::TRANSIENT,
            &m_sFacetName,
            ::getCppuType( &m_sFacetName )
        );

        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConditionValue" ) ),
            PROPERTY_ID_CONDITION_VALUE,
            PropertyAttribute::TRANSIENT,
            &m_sConditionValue,
            ::getCppuType( &m_sConditionValue )
        );

        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FormModel" ) ),
            PROPERTY_ID_FORM_MODEL,
            PropertyAttribute::TRANSIENT,
            &m_xWorkModel,
            ::getCppuType( &m_xWorkModel )
        );
    }

    //-------------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OAddConditionDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId * pId = 0;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //-------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAddConditionDialog::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new OAddConditionDialog( _rxFactory ) );
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAddConditionDialog::getImplementationName() throw(RuntimeException)
    {
        return OAddConditionDialog_GetImplementationName();
    }

    //-------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OAddConditionDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return OAddConditionDialog_GetSupportedServiceNames();
    }

    //-------------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OAddConditionDialog::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OAddConditionDialog::getInfoHelper()
    {
        return *const_cast< OAddConditionDialog* >( this )->getArrayHelper();
    }

    //------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OAddConditionDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProperties;
        describeProperties( aProperties );
        return new ::cppu::OPropertyArrayHelper( aProperties );
    }

    //------------------------------------------------------------------------------
    Dialog* OAddConditionDialog::createDialog(Window* _pParent)
    {
        if ( !m_xBinding.is() || !m_sFacetName.getLength() )
            throw RuntimeException( ::rtl::OUString(), *this );

        return new AddConditionDialog( _pParent, m_sFacetName, m_xBinding );
    }

    //------------------------------------------------------------------------------
    void OAddConditionDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        OAddConditionDialogBase::executedDialog( _nExecutionResult );
        if ( _nExecutionResult == RET_OK )
            m_sConditionValue = static_cast< AddConditionDialog* >( m_pDialog )->GetCondition();
    }

//........................................................................
} // namespace svxformv
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
