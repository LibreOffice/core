/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <comphelper/classids.hxx>
#include <bf_svx/svxids.hrc>
#include <bf_svx/impgrf.hxx>

#ifndef _SFXAPP_HXX //autogen
#include <bf_sfx2/app.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <bf_sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _SFXECODE_HXX //autogen
#include <bf_svtools/sfxecode.hxx>
#endif
#ifndef _FILTER_CONFIG_ITEM_HXX_
#include <bf_svtools/FilterConfigItem.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XARCHIVER_HPP_
#include <com/sun/star/util/XArchiver.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "sddll.hxx"

#include "sddll0.hxx"         // wird durchs Makefile generiert

#include "bf_sd/grdocsh.hxx"
#include "strings.hrc"
#include "strmname.h"


#ifndef _OFF_OFAIDS_HRC
#include <bf_offmgr/ofaids.hrc>
#endif

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <bf_svtools/moduleoptions.hxx>
#endif

#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {

TYPEINIT1( SdModuleDummy, SfxModule );

extern "C" { static void SAL_CALL thisModule() {} }

SFX_IMPL_MODULE_LIB(Sd, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DLL_NAME ) ) )

SFX_IMPL_OBJECTFACTORY_MOD(SdDrawDocShell, SFXOBJECTSHELL_STD_NORMAL, simpress,
                           SvGlobalName(BF_SO3_SIMPRESS_CLASSID), Sd, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DLL_NAME ) ) )
{
    SdDrawDocShell::Factory().SetDocumentServiceName( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) );
    SdDrawDocShell::Factory().GetFilterContainer()->SetDetectFilter( &SdDLL::DetectFilter );
}

SFX_IMPL_OBJECTFACTORY_MOD(SdGraphicDocShell, SFXOBJECTSHELL_STD_NORMAL, sdraw,
                           SvGlobalName(BF_SO3_SDRAW_CLASSID), Sd, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( DLL_NAME ) ) )
{
    SdGraphicDocShell::Factory().SetDocumentServiceName( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) );
    SdGraphicDocShell::Factory().GetFilterContainer()->SetDetectFilter( &SdDLL::DetectFilter );
}

SdDLL::SdDLL()
{
}

SdDLL::~SdDLL()
{
}

void SdDLL::LibInit()
{
    SfxApplication* pApp = SFX_APP();

    SfxObjectFactory *p1 = NULL;
    if(SvtModuleOptions().IsImpress())
    {
        SdDrawDocShell::RegisterFactory(SDT_SD_DOCFACTPRIO);
        p1 = &SdDrawDocShell::Factory();
    }

    SfxObjectFactory *p2 = NULL;
    if(SvtModuleOptions().IsDraw())
    {
        SdGraphicDocShell::RegisterFactory(SDT_SD_DOCFACTPRIO);
        p2 = &SdGraphicDocShell::Factory();
    }

    SD_MOD() = new SdModuleDummy( NULL, TRUE, p1, p2 );
}

void SdDLL::LibExit()
{
    FreeLibSd();
    DELETEZ( SD_MOD() );
}

