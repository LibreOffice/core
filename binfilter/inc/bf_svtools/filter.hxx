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

#ifndef _FILTER_HXX
#define _FILTER_HXX

#ifndef _FLTCALL_HXX
#include <bf_svtools/fltcall.hxx>
#endif

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

class Window;
struct ConvertData;

namespace binfilter
{

// -----------------------
// - GraphicFilter-Types -
// -----------------------

struct ImplDirEntryHelper
{
    static BOOL Exists( const INetURLObject& rObj );
    static void Kill( const String& rStr );
};



#define OPT_FILTERSECTION			"Graphic"

#define GRFILTER_OK					0
#define GRFILTER_OPENERROR			1
#define GRFILTER_IOERROR			2
#define GRFILTER_FORMATERROR		3
#define GRFILTER_VERSIONERROR		4
#define GRFILTER_FILTERERROR		5
#define GRFILTER_ABORT				6
#define GRFILTER_TOOBIG				7

#define GRFILTER_FORMAT_NOTFOUND	((USHORT)0xFFFF)
#define GRFILTER_FORMAT_DONTKNOW	((USHORT)0xFFFF)

#define GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG       00000001
#define GRFILTER_I_FLAGS_DONT_SET_LOGSIZE_FOR_JPEG  00000002
#define GRFILTER_I_FLAGS_FOR_PREVIEW                00000004
#define GRFILTER_I_FLAGS_ALLOW_PARTIAL_STREAMREAD   00000010

#define IMP_BMP					"SVBMP"
#define IMP_SVMETAFILE			"SVMETAFILE"
#define IMP_WMF					"SVWMF"
#define IMP_EMF					"SVEMF"
#define IMP_SVSGF				"SVSGF"
#define IMP_SVSGV				"SVSGV"
#define IMP_GIF					"SVIGIF"
#define IMP_PNG					"SVIPNG"
#define IMP_JPEG				"SVIJPEG"
#define IMP_XBM					"SVIXBM"
#define IMP_XPM					"SVIXPM"
#define EXP_BMP					"SVBMP"
#define EXP_SVMETAFILE			"SVMETAFILE"
#define EXP_WMF					"SVWMF"
#define EXP_EMF					"SVEMF"
#define EXP_JPEG				"SVEJPEG"
#define EXP_SVG					"SVESVG"
#define EXP_PNG					"SVEPNG"

#define BMP_SHORTNAME			"BMP"
#define GIF_SHORTNAME			"GIF"
#define JPG_SHORTNAME			"JPG"
#define MET_SHORTNAME			"MET"
#define PCT_SHORTNAME			"PCT"
#define PNG_SHORTNAME			"PNG"
#define SVM_SHORTNAME			"SVM"
#define TIF_SHORTNAME			"TIF"
#define WMF_SHORTNAME			"WMF"
#define EMF_SHORTNAME			"EMF"

// ------------------------------------
// - Info-Klasse fuer alle von uns
//	unterstuetzten Grafik-Fileformate
// ------------------------------------

#define	GFF_NOT	( (USHORT)0x0000 )
#define	GFF_BMP	( (USHORT)0x0001 )
#define	GFF_GIF	( (USHORT)0x0002 )
#define	GFF_JPG	( (USHORT)0x0003 )
#define	GFF_PCD	( (USHORT)0x0004 )
#define	GFF_PCX	( (USHORT)0x0005 )
#define	GFF_PNG	( (USHORT)0x0006 )
#define	GFF_TIF	( (USHORT)0x0007 )
#define	GFF_XBM	( (USHORT)0x0008 )
#define	GFF_XPM	( (USHORT)0x0009 )
#define	GFF_PBM	( (USHORT)0x000a )
#define	GFF_PGM	( (USHORT)0x000b )
#define	GFF_PPM	( (USHORT)0x000c )
#define	GFF_RAS	( (USHORT)0x000d )
#define	GFF_TGA	( (USHORT)0x000e )
#define	GFF_PSD	( (USHORT)0x000f )
#define	GFF_EPS	( (USHORT)0x0010 )
#define	GFF_DXF	( (USHORT)0x00f1 )
#define	GFF_MET	( (USHORT)0x00f2 )
#define	GFF_PCT	( (USHORT)0x00f3 )
#define	GFF_SGF	( (USHORT)0x00f4 )
#define	GFF_SVM	( (USHORT)0x00f5 )
#define	GFF_WMF	( (USHORT)0x00f6 )
#define GFF_SGV	( (USHORT)0x00f7 )
#define	GFF_EMF	( (USHORT)0x00f8 )
#define	GFF_XXX	( (USHORT)0xffff )

// ---------------
// - RequestInfo -
// ---------------

struct RequestInfo
{
    BYTE*	pBuffer;
    ULONG	nRealBufferSize;
};

// ---------------------
// - GraphicDescriptor -
// ---------------------

class  GraphicDescriptor
{
    SvStream*			pFileStm;
    Link				aReqLink;
    String				aPathExt;
    Size				aPixSize;
    Size				aLogSize;
    SvStream*			pMemStm;
    SvStream*			pBaseStm;
    ULONG				nStmPos;
    USHORT				nBitsPerPixel;
    USHORT				nPlanes;
    USHORT				nFormat;
    BOOL				bCompressed;
    BOOL				bDataReady;
    BOOL				bLinked;
    BOOL				bLinkChanged;
    BOOL				bWideSearch;
    BOOL				bBaseStm;
    long				nExtra1;
    long				nExtra2;

