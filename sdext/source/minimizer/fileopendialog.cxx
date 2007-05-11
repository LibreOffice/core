/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileopendialog.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:50:19 $
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

#ifndef _FILEOPEN_DIALOG_HXX_
#include "fileopendialog.hxx"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _PPPOPTIMIZER_TOKEN_HXX
#include "pppoptimizertoken.hxx"
#endif
#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_CONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPREVIEW_HPP_
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERGROUPMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HDL_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif


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
void FileOpenDialog::setDefaultDirectory( const rtl::OUString& rDefaultDirectory )
{
    mxFilePicker->setDisplayDirectory( rDefaultDirectory );
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
