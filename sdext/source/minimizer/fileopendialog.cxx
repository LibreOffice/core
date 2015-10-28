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
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::ui::dialogs;

FileOpenDialog::FileOpenDialog( const Reference< XComponentContext >& rxContext )
{
    mxFilePicker = FilePicker::createWithMode( rxContext, TemplateDescription::FILESAVE_AUTOEXTENSION);
    mxFilePicker->setMultiSelectionMode( sal_False );

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
        OUString( "com.sun.star.document.FilterFactory" ), rxContext ), UNO_QUERY_THROW );
    Sequence< OUString > aFilterList( xFilters->getElementNames() );
    for ( int i = 0; i < aFilterList.getLength(); i++ )
    {
        try
        {
            Sequence< PropertyValue > aFilterProperties;
            if ( xFilters->getByName( aFilterList[ i ] ) >>= aFilterProperties )
            {
                FilterEntry aFilterEntry;
                bool bImpressFilter = false;
                for ( int j = 0; j < aFilterProperties.getLength(); j++ )
                {
                    PropertyValue& rProperty( aFilterProperties[ j ] );
                    switch( TKGet( rProperty.Name ) )
                    {
                        case TK_DocumentService :
                        {
                            OUString sDocumentService;
                            rProperty.Value >>= sDocumentService;
                            if ( sDocumentService == "com.sun.star.presentation.PresentationDocument" )
                                bImpressFilter = true;
                            else
                                j = aFilterProperties.getLength();
                        }
                        break;
                        case TK_Name :      rProperty.Value >>= aFilterEntry.maName; break;
                        case TK_UIName :    rProperty.Value >>= aFilterEntry.maUIName; break;
                        case TK_Type :      rProperty.Value >>= aFilterEntry.maType; break;
                        case TK_Flags :     rProperty.Value >>= aFilterEntry.maFlags; break;
                        default : break;
                    }
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
        OUString( "com.sun.star.document.TypeDetection" ), rxContext ), UNO_QUERY_THROW );

    for( std::vector< FilterEntry >::const_iterator aIter(aFilterEntryList.begin()), aEnd(aFilterEntryList.end()); aIter != aEnd; ++aIter )
    {
        Sequence< PropertyValue > aTypeProperties;
        try
        {
            if ( xTypes->getByName( aIter->maType ) >>= aTypeProperties )
            {
                Sequence< OUString > aExtensions;
                for ( int i = 0; i < aTypeProperties.getLength(); i++ )
                {
                    if( aTypeProperties[ i ].Name == "Extensions" )
                    {
                        aTypeProperties[ i ].Value >>= aExtensions;
                        break;
                    }
                }
                if ( aExtensions.getLength() )
                {
                    // The filter title must be formed in the same way it is
                    // currently done in the internal implementation:
                    OUString aTitle(
                        aIter->maUIName + " (." + aExtensions[0] + ")");
                    OUString aFilter("*." + aExtensions[0]);
                    mxFilePicker->appendFilter(aTitle, aFilter);
                    if ( aIter->maFlags & 0x100 )
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
    return aFileSeq.getLength() ? aFileSeq[ 0 ] : OUString();
};
OUString FileOpenDialog::getFilterName() const
{
    OUString aFilterName;
    Reference< XFilterManager > xFilterManager( mxFilePicker, UNO_QUERY_THROW );
    OUString aUIName( xFilterManager->getCurrentFilter() );
    for( std::vector< FilterEntry >::const_iterator aIter(aFilterEntryList.begin()), aEnd(aFilterEntryList.end()); aIter != aEnd; ++aIter )
    {
        if ( aIter->maUIName == aUIName )
        {
            aFilterName = aIter->maName;
            break;
        }
    }
    return aFilterName;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
