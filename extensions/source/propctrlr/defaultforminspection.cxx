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

#include "defaultforminspection.hxx"
#include "pcrcommon.hxx"
#include "propresid.hrc"
#include "formresid.hrc"
#include "modulepcr.hxx"
#include "propctrlr.hrc"
#include "formmetadata.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DefaultFormComponentInspectorModel()
{
    ::pcr::OAutoRegistration< ::pcr::DefaultFormComponentInspectorModel > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::inspection::PropertyCategoryDescriptor;
    using ::com::sun::star::beans::UnknownPropertyException;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::lang::IllegalArgumentException;

    //====================================================================
    //= DefaultFormComponentInspectorModel
    //====================================================================
    //--------------------------------------------------------------------
    DefaultFormComponentInspectorModel::DefaultFormComponentInspectorModel( bool _bUseFormFormComponentHandlers )
        :ImplInspectorModel()
        ,m_bUseFormComponentHandlers( _bUseFormFormComponentHandlers )
        ,m_bConstructed( false )
        ,m_pInfoService( new OPropertyInfoService )
    {
    }

    //------------------------------------------------------------------------
    DefaultFormComponentInspectorModel::~DefaultFormComponentInspectorModel()
    {
    }

    //------------------------------------------------------------------------
    OUString SAL_CALL DefaultFormComponentInspectorModel::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //------------------------------------------------------------------------
    Sequence< OUString > SAL_CALL DefaultFormComponentInspectorModel::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //------------------------------------------------------------------------
    OUString DefaultFormComponentInspectorModel::getImplementationName_static(  ) throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.extensions.DefaultFormComponentInspectorModel");
    }

    //------------------------------------------------------------------------
    Sequence< OUString > DefaultFormComponentInspectorModel::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = OUString("com.sun.star.form.inspection.DefaultFormComponentInspectorModel");
        return aSupported;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL DefaultFormComponentInspectorModel::Create( const Reference< XComponentContext >& )
    {
        return *new DefaultFormComponentInspectorModel();
    }

    //--------------------------------------------------------------------
    Sequence< Any > SAL_CALL DefaultFormComponentInspectorModel::getHandlerFactories() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // service names for all our handlers
        struct
        {
            const sal_Char* serviceName;
            bool            isFormOnly;
        } aFactories[] = {

            // a generic handler for form component properties (must precede the ButtonNavigationHandler)
            { "com.sun.star.form.inspection.FormComponentPropertyHandler", false },

            // generic virtual edit properties
            { "com.sun.star.form.inspection.EditPropertyHandler", false },

            // a handler which virtualizes the ButtonType property, to provide additional types like
            // "move to next record"
            { "com.sun.star.form.inspection.ButtonNavigationHandler", false },

            // a handler for script events bound to form components or dialog elements
            { "com.sun.star.form.inspection.EventHandler", false },

            // a handler which introduces virtual properties for binding controls to spreadsheet cells
            { "com.sun.star.form.inspection.CellBindingPropertyHandler", false },

            // properties related to binding to an XForms DOM node
            { "com.sun.star.form.inspection.XMLFormsPropertyHandler", true },

            // properties related to the XSD data against which a control content is validated
            { "com.sun.star.form.inspection.XSDValidationPropertyHandler", true },

            // a handler which cares for XForms submissions
            { "com.sun.star.form.inspection.SubmissionPropertyHandler", true },

            // a handler which cares for geometry properties of form controls
            { "com.sun.star.form.inspection.FormGeometryHandler", true }
        };

        sal_Int32 nFactories = SAL_N_ELEMENTS( aFactories );
        Sequence< Any > aReturn( nFactories );
        Any* pReturn = aReturn.getArray();
        for ( sal_Int32 i = 0; i < nFactories; ++i )
        {
            if ( aFactories[i].isFormOnly && !m_bUseFormComponentHandlers )
                continue;
            *pReturn++ <<= OUString::createFromAscii( aFactories[i].serviceName );
        }
        aReturn.realloc( pReturn - aReturn.getArray() );

        return aReturn;
    }

    //--------------------------------------------------------------------
    Sequence< PropertyCategoryDescriptor > SAL_CALL DefaultFormComponentInspectorModel::describeCategories(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        struct
        {
            const sal_Char* programmaticName;
            sal_uInt16          uiNameResId;
            const sal_Char* helpId;
        } aCategories[] = {
            { "General",    RID_STR_PROPPAGE_DEFAULT,   HID_FM_PROPDLG_TAB_GENERAL },
            { "Data",       RID_STR_PROPPAGE_DATA,      HID_FM_PROPDLG_TAB_DATA },
            { "Events",     RID_STR_EVENTS,             HID_FM_PROPDLG_TAB_EVT }
        };

        sal_Int32 nCategories = SAL_N_ELEMENTS( aCategories );
        Sequence< PropertyCategoryDescriptor > aReturn( nCategories );
        PropertyCategoryDescriptor* pReturn = aReturn.getArray();
        for ( sal_Int32 i=0; i<nCategories; ++i, ++pReturn )
        {
            pReturn->ProgrammaticName = OUString::createFromAscii( aCategories[i].programmaticName );
            pReturn->UIName = PcrRes( aCategories[i].uiNameResId ).toString();
            pReturn->HelpURL = HelpIdUrl::getHelpURL( aCategories[i].helpId );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultFormComponentInspectorModel::getPropertyOrderIndex( const OUString& _rPropertyName ) throw (RuntimeException)
    {
        sal_Int32 nPropertyId( m_pInfoService->getPropertyId( _rPropertyName ) );
        if ( nPropertyId == -1 )
        {
            if ( _rPropertyName.indexOf( ';' ) != -1 )
                // it's an event. Just give it an arbitrary number - events will be on a separate
                // page, and by definition, if two properties have the same OrderIndex, then
                // they will be ordered as they appear in the handler's getSupportedProperties.
                return 1000;
            return 0;
        }
        return m_pInfoService->getPropertyPos( nPropertyId );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DefaultFormComponentInspectorModel::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException)
    {
        if ( m_bConstructed )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.empty() )
        {   // constructor: "createDefault()"
            createDefault();
            return;
        }

        sal_Int32 nMinHelpTextLines( 0 ), nMaxHelpTextLines( 0 );
        if ( arguments.size() == 2 )
        {   // constructor: "createWithHelpSection( long, long )"
            if ( !( arguments[0] >>= nMinHelpTextLines ) || !( arguments[1] >>= nMaxHelpTextLines ) )
                throw IllegalArgumentException( OUString(), *this, 0 );
            createWithHelpSection( nMinHelpTextLines, nMaxHelpTextLines );
            return;
        }

        throw IllegalArgumentException( OUString(), *this, 0 );
    }

    //--------------------------------------------------------------------
    void DefaultFormComponentInspectorModel::createDefault()
    {
        m_bConstructed = true;
    }

    //--------------------------------------------------------------------
    void DefaultFormComponentInspectorModel::createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines )
    {
        if ( ( _nMinHelpTextLines <= 0 ) || ( _nMaxHelpTextLines <= 0 ) || ( _nMinHelpTextLines > _nMaxHelpTextLines ) )
            throw IllegalArgumentException( OUString(), *this, 0 );

        enableHelpSectionProperties( _nMinHelpTextLines, _nMaxHelpTextLines );
        m_bConstructed = true;
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
