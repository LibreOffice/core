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


#include "XFormsModelContext.hxx"
#include <vector>
#include <utility>
#include "xmloff/xformsimport.hxx"
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
using rtl::OUString;

SvXMLImportContext* createXFormsModelContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName )
{
    return new XFormsModelContext( rImport, nPrefix, rLocalName );
}

void bindXFormsValueBinding(
    Reference<XModel> xModel,
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
    Reference<XModel> xModel,
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
    Reference<XModel> xModel,
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
        Sequence< ::rtl::OUString > aSettingsForModels( xModelSettings->getElementNames() );
        for (   const ::rtl::OUString* pModelName = aSettingsForModels.getConstArray();
                pModelName != aSettingsForModels.getConstArray() + aSettingsForModels.getLength();
                ++pModelName
            )
        {
            // the settings for this particular model
            Sequence< PropertyValue > aModelSettings;
            OSL_VERIFY( xModelSettings->getByName( *pModelName ) >>= aModelSettings );

            // the model itself
            if ( !_rXForms->hasByName( *pModelName ) )
            {
                OSL_FAIL( "applyXFormsSettings: have settings for a non-existent XForms model!" );
                continue;
            }

            // propagate the settings, being tolerant by omitting properties which are not supported
            Reference< XPropertySet > xModelProps( _rXForms->getByName( *pModelName ), UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xModelPSI( xModelProps->getPropertySetInfo(), UNO_SET_THROW );

            for (   const PropertyValue* pSetting = aModelSettings.getConstArray();
                    pSetting != aModelSettings.getConstArray() + aModelSettings.getLength();
                    ++pSetting
                )
            {
                if ( !xModelPSI->hasPropertyByName( pSetting->Name ) )
                {
                    OSL_FAIL( "applyXFormsSettings: non-existent model property!" );
                    continue;
                }

                xModelProps->setPropertyValue( pSetting->Name, pSetting->Value );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
