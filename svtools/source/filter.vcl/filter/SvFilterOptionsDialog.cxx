/*************************************************************************
 *
 *  $RCSfile: SvFilterOptionsDialog.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:38:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_FILTER_OPTIONS_DIALOG_HXX_
#include "SvFilterOptionsDialog.hxx"
#endif
#ifndef _FILTER_CONFIG_ITEM_HXX_
#include "FilterConfigItem.hxx"
#endif
#ifndef _FILTER_HXX
#include "filter.hxx"
#endif
#ifndef _FILTER_CONFIG_CACHE_HXX_
#include "FilterConfigCache.hxx"
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#include "solar.hrc"
#include "fltcall.hxx"
#include "dlgexpor.hxx"
#include "dlgejpg.hxx"
#include <uno/mapping.hxx>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XVIEWDATASUPPLIER_HPP_
#include <com/sun/star/document/XViewDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <syslocale.hxx>
#endif

#if defined WIN || (defined OS2 && !defined ICC)
#define EXPDLG_FUNCTION_NAME    "_DoExportDialog"
#else
#define EXPDLG_FUNCTION_NAME    "DoExportDialog"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

// -------------------------
// - SvFilterOptionsDialog -
// -------------------------

uno::Reference< uno::XInterface >
    SAL_CALL SvFilterOptionsDialog_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory > & _rxFactory )
{
    return static_cast< ::cppu::OWeakObject* > ( new SvFilterOptionsDialog( _rxFactory ) );
}

OUString SvFilterOptionsDialog_getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.svtools.SvFilterOptionsDialog" ) );
}
#define SERVICE_NAME "com.sun.star.ui.dialog.FilterOptionsDialog"
sal_Bool SAL_CALL SvFilterOptionsDialog_supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SERVICE_NAME ) );
}

uno::Sequence< OUString > SAL_CALL SvFilterOptionsDialog_getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

// -----------------------------------------------------------------------------

SvFilterOptionsDialog::SvFilterOptionsDialog( const uno::Reference< lang::XMultiServiceFactory > & xMgr ) :
    rxMgr       ( xMgr ),
    eFieldUnit  ( FUNIT_CM )
{
}

// -----------------------------------------------------------------------------

SvFilterOptionsDialog::~SvFilterOptionsDialog()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SvFilterOptionsDialog::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SvFilterOptionsDialog::release() throw()
{
    OWeakObject::release();
}

// XInitialization
void SAL_CALL SvFilterOptionsDialog::initialize( const uno::Sequence< uno::Any > & aArguments )
    throw ( uno::Exception, uno::RuntimeException )
{
}

// XServiceInfo
OUString SAL_CALL SvFilterOptionsDialog::getImplementationName()
    throw( uno::RuntimeException )
{
    return SvFilterOptionsDialog_getImplementationName();
}
sal_Bool SAL_CALL SvFilterOptionsDialog::supportsService( const OUString& rServiceName )
    throw( uno::RuntimeException )
{
    return SvFilterOptionsDialog_supportsService( rServiceName );
}
uno::Sequence< OUString > SAL_CALL SvFilterOptionsDialog::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return SvFilterOptionsDialog_getSupportedServiceNames();
}


// XPropertyAccess
uno::Sequence< beans::PropertyValue > SvFilterOptionsDialog::getPropertyValues()
        throw ( uno::RuntimeException )
{
    sal_Int32 i, nCount;
    for ( i = 0, nCount = aMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( aMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) )
            break;
    }
    if ( i == nCount )
        aMediaDescriptor.realloc( ++nCount );

    // the "FilterData" Property is an Any that will contain our PropertySequence of Values
    aMediaDescriptor[ i ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "FilterData" ) );
    aMediaDescriptor[ i ].Value <<= aFilterDataSequence;
    return aMediaDescriptor;
}

void SvFilterOptionsDialog::setPropertyValues( const uno::Sequence< beans::PropertyValue > & aProps )
        throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException,
                uno::RuntimeException )
{
    aMediaDescriptor = aProps;

    sal_Int32 i, nCount;
    for ( i = 0, nCount = aMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( aMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) )
        {
            aMediaDescriptor[ i ].Value >>= aFilterDataSequence;
            break;
        }
    }
}

// XExecutableDialog
void SvFilterOptionsDialog::setTitle( const OUString& aTitle )
    throw ( uno::RuntimeException )
{
    aDialogTitle = aTitle;
}

sal_Int16 SvFilterOptionsDialog::execute()
    throw ( uno::RuntimeException )
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    String aFilterName( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
    String aInternalFilterName;
    sal_Int32 i, nCount = aMediaDescriptor.getLength();
    for ( i = 0; i < nCount; i++ )
    {
        if ( aMediaDescriptor[ i ].Name.equals( aFilterName ) )
        {
            OUString aStr;
            aMediaDescriptor[ i ].Value >>= aStr;
            aInternalFilterName = aStr;
            aInternalFilterName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "draw_" ) ), String(), 0 );
            aInternalFilterName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "impress_" ) ), String(), 0 );
            break;
       }
    }
    if ( aInternalFilterName.Len() )
    {
        GraphicFilter aGraphicFilter( sal_True );

        sal_uInt16 nFormat, nFilterCount = aGraphicFilter.pConfig->GetExportFormatCount();
        for ( nFormat = 0; nFormat < nFilterCount; nFormat++ )
        {
            if ( aGraphicFilter.pConfig->GetExportInternalFilterName( nFormat ) == aInternalFilterName )
                break;
        }
        if ( nFormat < nFilterCount )
        {
            FltCallDialogParameter aFltCallDlgPara( Application::GetDefDialogParent(), NULL, eFieldUnit );
            aFltCallDlgPara.aFilterData = aFilterDataSequence;

            String  aFilterName( aGraphicFilter.pConfig->GetExportFilterName( nFormat ) );
            if ( aGraphicFilter.pConfig->IsExportInternalFilter( nFormat ) )
            {
                // Export-Dialog fuer Bitmap's, SVM's und WMF's
                if( ( aFilterName.EqualsIgnoreCaseAscii( EXP_BMP ) ) ||
                    ( aFilterName.EqualsIgnoreCaseAscii( EXP_SVMETAFILE ) ) ||
                    ( aFilterName.EqualsIgnoreCaseAscii( EXP_WMF ) ) ||
                    ( aFilterName.EqualsIgnoreCaseAscii( EXP_EMF ) ) ||
                    ( aFilterName.EqualsIgnoreCaseAscii( EXP_JPEG ) ) )
                {
                    ByteString  aResMgrName( "svt", 3 );
                    ResMgr*     pResMgr;

                    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
                    pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILanguage() );
                    aFltCallDlgPara.pResMgr = pResMgr;
                    // JPEG-Dialog
                    if( aFilterName.EqualsIgnoreCaseAscii( EXP_JPEG ) )
                    {
                        if ( DlgExportEJPG( aFltCallDlgPara ).Execute() == RET_OK )
                            nRet = ui::dialogs::ExecutableDialogResults::OK;
                    }
                    else if( aFilterName.EqualsIgnoreCaseAscii( EXP_BMP ) )
                    {
                        // Fuer Vektorformate nehmen wir den Vektor-Dialog
                        aFltCallDlgPara.aFilterExt = aGraphicFilter.pConfig->GetExportFormatShortName( nFormat );
                        if ( DlgExportPix( aFltCallDlgPara ).Execute() == RET_OK )
                            nRet = ui::dialogs::ExecutableDialogResults::OK;
                    }
                    else
                    {
                        aFltCallDlgPara.aFilterExt = aGraphicFilter.pConfig->GetExportFormatShortName( nFormat );
                        if ( DlgExportVec( aFltCallDlgPara ).Execute() == RET_OK )
                            nRet = ui::dialogs::ExecutableDialogResults::OK;
                    }
                    delete pResMgr;
                }
            }
            else    // ladbare Filter
            {
                xub_StrLen i, nTokenCount = aGraphicFilter.aFilterPath.GetTokenCount( ';' );
                for ( i = 0; i < nTokenCount; i++ )
                {

                    OUString aPathURL;

                    ::osl::FileBase::getFileURLFromSystemPath( aGraphicFilter.aFilterPath.GetToken( i ), aPathURL );
                    aPathURL += OUString( sal_Unicode( '/' ) );

                    OUString aSystemPath;
                    ::osl::FileBase::getSystemPathFromFileURL( aPathURL, aSystemPath );
                    aSystemPath += OUString( aFilterName );

                    ::vos::OModule aLibrary( aSystemPath );
                    PFilterDlgCall  pFunc = (PFilterDlgCall) aLibrary.getSymbol( UniString::CreateFromAscii( EXPDLG_FUNCTION_NAME ) );
                    // Dialog in DLL ausfuehren
                    if( pFunc )
                    {
                        if ( (*pFunc)( aFltCallDlgPara ) )
                            nRet = ui::dialogs::ExecutableDialogResults::OK;
                    }
                }
            }
            // taking the out parameter from the dialog
            aFilterDataSequence = aFltCallDlgPara.aFilterData;
        }
    }
    return nRet;
}

// XEmporter
void SvFilterOptionsDialog::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
        throw ( lang::IllegalArgumentException, uno::RuntimeException )
{
    // try to set the corresponding metric unit
    String aConfigPath;
    uno::Reference< lang::XServiceInfo > xServiceInfo
            ( xDoc, uno::UNO_QUERY );
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) ) )
            aConfigPath = String( RTL_CONSTASCII_USTRINGPARAM( "Office.Impress/Layout/Other/MeasureUnit" ) );
        else if ( xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) ) )
            aConfigPath = String( RTL_CONSTASCII_USTRINGPARAM( "Office.Draw/Layout/Other/MeasureUnit" ) );
        if ( aConfigPath.Len() )
        {
            FilterConfigItem aConfigItem( aConfigPath );
            String aPropertyName;
            SvtSysLocale aSysLocale;
            if ( aSysLocale.GetLocaleDataPtr()->getMeasurementSystemEnum() == MEASURE_METRIC )
                aPropertyName = String( RTL_CONSTASCII_USTRINGPARAM( "Metric" ) );
            else
                aPropertyName = String( RTL_CONSTASCII_USTRINGPARAM( "NonMetric" ) );
            eFieldUnit = (FieldUnit)aConfigItem.ReadInt32( aPropertyName, FUNIT_CM );
        }
    }
}

