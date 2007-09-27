/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mdiexp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:05:59 $
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
#ifndef _MDIEXP_HXX
#define _MDIEXP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TBLENUM_HXX
#include <tblenum.hxx>
#endif
class UniString;
class SwRect;
class Size;
class Dialog;
class ViewShell;
class SwDoc;
class SwDocShell;

extern void ScrollMDI(ViewShell* pVwSh, const SwRect &, USHORT nRangeX, USHORT nRangeY);
extern BOOL IsScrollMDI(ViewShell* pVwSh, const SwRect &);
extern void SizeNotify(ViewShell* pVwSh, const Size &);

//Update der Statusleiste, waehrend einer Action.
extern void PageNumNotify( ViewShell* pVwSh,
                            USHORT nPhyNum,
                            USHORT nVirtNum,
                           const UniString& rPg );

enum FlyMode { FLY_DRAG_START, FLY_DRAG, FLY_DRAG_END };
extern void FrameNotify( ViewShell* pVwSh, FlyMode eMode = FLY_DRAG );

void StartProgress     ( USHORT nMessId, long nStartVal, long nEndVal, SwDocShell *pDocSh = 0 );
void EndProgress       ( SwDocShell *pDocSh = 0 );
void SetProgressState  ( long nPosition, SwDocShell *pDocShell );
void SetProgressText   ( USHORT nMessId, SwDocShell *pDocShell );
void RescheduleProgress( SwDocShell *pDocShell );

void EnableCmdInterface(BOOL bEnable = TRUE);

Dialog* GetSearchDialog();

void RepaintPagePreview( ViewShell* pVwSh, const SwRect& rRect );

// ndgrf.cxx
// alle QuickDraw-Bitmaps des speziellen Docs loeschen
void DelAllGrfCacheEntries( SwDoc* pDoc );

// ChgMode fuer Tabellen aus der Konfiguration lesen
TblChgMode GetTblChgDefaultMode();

BOOL JumpToSwMark( ViewShell* pVwSh, const UniString& rMark );


#endif
