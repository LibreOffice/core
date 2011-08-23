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

#include <stdio.h>


#include <vcl/graph.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_so3/embobj.hxx>
#include <bf_so3/ipobj.hxx>
#include <sot/exchange.hxx>

namespace binfilter {

// Vordefinierte OLE-1-IDs
// Unter Windows koennen wir uns noch auf die RegDB stuerzen, sonst
// nehmen wir eben diese IDs
// Standardformat:

//     0003xxxx-0000-0000-C000-000000000046

struct ClsIDs {
    UINT32		nId;
    const sal_Char* pSvrName;
    const sal_Char* pDspName;
} aClsIDs[] = {

    { 0x000212F0, "MSWordArt",     		"Microsoft Word Art"	 		},
    { 0x000212F0, "MSWordArt.2",   		"Microsoft Word Art 2.0" 		},

    // MS Apps
    { 0x00030000, "ExcelWorksheet",		"Microsoft Excel Worksheet"		},
    { 0x00030001, "ExcelChart",			"Microsoft Excel Chart"			},
    { 0x00030002, "ExcelMacrosheet",	"Microsoft Excel Macro"			},
    { 0x00030003, "WordDocument",		"Microsoft Word Document"		},
    { 0x00030004, "MSPowerPoint",		"Microsoft PowerPoint"			},
    { 0x00030005, "MSPowerPointSho",	"Microsoft PowerPoint Slide Show"},
    { 0x00030006, "MSGraph",			"Microsoft Graph"				},
    { 0x00030007, "MSDraw",				"Microsoft Draw"				},
    { 0x00030008, "Note-It",			"Microsoft Note-It"				},
    { 0x00030009, "WordArt",			"Microsoft Word Art"			},
    { 0x0003000a, "PBrush",				"Microsoft PaintBrush Picture"	},
    { 0x0003000b, "Equation",			"Microsoft Equation Editor"		},
    { 0x0003000c, "Package",			"Package"						},
    { 0x0003000d, "SoundRec",			"Sound"							},
    { 0x0003000e, "MPlayer",			"Media Player"					},
    // MS Demos
    { 0x0003000f, "ServerDemo",			"OLE 1.0 Server Demo"			},
    { 0x00030010, "Srtest",				"OLE 1.0 Test Demo"				},
    { 0x00030011, "SrtInv",				"OLE 1.0 Inv Demo"				},
    { 0x00030012, "OleDemo",			"OLE 1.0 Demo"					},

    // Coromandel / Dorai Swamy / 718-793-7963
    { 0x00030013, "CoromandelIntegra",	"Coromandel Integra"			},
    { 0x00030014, "CoromandelObjServer","Coromandel Object Server"		},

    // 3-d Visions Corp / Peter Hirsch / 310-325-1339
    { 0x00030015, "StanfordGraphics",	"Stanford Graphics"				},

    // Deltapoint / Nigel Hearne / 408-648-4000
    { 0x00030016, "DGraphCHART",		"DeltaPoint Graph Chart"		},
    { 0x00030017, "DGraphDATA",			"DeltaPoint Graph Data"			},

    // Corel / Richard V. Woodend / 613-728-8200 x1153
    { 0x00030018, "PhotoPaint",			"Corel PhotoPaint"				},
    { 0x00030019, "CShow",				"Corel Show"					},
    { 0x0003001a, "CorelChart",			"Corel Chart"					},
    { 0x0003001b, "CDraw",				"Corel Draw"					},

    // Inset Systems / Mark Skiba / 203-740-2400
    { 0x0003001c, "HJWIN1.0",			"Inset Systems"					},

    // Mark V Systems / Mark McGraw / 818-995-7671
    { 0x0003001d, "ObjMakerOLE",		"MarkV Systems Object Maker"	},

    // IdentiTech / Mike Gilger / 407-951-9503
    { 0x0003001e, "FYI",				"IdentiTech FYI"				},
    { 0x0003001f, "FYIView",			"IdentiTech FYI Viewer"			},

    // Inventa Corporation / Balaji Varadarajan / 408-987-0220
    { 0x00030020, "Stickynote",			"Inventa Sticky Note"			},

    // ShapeWare Corp. / Lori Pearce / 206-467-6723
    { 0x00030021, "ShapewareVISIO10",   "Shapeware Visio 1.0"			},
    { 0x00030022, "ImportServer",		"Spaheware Import Server"		},

    // test app SrTest
    { 0x00030023, "SrvrTest",			"OLE 1.0 Server Test"			},

    // test app ClTest.  Doesn't really work as a server but is in reg db
    { 0x00030025, "Cltest",				"OLE 1.0 Client Test"			},

    // Microsoft ClipArt Gallery   Sherry Larsen-Holmes
    { 0x00030026, "MS_ClipArt_Gallery",	"Microsoft ClipArt Gallery"		},
    // Microsoft Project  Cory Reina
    { 0x00030027, "MSProject",			"Microsoft Project"				},

    // Microsoft Works Chart
    { 0x00030028, "MSWorksChart",		"Microsoft Works Chart"			},

    // Microsoft Works Spreadsheet
    { 0x00030029, "MSWorksSpreadsheet",	"Microsoft Works Spreadsheet"	},

    // AFX apps - Dean McCrory
    { 0x0003002A, "MinSvr",				"AFX Mini Server"				},
    { 0x0003002B, "HierarchyList",		"AFX Hierarchy List"			},
    { 0x0003002C, "BibRef",				"AFX BibRef"					},
    { 0x0003002D, "MinSvrMI",			"AFX Mini Server MI"			},
    { 0x0003002E, "TestServ",			"AFX Test Server"				},

    // Ami Pro
    { 0x0003002F, "AmiProDocument",		"Ami Pro Document"				},

    // WordPerfect Presentations For Windows
    { 0x00030030, "WPGraphics",			"WordPerfect Presentation"		},
    { 0x00030031, "WPCharts",			"WordPerfect Chart"				},

    // MicroGrafx Charisma
    { 0x00030032, "Charisma",			"MicroGrafx Charisma"			},
    { 0x00030033, "Charisma_30",		"MicroGrafx Charisma 3.0"		},
    { 0x00030034, "CharPres_30",		"MicroGrafx Charisma 3.0 Pres"	},
    // MicroGrafx Draw
    { 0x00030035, "Draw",				"MicroGrafx Draw"				},
    // MicroGrafx Designer
    { 0x00030036, "Designer_40",		"MicroGrafx Designer 4.0"		},

    // STAR DIVISION
//	{ 0x000424CA, "StarMath",			"StarMath 1.0"					},
    { 0x00043AD2, "FontWork",			"Star FontWork"					},
//	{ 0x000456EE, "StarMath2",			"StarMath 2.0"					},

    { 0, "", "" } };

// Da wir nicht die gesamte SO-Lib inklusive OLE1 dran haben wollen,
// sei hier die Leseroutine fuer ein StarObject noch mit drin.

#define SO_SIGNATURE 0x534F6D64             // Signatur ('mdSO' im INTEL-Format)
#define	SO_VERSION	 0x00000002

struct StarObjectFileHdr
{
    UINT32  nSize;                          // Groesse der Struktur
    UINT32  nSignature;                     // Signatur
    UINT32  nVersion;                       // Version
    UINT32  nWidth;                         // Breite in 1/100 mm
    UINT32  nHeight;                        // Hoehe in 1/100 mm
    UINT32  nObjName;                       // Laenge Objektname
    UINT32  nServerName;                    // Laenge Servername
    UINT32  nPrivData;                      // Laenge private Daten
    UINT32  nPicData;                       // Laenge GDIMetafile
};

// Laden eines StarObjects. Die Daten werden in den Parametern
// uebergeben, der Returnwert ist der Stream-Fehlercode.
#define PIC_READ	1
#define DATA_READ	2
static ULONG LoadStarObject
    ( SvStream& rSrc, String& rSvrName, String& rObjName,
      GDIMetaFile & rPic, SvStream & rDataStm )
{
    ULONG nRet = 0;
    StarObjectFileHdr aHdr;
    long pos = rSrc.Tell();
    aHdr.nSize = 0L;
    rSrc >> aHdr.nSize;
    rSrc.Seek( pos );

    if ( aHdr.nSize != sizeof( aHdr ) )
    {
        rSrc.SetError( SVSTREAM_GENERALERROR );
        return 0;
    }

    rSrc >> aHdr.nSize
         >> aHdr.nSignature
         >> aHdr.nVersion
         >> aHdr.nWidth
         >> aHdr.nHeight
         >> aHdr.nObjName
         >> aHdr.nServerName
         >> aHdr.nPrivData
         >> aHdr.nPicData;

    if ( aHdr.nSignature != SO_SIGNATURE || aHdr.nVersion > SO_VERSION )
    {
        rSrc.SetError( SVSTREAM_GENERALERROR );
        return 0;
    }

    sal_Char cBuf[ 128 ];
    rSrc.Read( cBuf, aHdr.nObjName );
    if ( rSrc.GetError() )
        return 0;
    rObjName = String(
        cBuf, sal::static_int_cast< xub_StrLen >( aHdr.nObjName ),
        gsl_getSystemTextEncoding() );

    rSrc.Read( cBuf, aHdr.nServerName );
    if ( rSrc.GetError() )
        return 0;
    rSvrName = String(
        cBuf, sal::static_int_cast< xub_StrLen >( aHdr.nServerName ),
        gsl_getSystemTextEncoding() );
    if ( aHdr.nPrivData )
    {
        UINT32 nLen;
        rSrc >> nLen;
        if ( rSrc.GetError() )
            return 0;

        BYTE * pData = new BYTE[ nLen ];
        rDataStm << nLen;
        rSrc.Read( pData, nLen );
        rDataStm.Write( pData, nLen );
        delete [] pData;
        if ( rDataStm.GetError() )
        {
            rSrc.SetError( rDataStm.GetError() );
            return 0;
        }
        nRet = DATA_READ;
    }
    if ( aHdr.nPicData )
    {
        if( aHdr.nVersion < SO_VERSION )
        {
            // Alte Grafiken
            rSrc >> rPic;
        }
        else
        {
            // neue, portable oder nicht portable Grafiken
            Graphic aGrf;
            aGrf.ReadEmbedded( rSrc );
            rPic = aGrf.GetGDIMetaFile();
        }
        nRet |= PIC_READ;
    }
    if ( rSrc.GetError() )
        return 0;
    return nRet;
}

BOOL SvEmbeddedObject::InsertStarObject
    ( const String &rObjName, const String& rStgName,
      SvStream& rSrc, SvPersist* pPersist )
{
    ULONG nRead;
    SvStorageRef rpDst( pPersist->GetStorage() );
    String aObjName, aSvrName;
    GDIMetaFile aPic;
    SvMemoryStream aDataStm;

    nRead = LoadStarObject( rSrc, aSvrName, aObjName, aPic, aDataStm );
    if( rSrc.GetError() != SVSTREAM_OK )
        return FALSE;

    // Es kann losgehen!
    // Erstmal den ClassID sowie die Bschreibung suchen
    ClsIDs* pIds;
    for( pIds = aClsIDs; pIds->nId; pIds++ )
    {
        if( COMPARE_EQUAL == aSvrName.CompareToAscii( pIds->pSvrName ) )
            break;
    }
    SvGlobalName* pClsId = NULL;
    String aShort, aFull;
    if( pIds->nId )
    {
        // gefunden!
        pClsId = new SvGlobalName( pIds->nId, 0, 0, 0xc0,0,0,0,0,0,0,0x46 );
        aFull = String( pIds->pDspName, gsl_getSystemTextEncoding() );
        aShort = aSvrName;
    }
    else if( (COMPARE_EQUAL == aSvrName.CompareToAscii( "StarMath2" ))
        || (COMPARE_EQUAL == aSvrName.CompareToAscii( "StarMath" )) )
    {
        // Sonderfall: In StarMath 3 umwandeln
        pClsId = new SvGlobalName;
        pClsId->MakeId( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "D4590460-35FD-101C-B12A-04021C007002" ) ) );
        aFull = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarMath 3.0" ) );
        aShort = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "StarMath" ) );
    }

    if( !pClsId )
    {
        rpDst->SetError( SVSTREAM_GENERALERROR );
        return FALSE;
    }

    // Class-ID und Daten sind vorhanden. Nun muss noch der Storage
    // aufgebaut werden.
    // Main Storage einrichten
    if( rpDst->IsContained( rStgName ) )
        rpDst->Remove( rStgName );
    SvStorageRef rpObj = rpDst->OpenOLEStorage( rStgName,
                            STREAM_READWRITE| STREAM_SHARE_DENYALL );
