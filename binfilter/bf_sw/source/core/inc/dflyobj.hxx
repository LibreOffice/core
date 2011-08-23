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
#ifndef _DFLYOBJ_HXX
#define	_DFLYOBJ_HXX

#ifndef _SVDOVIRT_HXX //autogen
#include <bf_svx/svdovirt.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
namespace binfilter {

class SwFlyFrm;
class SwFrmFmt;
class SdrObjMacroHitRec;

const UINT32 SWGInventor =	UINT32('S')*0x00000001+
                            UINT32('W')*0x00000100+
                            UINT32('G')*0x00010000;

const UINT16 SwFlyDrawObjIdentifier = 0x0001;
const UINT16 SwDrawFirst			= 0x0001;

//---------------------------------------
//SwFlyDrawObj, Die DrawObjekte fuer Flys.

class SwFlyDrawObj : public SdrObject
{
    SfxItemSet*					mpLocalItemSet;

public:
    TYPEINFO();

    SwFlyDrawObj();
    ~SwFlyDrawObj();


    // ItemSet access

    //Damit eine Instanz dieser Klasse beim laden erzeugt werden kann
    //(per Factory).
    virtual UINT32 GetObjInventor()		const;
    virtual UINT16 GetObjIdentifier()	const;
};

//---------------------------------------
//SwVirtFlyDrawObj, die virtuellen Objekte fuer Flys.
//Flys werden immer mit virtuellen Objekten angezeigt. Nur so koennen sie
//ggf. mehrfach angezeigt werden (Kopf-/Fusszeilen).

class SwVirtFlyDrawObj : public SdrVirtObj
{
    SwFlyFrm *pFlyFrm;

public:
    TYPEINFO();

    SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly);
    ~SwVirtFlyDrawObj();

    //Ueberladene Methoden der Basisklasse SdrVirtObj

    //Wir nehemen die Groessenbehandlung vollstaendig selbst in die Hand.
    virtual const Rectangle& GetBoundRect() const;
    virtual const Rectangle& GetSnapRect()	const;

          SwFrmFmt *GetFmt();

    // Get Methoden fuer die Fly Verpointerung
          SwFlyFrm* GetFlyFrm()			{ return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const	{ return pFlyFrm; }

    void SetRect() const;
    void _SetRectsDirty()				{ SetRectsDirty(); }

    // ist eine URL an einer Grafik gesetzt, dann ist das ein Makro-Object
};


} //namespace binfilter
#endif
