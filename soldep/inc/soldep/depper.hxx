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

#ifndef _FMRWRK_DEPPER_HXX
#define _FMRWRK_DEPPER_HXX

#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/resmgr.hxx>
#include <soldep/objwin.hxx>
#include <soldep/depwin.hxx>
#include <soldep/graphwin.hxx>
#include <soldep/tbox.hxx>
#include <soldep/soldlg.hrc>
#include <soldep/hashtbl.hxx>
#include <soldep/hashobj.hxx>
#include <soldep/soldlg.hxx>
#include <soldep/prj.hxx>
#include <soldep/sdtresid.hxx>

DECLARE_HASHTABLE_OWNER(SolIdMapper,ByteString,MyHashObject*)

// just for conversion - convert char* to String
class ProgressBar;
class FixedText;

#define SOLDEPL_NAME "StarDepend V1.0"

#define DEPPER_ID   4711

//User Events
#define VCLEVENT_USER_MOUSEBUTTON_DOWN      5000
#define VCLEVENT_USER_MOUSEBUTTON_DOWN_ALT  5001
#define VCLEVENT_USER_MOUSEBUTTON_DOWN_CTRL 5002
#define VCLEVENT_USER_MOUSEBUTTON_UP        5003
#define VCLEVENT_USER_MOUSEBUTTON_UP_ALT    5004
#define VCLEVENT_USER_MOUSEBUTTON_UP_CTRL   5005
#define VCLEVENT_USER_MOUSEBUTTON_UP_SHFT   5006
#define VCLEVENT_USER_MOUSEBUTTON_DOWN_DBLCLICK 5007
#define VCLEVENT_USER_TBOX_RESIZE           5008
#define VCLEVENT_USER_TBOX_RESIZE_APP       5009
#define VCLEVENT_USER_TBOX_FIND             5010
#define VCLEVENT_USER_TBOX_HIDE_INDEPENDEND 5011
#define VCLEVENT_USER_TBOX_SELECT_WORKSPACE 5012
#define VCLEVENT_USER_TBOX_BACK             5013

#define     OBJWIN_X_SPACING        50
#define     OBJWIN_Y_SPACING        12
#define     DEPPER_MAX_DEPTH        100
#define     DEPPER_MAX_WIDTH        12
#define     DEPPER_MAX_LEVEL_WIDTH      10 * DEPPER_MAX_WIDTH


#define DEP_OK                      0
#define DEP_OBJECT_NOT_FOUND        1
#define DEP_STARTID_NOT_FOUND       2
#define DEP_ENDID_NOT_FOUND         3
#define DEP_CONNECTOR_NOT_FOUND     4
#define DEP_ENDLES_RECURSION_FOUND  DEPPER_MAX_DEPTH + 1

#define DEPPOPUP_NEW                                1
#define DEPPOPUP_AUTOARRANGE                        2
#define DEPPOPUP_LOAD                               3
#define DEPPOPUP_SAVE                               4
#define DEPPOPUP_WRITE_SOURCE                       5
#define DEPPOPUP_READ_SOURCE                        6
#define DEPPOPUP_OPEN_SOURCE                        7
#define DEPPOPUP_ZOOMIN                             8
#define DEPPOPUP_ZOOMOUT                            9
#define DEPPOPUP_CLEAR                              10
#define DEPPOPUP_CLOSE                              11
#define DEPPOPUP_HELP                               12
#define DEPPOPUP_SHOW_TOOLBOX                       13
#define DEPPOPUP_TEST                               0xff

struct depper_head
{
    ULONG nID;
    ULONG nObjectCount;
    ULONG nCnctrCount;

};

enum DistType
{
    BOTTOMUP = 0,
    TOPDOWN = 1,
    BOTH = 2
};


class Depper : public SolDevDll, public Window
{
    BOOL    mbIsPrjView;
    ULONG   mnMinDynXOffs;
    Point   maDefPos;
    Size    maDefSize;

    long    nZoomed;

    ULONG   mnViewMask;

