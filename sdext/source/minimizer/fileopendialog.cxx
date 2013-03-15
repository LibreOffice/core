/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "fileopendialog.hxx"
#include <sal/types.h>
#include "pppoptimizertoken.hxx"
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
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
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>

#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::ui::dialogs;

using ::com::sun::star::awt::XWindow;
using ::rtl::OUString;

namespace
{
    inline bool lcl_isSystemDialog(
        const Reference< XInterface > &rxIfce )
    {
        Reference< XServiceInfo > xInfo( rxIfce, UNO_QUERY );
        if ( !xInfo.is() )
            return false;

        return xInfo->supportsService(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.ui.dialogs.SystemFilePicker" ) ) );
    }
}

FileOpenDialog::FileOpenDialog(
    const Reference< XComponentContext >& rxContext,
    const Reference< XWindow > &rxParent )
    : mxContext( rxContext )
{
    mxFilePicker = Reference < XFilePicker >(
        mxContext->getServiceManager()->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.ui.dialogs.FilePicker" ) ),
                    rxContext ), UNO_QUERY_THROW );
    Reference< XInitialization > xInit( mxFilePicker, UNO_QUERY_THROW );
    bool bIsSystemDlg = lcl_isSystemDialog( mxFilePicker );
    Sequence< Any > aInitPropSeq( bIsSystemDlg ? 1 : 2 );
    if ( bIsSystemDlg )
    {
        aInitPropSeq[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION;
        xInit->initialize( aInitPropSeq );
    }
    else
    {
        aInitPropSeq[ 0 ] <<= NamedValue(
            OUString(RTL_CONSTASCII_USTRINGPARAM("TemplateDescription")),
            makeAny( (sal_Int16)TemplateDescription::FILESAVE_AUTOEXTENSION));
        aInitPropSeq[ 1 ] <<= NamedValue(
            OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow")),
            makeAny( rxParent ));
        xInit->initialize( aInitPropSeq );
    }

    mxFilePicker->setMultiSelectionMode( sal_False );

    Reference< XFilePickerControlAccess > xAccess( mxFilePicker, UNO_QUERY );
    if ( xAccess.is() )
    {
        Any aValue( static_cast< sal_Bool >( sal_True ) );
        try
        {
            xAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue );
        }
        catch( com::sun::star::uno::Exception& )
        {}
    }

    // collecting a list of impress filters
    Reference< XNameAccess > xFilters( mxContext->getServiceManager()->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ) ), rxContext ), UNO_QUERY_THROW );
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

    Reference< XNameAccess > xTypes( mxContext->getServiceManager()->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ) ), rxContext ), UNO_QUERY_THROW );
    Sequence< OUString > aTypeList( xFilters->getElementNames() );

//  mxFilePicker->setDefaultName( );

    const char filter[] = "*.";
    // the filter title must be formed in the same way it is currently done
    // in the internal implementation: "UIName (.<extension>)"
    rtl::OUStringBuffer aUIName;
    // the filter must be in the form "*.<extension>"
    rtl::OUStringBuffer aFilter;
    rtl::OUString aExtension;
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
                    aExtension = aExtensions[0];
                    // form the title: "<UIName> (.<extension)"
                    aUIName.append( aIter->maUIName );
                    aUIName.appendAscii( RTL_CONSTASCII_STRINGPARAM( " (." ));
                    aUIName.append( aExtension );
                    aUIName.append( sal_Unicode( ')' ) );
                    // form the filter: "(*.<extension>)"
                    aFilter.appendAscii( RTL_CONSTASCII_STRINGPARAM( filter ) );
                    aFilter.append( aExtensions[0] );

                    xFilterManager->appendFilter( aUIName.makeStringAndClear(),
                                                  aFilter.makeStringAndClear() );
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
