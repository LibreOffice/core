/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xformsimport.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:32:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "xformsimport.hxx"
#include "XFormsModelContext.hxx"
#include <vector>
#include <utility>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/submission/XSubmission.hpp>
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
#include <rtl/ustring.hxx>
#include <xformsapi.hxx>

using std::pair;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::beans::XPropertySet;
using com::sun::star::frame::XModel;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::form::binding::XBindableValue;
using com::sun::star::form::binding::XListEntrySource;
using com::sun::star::form::binding::XListEntrySink;
using com::sun::star::form::submission::XSubmission;
using com::sun::star::form::submission::XSubmissionSupplier;
using rtl::OUString;

SvXMLImportContext* createXFormsModelContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
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
        lcl_findXFormsBinding( xModel, aPair.second ),
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
    pair<Reference<XPropertySet>,OUString> aPair )
{
    Reference<XListEntrySink> xListEntrySink(
        aPair.first,
        UNO_QUERY );
    Reference<XListEntrySource> xListEntrySource(
        lcl_findXFormsBinding( xModel, aPair.second ),
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
        lcl_findXFormsSubmission( xModel, aPair.second ),
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
