/*************************************************************************
 *
 *  $RCSfile: sddll.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:15:58 $
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

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include <svx/editeng.hxx>

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#pragma hdrstop

#include "sddll.hxx"
#include "docshell.hxx"
#include "grdocsh.hxx"
#include "sdresid.hxx"
#include "sdobjfac.hxx"
#include "cfgids.hxx"
#include "strmname.h"
#include "SdShapeTypes.hxx"

#include <svx/SvxShapeTypes.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <tools/urlobj.hxx>
#include <svx/impgrf.hxx>
#include <svtools/FilterConfigItem.hxx>

#ifndef _COM_SUN_STAR_UTIL_XARCHIVER_HPP_
#include <com/sun/star/util/XArchiver.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;


/*************************************************************************
|*
|* Init
|*
\************************************************************************/

void SdDLL::Init()
{
    if ( SD_MOD() )
        return;

    SfxObjectFactory* pDrawFact = NULL;
    SfxObjectFactory* pImpressFact = NULL;

    if (SvtModuleOptions().IsImpress())
        pImpressFact = &SdDrawDocShell::Factory();

    if (SvtModuleOptions().IsDraw())
        pDrawFact = &SdGraphicDocShell::Factory();

    // the SdModule must be created
     SdModule** ppShlPtr = (SdModule**) GetAppData(SHL_DRAW);
    (*ppShlPtr) = new SdModule( pImpressFact, pDrawFact);

    if (SvtModuleOptions().IsImpress())
    {
        // Register the Impress shape types in order to make the shapes accessible.
        SdDrawDocShell::RegisterFactory( SDT_SD_DOCFACTPRIO );
        ::accessibility::RegisterImpressShapeTypes ();
    }

    if (SvtModuleOptions().IsDraw())
        SdGraphicDocShell::RegisterFactory( SDT_SD_DOCFACTPRIO );

    // register your view-factories here
    RegisterFactorys();

    // register your shell-interfaces here
    RegisterInterfaces();

    // register your controllers here
    RegisterControllers();

    // Objekt-Factory eintragen
    SdrObjFactory::InsertMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));
}



/*************************************************************************
|*
|* Exit
|*
\************************************************************************/

void SdDLL::Exit()
{
    // called directly befor unloading the DLL
    // do whatever you want, Sd-DLL is accessible

    // Objekt-Factory austragen
    SdrObjFactory::RemoveMakeUserDataHdl(LINK(&aSdObjectFactory, SdObjectFactory, MakeUserData));

    // the SdModule must be destroyed
    SdModule** ppShlPtr = (SdModule**) GetAppData(SHL_DRAW);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
}


