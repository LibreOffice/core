/*************************************************************************
 *
 *  $RCSfile: msoleexp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 13:26:49 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_INTERNO_HXX
#include <sfx2/interno.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "msoleexp.hxx"

#define CREATE_CONST_ASC(s) String::CreateFromAscii( \
    RTL_CONSTASCII_STRINGPARAM(s))

using namespace ::com::sun::star;

SvGlobalName GetEmbeddedVersion( const SvGlobalName& aAppName )
{
    if ( aAppName == SvGlobalName( SO3_SM_CLASSID_60 ) )
            return SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_60 );
    else if ( aAppName == SvGlobalName( SO3_SW_CLASSID_60 ) )
            return SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_60 );
    else if ( aAppName == SvGlobalName( SO3_SC_CLASSID_60 ) )
            return SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_60 );
    else if ( aAppName == SvGlobalName( SO3_SDRAW_CLASSID_60 ) )
            return SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_60 );
    else if ( aAppName == SvGlobalName( SO3_SIMPRESS_CLASSID_60 ) )
            return SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_60 );
    else if ( aAppName == SvGlobalName( SO3_SCH_CLASSID_60 ) )
            return SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_60 );

    return SvGlobalName();
}

String GetStorageType( const SvGlobalName& aEmbName )
{
    if ( aEmbName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_60 ) )
            return String::CreateFromAscii( "soffice.StarMathDocument.6" );
    else if ( aEmbName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_60 ) )
            return String::CreateFromAscii( "soffice.StarWriterDocument.6" );
    else if ( aEmbName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_60 ) )
            return String::CreateFromAscii( "soffice.StarCalcDocument.6" );
    else if ( aEmbName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_60 ) )
            return String::CreateFromAscii( "soffice.StarDrawDocument.6" );
    else if ( aEmbName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_60 ) )
            return String::CreateFromAscii( "soffice.StarImpressDocument.6" );
    else if ( aEmbName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_60 ) )
            return String::CreateFromAscii( "soffice.StarChartDocument.6" );

    return String();
}

sal_Bool UseOldMSExport()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();

    if ( xFactory.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xProvider( xFactory->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider"))),
                    uno::UNO_QUERY);
        if ( xProvider.is() )
        {
            try {
                uno::Sequence< uno::Any > aArg( 1 );
                aArg[0] <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common/InternalMSExport") );
                uno::Reference< container::XNameAccess > xNameAccess(
                    xProvider->createInstanceWithArguments(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) ),
                        aArg ),
                    uno::UNO_QUERY );
                if ( xNameAccess.is() )
                {
                    uno::Any aResult = xNameAccess->getByName(
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseOldExport" ) ) );

                    sal_Bool bResult;
                    if ( aResult >>= bResult )
                        return bResult;
                }
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    OSL_ENSURE( sal_False, "Could not get access to configuration entry!\n" );
    return sal_False;
}

void SvxMSExportOLEObjects::ExportOLEObject( SvInPlaceObject& rObj,
                                                SvStorage& rDestStg )
{
    SvGlobalName aOwnGlobalName;
    const SfxFilter* pExpFilter = NULL;
    SfxInPlaceObjectRef xSfxIPObj( &rObj );
    if( xSfxIPObj.Is() && xSfxIPObj->GetObjectShell() )
    {
        static struct _ObjExpType {
            UINT32 nFlag;
            const char* pFilterNm;
            // GlobalNameId
            struct _GlobalNameIds {
                UINT32 n1;
                USHORT n2, n3;
                BYTE b8, b9, b10, b11, b12, b13, b14, b15;
            } aGlNmIds[4];
        } aArr[] = {
            { OLE_STARMATH_2_MATHTYPE, "MathType 3.x",
                {SO3_SM_CLASSID_60, SO3_SM_CLASSID_50,
                 SO3_SM_CLASSID_40, SO3_SM_CLASSID_30 }},
            { OLE_STARWRITER_2_WINWORD, "MS Word 97",
                {SO3_SW_CLASSID_60, SO3_SW_CLASSID_50,
                 SO3_SW_CLASSID_40, SO3_SW_CLASSID_30 }},
            { OLE_STARCALC_2_EXCEL, "MS Excel 97",
                {SO3_SC_CLASSID_60, SO3_SC_CLASSID_50,
                 SO3_SC_CLASSID_40, SO3_SC_CLASSID_30 }},
            { OLE_STARIMPRESS_2_POWERPOINT, "MS PowerPoint 97",
                {SO3_SIMPRESS_CLASSID_60, SO3_SIMPRESS_CLASSID_50,
                 SO3_SIMPRESS_CLASSID_40, SO3_SIMPRESS_CLASSID_30 }},
            { 0, "",
                {SO3_SCH_CLASSID_60, SO3_SCH_CLASSID_50,
                 SO3_SCH_CLASSID_40, SO3_SCH_CLASSID_30 }},
            { 0, "",
                {SO3_SDRAW_CLASSID_60, SO3_SDRAW_CLASSID_50,    // SJ: !!!! SO3_SDRAW_CLASSID is only available up from
                 SO3_SDRAW_CLASSID_60, SO3_SDRAW_CLASSID_50 }}, // ver 5.0, it is purpose to have double entrys here.
            { 0xffff,0 }
        };
        for( const _ObjExpType* pArr = aArr; !pExpFilter && ( pArr->nFlag != 0xffff ); ++pArr )
        {
            for ( int n = 0; n < 4; ++n )
            {
                const _ObjExpType::_GlobalNameIds& rId = pArr->aGlNmIds[ n ];
                SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                            rId.b8, rId.b9, rId.b10, rId.b11,
                            rId.b12, rId.b13, rId.b14, rId.b15 );
                if( *xSfxIPObj->GetSvFactory() == aGlbNm )
                {
                    aOwnGlobalName = aGlbNm;

                    if( GetFlags() & pArr->nFlag )
                    {
                        const SfxObjectFactory& rFact = xSfxIPObj->
                                                GetObjectShell()->GetFactory();
                        if( rFact.GetFilterContainer() )
                            pExpFilter = rFact.GetFilterContainer()->
                                GetFilter4FilterName( String::CreateFromAscii(
                                                pArr->pFilterNm ));
                        break;
                    }
                }
            }
        }
    }

    if( pExpFilter )                        // use this filter for the export
    {
        SfxMedium aMed( &rDestStg, FALSE );
        aMed.SetFilter( pExpFilter );
        xSfxIPObj->GetObjectShell()->ConvertTo( aMed );
    }
    else if( aOwnGlobalName != SvGlobalName() )
    {
        SvGlobalName aEmbName = GetEmbeddedVersion( aOwnGlobalName );
        if ( aEmbName != SvGlobalName() && !UseOldMSExport() )
        {
            // this is a StarOffice 6.1 embedded object
            rDestStg.SetVersion( SOFFICE_FILEFORMAT_31 );
            rDestStg.SetClass( aEmbName,
                                SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE,
                                GetStorageType( aEmbName ) );
            SotStorageStreamRef xExtStm = rDestStg.OpenSotStream(
                                            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "properties_stream" ) ),
                                            STREAM_STD_READWRITE );

            sal_Bool bExtentSuccess = sal_False;
            if( !xExtStm->GetError() )
            {
                // write extent
                Rectangle aVisArea = xSfxIPObj->GetVisArea( ASPECT_CONTENT );
                sal_Int32 pRect[4];
                pRect[0] = aVisArea.Left();
                pRect[1] = aVisArea.Right();
                pRect[2] = aVisArea.Top();
                pRect[3] = aVisArea.Bottom();

                sal_Int8 aWriteSet[16];
                for ( int ind = 0; ind < 4; ind++ )
                {
                    sal_Int32 nVal = pRect[ind];
                    for ( int nByte = 0; nByte < 4; nByte++ )
                    {
                        aWriteSet[ind*4+nByte] = nVal % 0x100;
                        nVal /= 0x100;
                    }
                }
                bExtentSuccess = ( xExtStm->Write( aWriteSet, 16 ) == 16 );
            }

            if ( bExtentSuccess )
            {
                SotStorageStreamRef xEmbStm = rDestStg.OpenSotStream(
                                                String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "package_stream" ) ),
                                                STREAM_STD_READWRITE );

                if( !xEmbStm->GetError() )
                {
                    SvStorageRef xEmbStor = new SvStorage( sal_True, *xEmbStm );
                    if( !xEmbStor->GetError() )
                    {
                        rObj.DoSaveAs( &xEmbStor );
                        rObj.DoSaveCompleted();
                        xEmbStor->Commit();
                    }
                }
            }
        }
        else
        {
            rDestStg.SetVersion( SOFFICE_FILEFORMAT_50 );
            rObj.DoSaveAs( &rDestStg );
            rObj.DoSaveCompleted();
        }
    }
    else
    {
        rDestStg.SetVersion( SOFFICE_FILEFORMAT_31 );
        rObj.DoSaveAs( &rDestStg );
        rObj.DoSaveCompleted();
    }
    //We never need this stream: See #99809# and #i2179#
    rDestStg.Remove(CREATE_CONST_ASC(SVEXT_PERSIST_STREAM));
}



