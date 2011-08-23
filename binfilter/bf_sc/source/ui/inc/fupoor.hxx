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

#ifndef _SC_FUPOOR_HXX
#define _SC_FUPOOR_HXX

#include <vcl/event.hxx>
#include <vcl/timer.hxx>
#include <bf_sfx2/request.hxx>
class Window;
class Dialog;
namespace binfilter {

class SdrView;
class ScTabViewShell;
class SdrModel;

// #98185# Create default drawing objects via keyboard
class SdrObject;

//	Return-Werte fuer Command
#define SC_CMD_NONE		0
#define SC_CMD_USED		1
#define SC_CMD_IGNORE	2

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuPoor
{
 protected:
    SdrView* 		pView;
    ScTabViewShell*	pViewShell;
    Window*			pWindow;
    SdrModel* 		pDrDoc;

    SfxRequest		aSfxRequest;
    Dialog* 		pDialog;

    Timer			aScrollTimer;			// fuer Autoscrolling
    DECL_LINK( ScrollHdl, Timer * );

    Timer			aDragTimer; 			// fuer Drag&Drop
    DECL_LINK( DragTimerHdl, Timer * );
    BOOL			bIsInDragMode;
    Point			aMDPos; 				// Position von MouseButtonDown

    // #95491# member to hold state of the mouse buttons for creation
    // of own MouseEvents (like in ScrollHdl)
private:
    sal_uInt16		mnCode;

 public:
    FuPoor(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuPoor();

    USHORT GetSlotID() const { DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 USHORT GetSlotID() const { return( aSfxRequest.GetSlot() );}
    void	StopDragTimer();
};



} //namespace binfilter
#endif		// _SD_FUPOOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