    ProgressBar*    pSubBar;
    ProgressBar*    pMainBar;
    FixedText*      pSubText;
    FixedText*      pMainText;
    SolAutoarrangeDlg maArrangeDlg;


protected:
    Prj*            mpPrj;
    SolIdMapper*    mpSolIdMapper;
    SolIdMapper*    mpPrjIdMapper;
    StarWriter*     mpStarWriter;
    StarWriter*     mpPrjStarWriter;        //for inside prj-view
    ULONG           mnSolWinCount;
    ULONG           mnSolLastId;
    ULONG           mnPrjWinCount;
    ULONG           mnPrjLastId;
    ObjectList*         mpObjectList;           //Extended ObjWinList class
    ObjectList*         mpObjectPrjList;        //for module internal dependencies
    DepWin*             mpBaseWin;
    DepWin*             mpBasePrjWin;           //for inside prj-view
    GraphWin*           mpGraphWin;
    GraphWin*           mpGraphPrjWin;          //for inside prj-view
    Window*             mpProcessWin;
    Window*             mpParentProcessWin;
    ObjectWin*          ObjIdToPtr( ObjectList* pObjLst, ULONG nId );
    SoldepToolBox       maToolBox;
    FloatingWindow      maTaskBarFrame; // For ToolBox

    ULONG       AddObjectToList(DepWin* pParentWin, ObjectList* pObjLst, ULONG &LastID, ULONG &WinCount, ByteString& rBodyText, BOOL bInteract=TRUE );
    ULONG       AddObjectToList( DepWin* pParentWin, ObjectList* pObjLst, ULONG &LastID, ULONG &WinCount, ByteString& rBodyText, Point& rPos, Size& rSize );
    ObjectWin*  RemoveObjectFromList( ObjectList* pObjLst, ULONG &WinCount, USHORT nId, BOOL bDelete );
    USHORT      AddConnectorToObjects( ObjectList* pObjLst, ULONG nStartId, ULONG nEndId );
    USHORT      RemoveConnectorFromObjects( ObjectList* pObjLst, ULONG nStartId, ULONG nEndId );
    USHORT      AddConnectorToObjects( ObjectWin* pStartWin, ObjectWin* pEndWin );
    USHORT      RemoveConnectorFromObjects( ObjectWin* pStartWin, ObjectWin* pEndWin );

public:
    Depper( Window* pBaseWindow );
    ~Depper();
    BOOL                TogglePrjViewStatus();
    void                SetPrjViewStatus(BOOL state) { mbIsPrjView = state; }
    BOOL                IsPrjView() { return mbIsPrjView; }
    GraphWin*           GetGraphWin() { return (!mbIsPrjView) ? mpGraphWin : mpGraphPrjWin; }   //scrollbar window
    DepWin*             GetDepWin() { return (!mbIsPrjView) ? mpBaseWin : mpBasePrjWin; }       //content of scrollbar window
    ProgressBar*        GetMainProgressBar() { return pMainBar; }
    void                SetMainProgressBar( USHORT i);
    void                UpdateMainProgressBar(USHORT i, USHORT nScaleVal, USHORT &nStep, BOOL bCountingDown = FALSE);
    void                UpdateSubProgrssBar(ULONG i);
    void                SetTitle( const String &rTitle ) { mpProcessWin->SetText( rTitle ); }
    virtual void        RemoveAllObjects( ObjectList* ObjLst );
    USHORT              Save( const ByteString& rFileName );
//  USHORT              Load( const ByteString& rFileName );
    virtual USHORT      WriteSource();
    virtual USHORT      ReadSource( BOOL bUpdater );
    virtual USHORT      OpenSource();
    void                SetDefPos( Point& rPos ) { maDefPos = rPos; };
    Point&              GetDefPos() { return maDefPos; };
    void                SetDefSize( Size& rSize ) { maDefSize = rSize; };
    Size&               GetDefSize() { return maDefSize; };
    virtual USHORT      AutoArrangeDlgStart();
    virtual USHORT      AutoArrangeDlgStop();
    virtual USHORT      Zoom( MapMode& rMapMode );
    virtual BOOL        ViewContent( ByteString& ) { return FALSE; };
    virtual USHORT      CloseWindow() { return 0; };
    virtual void        ShowHelp(){};
    ObjectList*         GetObjectList() { return (!mbIsPrjView) ? mpObjectList : mpObjectPrjList; }
    ULONG               HandleNewPrjDialog( ByteString &rBodyText );
    ULONG               HandleNewDirectoryDialog( ByteString &rBodyText );
    void                HideObjectsAndConnections( ObjectList* pObjLst );
    void                ShowObjectsAndConnections( ObjectList* pObjLst );
    //virtual void        MouseButtonDown( const MouseEvent& rMEvt );
//#ifdef DEBUG
    virtual void        test(){};
    DECL_LINK( PopupSelected, PopupMenu* );
//#endif

    //void SetViewMask( ULONG nMask );
    ULONG GetViewMask() { return mnViewMask; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
