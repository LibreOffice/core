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


#include "fileopendialog.hxx"
#include <sal/types.h>
#include "pppoptimizertoken.hxx"
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/view/XControlAccess.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;

FileOpenDialog::FileOpenDialog( const Reference< XComponentContext >& rxContext )
{
    mxFilePicker = FilePicker::createWithMode( rxContext, TemplateDescription::FILESAVE_AUTOEXTENSION);
    mxFilePicker->setMultiSelectionMode( false );

    Reference< XFilePickerControlAccess > xAccess( mxFilePicker, UNO_QUERY );
    if ( xAccess.is() )
    {
        try
        {
            xAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, Any( true ) );
        }
        catch( css::uno::Exception& )
        {}
    }

    // collecting a list of impress filters
    Reference< XNameAccess > xFilters( rxContext->getServiceManager()->createInstanceWithContext(
        u"com.sun.star.document.FilterFactory"_ustr, rxContext ), UNO_QUERY_THROW );
    const Sequence< OUString > aFilterList( xFilters->getElementNames() );
    for ( const auto& rFilter : aFilterList )
    {
        try
        {
            Sequence< PropertyValue > aFilterProperties;
            if ( xFilters->getByName( rFilter ) >>= aFilterProperties )
            {
                FilterEntry aFilterEntry;
                bool bImpressFilter = false;
                for (const PropertyValue& rProperty : aFilterProperties)
                {
                    bool bStop = false;
                    switch( TKGet( rProperty.Name ) )
                    {
                        case TK_DocumentService :
                        {
                            OUString sDocumentService;
                            rProperty.Value >>= sDocumentService;
                            if ( sDocumentService == "com.sun.star.presentation.PresentationDocument" )
                                bImpressFilter = true;
                            else
                                bStop = true;
                        }
                        break;
                        case TK_Name :      rProperty.Value >>= aFilterEntry.maFilterEntryName; break;
                        case TK_UIName :    rProperty.Value >>= aFilterEntry.maUIName; break;
                        case TK_Type :      rProperty.Value >>= aFilterEntry.maType; break;
                        case TK_Flags :     rProperty.Value >>= aFilterEntry.maFlags; break;
                        default : break;
                    }

                    if (bStop)
                        break;
                }
                if ( bImpressFilter && ( ( aFilterEntry.maFlags & 3 ) == 3 ) )
                {
                    aFilterEntryList.push_back( aFilterEntry );
                }
            }
        }
        catch( Exception& )
        {
        }
    }

    Reference< XNameAccess > xTypes( rxContext->getServiceManager()->createInstanceWithContext(
        u"com.sun.star.document.TypeDetection"_ustr, rxContext ), UNO_QUERY_THROW );

    for (auto& rFilterEntry : aFilterEntryList)
    {
        Sequence< PropertyValue > aTypeProperties;
        try
        {
            if ( xTypes->getByName( rFilterEntry.maType ) >>= aTypeProperties )
            {
                Sequence< OUString > aExtensions;
                auto pProp = std::find_if(std::cbegin(aTypeProperties), std::cend(aTypeProperties),
                    [](const PropertyValue& rProp) { return rProp.Name == "Extensions"; });
                if (pProp != std::cend(aTypeProperties))
                    pProp->Value >>= aExtensions;
                if ( aExtensions.hasElements() )
                {
                    // The filter title must be formed in the same way it is currently done in the
                    // internal implementation (see sfx2::appendFiltersForSave). And we will look
                    // for the same string returned from the dialog, so save it to maUIName:
                    OUString aTitle(
                        rFilterEntry.maUIName + " (." + aExtensions[0] + ")");
                    rFilterEntry.maUIName = aTitle;
                    OUString aFilter("*." + aExtensions[0]);
                    mxFilePicker->appendFilter(aTitle, aFilter);
                    if ( rFilterEntry.maFlags & 0x100 )
                        mxFilePicker->setCurrentFilter(aTitle);
                }
            }
        }
        catch ( const Exception& )
        {
        }
    }
}
FileOpenDialog::~FileOpenDialog()
{
}
sal_Int16 FileOpenDialog::execute()
{
    return mxFilePicker->execute();
}
void FileOpenDialog::setDefaultName( const OUString& rDefaultName )
{
    mxFilePicker->setDefaultName( rDefaultName );
}
OUString FileOpenDialog::getURL() const
{
    Sequence< OUString > aFileSeq( mxFilePicker->getSelectedFiles() );
    return aFileSeq.hasElements() ? aFileSeq[ 0 ] : OUString();
};
OUString FileOpenDialog::getFilterName() const
{
    OUString aFilterName;
    Reference< XFilterManager > xFilterManager( mxFilePicker, UNO_QUERY_THROW );
    OUString aUIName( xFilterManager->getCurrentFilter() );
    auto aIter = std::find_if(aFilterEntryList.begin(), aFilterEntryList.end(),
        [&aUIName](const FilterEntry& rFilterEntry) { return rFilterEntry.maUIName == aUIName; });
    if (aIter != aFilterEntryList.end())
        aFilterName = aIter->maFilterEntryName;
    return aFilterName;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
