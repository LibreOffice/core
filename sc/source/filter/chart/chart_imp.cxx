/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/docfile.hxx>
#include "xmlwrap.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;

ScXMLChartExportWrapper::ScXMLChartExportWrapper( css::uno::Reference< css::frame::XModel > xModel, SfxMedium& rMed ):
    mxModel(xModel),
    mrMedium(rMed)
{

}

bool ScXMLChartExportWrapper::Export()
{
    if ( !mxStorage.is() )
        mxStorage = mrMedium.GetOutputStorage();

    uno::Reference< document::XStorageBasedDocument>(mxModel, uno::UNO_QUERY_THROW)->storeToStorage(mxStorage, uno::Sequence< beans::PropertyValue >() );
    return true;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
