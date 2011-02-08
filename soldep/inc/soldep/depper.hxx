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
    sal_uIntPtr nID;
    sal_uIntPtr nObjectCount;
    sal_uIntPtr nCnctrCount;

};

enum DistType
{
    BOTTOMUP = 0,
    TOPDOWN = 1,
    BOTH = 2
};


class Depper : public SolDevDll, public Window
{
    sal_Bool    mbIsPrjView;
    sal_uIntPtr mnMinDynXOffs;
    Point   maDefPos;
    Size    maDefSize;

    long    nZoomed;

    sal_uIntPtr mnViewMask;

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
    sal_uIntPtr         mnSolWinCount;
    sal_uIntPtr         mnSolLastId;
    sal_uIntPtr         mnPrjWinCount;
    sal_uIntPtr         mnPrjLastId;
    ObjectList*         mpObjectList;           //Extended ObjWinList class
    ObjectList*         mpObjectPrjList;        //for module internal dependencies
    DepWin*             mpBaseWin;
    DepWin*             mpBasePrjWin;           //for inside prj-view
    GraphWin*           mpGraphWin;
    GraphWin*           mpGraphPrjWin;          //for inside prj-view
    Window*             mpProcessWin;
    Window*             mpParentProcessWin;
    ObjectWin*          ObjIdToPtr( ObjectList* pObjLst, sal_uIntPtr nId );
    SoldepToolBox       maToolBox;
    FloatingWindow      maTaskBarFrame; // For ToolBox

    sal_uIntPtr       AddObjectToList(DepWin* pParentWin, ObjectList* pObjLst, sal_uIntPtr &LastID, sal_uIntPtr &WinCount, ByteString& rBodyText, sal_Bool bInteract=sal_True );
    sal_uIntPtr       AddObjectToList( DepWin* pParentWin, ObjectList* pObjLst, sal_uIntPtr &LastID, sal_uIntPtr &WinCount, ByteString& rBodyText, Point& rPos, Size& rSize );
    ObjectWin*  RemoveObjectFromList( ObjectList* pObjLst, sal_uIntPtr &WinCount, sal_uInt16 nId, sal_Bool bDelete );
    sal_uInt16      AddConnectorToObjects( ObjectList* pObjLst, sal_uIntPtr nStartId, sal_uIntPtr nEndId );
    sal_uInt16      RemoveConnectorFromObjects( ObjectList* pObjLst, sal_uIntPtr nStartId, sal_uIntPtr nEndId );
    sal_uInt16      AddConnectorToObjects( ObjectWin* pStartWin, ObjectWin* pEndWin );
    sal_uInt16      RemoveConnectorFromObjects( ObjectWin* pStartWin, ObjectWin* pEndWin );

public:
    Depper( Window* pBaseWindow );
    ~Depper();
    sal_Bool                TogglePrjViewStatus();
    void                SetPrjViewStatus(sal_Bool state) { mbIsPrjView = state; }
    sal_Bool                IsPrjView() { return mbIsPrjView; }
    GraphWin*           GetGraphWin() { return (!mbIsPrjView) ? mpGraphWin : mpGraphPrjWin; }   //scrollbar window
    DepWin*             GetDepWin() { return (!mbIsPrjView) ? mpBaseWin : mpBasePrjWin; }       //content of scrollbar window
    ProgressBar*        GetMainProgressBar() { return pMainBar; }
    void                SetMainProgressBar( sal_uInt16 i);
    void                UpdateMainProgressBar(sal_uInt16 i, sal_uInt16 nScaleVal, sal_uInt16 &nStep, sal_Bool bCountingDown = sal_False);
    void                UpdateSubProgrssBar(sal_uIntPtr i);
    void                SetTitle( const String &rTitle ) { mpProcessWin->SetText( rTitle ); }
    virtual void        RemoveAllObjects( ObjectList* ObjLst );
    sal_uInt16              Save( const ByteString& rFileName );
//  sal_uInt16              Load( const ByteString& rFileName );
    virtual sal_uInt16      WriteSource();
    virtual sal_uInt16      ReadSource( sal_Bool bUpdater );
    virtual sal_uInt16      OpenSource();
    void                SetDefPos( Point& rPos ) { maDefPos = rPos; };
    Point&              GetDefPos() { return maDefPos; };
    void                SetDefSize( Size& rSize ) { maDefSize = rSize; };
    Size&               GetDefSize() { return maDefSize; };
    virtual sal_uInt16      AutoArrangeDlgStart();
    virtual sal_uInt16      AutoArrangeDlgStop();
    virtual sal_uInt16      Zoom( MapMode& rMapMode );
    virtual sal_Bool        ViewContent( ByteString& ) { return sal_False; };
    virtual sal_uInt16      CloseWindow() { return 0; };
    virtual void        ShowHelp(){};
    ObjectList*         GetObjectList() { return (!mbIsPrjView) ? mpObjectList : mpObjectPrjList; }
    sal_uIntPtr               HandleNewPrjDialog( ByteString &rBodyText );
    sal_uIntPtr               HandleNewDirectoryDialog( ByteString &rBodyText );
    void                HideObjectsAndConnections( ObjectList* pObjLst );
    void                ShowObjectsAndConnections( ObjectList* pObjLst );
    //virtual void        MouseButtonDown( const MouseEvent& rMEvt );
//#ifdef DEBUG
    virtual void        test(){};
    DECL_LINK( PopupSelected, PopupMenu* );
//#endif

    //void SetViewMask( sal_uIntPtr nMask );
    sal_uIntPtr GetViewMask() { return mnViewMask; }
};

#endif