ULONG __EXPORT SdDLL::DetectFilter(SfxMedium& rMedium, const SfxFilter** pFilter, SfxFilterFlags nMust, SfxFilterFlags nDont)
{
    ULONG nReturn = ERRCODE_ABORT;  // Erkennung fehlgeschlagen, Filter ungueltig
    BOOL bStorage = FALSE;

    if( *pFilter && (*pFilter)->GetFilterFlags() & SFX_FILTER_PACKED )
    {
        uno::Reference< lang::XMultiServiceFactory > xSMgr( ::legacy_binfilters::getLegacyProcessServiceFactory() );
        uno::Reference< util::XArchiver > xPacker( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.util.Archiver" ) ), uno::UNO_QUERY );
        if( xPacker.is() )
        {
            // extract extra data
            OUString aPath( rMedium.GetOrigURL() );
            OUString aExtraData( xPacker->getExtraData( aPath ) );
            const OUString aSig1= OUString::createFromAscii( "private:" );
            String aTmp;
            aTmp += sal_Unicode( '?' );
            aTmp += (*pFilter)->GetFilterContainer()->GetName();
            const OUString aSig2( aTmp );
            INT32 nIndex1 = aExtraData.indexOf( aSig1 );
            INT32 nIndex2 = aExtraData.indexOf( aSig2 );
            if( nIndex1 == 0 && nIndex2 != -1 )
                return ERRCODE_NONE;
        }
    }
    else if (rMedium.GetError() == SVSTREAM_OK)
    {
        if (rMedium.IsStorage())
        {
            bStorage = TRUE;
            // Storage
            SvStorage* pStorage = rMedium.GetStorage();
            if ( pStorage == NULL ) return ULONG_MAX;


            // Erkennung ueber contained streams (StarDraw 3.0)
            if( ( pStorage->IsContained( pStarDrawDoc ) && pStorage->IsStream( pStarDrawDoc ) )	||
                ( pStorage->IsContained( pStarDrawDoc3 ) && pStorage->IsStream( pStarDrawDoc3 ) ) )
            {
                const SfxFilter* pFilt = NULL;
                SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
                SfxFilterContainer *pDraw = rMatcher.GetContainer(
                                            UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "sdraw" ) ) ); // Name kennt ihr besser
                SfxFilterContainer *pImpr = rMatcher.GetContainer(
                                            UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "simpress" ) )); // dito
                ULONG nStorFmt = pStorage->GetFormat();
                if (*pFilter)
                {
                    if ( (*pFilter)->GetFilterContainer() == pDraw)
                    {
                        if(SvtModuleOptions().IsDraw())
                        {
                            pFilt = pDraw->GetFilter4ClipBoardId( nStorFmt );
                            nReturn = ERRCODE_SFX_FORCEQUIET;
                        }
                        else
                        {
                            pFilt = pImpr->GetFilter4ClipBoardId( nStorFmt );
                            nReturn = ERRCODE_SFX_FORCEQUIET;
                        }
                    }
                    else if ( (*pFilter)->GetFilterContainer() == pImpr)
                    {
                        if(SvtModuleOptions().IsImpress())
                        {
                            pFilt = pImpr->GetFilter4ClipBoardId( nStorFmt );
                            nReturn = ERRCODE_SFX_FORCEQUIET;
                        }
                        else
                        {
                            pFilt = pDraw->GetFilter4ClipBoardId( nStorFmt );
                            nReturn = ERRCODE_SFX_FORCEQUIET;
                        }
                    }
                }
                else if(SvtModuleOptions().IsImpress())
                {
                    pFilt = pImpr->GetFilter4ClipBoardId( nStorFmt );
                    nReturn = ERRCODE_NONE;
                }
                else if(SvtModuleOptions().IsDraw())
                {
                    pFilt = pDraw->GetFilter4ClipBoardId( nStorFmt );
                    nReturn = ERRCODE_NONE;
                }

                if (pFilt &&
                    (pFilt->GetFilterFlags() & nMust) == nMust &&
                    (pFilt->GetFilterFlags() & nDont) == 0)
                {
                    *pFilter = pFilt;
                }
                else
                {
                    *pFilter = NULL;
                    nReturn = ERRCODE_NONE;
                }
            }
            else if(SvtModuleOptions().IsImpress())
            {
                // Erkennung ueber contained streams (PowerPoint 97-Filter)
                String aStreamName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PowerPoint Document" ) );

                if (pStorage->IsContained( aStreamName ) &&	pStorage->IsStream( aStreamName ) )
                {
                    String aFileName(rMedium.GetName());
                    aFileName.ToUpperAscii();

                    if( aFileName.SearchAscii( ".POT" ) == STRING_NOTFOUND )
                        *pFilter = SFX_APP()->GetFilter(SdDrawDocShell::Factory(), pFilterPowerPoint97);
                    else
                        *pFilter = SFX_APP()->GetFilter(SdDrawDocShell::Factory(), pFilterPowerPoint97Template);

                    return ERRCODE_NONE;
                }
            }
        }

        // XML filter(s)
        if( *pFilter )
        {
            ::rtl::OUString aName = (*pFilter)->GetFilterName();

            if( aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("StarOffice XML (Draw)")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("StarOffice XML (Impress)")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("draw_StarOffice_XML_Impress")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("impress_StarOffice_XML_Draw")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("impress_StarOffice_XML_Impress_Template")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("draw_StarOffice_XML_Draw_Template")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("draw_StarOffice_XML_Draw")) ||
                aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("impress_StarOffice_XML_Impress")))
            {
                if( rMedium.IsStorage() )
                {
                    SvStorage* pStorage = rMedium.GetStorage();

                    nReturn = ERRCODE_ABORT;

                    if( pStorage && ( pStorage->IsStream( pStarDrawXMLContent ) || pStorage->IsStream( pStarDrawOldXMLContent ) ) )
                    {
                        const ULONG nStorageFormat = pStorage->GetFormat();
                        if(	nStorageFormat == 0 ||
                            nStorageFormat == SOT_FORMATSTR_ID_STARIMPRESS_60 ||
                            nStorageFormat == SOT_FORMATSTR_ID_STARDRAW_60  )
                        {
                            nReturn = ERRCODE_NONE;
                        }
                    }
                }
            }
        }

        String aFileName(rMedium.GetName());
        aFileName.ToUpperAscii();

        if (nReturn == ERRCODE_ABORT)
        {
            if( bStorage )         // aber keine Clipboard-Id #55337#
            {
                *pFilter = NULL;
            }
            else
            {
                // Vektorgraphik?
                SvStream* pStm = rMedium.GetInStream();

                if( !pStm )
                    nReturn = ERRCODE_IO_GENERAL;
                else
                {
                    pStm->Seek( STREAM_SEEK_TO_BEGIN );

                    const String		aFileName( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                    GraphicDescriptor	aDesc( *pStm, &aFileName );
                    GraphicFilter*		pGrfFilter = GetGrfFilter();

                    if( !aDesc.Detect( FALSE ) )
                    {
                        *pFilter = NULL;
                        nReturn = ERRCODE_ABORT;
                        INetURLObject aURL( aFileName );
                        if( aURL.getExtension().equalsIgnoreAsciiCaseAscii( "cgm" ) )
                        {
                            sal_uInt8 n8;
                            pStm->Seek( STREAM_SEEK_TO_BEGIN );
                            *pStm >> n8;
                            if ( ( n8 & 0xf0 ) == 0 )		// we are supporting binary cgm format only, so
                            {								// this is a small test to exclude cgm text
                                const String aName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "CGM - Computer Graphics Metafile" ) );
                                *pFilter = SFX_APP()->GetFilter( SdDrawDocShell::Factory(), aName );
                                nReturn = ERRCODE_NONE;
                            }
                        }
                    }
                    else
                    {
                        if( SvtModuleOptions().IsDraw() )
                        {
                            String aShortName( aDesc.GetImportFormatShortName( aDesc.GetFileFormat() ) );
                            const String aName( pGrfFilter->GetImportFormatTypeName( pGrfFilter->GetImportFormatNumberForShortName( aShortName ) ) );

                            if ( *pFilter && aShortName.EqualsIgnoreCaseAscii( "PCD" ) )    // there is a multiple pcd selection possible
                            {
                                sal_Int32 nBase = 2;    // default Base0
                                String aFilterTypeName( (*pFilter)->GetRealTypeName() );
                                if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base4" ) == COMPARE_EQUAL )
                                    nBase = 1;
                                else if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base16" ) == COMPARE_EQUAL )
                                    nBase = 0;
                                String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Import/PCD" ) );
                                FilterConfigItem aFilterConfigItem( aFilterConfigPath );
                                aFilterConfigItem.WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ), nBase );
                            }

                            *pFilter = SFX_APP()->GetFilter( SdGraphicDocShell::Factory(), aName );
                            nReturn = ERRCODE_NONE;
                        }
                        else
                        {
                            nReturn = ERRCODE_ABORT;
                            *pFilter = NULL;
                        }
                    }
                }
            }
        }
    }
    else
    {
        nReturn = rMedium.GetError();
    }

    return nReturn;
}



/*************************************************************************
|*
|* Modul laden
|*
\************************************************************************/

SfxModule* SdModuleDummy::Load()
{
    return (LoadLibSd() ? SD_MOD() : NULL);
}

}
