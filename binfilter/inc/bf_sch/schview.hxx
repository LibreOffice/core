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

#ifndef _SCH_SCHVIEW_HXX
#define _SCH_SCHVIEW_HXX

#include <bf_svx/svdedxv.hxx>
#include <bf_svx/view3d.hxx>
#include <bf_svx/svdpage.hxx>

#include "chtmodel.hxx"
#include "docshell.hxx"

class Graphic;
namespace binfilter {

class SchViewShell;
class SchPage;
class SchWindow;
class ChartModel;
class SchDragServerRef;
class E3dScene;
} //namespace binfilter

namespace binfilter {//STRIP009
DECLARE_LIST(E3dLogicalObjList, E3dObject*)//STRIP008 DECLARE_LIST(E3dLogicalObjList, E3dObject*);

/*************************************************************************
|*
|* View
|*
\************************************************************************/

class SchView : public E3dView
{
 protected:
    ChartModel*   pDoc;
    SchChartDocShell*   pDocSh;
    SchViewShell*       pViewSh;
    BOOL                bDragActive;
    USHORT              nLogicalMarked;
    USHORT              nLogicalEntered;

    Timer aTimer;
    void             Construct();
//  	SchDragServerRef CreateDataObject(SchView* pView, const Point& rDragPos);
//  	SchDragServerRef CreateDataObject(ChartModel* pDocument);
    void                SetMarkHandles();

 public:

    //void NotifySelection();
    DECL_LINK(NotifySelection,void*);

    SchView(SchChartDocShell* pDocSh, OutputDevice* pOutDev, SchViewShell* pShell);
    virtual ~SchView();

    ChartModel& GetDoc() const { return *pDoc; }


    // clipboard / drag and drop methods using XTransferable

    void UpdateSelectionClipboard( BOOL bForceDeselect );


    BOOL IsActive() const { return bDragActive; }



    virtual void MarkListHasChanged();


    FASTBOOL	IsLogicalGroupMarked() { return (nLogicalMarked != 0); }
    FASTBOOL	IsLogicalGroupEntered() { return (nLogicalEntered != 0); }



    SchChartDocShell* GetDocShell() const { return pDocSh; }

    // drag and drop


    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

} //namespace binfilter
#endif		// _SCH_SCHVIEW_HXX
