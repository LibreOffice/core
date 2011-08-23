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
#ifndef _SVX_DLGUTIL_HXX
#define _SVX_DLGUTIL_HXX

// include ---------------------------------------------------------------
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif
namespace binfilter {

// macro -----------------------------------------------------------------

// typedef ---------------------------------------------------------------


// Functions -------------------------------------------------------------

// HM- und LanguageStrings aus der Resource laden

// FieldUnit im MetricField oder -Box umsetzen

FieldUnit GetModuleFieldUnit( const SfxItemSet* pSet = NULL );

// Metriken umrechnen
long 		CalcToUnit( float nIn, SfxMapUnit eUnit );
long 		CalcToPoint( long nIn, SfxMapUnit eUnit, USHORT nFaktor );

long		ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl );




// to Twips

// to CM

// to MM

// to Inch

// to Point

// To Pica

// generische Wandlung

// replacement of the SfxIniManager

#define OUTPUT_DRAWMODE_COLOR		(DRAWMODE_DEFAULT)
#define OUTPUT_DRAWMODE_CONTRAST	(DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT)

}//end of namespace binfilter
#endif

