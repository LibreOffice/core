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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "axisid.hxx"
#include "glob.hxx"
#include <tools/debug.hxx>
namespace binfilter {
/*************************************************************************
|*
|* Objekt-Id ermitteln;
|* Liefert -1, wenn das Objekt keine Id hat
|*
\************************************************************************/


/*************************************************************************
|*
|* Objekt mit Id suchen;
|* liefert NULL, wenn kein Objekt gefunden wurde.
|*
\************************************************************************/


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchAxisId::SchAxisId () :
/*N*/ 	SdrObjUserData (SchInventor, SCH_AXIS_ID, 0),
/*N*/ 	nAxisId (0)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SchAxisId::SchAxisId (long nId) :
/*N*/ 	SdrObjUserData (SchInventor, SCH_AXIS_ID, 0),
/*N*/ 	nAxisId (nId)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Kopier-Konstruktor
|*
\************************************************************************/


/*************************************************************************
|*
|* Kopie erzeugen
|*
\************************************************************************/

/*N*/ SdrObjUserData* SchAxisId::Clone(SdrObject *pObj) const
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 return new SchAxisId (*this);
/*N*/ }

/*************************************************************************
|*
|* Daten in Stream schreiben
|*
\************************************************************************/

/*N*/ void SchAxisId::WriteData(SvStream& rOut)
/*N*/ {
/*N*/ 	SdrObjUserData::WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << nAxisId;
/*N*/ }


/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

/*N*/ void SchAxisId::ReadData(SvStream& rIn)
/*N*/ {
/*N*/ 	SdrObjUserData::ReadData(rIn);
/*N*/ 
/*N*/ 	rIn >> nAxisId;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
