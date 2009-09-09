/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fileopendialog.cxx,v $
 *
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "fileopendialog.hxx"
#include <sal/types.h>
#include "pppoptimizertoken.hxx"
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HDL_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;

FileOpenDialog::FileOpenDialog( const Reference< XComponentContext >& rxMSF ) :
    mxMSF( rxMSF )
{
    Sequence< Any > aInitPropSeq( 1 );
    aInitPropSeq[ 0 ] <<= (sal_Int16)TemplateDescription::FILESAVE_AUTOEXTENSION;   // TemplateDescription.FILEOPEN_SIMPLE

    mxFilePicker = Reference < XFilePicker >( mxMSF->getServiceManager()->createInstanceWithArgumentsAndContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker" ) ), aInitPropSeq, rxMSF ),UNO_QUERY_THROW );
    mxFilePicker->setMultiSelectionMode( sal_False );


    // collecting a list of impress filters
    Reference< XNameAccess > xFilters( mxMSF->getServiceManager()->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ) ), rxMSF ), UNO_QUERY_THROW );
    Sequence< OUString > aFilterList( xFilters->getElementNames() );
    for ( int i = 0; i < aFilterList.getLength(); i++ )
    {
        try
        {
            Sequence< PropertyValue > aFilterProperties;
            if ( xFilters->getByName( aFilterList[ i ] ) >>= aFilterProperties )
            {
                FilterEntry aFilterEntry;
                sal_Bool bImpressFilter = sal_False;
                for ( int j = 0; j < aFilterProperties.getLength(); j++ )
                {
                    PropertyValue& rProperty( aFilterProperties[ j ] );
                    switch( TKGet( rProperty.Name ) )
                    {
                        case TK_DocumentService :
                        {
                            rtl::OUString sDocumentService;
                            rProperty.Value >>= sDocumentService;
                            if ( sDocumentService == OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) )
                                bImpressFilter = sal_True;
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

    Reference< XNameAccess > xTypes( mxMSF->getServiceManager()->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ) ), rxMSF ), UNO_QUERY_THROW );
    Sequence< OUString > aTypeList( xFilters->getElementNames() );

//  mxFilePicker->setDefaultName( );

    Reference< XFilterManager > xFilterManager( mxFilePicker, UNO_QUERY_THROW );
    std::vector< FilterEntry >::iterator aIter( aFilterEntryList.begin() );
    while( aIter != aFilterEntryList.end() )
    {
        Sequence< PropertyValue > aTypeProperties;
        try
        {
            if ( xTypes->getByName( aIter->maType ) >>= aTypeProperties )
            {
                Sequence< OUString > aExtensions;
                for ( int i = 0; i < aTypeProperties.getLength(); i++ )
                {
                    switch( TKGet( aTypeProperties[ i ].Name ) )
                    {
                        case TK_Extensions : aTypeProperties[ i ].Value >>= aExtensions; break;
                        default: break;
                    }
                }
                if ( aExtensions.getLength() )
                {
                    xFilterManager->appendFilter( aIter->maUIName, aExtensions[ 0 ] );
                    if ( aIter->maFlags & 0x100 )
                        xFilterManager->setCurrentFilter( aIter->maUIName );
                }
            }
        }
        catch ( Exception& )
        {
        }
        aIter++;
    }
}
FileOpenDialog::~FileOpenDialog()
{
}
sal_Int16 FileOpenDialog::execute()
{
    return mxFilePicker->execute();
}
void FileOpenDialog::setDefaultName( const rtl::OUString& rDefaultName )
{
    mxFilePicker->setDefaultName( rDefaultName );
}
::rtl::OUString FileOpenDialog::getURL() const
{
    Sequence< OUString > aFileSeq( mxFilePicker->getFiles() );
    return aFileSeq.getLength() ? aFileSeq[ 0 ] : OUString();
};
::rtl::OUString FileOpenDialog::getFilterName() const
{
    rtl::OUString aFilterName;
    Reference< XFilterManager > xFilterManager( mxFilePicker, UNO_QUERY_THROW );
    rtl::OUString aUIName( xFilterManager->getCurrentFilter() );
    std::vector< FilterEntry >::const_iterator aIter( aFilterEntryList.begin() );
    while( aIter != aFilterEntryList.end() )
    {
        if ( aIter->maUIName == aUIName )
        {
            aFilterName = aIter->maName;
            break;
        }
        aIter++;
    }
    return aFilterName;
};