//	SvStorageRef rpObj = rpDst->OpenStorage( rStgName,
//							STREAM_READWRITE| STREAM_SHARE_DENYALL );
    if( rpObj.Is() && rpObj->GetError() )
    {
        rpDst->SetError( rpObj->GetError() ); rpObj.Clear();
    }
    if( rpObj.Is() )
    {
        // 1. die OLE-Daten
        ULONG nCbFmt = SotExchange::RegisterFormatName( aShort );
        rpObj->SetClass( *pClsId, nCbFmt, aFull );
    }
    delete pClsId;

    // 2. Den Daten-Stream aufsetzen
    if( rpObj.Is() && (nRead & DATA_READ) )
    {
        SvStorageStreamRef rpStrm =
            rpObj->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) ),
                            STREAM_WRITE| STREAM_SHARE_DENYALL );
        if( rpStrm.Is() && rpStrm->GetError() )
        {
            rpDst->SetError( rpStrm->GetError() ); rpStrm.Clear();
        }
        if( rpStrm.Is() )
        {
            aDataStm.Seek( 0 );
            *rpStrm << aDataStm;
            rpDst->SetError( rpObj->GetError() );
            if( rpDst->GetError() )
                rpObj.Clear();
        }
    }

    // 3. Den Picture-Stream aufsetzen
    if( rpObj.Is() && (nRead & PIC_READ) )
    {
        ULONG nErr = SVSTREAM_OK;
        if( !SvEmbeddedObject::MakeContentStream( rpObj, aPic ) )
            nErr = SVSTREAM_WRITE_ERROR;
        else
            nErr = rpObj->GetError();
        rpDst->SetError( nErr );
        if( nErr != SVSTREAM_OK )
            rpObj.Clear();
    }

    // 4. Alles committen
    if( rpObj.Is() )
    {
        rpObj->Commit();
        rpDst->SetError( rpObj->GetError() );
    }
    // Sonst evtl. den (defekten) Storage weg
    else if( rpDst->IsContained( rStgName ) )
        rpDst->Remove( rStgName );
    rpDst->Commit();
    if( rpDst->GetError() == SVSTREAM_OK )
    {
        SvObjectRef rpOleObj =
            ((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndLoad( rpObj );
        SvInPlaceObjectRef rpInplaceObj( &rpOleObj );
        if( rpInplaceObj.Is() )
        {
            SvInfoObjectRef rpInfo =
                new SvEmbeddedInfoObject( rpInplaceObj, rObjName );
            return pPersist->Insert( rpInfo );
        }
    }
    return FALSE;
}

// Laden des Pictures eines StarObjects
// (falls es nicht als OLE-Objekt ladbar ist)

BOOL SvEmbeddedObject::LoadStarObjectPicture( SvStream& rSrc, GDIMetaFile& rMTF )
{
    String aObjName, aSvrName;
    GDIMetaFile  aPic;
    SvMemoryStream aDataStm;

    ULONG nErr = LoadStarObject( rSrc, aSvrName, aObjName, aPic, aDataStm );
    (void)nErr;
    if( rSrc.GetError() != SVSTREAM_OK )
        return FALSE;

    rMTF = aPic;
    return TRUE;
}

}
