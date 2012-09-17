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
#include <ModelEventListener.hxx>
#include <PropertyIds.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;




ModelEventListener::ModelEventListener()
{
}


ModelEventListener::~ModelEventListener()
{
}


void ModelEventListener::notifyEvent( const document::EventObject& rEvent ) throw (uno::RuntimeException)
{
    if ( rEvent.EventName == "OnFocus" )
    {
        try
        {
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

            uno::Reference< text::XDocumentIndexesSupplier> xIndexesSupplier( rEvent.Source, uno::UNO_QUERY );
            //remove listener
            uno::Reference<document::XEventBroadcaster>(rEvent.Source, uno::UNO_QUERY )->removeEventListener(
            uno::Reference<document::XEventListener>(this));

            uno::Reference< container::XIndexAccess > xIndexes = xIndexesSupplier->getDocumentIndexes();

            sal_Int32 nIndexes = xIndexes->getCount();
            for( sal_Int32 nIndex = 0; nIndex < nIndexes; ++nIndex)
            {
                uno::Reference< text::XDocumentIndex> xIndex( xIndexes->getByIndex( nIndex ), uno::UNO_QUERY );
                xIndex->update();
            }

            // If we have PAGEREF fields, update fields as well.
            uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(rEvent.Source, uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xEnumeration(xTextFieldsSupplier->getTextFields()->createEnumeration(), uno::UNO_QUERY);
            sal_Int32 nIndex = 0;
            while(xEnumeration->hasMoreElements())
            {
                try
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(xEnumeration->nextElement(), uno::UNO_QUERY);
                    sal_Int16 nSource = 0;
                    xPropertySet->getPropertyValue(rPropNameSupplier.GetName(PROP_REFERENCE_FIELD_SOURCE)) >>= nSource;
                    sal_Int16 nPart = 0;
                    xPropertySet->getPropertyValue(rPropNameSupplier.GetName(PROP_REFERENCE_FIELD_PART)) >>= nPart;
                    if (nSource == text::ReferenceFieldSource::BOOKMARK && nPart == text::ReferenceFieldPart::PAGE)
                        ++nIndex;
                }
                catch( const beans::UnknownPropertyException& )
                {
                    // doesn't even have such a property? ignore
                }
            }
            if (nIndex)
            {
                uno::Reference<util::XRefreshable> xRefreshable(xTextFieldsSupplier->getTextFields(), uno::UNO_QUERY);
                xRefreshable->refresh();
            }
        }
        catch( const uno::Exception& rEx )
        {
            SAL_WARN("writerfilter", "exception while updating indexes: " << rEx.Message);
        }
    }
}


void ModelEventListener::disposing( const lang::EventObject& rEvent ) throw (uno::RuntimeException)
{
    try
    {
        uno::Reference<document::XEventBroadcaster>(rEvent.Source, uno::UNO_QUERY )->removeEventListener(
            uno::Reference<document::XEventListener>(this));
    }
    catch( const uno::Exception& )
    {
    }
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
