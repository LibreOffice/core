/*************************************************************************
 *
 *  $RCSfile: dlgutil.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-23 09:29:36 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

// macro -----------------------------------------------------------------

#if SUPD<609
#define GET_MODULE_FIELDUNIT( eFieldUnit )                                  \
{                                                                           \
    SfxApplication* pSfxApp = SFX_APP();                                    \
    eFieldUnit = pSfxApp->GetOptions().GetMetric();                         \
    SfxModule* pModule = pSfxApp->GetActiveModule();                        \
                                                                            \
    if ( pModule )                                                          \
    {                                                                       \
        const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );     \
                                                                            \
        if ( pItem )                                                        \
            eFieldUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();  \
    }                                                                       \
}
#endif

// typedef ---------------------------------------------------------------

typedef long (*FUNC_CONVERT)(long);

// Functions -------------------------------------------------------------

// HM- und LanguageStrings aus der Resource laden
String  GetLanguageString( LanguageType eType );
String  GetDicInfoStr( const String& rName, const USHORT nLang,
                       const BOOL bNeg );

// FieldUnit im MetricField oder -Box umsetzen
void SetFieldUnit( MetricField& rCtrl,
                   FieldUnit eUnit, BOOL bAll = FALSE );
void SetFieldUnit( MetricBox& rCtrl,
                   FieldUnit eUnit, BOOL bAll = FALSE );

FieldUnit GetModuleFieldUnit( const SfxItemSet* pSet = NULL );

// Metriken umrechnen
long        CalcToUnit( float nIn, SfxMapUnit eUnit );
long        CalcToPoint( long nIn, SfxMapUnit eUnit, USHORT nFaktor );

long        ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl );
long        ControlToItem( long nIn, SfxFieldUnit eCtrl, SfxMapUnit eItem );

FieldUnit   MapToFieldUnit( const SfxMapUnit eUnit );
MapUnit     FieldToMapUnit( const SfxFieldUnit eUnit );

long        ConvertValueToMap( long nVal, SfxMapUnit eUnit );
long        ConvertValueToUnit( long nVal, SfxMapUnit eUnit );

void        SetMetricValue( MetricField& rField,
                            long lCoreValue, SfxMapUnit eUnit );
long        GetCoreValue( const MetricField& rField, SfxMapUnit eUnit );

// to Twips
long    CMToTwips( long nIn );
long    MMToTwips( long nIn );
long    InchToTwips( long nIn );
long    PointToTwips( long nIn );
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


#endif

