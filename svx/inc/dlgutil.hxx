/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgutil.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:35:13 $
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
#ifndef _SVX_DLGUTIL_HXX
#define _SVX_DLGUTIL_HXX

// include ---------------------------------------------------------------
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// macro -----------------------------------------------------------------

// typedef ---------------------------------------------------------------

typedef long (*FUNC_CONVERT)(long);

// Functions -------------------------------------------------------------

// HM- und LanguageStrings aus der Resource laden
SVX_DLLPUBLIC String    GetLanguageString( LanguageType eType );
SVX_DLLPUBLIC String    GetDicInfoStr( const String& rName, const USHORT nLang,
                       const BOOL bNeg );

// FieldUnit im MetricField oder -Box umsetzen
SVX_DLLPUBLIC void SetFieldUnit( MetricField& rCtrl,
                   FieldUnit eUnit, BOOL bAll = FALSE );
SVX_DLLPUBLIC void SetFieldUnit( MetricBox& rCtrl,
                   FieldUnit eUnit, BOOL bAll = FALSE );

SVX_DLLPUBLIC FieldUnit GetModuleFieldUnit( const SfxItemSet* pSet = NULL );

// Metriken umrechnen
SVX_DLLPUBLIC long      CalcToUnit( float nIn, SfxMapUnit eUnit );
SVX_DLLPUBLIC long      CalcToPoint( long nIn, SfxMapUnit eUnit, USHORT nFaktor );

SVX_DLLPUBLIC long      ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl );
long        ControlToItem( long nIn, SfxFieldUnit eCtrl, SfxMapUnit eItem );

SVX_DLLPUBLIC FieldUnit MapToFieldUnit( const SfxMapUnit eUnit );
MapUnit     FieldToMapUnit( const SfxFieldUnit eUnit );

long        ConvertValueToMap( long nVal, SfxMapUnit eUnit );
long        ConvertValueToUnit( long nVal, SfxMapUnit eUnit );

SVX_DLLPUBLIC void      SetMetricValue( MetricField& rField,
                            long lCoreValue, SfxMapUnit eUnit );
SVX_DLLPUBLIC long      GetCoreValue( const MetricField& rField, SfxMapUnit eUnit );

// to Twips
long    CMToTwips( long nIn );
long    MMToTwips( long nIn );
long    InchToTwips( long nIn );
SVX_DLLPUBLIC long  PointToTwips( long nIn );
long    PicaToTwips( long nIn );

// to CM
long    TwipsToCM( long nIn );
long    InchToCM( long nIn );
long    MMToCM( long nIn );
long    PointToCM( long nIn );
long    PicaToCM( long nIn );

// to MM
long    TwipsToMM( long nIn );
long    CMToMM( long nIn );
long    InchToMM( long nIn );
long    PointToMM( long nIn );
long    PicaToMM( long nIn );

// to Inch
long    TwipsToInch(long nIn );
long    CMToInch(long nIn );
long    MMToInch(long nIn );
long    PointToInch(long nIn );
long    PicaToInch(long nIn );

// to Point
long    TwipsToPoint(long nIn );
long    InchToPoint(long nIn );
long    CMToPoint(long nIn );
long    MMToPoint(long nIn );
long    PicaToPoint(long nIn );

// To Pica
long    TwipsToPica(long nIn );
long    InchToPica(long nIn );
long    PointToPica(long nIn );
long    CMToPica(long nIn );
long    MMToPica(long nIn );

// generische Wandlung
long    TransformMetric( long nVal, FieldUnit aOld, FieldUnit aNew );

// replacement of the SfxIniManager
String      ConvertPosSizeToIniString( const Point& rPos, const Size& rSize );
sal_Bool    ConvertIniStringToPosSize(  const String& rIniStr, Point& rPos, Size& rSize );


#define OUTPUT_DRAWMODE_COLOR       (DRAWMODE_DEFAULT)
#define OUTPUT_DRAWMODE_CONTRAST    (DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT)

#endif

