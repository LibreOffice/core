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
#include <helpids.h>
#include <strings.hrc>
#include "modulepcr.hxx"
#include "formmetadata.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sal/macros.h>


namespace pcr
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::inspection::PropertyCategoryDescriptor;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::lang::IllegalArgumentException;

    DefaultFormComponentInspectorModel::DefaultFormComponentInspectorModel( bool _bUseFormFormComponentHandlers )
        :ImplInspectorModel()
        ,m_bUseFormComponentHandlers( _bUseFormFormComponentHandlers )
        ,m_bConstructed( false )
        ,m_pInfoService( new OPropertyInfoService )
    {
    }


    DefaultFormComponentInspectorModel::~DefaultFormComponentInspectorModel()
    {
    }


    OUString SAL_CALL DefaultFormComponentInspectorModel::getImplementationName(  )
    {
        return "org.openoffice.comp.extensions.DefaultFormComponentInspectorModel";
    }


    Sequence< OUString > SAL_CALL DefaultFormComponentInspectorModel::getSupportedServiceNames(  )
    {
        return { "com.sun.star.form.inspection.DefaultFormComponentInspectorModel" };
    }


    Sequence< Any > SAL_CALL DefaultFormComponentInspectorModel::getHandlerFactories()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // service names for all our handlers
        static struct
        {
            const char* serviceName;
            bool        isFormOnly;
        } const aFactories[] = {

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


    Sequence< PropertyCategoryDescriptor > SAL_CALL DefaultFormComponentInspectorModel::describeCategories(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        static struct
        {
            const char* programmaticName;
            const char* uiNameResId;
            const char* helpId;
        } const aCategories[] = {
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
            pReturn->UIName = PcrRes( aCategories[i].uiNameResId );
            pReturn->HelpURL = HelpIdUrl::getHelpURL( aCategories[i].helpId );
        }

        return aReturn;
    }


    ::sal_Int32 SAL_CALL DefaultFormComponentInspectorModel::getPropertyOrderIndex( const OUString& _rPropertyName )
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


    void SAL_CALL DefaultFormComponentInspectorModel::initialize( const Sequence< Any >& _arguments )
    {
        if ( m_bConstructed )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.empty() )
        {   // constructor: "createDefault()"
            m_bConstructed = true;
            return;
        }

        if ( arguments.size() == 2 )
        {   // constructor: "createWithHelpSection( long, long )"
            sal_Int32 nMinHelpTextLines( 0 ), nMaxHelpTextLines( 0 );
            if ( !( arguments[0] >>= nMinHelpTextLines ) || !( arguments[1] >>= nMaxHelpTextLines ) )
                throw IllegalArgumentException( OUString(), *this, 0 );
            createWithHelpSection( nMinHelpTextLines, nMaxHelpTextLines );
            return;
        }

        throw IllegalArgumentException( OUString(), *this, 0 );
    }


    void DefaultFormComponentInspectorModel::createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines )
    {
        if ( ( _nMinHelpTextLines <= 0 ) || ( _nMaxHelpTextLines <= 0 ) || ( _nMinHelpTextLines > _nMaxHelpTextLines ) )
            throw IllegalArgumentException( OUString(), *this, 0 );

        enableHelpSectionProperties( _nMinHelpTextLines, _nMaxHelpTextLines );
        m_bConstructed = true;
    }


} // namespace pcr

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propctrlr_DefaultFormComponentInspectorModel_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::DefaultFormComponentInspectorModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
