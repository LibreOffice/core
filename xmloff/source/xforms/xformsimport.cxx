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


#include "XFormsModelContext.hxx"
#include <vector>
#include <utility>
#include <xmloff/xformsimport.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/submission/XSubmission.hpp>
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <rtl/ustring.hxx>
#include <xformsapi.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <tools/diagnose_ex.h>

using std::pair;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::UNO_SET_THROW;
using com::sun::star::uno::Sequence;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::PropertyValue;
using com::sun::star::frame::XModel;
using com::sun::star::container::XNameAccess;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::form::binding::XBindableValue;
using com::sun::star::form::binding::XListEntrySource;
using com::sun::star::form::binding::XListEntrySink;
using com::sun::star::form::submission::XSubmission;
using com::sun::star::form::submission::XSubmissionSupplier;

SvXMLImportContext* createXFormsModelContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName )
{
    return new XFormsModelContext( rImport, nPrefix, rLocalName );
}

void bindXFormsValueBinding(
    Reference<XModel> const & xModel,
    pair<Reference<XPropertySet>,OUString> aPair )
{
    Reference<XBindableValue> xBindable(
        aPair.first,
        UNO_QUERY );
    Reference<XValueBinding> xBinding(
        xforms_findXFormsBinding( xModel, aPair.second ),
        UNO_QUERY );

    if( xBindable.is() && xBinding.is() )
    {
        try
        {
            xBindable->setValueBinding( xBinding );
        }
        catch( const Exception& )
        {
            // ignore problems during binding
            // TODO: call XML error handling
        }
    }
}

void bindXFormsListBinding(
    Reference<XModel> const & xModel,
    ::pair<Reference<XPropertySet>,OUString> aPair )
{
    Reference<XListEntrySink> xListEntrySink(
        aPair.first,
        UNO_QUERY );
    Reference<XListEntrySource> xListEntrySource(
        xforms_findXFormsBinding( xModel, aPair.second ),
        UNO_QUERY );

    if( xListEntrySink.is() && xListEntrySource.is() )
    {
        try
        {
            xListEntrySink->setListEntrySource( xListEntrySource );
        }
        catch( const Exception& )
        {
            // ignore problems during binding
            // TODO: call XML error handling
        }
    }
}

void bindXFormsSubmission(
    Reference<XModel> const & xModel,
    pair<Reference<XPropertySet>,OUString> aPair )
{
    Reference<XSubmissionSupplier> xSubmissionSupp( aPair.first, UNO_QUERY );
    Reference<XSubmission> xSubmission(
        xforms_findXFormsSubmission( xModel, aPair.second ),
        UNO_QUERY );

    if( xSubmissionSupp.is() && xSubmission.is() )
    {
        try
        {
            xSubmissionSupp->setSubmission( xSubmission );
        }
        catch( const Exception& )
        {
            // ignore problems during binding
            // TODO: call XML error handling
        }
    }
}

void applyXFormsSettings( const Reference< XNameAccess >& _rXForms, const Sequence< PropertyValue >& _rSettings )
{
    OSL_PRECOND( _rXForms.is(), "applyXFormsSettings: invalid XForms container!" );
    if ( !_rXForms.is() )
        return;

    ::comphelper::NamedValueCollection aSettings( _rSettings );
    Reference< XNameAccess > xModelSettings( aSettings.get( "XFormModels" ), UNO_QUERY );
    if ( !xModelSettings.is() )
    {
        OSL_FAIL( "applyXFormsSettings: wrong type for the XFormModels settings!" );
        return;
    }

    try
    {
        Sequence< OUString > aSettingsForModels( xModelSettings->getElementNames() );
        for ( auto const & modelName : aSettingsForModels )
        {
            // the settings for this particular model
            Sequence< PropertyValue > aModelSettings;
            OSL_VERIFY( xModelSettings->getByName( modelName ) >>= aModelSettings );

            // the model itself
            if ( !_rXForms->hasByName( modelName ) )
            {
                OSL_FAIL( "applyXFormsSettings: have settings for a non-existent XForms model!" );
                continue;
            }

            // propagate the settings, being tolerant by omitting properties which are not supported
            Reference< XPropertySet > xModelProps( _rXForms->getByName( modelName ), UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xModelPSI( xModelProps->getPropertySetInfo(), UNO_SET_THROW );

            for ( auto const & setting : aModelSettings )
            {
                if ( !xModelPSI->hasPropertyByName( setting.Name ) )
                {
                    OSL_FAIL( "applyXFormsSettings: non-existent model property!" );
                    continue;
                }

                xModelProps->setPropertyValue( setting.Name, setting.Value );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