ULONG SdDLL::DetectFilter(SfxMedium& rMedium, const SfxFilter** ppFilter,
                          SfxFilterFlags nMust, SfxFilterFlags nDont)
{
    ULONG nReturn = ERRCODE_ABORT;  // Erkennung fehlgeschlagen, Filter ungueltig
    BOOL bStorage = FALSE;

    if( *ppFilter && (*ppFilter)->GetFilterFlags() & SFX_FILTER_PACKED )
    {
        uno::Reference< lang::XMultiServiceFactory > xSMgr( ::comphelper::getProcessServiceFactory() );
        uno::Reference< util::XArchiver > xPacker( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.util.Archiver" ) ), uno::UNO_QUERY );
        if( xPacker.is() )
        {
            // extract extra data
            OUString aPath( rMedium.GetOrigURL() );
            OUString aExtraData( xPacker->getExtraData( aPath ) );
            const OUString aSig1= OUString::createFromAscii( "private:" );
            String aTmp;
            aTmp += sal_Unicode( '?' );
            aTmp += String::CreateFromAscii("simpress");
            const OUString aSig2( aTmp );
            INT32 nIndex1 = aExtraData.indexOf( aSig1 );
            INT32 nIndex2 = aExtraData.indexOf( aSig2 );
            if( nIndex1 == 0 && nIndex2 != -1 )
                return ERRCODE_NONE;
        }
    }
    else if (rMedium.GetError() == SVSTREAM_OK)
    {
        if ( rMedium.IsStorage() )
        {
            bStorage = TRUE;
            SotStorageRef xStorage = rMedium.GetStorage();
            if ( !xStorage.Is() )
                return ULONG_MAX;

            if( SvtModuleOptions().IsImpress() )
            {
                // Erkennung ueber contained streams (PowerPoint 97-Filter)
                String aStreamName = UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "PowerPoint Document" ) );
                if ( xStorage->IsContained( aStreamName ) && xStorage->IsStream( aStreamName ) )
                {
                    String aFileName(rMedium.GetName());
                    aFileName.ToUpperAscii();

                    if( aFileName.SearchAscii( ".POT" ) == STRING_NOTFOUND )
                        *ppFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97);
                    else
                        *ppFilter = SfxFilter::GetFilterByName( pFilterPowerPoint97Template );

                    return ERRCODE_NONE;
                }
            }

            const SfxFilter* pFilter = *ppFilter;
            if ( *ppFilter )
            {
                if ( (*ppFilter)->GetFormat() == xStorage->GetFormat() )
                    pFilter = *ppFilter;
            }

             if( !pFilter && SvtModuleOptions().IsImpress() )
            {
                SfxFilterMatcher aMatcher( String::CreateFromAscii("simpress") );
                pFilter = aMatcher.GetFilter4ClipBoardId( xStorage->GetFormat() );
                if ( pFilter )
                    *ppFilter = pFilter;
            }

            if( !pFilter && SvtModuleOptions().IsDraw() )
            {
                SfxFilterMatcher aMatcher( String::CreateFromAscii("sdraw") );
                pFilter = aMatcher.GetFilter4ClipBoardId( xStorage->GetFormat() );
                if ( pFilter )
                    *ppFilter = pFilter;
            }

            if ( pFilter &&
                ( pFilter->GetFilterFlags() & nMust ) == nMust &&
                ( pFilter->GetFilterFlags() & nDont ) == 0 )
            {
                *ppFilter = pFilter;
                nReturn = ERRCODE_NONE;
            }
            else
            {
                *ppFilter = NULL;
                nReturn = ERRCODE_NONE;
            }
        }

        String aFileName( rMedium.GetName() );
        aFileName.ToUpperAscii();

        if ( nReturn == ERRCODE_ABORT )
        {
            if( bStorage )         // aber keine Clipboard-Id #55337#
            {
                *ppFilter = NULL;
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

                    const String        aFileName( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                    GraphicDescriptor   aDesc( *pStm, &aFileName );
                    GraphicFilter*      pGrfFilter = GetGrfFilter();

                    if( !aDesc.Detect( FALSE ) )
                    {
                        if( SvtModuleOptions().IsImpress() )
                        {
                            *ppFilter = NULL;
                            nReturn = ERRCODE_ABORT;
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
                                    *ppFilter = aMatch.GetFilter4FilterName( aName );
                                    nReturn = ERRCODE_NONE;
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

                            if ( *ppFilter && aShortName.EqualsIgnoreCaseAscii( "PCD" ) )    // there is a multiple pcd selection possible
                            {
                                sal_Int32 nBase = 2;    // default Base0
                                String aFilterTypeName( (*ppFilter)->GetRealTypeName() );
                                if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base4" ) == COMPARE_EQUAL )
                                    nBase = 1;
                                else if ( aFilterTypeName.CompareToAscii( "pcd_Photo_CD_Base16" ) == COMPARE_EQUAL )
                                    nBase = 0;
                                String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Import/PCD" ) );
                                FilterConfigItem aFilterConfigItem( aFilterConfigPath );
                                aFilterConfigItem.WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ), nBase );
                            }

                            SfxFilterMatcher aMatch( String::CreateFromAscii("draw") );
                            *ppFilter = aMatch.GetFilter4FilterName( aName );
                            nReturn = ERRCODE_NONE;
                        }
                        else
                        {
                            nReturn = ERRCODE_ABORT;
                            *ppFilter = NULL;
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





