/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <string.h>
#include <stdio.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/processfactory.hxx>


#include <tools/config.hxx>

#include <bf_so3/factory.hxx>

#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbxprop.hxx>
#include <basic/sbxmeth.hxx>
#include <bf_svtools/solar.hrc>
#include <tools/resmgr.hxx>

#include <binddata.hxx>

#include <soimpl.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_so3/embobj.hxx>
#include "bf_so3/ipclient.hxx"
#include <bf_so3/ipobj.hxx>
#include "bf_so3/outplace.hxx"
#include <bf_so3/ipenv.hxx>
#include <bf_so3/protocol.hxx>
#include "bf_so3/plugin.hxx"
#include "bf_so3/soerr.hxx"
#include <comphelper/classids.hxx>
#include "insdlg.hxx"

#include <sot/formats.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;

namespace binfilter {

/************** class SoDll *********************************************/
/*************************************************************************
|*    SoDll::SoDll
|*
|*    Beschreibung
*************************************************************************/
static SoDll* pDll=NULL;

SoDll* SoDll::GetOrCreate()
{
    if ( !pDll )
        pDll = new SoDll;
    return pDll;
}

SoDll::SoDll()
    : bInit( FALSE )
    , bSelfInit( FALSE )
    , pResMgr( NULL )
    , pSvObjectFactory( NULL )
    , pSvStorageStreamFactory( NULL )
    , pSvStorageFactory( NULL )
    , pSvEmbeddedObjectFactory( NULL )
    , pSvEmbeddedClientFactory( NULL )
    , pSvInPlaceObjectFactory( NULL )
    , pSvPlugInObjectFactory( NULL )
    , pSvAppletObjectFactory( NULL )
    , pSvInPlaceClientFactory( NULL )
    , pSvPersistFactory( NULL )
    , pSvPseudoObjectFactory( NULL )
    , pSvSimplePersistFactory( NULL )
    , pSvObjectContainerFactory( NULL )
    , pSvFactory_ImplFactory( NULL )
    , pDfltPlugInFactory( NULL )
    , pDfltAppletFactory( NULL )
    , pSoBindingFactory( NULL )
    , pContEnvList( NULL )
    , pIPActiveClientList( NULL )
    , pIPActiveObjectList( NULL )
    , pUIShowIPEnv( NULL )
    , pIEOPDflt( NULL )
    , aSvInterface( 0xB34BB240L, 0x4BD8, 0x101C, 0x8D, 0x86,
                    0x4A, 0x04, 0x12, 0x94, 0x26, 0x0D )
    , pPlugInVerbList( NULL )
    , nPlugInDocFormat( 0 )
    , pAppletVerbList( NULL )
    , nAppletDocFormat( 0 )
    , pDeathTimer( NULL )
    , pBindingData( 0 )
    , pConvTable( NULL )
    , nConvTableEntries( 0 )
{
}


SoDll::~SoDll()
{
    delete pResMgr;
    delete pIPActiveClientList;
    delete pIPActiveObjectList;
    delete pContEnvList;
    SvEditObjectProtocol::Imp_DeleteDefault();
    delete pDeathTimer;
    delete pPlugInVerbList;
    delete pAppletVerbList;

    SvBindingData::Delete();
    delete [] pConvTable;
}


ResMgr * SoDll::GetResMgr()
{
    // resources are in the OFA res mgr
    if ( !pResMgr )
    {
        ByteString aMgrName = ByteString( "bf_ofa" );
        pResMgr = ResMgr::CreateResMgr( aMgrName.GetBuffer() );
    }
    return pResMgr;
}

/************** class SvFactory *****************************************/
/*************************************************************************
|*    SvFactory::SvFactory()
|*
|*    Beschreibung
*************************************************************************/
TYPEINIT0(SvFactory);

SvFactory::SvFactory( const SvGlobalName & rName,
                      const String & rClassName,
                      CreateInstanceType pCreateFuncP )
    : SotFactory( rName, rClassName, pCreateFuncP )
    , nRegisterId   ( 0 )
{
}


//=========================================================================
SvFactory::~SvFactory()
{
}

/*************************************************************************
|*    SvFactory::Init()
|*
|*    Beschreibung
*************************************************************************/

BOOL SvFactory::Init()
{
    SoDll* pSoApp = SOAPP;
    pSoApp->bInit = TRUE;
    pSoApp->bSelfInit = TRUE;
    if( pSoApp->bSelfInit )
    {
        // Lokal anmelden
        pSoApp->aInfoClassMgr.SV_CLASS_REGISTER( SvEmbeddedInfoObject );
        pSoApp->aInfoClassMgr.SV_CLASS_REGISTER( SvInfoObject );
        SvPlugInObject::ClassFactory();
        // SvAppletObject::ClassFactory();
        SvOutPlaceObject::ClassFactory();
    }

    return pSoApp->bInit;
}

String SvFactory::GetServiceName( const SvGlobalName& rClassName )
{
    ::rtl::OUString aServiceName;
    if( SvGlobalName(SO3_SC_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Calc.SpreadsheetDocument");
    }
    else if( SvGlobalName(SO3_SW_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Writer.TextDocument");
    }
    else if( SvGlobalName(SO3_SWWEB_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Writer.WebDocument");
    }
    else if( SvGlobalName(SO3_SWGLOB_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Writer.GlobalDocument");
    }
    else if( SvGlobalName(SO3_SIMPRESS_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Draw.PresentationDocument");
    }
    else if( SvGlobalName(SO3_SDRAW_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Draw.DrawingDocument");
    }
    else if( SvGlobalName(SO3_SCH_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Chart.ChartDocument");
    }
    else if( SvGlobalName(SO3_SM_CLASSID_60) == rClassName )
    {
        aServiceName = ::rtl::OUString::createFromAscii("com.sun.star.comp.Math.FormulaDocument");
    }

    return aServiceName;
}

SvObject* TryCreate( const SvGlobalName& rClassName )
{
    ::rtl::OUString aServiceName = SvFactory::GetServiceName( rClassName );
    if ( aServiceName.getLength() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XModel > xDoc(
            ::comphelper::getProcessServiceFactory()->createInstance( aServiceName ), UNO_QUERY );
        if ( xDoc.is() )
        {
            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aProps( 1 );
            aProps[0].Name = ::rtl::OUString::createFromAscii( "SetEmbedded" );
            aProps[0].Value <<= sal_True;
            xDoc->attachResource( ::rtl::OUString(), aProps );

            ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( xDoc, UNO_QUERY );
            ::com::sun::star::uno::Sequence < sal_Int8 > aSeq( (sal_Int8*) SvGlobalName( SO3_GLOBAL_CLASSID ).GetBytes(), 16 );
            sal_Int64 nHandle = xObj->getSomething( aSeq );
            if ( nHandle )
                return reinterpret_cast<SvObject*>(sal::static_int_cast<sal_IntPtr>(nHandle)); 
                // return (SvObject*) (sal_Int32*) nHandle;
        }
    }

    return 0;
}

/*************************************************************************
|*    SvFactory::Create()
|*
|*    Beschreibung
*************************************************************************/
SvObjectRef SvFactory::Create( const SvGlobalName & rClassName ) const
{
    SotObject *pBasicObj=NULL;
    pBasicObj = TryCreate( rClassName );
    if ( pBasicObj )
        return SvObjectRef( pBasicObj );

    const SvFactory * pFact = this;
    if ( !pFact || rClassName != *this )
        pFact = PTR_CAST( SvFactory, Find( rClassName ) );

    if( pFact )
    {
        pFact->CreateInstance( &pBasicObj );
    }
    else if( !this || (SotFactory *)this == SvEmbeddedObject::ClassFactory()
                  || (SotFactory *)this == (SotFactory *)SvInPlaceObject::ClassFactory() )
    {
        SvOutPlaceObject::ClassFactory()->CreateInstance( &pBasicObj );
    }
    else
    {
        CreateInstance( &pBasicObj );
    }

    SvObjectRef aObj( pBasicObj );
    return aObj;
}


/*************************************************************************
|*    SvFactory::Create()
|*
|*    Beschreibung
*************************************************************************/
SvObjectRef SvFactory::CreateAndLoad( SvStorage * pStor, BOOL ) const
{
    // Storage sichern
    SvStorageRef aStor = pStor;
    SvGlobalName aClassName_ = pStor->GetClassName();
    aClassName_ = GetAutoConvertTo( aClassName_ );

    const ::binfilter::SvObjectServer* pInternalServer = SvOutPlaceObject::GetInternalServer_Impl( aClassName_ );
    if ( pInternalServer )
    {
        // this is a StarOffice 6.1 embedded object
        SotStorageStreamRef xEmbStm = pStor->OpenSotStream(
                        String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "package_stream" ) ),
                        STREAM_STD_READ );

        if( !xEmbStm->GetError() )
        {
            SvStorageRef xEmbStor = new SvStorage( *xEmbStm );
            if( !xEmbStor->GetError() )
            {
                SvPersistRef aPObj( &Create( pInternalServer->GetClassName() ) );
                if( aPObj.Is() && aPObj->DoLoad( xEmbStor ) )
                    return &aPObj;
            }
        }
    }
    else
    {
        SvPersistRef aPObj( &Create( aClassName_ ) );
        if( aPObj.Is() && aPObj->DoLoad( pStor ) )
            return &aPObj;
    }

    return SvObjectRef();
}

/*************************************************************************
|*    SvFactory::CreateAndInit()
|*
|*    Beschreibung
*************************************************************************/
SvObjectRef SvFactory::CreateAndInit( const SvGlobalName & rClassName,
                                      SvStorage * pStor ) const
{
    // Storage sichern
    SvStorageRef aStor = pStor;
    SvEmbeddedObjectRef aEObj( &Create( rClassName ) );
    if( aEObj.Is() && aEObj->DoInitNew( pStor ) )
        return &aEObj;

    return SvObjectRef();
}

/*************************************************************************
|*    SvFactory::Register()
|*
|*    Beschreibung
*************************************************************************/
void SvFactory::Register()
{
    DBG_ASSERT( !IsRegistered(), "SvFactory::Register: register twice" );
    if( !IsRegistered() )
    {
        nRegisterId = 1; // Wert ist unwichtig, aber != 0
        DBG_ASSERT( IsRegistered(), "SvFactory::Register: cannot register" );
    }
}

//=========================================================================
ConvertTo_Impl (* SetupConvertTable_Impl( USHORT * pCount )) [SO3_OFFICE_VERSIONS]
{
    // Die alten Eintr"age m"ussen vor den neueren stehen, damit der
    // Algorithmus besser "uber mehrere Konvertierungsstufen laufen kann.
    // Neue Eintr"age d"urfen nur angeh"angt werden. Sonst m"ussen alle
    // Vorg"anger und Nachfolger Indizes "uberarbeitet werden.
    SoDll * pSoApp = SOAPP;
    if( !pSoApp->pConvTable )
    {
        pSoApp->nConvTableEntries = 8;
        pSoApp->pConvTable = new ConvertTo_Impl[8][SO3_OFFICE_VERSIONS];

        pSoApp->pConvTable[0][0] = ConvertTo_Impl
        (// StarWiter 3.0
          SvGlobalName( 0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70,0x02 ),
          *SvInPlaceObject::ClassFactory(),
          SOT_FORMATSTR_ID_STARWRITER_30
        );
        pSoApp->pConvTable[0][1] = ConvertTo_Impl
        (// StarWiter 4.0
          SvGlobalName( 0x8b04e9b0L, 0x420e, 0x11d0, 0xa4, 0x5e, 0x0,  0xa0, 0x24, 0x9d, 0x57, 0xb1 ),
          SOT_FORMATSTR_ID_STARWRITER_40
        );
        pSoApp->pConvTable[0][2] = ConvertTo_Impl
        (// StarWiter 5.0
          SvGlobalName( SO3_SW_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARWRITER_50
        );
        pSoApp->pConvTable[0][3] = ConvertTo_Impl
        (// StarWiter
          SvGlobalName( SO3_SW_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARWRITER_60
        );
        pSoApp->pConvTable[0][4] = ConvertTo_Impl
        (// StarWiter
          SvGlobalName( SO3_SW_CLASSID ),
          SOT_FORMATSTR_ID_STARWRITER_60
        );

        pSoApp->pConvTable[1][0] = ConvertTo_Impl
        (// StarDraw 3.0
          SvGlobalName( 0xAF10AAE0L, 0xB36D, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70, 0x02 ),
          *SvInPlaceObject::ClassFactory(),
          SOT_FORMATSTR_ID_STARDRAW
        );
        pSoApp->pConvTable[1][1] = ConvertTo_Impl
        (// StarDraw 4.0
          SvGlobalName( 0x012d3cc0L, 0x4216, 0x11d0, 0x89, 0xcb, 0x00, 0x80, 0x29, 0xe4, 0xb0, 0xb1 ),
          SOT_FORMATSTR_ID_STARDRAW_40
        );
        pSoApp->pConvTable[1][2] = ConvertTo_Impl
        (// StarImpress 5.0
          SvGlobalName( SO3_SIMPRESS_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARIMPRESS_50
        );
        pSoApp->pConvTable[1][3] = ConvertTo_Impl
        (// StarImpress
          SvGlobalName( SO3_SIMPRESS_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARIMPRESS_60
        );
        pSoApp->pConvTable[1][4] = ConvertTo_Impl
        (// StarImpress
          SvGlobalName( SO3_SIMPRESS_CLASSID ),
          SOT_FORMATSTR_ID_STARIMPRESS_60
        );

        pSoApp->pConvTable[2][0] = ConvertTo_Impl
        (// StarChart 3.0
          SvGlobalName( 0xFB9C99E0L, 0x2C6D, 0x101C, 0x8E, 0x2C, 0x00, 0x00, 0x1B, 0x4C, 0xC7, 0x11 ),
          SOT_FORMATSTR_ID_STARCHART
        );
        pSoApp->pConvTable[2][1] = ConvertTo_Impl
        (// StarChart 4.0
          SvGlobalName( 0x02b3b7e0L, 0x4225, 0x11d0, 0x89, 0xca, 0x00, 0x80, 0x29, 0xe4, 0xb0, 0xb1 ),
          SOT_FORMATSTR_ID_STARCHART_40
        );
        pSoApp->pConvTable[2][2] = ConvertTo_Impl
        (// StarChart 5.0
          SvGlobalName( SO3_SCH_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARCHART_50
        );
        pSoApp->pConvTable[2][3] = ConvertTo_Impl
        (// StarChart
          SvGlobalName( SO3_SCH_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARCHART_60
        );
        pSoApp->pConvTable[2][4] = ConvertTo_Impl
        (// StarChart
          SvGlobalName( SO3_SCH_CLASSID ),
          SOT_FORMATSTR_ID_STARCHART_60
        );

        pSoApp->pConvTable[3][0] = ConvertTo_Impl
        (// StarMath 3.0
          SvGlobalName( 0xD4590460L, 0x35FD, 0x101C, 0xB1, 0x2A, 0x04, 0x02, 0x1C, 0x00, 0x70, 0x02 ),
          SOT_FORMATSTR_ID_STARMATH
        );
        pSoApp->pConvTable[3][1] = ConvertTo_Impl
        (// StarMath 4.0
          SvGlobalName( 0x02b3b7e1L, 0x4225, 0x11d0, 0x89, 0xca, 0x00, 0x80, 0x29, 0xe4, 0xb0, 0xb1 ),
          SOT_FORMATSTR_ID_STARMATH_40
        );
        pSoApp->pConvTable[3][2] = ConvertTo_Impl
        (// StarMath 5.0
          SvGlobalName( SO3_SM_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARMATH_50
        );
        pSoApp->pConvTable[3][3] = ConvertTo_Impl
        (// StarMath
          SvGlobalName( SO3_SM_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARMATH_60
        );
        pSoApp->pConvTable[3][4] = ConvertTo_Impl
        (// StarMath
          SvGlobalName( SO3_SM_CLASSID ),
          SOT_FORMATSTR_ID_STARMATH_60
        );

        pSoApp->pConvTable[4][0] = ConvertTo_Impl
        (// StarCalc 3.0
          SvGlobalName( 0x3F543FA0L, 0xB6A6, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70, 0x02 ),
          *SvInPlaceObject::ClassFactory(),
          SOT_FORMATSTR_ID_STARCALC
        );
        pSoApp->pConvTable[4][1] = ConvertTo_Impl
        (// StarCalc 4.0
          SvGlobalName( 0x6361d441L, 0x4235, 0x11d0, 0x89, 0xcb, 0x00, 0x80, 0x29, 0xe4, 0xb0, 0xb1 ),
          SOT_FORMATSTR_ID_STARCALC_40
        );
        pSoApp->pConvTable[4][2] = ConvertTo_Impl
        (// StarCalc 5.0
          SvGlobalName( SO3_SC_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARCALC_50
        );
        pSoApp->pConvTable[4][3] = ConvertTo_Impl
        (// StarCalc
          SvGlobalName( SO3_SC_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARCALC_60
        );
        pSoApp->pConvTable[4][4] = ConvertTo_Impl
        (// StarCalc
          SvGlobalName( SO3_SC_CLASSID ),
          SOT_FORMATSTR_ID_STARCALC_60
        );

        pSoApp->pConvTable[5][0] = ConvertTo_Impl
        (// StarDraw 3.0
          SvGlobalName( 0xAF10AAE0L, 0xB36D, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70, 0x02 ),
          *SvInPlaceObject::ClassFactory(),
          SOT_FORMATSTR_ID_STARDRAW
        );
        pSoApp->pConvTable[5][1] = ConvertTo_Impl
        (// StarDraw 4.0
          SvGlobalName( 0x012d3cc0L, 0x4216, 0x11d0, 0x89, 0xcb, 0x00, 0x80, 0x29, 0xe4, 0xb0, 0xb1 ),
          SOT_FORMATSTR_ID_STARDRAW_40
        );
        pSoApp->pConvTable[5][2] = ConvertTo_Impl
        (// StarDraw 5.0
          SvGlobalName( SO3_SDRAW_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARDRAW_50
        );
        pSoApp->pConvTable[5][3] = ConvertTo_Impl
        (// StarDraw
          SvGlobalName( SO3_SDRAW_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARDRAW_60
        );
        pSoApp->pConvTable[5][4] = ConvertTo_Impl
        (// StarDraw
          SvGlobalName( SO3_SDRAW_CLASSID ),
          SOT_FORMATSTR_ID_STARDRAW_60
        );

        pSoApp->pConvTable[6][0] = ConvertTo_Impl
        (// StarWiter 3.0
          SvGlobalName( 0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70,0x02 ),
          *SvInPlaceObject::ClassFactory(),
          SOT_FORMATSTR_ID_STARWRITER_30
        );
        pSoApp->pConvTable[6][1] = ConvertTo_Impl
        (// StarWiter 4.0
          SvGlobalName( SO3_SWWEB_CLASSID_40 ),
          SOT_FORMATSTR_ID_STARWRITERWEB_40
        );
        pSoApp->pConvTable[6][2] = ConvertTo_Impl
        (// StarWiter 5.0
          SvGlobalName( SO3_SWWEB_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARWRITERWEB_50
        );
        pSoApp->pConvTable[6][3] = ConvertTo_Impl
        (// StarWiter
          SvGlobalName( SO3_SWWEB_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARWRITERWEB_60
        );
        pSoApp->pConvTable[6][4] = ConvertTo_Impl
        (// StarWiter
          SvGlobalName( SO3_SWWEB_CLASSID ),
          SOT_FORMATSTR_ID_STARWRITERWEB_60
        );

        pSoApp->pConvTable[7][0] = ConvertTo_Impl
        (// StarWiter 3.0
          SvGlobalName( 0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70,0x02 ),
          *SvInPlaceObject::ClassFactory(),
          SOT_FORMATSTR_ID_STARWRITER_30
        );
        pSoApp->pConvTable[7][1] = ConvertTo_Impl
        (// StarWiter 4.0
          SvGlobalName( SO3_SWGLOB_CLASSID_40 ),
          SOT_FORMATSTR_ID_STARWRITERGLOB_40
        );
        pSoApp->pConvTable[7][2] = ConvertTo_Impl
        (// StarWiter 5.0
          SvGlobalName( SO3_SWGLOB_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARWRITERGLOB_50
        );
        pSoApp->pConvTable[7][3] = ConvertTo_Impl
        (// StarWiter
          SvGlobalName( SO3_SWGLOB_CLASSID_60 ),
          SOT_FORMATSTR_ID_STARWRITERGLOB_60
        );
        pSoApp->pConvTable[7][4] = ConvertTo_Impl
        (// StarWiter
          SvGlobalName( SO3_SWGLOB_CLASSID ),
          SOT_FORMATSTR_ID_STARWRITERGLOB_60
        );

#if 0
        // obselete, because Image has been removed
        pSoApp->pConvTable[2][0] = ConvertTo_Impl
        (// StarImage 3.0
          SvGlobalName( 0xEA60C941L, 0x2C6C, 0x101C, 0x8E, 0x2C, 0x00, 0x00, 0x1B, 0x4C, 0xC7, 0x11 ),
          SOT_FORMATSTR_ID_STARIMAGE
        );
        pSoApp->pConvTable[2][1] = ConvertTo_Impl
        (// StarImage 4.0
          SvGlobalName( 0x447BB8A0L, 0x41FB, 0x11D0, 0x89, 0xCA, 0x00, 0x80, 0x29, 0xE4, 0xB0, 0xB1 ),
          SOT_FORMATSTR_ID_STARIMAGE_40
        );
        pSoApp->pConvTable[2][2] = ConvertTo_Impl
        (// StarImage 5.0
          SvGlobalName( SO3_SIM_CLASSID_50 ),
          SOT_FORMATSTR_ID_STARIMAGE_50
        );
        pSoApp->pConvTable[2][3] = ConvertTo_Impl
        (// StarImage 5.0
          SvGlobalName( SO3_SIM_CLASSID ),
          SOT_FORMATSTR_ID_STARIMAGE_50
        );
        pSoApp->pConvTable[2][4] = ConvertTo_Impl
        (// StarImage 5.0
          SvGlobalName( SO3_SIM_CLASSID ),
          SOT_FORMATSTR_ID_STARIMAGE_50
        );
#endif
    }
    *pCount = pSoApp->nConvTableEntries;
    return pSoApp->pConvTable;
}

//=========================================================================
SvGlobalName SvFactory::GetAutoConvertTo
(
    const SvGlobalName & rClass
)
/*  [Beschreibung]

    Ist f"ur diese KlassenId eine Konvertierung installiert, dann wird
    die Klasse, in die Konvertiert werden soll, zur"uckgegeben.
    Ansonsten wird rClass zur"ueckgegeben.

    [R"uckgabewert]

    SvGlobalName    Die KlassenId, die statt der "ubergebenen benutzt
                    werden soll.

    [Querverweise]

    <SvFactory::SetAutoConvertTo>,
*/
{
    SvGlobalName aRet = rClass;

    USHORT nCount;
    ConvertTo_Impl (* pTable)[SO3_OFFICE_VERSIONS] = SetupConvertTable_Impl( &nCount );
    for( USHORT i = 0; i < nCount; i++ )
    {
        for( USHORT n = 0; n < SO3_OFFICE_VERSIONS; n++ )
        {
            if( pTable[i][n].aName == aRet )
            {
                // #110743#
                // return pTable[i][SO3_OFFICE_VERSIONS -1].aName;
                if  (n < SO3_OFFICE_VERSIONS-2)
                {
                    return pTable[i][SO3_OFFICE_VERSIONS -3].aName;
                }
                else
                {
                    return pTable[i][SO3_OFFICE_VERSIONS -1].aName;
                }
            }
        }
    }

    return aRet;
}

//=========================================================================
SvGlobalName SvFactory::GetSvClass
(
    INT32 nFileFormat,
    const SvGlobalName & rClass
)
/*  [Beschreibung]

    Die So2 Klasse die dieses Objekt in der nFileFormat Version serven soll.

    [R"uckgabewert]

    SvGlobalName    Die KlassenId, die statt der "ubergebenen benutzt
                    werden soll.

    [Querverweise]

    <SvFactory::SetAutoConvertTo>,
*/
{
    SvGlobalName aRet = rClass;

    USHORT nCount;
    ConvertTo_Impl (*pTable)[SO3_OFFICE_VERSIONS] = SetupConvertTable_Impl( &nCount );
    for( USHORT i = 0; i < nCount; i++ )
    {
        for( USHORT n = 0; n < SO3_OFFICE_VERSIONS; n++ )
        {
            if( pTable[i][n].aName == aRet )
            {
                if( nFileFormat <= SOFFICE_FILEFORMAT_31 )
                    return pTable[i][0].aSvName;
                else if( nFileFormat <= SOFFICE_FILEFORMAT_40 )
                    return pTable[i][1].aSvName;
                else if( nFileFormat <= SOFFICE_FILEFORMAT_50 )
                    return pTable[i][2].aSvName;
                else if( nFileFormat <= SOFFICE_FILEFORMAT_60 )
                    return pTable[i][3].aSvName;
                else
                    return aRet;
                /*
                for( USHORT m = 0; m < SO3_OFFICE_VERSIONS; m++ )
                {
                    if( pTable[i][m].aFormat == nFileFormat )
                        return pTable[i][m].aSvName;
                }
                return aRet;
                */
            }
        }
    }
    return aRet;
}

//=========================================================================
BOOL SvFactory::IsIntern31
(
    const SvGlobalName & rClass
)
/*  [Beschreibung]

    Ist es in der 31 Version ein internes Objekt.

    [R"uckgabewert]

    BOOL            TRUE, es ist 31 intern.

    [Querverweise]
*/
{
    SvGlobalName aRet = rClass;

    USHORT nCount;
    ConvertTo_Impl (*pTable)[SO3_OFFICE_VERSIONS] = SetupConvertTable_Impl( &nCount );
    for( USHORT i = 0; i < nCount; i++ )
    {
        for( USHORT n = 0; n < SO3_OFFICE_VERSIONS; n++ )
        {
            if( pTable[i][n].aName == aRet )
                // Wenn es ein InPlace-Objekt ist, muss es extern sein
                return *SvInPlaceObject::ClassFactory()	!= pTable[i][0].aSvName;
        }
    }
    return FALSE;
}
//=========================================================================
BOOL SvFactory::IsIntern
(
    const SvGlobalName & rClass,
     long *pFileFormat
)
/*  [Description]

    Is it an internal object

    [Return Value]

    BOOL            TRUE, if it is an internal object

    [Cross reference]

*/
{
    SvGlobalName aRet = rClass;

    USHORT nCount;
    ConvertTo_Impl (*pTable)[SO3_OFFICE_VERSIONS] = SetupConvertTable_Impl( &nCount );
    for( USHORT i = 0; i < nCount; i++ )
    {
        for( USHORT n = 0; n < SO3_OFFICE_VERSIONS; n++ )
        {
            if( pTable[i][n].aName == aRet )
            {
                if( pFileFormat )
                {
                    switch( n )
                    {
                    case 0: *pFileFormat = SOFFICE_FILEFORMAT_31; break;
                    case 1: *pFileFormat = SOFFICE_FILEFORMAT_40; break;
                    case 2: *pFileFormat = SOFFICE_FILEFORMAT_50; break;
                    case 3: *pFileFormat = SOFFICE_FILEFORMAT_60; break;
                    default:
                            DBG_ERROR( "unexepected class id" );
                            break;
                    }
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

SvGlobalName SvFactory::GetServerName( long nStorageFormat )
{
    SvGlobalName aRet;

    USHORT nCount;
    ConvertTo_Impl (*pTable)[SO3_OFFICE_VERSIONS] = SetupConvertTable_Impl( &nCount );
    for( USHORT i = 0; i < nCount; i++ )
    {
        for( USHORT n = 0; n < SO3_OFFICE_VERSIONS; n++ )
        {
            if( pTable[i][n].aFormat == nStorageFormat )
                return pTable[i][n].aName;
        }
    }
    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
