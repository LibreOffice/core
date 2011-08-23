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


#include "xdef.hxx"












#include "globl3d.hxx"

#include "polysc3d.hxx"








namespace binfilter {

#define ITEMVALUE(ItemSet,Id,Cast)	((const Cast&)(ItemSet).Get(Id)).GetValue()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(E3dPolyScene, E3dScene);

/*N*/ E3dPolyScene::E3dPolyScene()
/*N*/ :	E3dScene()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dPolyScene::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_POLYSCENE_ID;
/*N*/ }

/*************************************************************************
|*
|* Die Kontur fuer TextToContour
|*
\************************************************************************/


/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/


/*************************************************************************
|*
|* Paint
|*
\************************************************************************/

/*N*/ FASTBOOL E3dPolyScene::Paint(ExtOutputDevice& rOut,
/*N*/ 	const SdrPaintInfoRec& rInfoRec) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }


/*************************************************************************
|*
|* Geometrie zeichnen mit clipping Beruecksichtigung
|*
\************************************************************************/


/*************************************************************************
|*
|* Zeichenroutine fuer 3D
|*
\************************************************************************/


/*************************************************************************
|*
|* Schatten aller Objekte zeichnen
|*
\************************************************************************/


/*************************************************************************
|*
|* Verminderte Darstellungsqualitaet, zeichne als WireFrame OHNE renderer
|*
\************************************************************************/



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
