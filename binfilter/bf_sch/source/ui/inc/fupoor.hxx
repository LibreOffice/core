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

#ifndef _SCH_FUPOOR_HXX
#define _SCH_FUPOOR_HXX
//------------------------------------------------------------------
//
// dieses Define dient nur zum Testen und darf auf keinen Fall aktiv bleiben
// Bei Umstellungen alle Files nach SIG_CHARTMODELDEFSCHDOC durchsuchen
// #define ChartModel SchChartDocument
//------------------------------------------------------------------

#include <vcl/dialog.hxx>

#include "objid.hxx"
namespace binfilter {

class SchView;
class SchViewShell;
class SchWindow;
class ChartModel;
class SfxRequest;

#define HITPIX	  2		// Hit-Toleranz in Pixel
#define DRGPIX	  2		// Drag MinMove in Pixel


/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class SchFuPoor
{
protected:
    SchView*		pView;
    SchViewShell*	pViewShell;
    SchWindow*		pWindow;
    ChartModel* pChDoc;

    USHORT			nSlotId;
    USHORT			nSlotValue;

    Dialog* 		pDialog;

    Timer			aDragTimer; 			// fuer Drag&Drop
    DECL_LINK(DragHdl, Timer*);
    BOOL			bIsInDragMode;
    Point			aMDPos; 				// Position von MouseButtonDown

    SdrObject   *pMarkedObj;
    SchObjectId *pMarkedObjId;
    UINT16      nMarkedID;

    BOOL			bFirstMouseMove;


public:

    SchFuPoor(SchViewShell* pViewSh, SchWindow* pWin, SchView* pView,
              ChartModel* pDoc, SfxRequest& rReq);
    virtual ~SchFuPoor();


    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet

    virtual void Activate();		// Function aktivieren
    virtual void Deactivate();		// Function deaktivieren



    USHORT GetSlotID() const { return( nSlotId ); }

};

} //namespace binfilter
#endif	// _SCH_FUPOOR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
