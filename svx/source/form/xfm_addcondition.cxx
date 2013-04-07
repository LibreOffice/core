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


#include "xfm_addcondition.hxx"

#include <comphelper/processfactory.hxx>
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
    Sequence< OUString > SAL_CALL OAddConditionDialog_GetSupportedServiceNames()
    {
        ::comphelper::StringSequence aSupported( 1 );
        aSupported.getArray()[0] = OUString( "com.sun.star.xforms.ui.dialogs.AddCondition"  );
        return aSupported;
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL OAddConditionDialog_GetImplementationName()
    {
        return OUString("org.openoffice.comp.svx.OAddConditionDialog");
    }

    //====================================================================
    //= OAddConditionDialog
    //====================================================================
    //--------------------------------------------------------------------
    OAddConditionDialog::OAddConditionDialog( const Reference< XComponentContext >& _rxORB )
        :OAddConditionDialogBase( _rxORB )
    {
        registerProperty(
            OUString( "Binding"  ),
            PROPERTY_ID_BINDING,
            PropertyAttribute::TRANSIENT,
            &m_xBinding,
            ::getCppuType( &m_xBinding )
        );

        registerProperty(
            OUString( "FacetName"  ),
            PROPERTY_ID_FACET_NAME,
            PropertyAttribute::TRANSIENT,
            &m_sFacetName,
            ::getCppuType( &m_sFacetName )
        );

        registerProperty(
            OUString( "ConditionValue"  ),
            PROPERTY_ID_CONDITION_VALUE,
            PropertyAttribute::TRANSIENT,
            &m_sConditionValue,
            ::getCppuType( &m_sConditionValue )
        );

        registerProperty(
            OUString( "FormModel"  ),
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
        return *( new OAddConditionDialog( comphelper::getComponentContext(_rxFactory) ) );
    }

    //-------------------------------------------------------------------------
    OUString SAL_CALL OAddConditionDialog::getImplementationName() throw(RuntimeException)
    {
        return OAddConditionDialog_GetImplementationName();
    }

    //-------------------------------------------------------------------------
    Sequence< OUString > SAL_CALL OAddConditionDialog::getSupportedServiceNames() throw(RuntimeException)
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
        if ( !m_xBinding.is() || m_sFacetName.isEmpty() )
            throw RuntimeException( OUString(), *this );

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