    void				ImpConstruct();

//#if 0 // _SOLAR__PRIVATE

    BOOL				ImpDetectBMP( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectGIF( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectJPG( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPCD( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPCX( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPNG( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectTIF( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectXBM( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectXPM( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPBM( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPGM( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPPM( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectRAS( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectTGA( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPSD( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectEPS( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectDXF( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectMET( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectPCT( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectSGF( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectSVM( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectWMF( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectSGV( SvStream& rStm, BOOL bExtendedInfo );
    BOOL				ImpDetectEMF( SvStream& rStm, BOOL bExtendedInfo );

//#endif

    GraphicDescriptor( const GraphicDescriptor& );
    GraphicDescriptor& operator=( const GraphicDescriptor& );

protected:

    SvStream&			GetSearchStream() const;
    const String&		GetPathExtension() const;

public:
    // Ctor, um einen Stream zu setzen. Es muss ::Detect() gerufen werden,
    // um das File zu identifizieren;
    // da einige Formate ( Mtf's ) keinen eindeutigen Header besitzen,
    // ist es sinnvoll den Filenamen (inkl. Ext. ) mitanzugeben,
    // da so das Format ueber die Extension ermittelt werden kann
    GraphicDescriptor( SvStream& rInStream, const String* pPath = NULL );

    // Dtor
    virtual ~GraphicDescriptor();

    // Startet die Detektion;
    // bei bExtendedInfo == TRUE werden soweit wie moeglich
    // Daten aus dem jeweiligen FileHeader ermittelt
    // ( Groesse, Farbtiefe usw. )
    virtual BOOL	Detect( BOOL bExtendedInfo = FALSE );

    // liefert das Fileformat nach erfolgreicher  Detektion zurueck;
    // wenn kein Format erkannt wurde, ist das Formart GFF_NOT
    USHORT			GetFileFormat() const { return nFormat; }

    // liefert die Pixel-Bildgroesse oder 0-Size zurueck
    const Size&		GetSizePixel() const { return (Size&) aPixSize; }

    // liefert die logische Bildgroesse in 1/100mm oder 0-Size zurueck
    const Size&		GetSize_100TH_MM() const { return (Size&) aLogSize; }

    // liefert die Bits/Pixel oder 0 zurueck
    USHORT			GetBitsPerPixel() const { return nBitsPerPixel; }

    // liefert die Anzahl der Planes oder 0 zurueck
    USHORT			GetPlanes() const { return nPlanes; }

    // zeigt an, ob das Bild evtl. komprimiert (wie auch immer) ist
    BOOL			IsCompressed() const { return bCompressed; }

    // gibt den LinkHdl zum Setzen der Bytes zurueck
    const Link&		GetRequestHdl() const { return aReqLink; }

    // gibt die Filternummer des Filters zurueck,
    // der im GraphicFilter zum Lesen dieses Formats
    // benoetigt wird
    static String GetImportFormatShortName( sal_uInt16 nFormat );
};

// -----------------
// - GraphicFilter -
// -----------------

struct FilterErrorEx
{
    ULONG	nFilterError;
    ULONG	nStreamError;
    long	nDummy1;
    long	nDummy2;
    long	nDummy3;
    long	nDummy4;

            FilterErrorEx() : nFilterError( 0UL ), nStreamError( 0UL ) {}
};

// -----------------------------------------------------------------------------

class FilterConfigCache;
class  GraphicFilter
{
    friend class SvFilterOptionsDialog;

private:

    void			ImplInit();
    ULONG			ImplSetError( ULONG nError, const SvStream* pStm = NULL );
    sal_uInt16		ImpTestOrFindFormat( const String& rPath, SvStream& rStream, sal_uInt16& rFormat );

                    DECL_LINK( FilterCallback, ConvertData* pData );

protected:

    String				aFilterPath;
    FilterConfigCache*	pConfig;
    FilterErrorEx*		pErrorEx;
    sal_Bool			bAbort;
    sal_Bool			bUseConfig;
    sal_Bool			bDummy1;
    sal_Bool			bDummy2;
    sal_Bool			bDummy3;
    sal_Bool			bDummy4;
    long				nDummy1;
    long				nDummy2;
    void*				pDummy1;
    void*				pDummy2;

public:

                    GraphicFilter( sal_Bool bUseConfig = sal_True );
                    ~GraphicFilter();

    void			SetFilterPath( const String& rFilterPath ) { aFilterPath = rFilterPath; };

    USHORT          GetImportFormatCount();
    USHORT          GetImportFormatNumber( const String& rFormatName );
    USHORT			GetImportFormatNumberForMediaType( const String& rMediaType );
    USHORT          GetImportFormatNumberForShortName( const String& rShortName );
    sal_uInt16		GetImportFormatNumberForTypeName( const String& rType );
    String          GetImportFormatName( USHORT nFormat );
    String          GetImportFormatTypeName( USHORT nFormat );
    String          GetImportFormatMediaType( USHORT nFormat );
    String          GetImportFormatShortName( USHORT nFormat );
    String			GetImportWildcard( USHORT nFormat, sal_Int32 nEntry = 0 );
    BOOL			IsImportPixelFormat( USHORT nFormat );

    USHORT          GetExportFormatCount();
    USHORT          GetExportFormatNumber( const String& rFormatName );
    USHORT          GetExportFormatNumberForMediaType( const String& rShortName );
    USHORT          GetExportFormatNumberForShortName( const String& rShortName );
    sal_uInt16		GetExportFormatNumberForTypeName( const String& rType );
    String          GetExportFormatName( USHORT nFormat );
    String          GetExportFormatTypeName( USHORT nFormat );
    String          GetExportFormatMediaType( USHORT nFormat );
    String          GetExportFormatShortName( USHORT nFormat );
    String			GetExportWildcard( USHORT nFormat, sal_Int32 nEntry = 0 );
    USHORT			ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
                                    USHORT nFormat = GRFILTER_FORMAT_DONTKNOW,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );
    USHORT			ExportGraphic( const Graphic& rGraphic, const String& rPath,
                                    SvStream& rOStm, USHORT nFormat = GRFILTER_FORMAT_DONTKNOW,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );

    USHORT          CanImportGraphic( const INetURLObject& rPath,
                                      USHORT nFormat = GRFILTER_FORMAT_DONTKNOW,
                                      USHORT * pDeterminedFormat = NULL);

    USHORT          ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                   USHORT nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   USHORT * pDeterminedFormat = NULL, sal_uInt32 nImportFlags = 0 );

    USHORT          CanImportGraphic( const String& rPath, SvStream& rStream,
                                      USHORT nFormat = GRFILTER_FORMAT_DONTKNOW,
                                      USHORT * pDeterminedFormat = NULL);

    USHORT          ImportGraphic( Graphic& rGraphic, const String& rPath,
                                   SvStream& rStream,
                                   USHORT nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   USHORT * pDeterminedFormat = NULL, sal_uInt32 nImportFlags = 0 );

    USHORT          ImportGraphic( Graphic& rGraphic, const String& rPath,
                                   SvStream& rStream,
                                   USHORT nFormat,
                                   USHORT * pDeterminedFormat, sal_uInt32 nImportFlags,
                                   com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData );

    void            Abort() { bAbort = TRUE; }

    void					ResetLastError();

    static 			GraphicFilter* GetGraphicFilter();
};

// ------------------------------------
// - Windows Metafile Lesen/Schreiben -
// ------------------------------------

 BOOL ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pConfigItem );
 BOOL WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF );

}

#endif  //_FILTER_HXX
