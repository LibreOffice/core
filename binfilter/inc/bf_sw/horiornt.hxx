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

#ifndef _HORIORNT_HXX
#define _HORIORNT_HXX
namespace binfilter {


//SwFmtHoriOrient, wie und woran orientiert --
//	sich der FlyFrm in der Hoizontalen ----------

enum SwHoriOrient
{
    HORI_NONE,		//Der Wert in nYPos gibt die RelPos direkt an.
    HORI_RIGHT,		//Der Rest ist fuer automatische Ausrichtung.
    HORI_CENTER,
    HORI_LEFT,
    HORI_INSIDE,
    HORI_OUTSIDE,
    HORI_FULL,			//Spezialwert fuer Tabellen
    HORI_LEFT_AND_WIDTH  //Auch fuer Tabellen

    //Die Verwendung fuer FlyFrms ist klar. Fuer Tabellen ist die
    //Auswertung folgendermassen definiert:
    //NONE 				  == Die Randattribute zaehlen.
    //LEFT, CENTER, RIGHT == Wunschbreite wird falls moegliche eingehalten
    //						 und wenn noch Platz zur Verfuegung steht wird
    //						 ausgerichtet.
    //FULL				  == Die Tabelle nutzt die ganze verfuegbare Breite.
    //LEFT_AND_WIDTH	  == Linker Rand und Wunschbreite werden beruecksichtigt.
};



} //namespace binfilter
#endif

