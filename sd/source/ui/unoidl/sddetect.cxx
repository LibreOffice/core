/*************************************************************************
 *
 *  $RCSfile: sddetect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:18:18 $
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

#include "sddetect.hxx"

#include <framework/interaction.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAPPEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#include <framework/interaction.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#include <ucbhelper/simpleinteractionrequest.hxx>
#endif

#include <rtl/ustring.h>
#include <rtl/logfile.hxx>
#include <svtools/itemset.hxx>
#include <vcl/window.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <sot/storinfo.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/impgrf.hxx>
#include <svtools/FilterConfigItem.hxx>

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XARCHIVER_HPP_
#include <com/sun/star/util/XArchiver.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "strmname.h"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

SdFilterDetect::SdFilterDetect( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
{
}

SdFilterDetect::~SdFilterDetect()
{
}

::rtl::OUString SAL_CALL SdFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    REFERENCE< XInputStream > xStream;
    REFERENCE< XContent > xContent;
    REFERENCE< XInteractionHandler > xInteraction;
    String aURL;
    ::rtl::OUString sTemp;
    String aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    String aPreselectedFilterName;      // a name describing the filter to use (from MediaDescriptor, usually from UI action)

    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Bool bWasReadOnly = sal_False, bReadOnly = sal_False;

    // now some parameters that can already be in the array, but may be overwritten or new inserted here
    // remember their indices in the case new values must be added to the array
    sal_Int32 nPropertyCount = lDescriptor.getLength();
    sal_Int32 nIndexOfFilterName = -1;
    sal_Int32 nIndexOfInputStream = -1;
    sal_Int32 nIndexOfContent = -1;
    sal_Int32 nIndexOfReadOnlyFlag = -1;
    sal_Int32 nIndexOfTemplateFlag = -1;
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // extract properties
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("URL")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( !aURL.Len() && lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aTypeName = sTemp;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aPreselectedFilterName = sTemp;

            // if the preselected filter name is not correct, it must be erased after detection
            // remember index of property to get access to it later
            nIndexOfFilterName = nProperty;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream")) )
            nIndexOfInputStream = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly")) )
            nIndexOfReadOnlyFlag = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("UCBContent")) )
            nIndexOfContent = nProperty;
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate")) )
        {
            lDescriptor[nProperty].Value >>= bOpenAsTemplate;
            nIndexOfTemplateFlag = nProperty;
        }
        else if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InteractionHandler")) )
            lDescriptor[nProperty].Value >>= xInteraction;
    }

    // can't check the type for external filters, so set the "dont" flag accordingly
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    //SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;

    SfxApplication* pApp = SFX_APP();
    SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, lDescriptor, *pSet );
    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_DOC_READONLY, FALSE );

    bWasReadOnly = pItem && pItem->GetValue();

    const SfxFilter* pFilter = 0;
    String aFilterName;
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        if( SvtModuleOptions().IsImpress() )
        {
            String aPattern( aPrefix );
            aPattern += String::CreateFromAscii("simpress");
            if ( aURL.Match( aPattern ) >= aPattern.Len() )
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
        }

        if( !pFilter && SvtModuleOptions().IsDraw() )
        {
            String aPattern( aPrefix );
            aPattern += String::CreateFromAscii("sdraw");
            USHORT nMatch = aURL.Match( aPattern );
            if ( aURL.Match( aPattern ) >= aPattern.Len() )
                pFilter = SfxFilter::GetDefaultFilterFromFactory( aURL );
        }
    }
    else
    {
        // ctor of SfxMedium uses owner transition of ItemSet
        SfxMedium aMedium( aURL, bWasReadOnly ? STREAM_STD_READ : STREAM_STD_READWRITE, FALSE, NULL, pSet );
        aMedium.UseInteractionHandler( TRUE );
        BOOL bIsStorage = aMedium.IsStorage();
        if ( aPreselectedFilterName.Len() )
            pFilter = SfxFilter::GetFilterByName( aPreselectedFilterName );
        else if( aTypeName.Len() )
        {
            SfxFilterMatcher aMatch;
            pFilter = aMatch.GetFilter4EA( aTypeName );
        }

        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
            // remember input stream and content and put them into the descriptor later
            // should be done here since later the medium can switch to a version
            xStream = aMedium.GetInputStream();
            xContent = aMedium.GetContent();
            bReadOnly = aMedium.IsReadOnly();

            if( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) )
            {
                uno::Reference< lang::XMultiServiceFactory > xSMgr( ::comphelper::getProcessServiceFactory() );
                uno::Reference< util::XArchiver > xPacker( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.util.Archiver" ) ), uno::UNO_QUERY );
                if( xPacker.is() )
                {
                    // extract extra data
                    OUString aPath( aMedium.GetOrigURL() );
                    OUString aExtraData( xPacker->getExtraData( aPath ) );
                    const OUString aSig1= OUString::createFromAscii( "private:" );
                    String aTmp;
                    aTmp += sal_Unicode( '?' );
                    aTmp += String::CreateFromAscii("simpress");
                    const OUString aSig2( aTmp );
                    INT32 nIndex1 = aExtraData.indexOf( aSig1 );
                    INT32 nIndex2 = aExtraData.indexOf( aSig2 );
                    if( nIndex1 != 0 || nIndex2 == -1 )
                        pFilter = 0;
                }
                else
                    pFilter = 0;
            }
            else if (aMedium.GetError() == SVSTREAM_OK)
            {
                if ( bIsStorage )
                {
                    // PowerPoint needs to be detected via StreamName, all other storage based formats are our own and can
                    // be detected by the ClipboardId, so except for the PPT filter all filters must have a ClipboardId set
                    SotStorageRef xStorage = aMedium.GetStorage();
                    if ( aMedium.GetLastStorageCreationState() != ERRCODE_NONE )
                    {
                        // error during storage creation means _here_ that the medium
                        // is broken, but we can not handle it in medium since unpossibility
                        // to create a storage does not _always_ means that the medium is broken
                        aMedium.SetError( aMedium.GetLastStorageCreationState() );
                        if ( xInteraction.is() )
                        {
                            OUString empty;
                            try
                            {
                                InteractiveAppException xException( empty,
                                                                REFERENCE< XInterface >(),
                                                                InteractionClassification_ERROR,
                                                                aMedium.GetError() );

                                REFERENCE< XInteractionRequest > xRequest(
                                    new ucbhelper::SimpleInteractionRequest( makeAny( xException ),
                                                                          ucbhelper::CONTINUATION_APPROVE ) );
                                xInteraction->handle( xRequest );
                            }
                            catch ( Exception & ) {};
                        }
                    }
                    else
                    {
                        String aStreamName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PowerPoint Document" ) );
                        if ( xStorage->IsContained( aStreamName ) && xStorage->IsStream( aStreamName ) && SvtModuleOptions().IsImpress() )
                        {
                            String aFileName(aMedium.GetName());
                            aFileName.ToUpperAscii();

                            if( aFileName.SearchAscii( ".POT" ) == STRING_NOTFOUND )
                                pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97);
                            else
                                pFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97Template );
                        }
                        else if ( pFilter && !pFilter->GetFormat() )
                            // preselected Filter has no ClipboardId -> doesn't match (see comment above)
                            pFilter = 0;

                        // now the "real" type detection: check if the filter has the right ClipboardId
                        if ( pFilter && pFilter->GetFormat() != xStorage->GetFormat() )
                            // preselected Filter has different ClipboardId -> doesn't match
                            pFilter = 0;

                        // if no filter was found until now we need to check all storage based formats for Impress and Draw
                        if( !pFilter && SvtModuleOptions().IsImpress() )
                        {
                            SfxFilterMatcher aMatcher( String::CreateFromAscii("simpress") );
                            pFilter = aMatcher.GetFilter4ClipBoardId( xStorage->GetFormat() );
                        }

                        // attention: using this method impress is always preferred if no flat detecion has been made
                        // this should been discussed!
                        if( !pFilter && SvtModuleOptions().IsDraw() )
                        {
                            SfxFilterMatcher aMatcher( String::CreateFromAscii("sdraw") );
                            pFilter = aMatcher.GetFilter4ClipBoardId( xStorage->GetFormat() );
                        }
                    }
                }
                else
                {
                    String aFileName( aMedium.GetName() );
                    aFileName.ToUpperAscii();

                    // Vektorgraphik?
                    SvStream* pStm = aMedium.GetInStream();
                    if( pStm )
                    {
                        pStm->Seek( STREAM_SEEK_TO_BEGIN );

                        const String        aFileName( aMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                        GraphicDescriptor   aDesc( *pStm, &aFileName );
                        GraphicFilter*      pGrfFilter = GetGrfFilter();
                        if( !aDesc.Detect( FALSE ) )
                        {
                            pFilter = 0;
                            if( SvtModuleOptions().IsImpress() )
                            {
                                INetURLObject aURL( aFileName );
                                if( aURL.getExtension().EqualsIgnoreCaseAscii( "cgm" ) )
                                {
                                    sal_uInt8 n8;
                                    pStm->Seek( STREAM_SEEK_TO_BEGIN );
                                    *pStm >> n8;
                                    if ( ( n8 & 0xf0 ) == 0 )       // we are supporting binary cgm format only, so
                                    {                               // this is a small test to exclude cgm text
                                        const String aName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "CGM - Computer Graphics Metafile" ) );
                                        SfxFilterMatcher aMatch( String::CreateFromAscii("simpress") );
                                        pFilter = aMatch.GetFilter4FilterName( aName );
                                    }
                                }
                            }
                        }
                        else
                        {
                            if( SvtModuleOptions().IsDraw() )
                            {
                                String aShortName( aDesc.GetImportFormatShortName( aDesc.GetFileFormat() ) );
                                const String aName( pGrfFilter->GetImportFormatTypeName( pGrfFilter->GetImportFormatNumberForShortName( aShortName ) ) );

                                if ( pFilter && aShortName.EqualsIgnoreCaseAscii( "PCD" ) )    // there is a multiple pcd selection possible
                                {
                                    sal_Int32 nBase = 2;    // default Base0
                                    String aFilterTypeName( pFilter->GetRealTypeName() );
                                    if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base4" ) == COMPARE_EQUAL )
                                        nBase = 1;
                                    else if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base16" ) == COMPARE_EQUAL )
                                        nBase = 0;
                                    String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Import/PCD" ) );
                                    FilterConfigItem aFilterConfigItem( aFilterConfigPath );
                                    aFilterConfigItem.WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ), nBase );
                                }

                                SfxFilterMatcher aMatch( String::CreateFromAscii("sdraw") );
                                pFilter = aMatch.GetFilter4FilterName( aName );
                            }
                            else
                            {
                                pFilter = NULL;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( pFilter )
    {
        aTypeName = pFilter->GetTypeName();
        aFilterName = pFilter->GetName();
    }

    if ( aFilterName.Len() )
    {
        // successful detection, get the filter name (without prefix)
        if ( nIndexOfFilterName != -1 )
            // convert to format with factory ( makes load more easy to implement )
            lDescriptor[nIndexOfFilterName].Value <<= ::rtl::OUString( aFilterName );
        else
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("FilterName");
            lDescriptor[nPropertyCount].Value <<= ::rtl::OUString( aFilterName );
            nPropertyCount++;
        }
/*
        if ( pFilter->IsOwnTemplateFormat() && nIndexOfTemplateFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("AsTemplate");
            lDescriptor[nPropertyCount].Value <<= sal_True;
            nPropertyCount++;
        }
*/
    }
    else
    {
        aFilterName.Erase();
        aTypeName.Erase();
    }

    if ( nIndexOfInputStream == -1 && xStream.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("InputStream");
        lDescriptor[nPropertyCount].Value <<= xStream;
        nPropertyCount++;
    }

    if ( nIndexOfContent == -1 && xContent.is() )
    {
        // if input stream wasn't part of the descriptor, now it should be, otherwise the content would be opend twice
        lDescriptor.realloc( nPropertyCount + 1 );
        lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("UCBContent");
        lDescriptor[nPropertyCount].Value <<= xContent;
        nPropertyCount++;
    }

    if ( bReadOnly != bWasReadOnly )
    {
        if ( nIndexOfReadOnlyFlag == -1 )
        {
            lDescriptor.realloc( nPropertyCount + 1 );
            lDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("ReadOnly");
            lDescriptor[nPropertyCount].Value <<= bReadOnly;
            nPropertyCount++;
        }
        else
            lDescriptor[nIndexOfReadOnlyFlag].Value <<= bReadOnly;
    }

    return aTypeName;
}

SFX_IMPL_SINGLEFACTORY( SdFilterDetect )

/* XServiceInfo */
UNOOUSTRING SAL_CALL SdFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SdFilterDetect::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SAL_CALL SdFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SdFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedTypeDetection"  );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING SdFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.draw.FormatDetector" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SdFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SdFilterDetect( xServiceManager ) );
}

