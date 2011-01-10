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

#ifndef _FMRWRK_SOLDEP_HXX
#define _FMRWRK_SOLDEP_HXX

#define SOURCEROOT "SOLARSRC"

#include <tools/string.hxx>

#include <soldep/depper.hxx>

#define SOURCEROOT "SOLARSRC"

class ObjectWin;

class SolDep : public Depper
{
    sal_Bool            mbBServer;          //call from build server
    sal_uIntPtr         mnMinDynXOffs;
    sal_uIntPtr         mnLevelOffset;
    sal_uIntPtr         mnXOffset;
    ObjWinList*     mpTravellerList;

    String      msSourceName;
    ByteString  msVersionMajor;
    ByteString  msVersionMinor;
    ByteString  msProject;
    ObjectWin*  mpFocusWin;
    sal_Bool        mbIsHide;

    GenericInformationList *mpStandLst;

    DECL_LINK( ChildWindowEventListener, VclSimpleEvent* );
    DECL_LINK( ToolSelect, SoldepToolBox* );

    SolarFileList* GetPrjListFromDir();

    Point       CalcPos( sal_uInt16 nSet, sal_uInt16 nIndex );
    sal_uIntPtr       CalcXOffset( sal_uIntPtr nObjectsToFit );
    double      CalcDistSum( ObjWinList* pObjList, DistType eDistType = TOPDOWN );
    sal_uInt16      Impl_Traveller( ObjectWin* pWin, sal_uInt16 nDepth );
    double      Impl_PermuteMin( ObjWinList& rObjList, Point* pPosArray, ObjWinList& rResultList,
                    double dMinDist, sal_uIntPtr nStart, sal_uIntPtr nSize, DistType eDisType = TOPDOWN );
    sal_uInt16      Load( const ByteString& rFileName );
    void        WriteToErrorFile();
    sal_Bool        MarkObjects( ObjectWin* pObjectWin );
    void        InitContextMenueMainWnd();
    void        InitContextMenuePrjViewWnd(DepWin* pWin);
protected:
    /// will be called for any VclWindowEvent events broadcasted by our VCL window
    virtual void ProcessChildWindowEvent( const VclWindowEvent& _rVclWindowEvent );
     inline bool isAlive() const        { return NULL != mpProcessWin; }


public:
    SolDep( Window* pBaseWindow );
    ~SolDep();
    sal_Bool                IsHideMode() { return mbIsHide;};
    void                ToggleHideDependency();

    virtual sal_uIntPtr     GetStart(SolIdMapper* pIdMapper, ObjectList* pObjList);
            sal_uIntPtr       GetStartPrj(SolIdMapper* pIdMapper, ObjectList* pObjList);
    virtual sal_uInt16      ReadSource( sal_Bool bUpdater = sal_False );
    virtual sal_uInt16      WriteSource();
    virtual sal_uInt16      OpenSource();
    sal_Bool                GetVersion();
    void                Init();
    void                Init( ByteString &rVersion, GenericInformationList *pVersionList = NULL );
    sal_Bool                InitPrj( ByteString& rListName );
//        using Depper::AddObject;
    virtual sal_uIntPtr     AddObject( ByteString& rBodyText, sal_Bool Interact=sal_True );
            sal_uIntPtr     AddPrjObject( ByteString& rBodyText, sal_Bool Interact=sal_True );
    virtual ObjectWin*  RemoveObject( sal_uInt16 nId, sal_Bool bDelete = sal_True );
    virtual void        RemoveAllObjects( ObjectList* pObjLst );
    virtual sal_uInt16      AddConnector( ObjectWin* pStartWin, ObjectWin* pEndWin );
            sal_uInt16      AddConnectorPrjView( ObjectWin* pStartWin, ObjectWin* pEndWin );
    virtual sal_uInt16      RemoveConnector( ObjectWin* pStartWin, ObjectWin* pEndWin );
            sal_uInt16      RemoveConnectorPrjView( ObjectWin* pStartWin, ObjectWin* pEndWin );
            sal_uInt16      AutoArrange( SolIdMapper* pIdMapper, ObjectList* pObjLst, sal_uIntPtr nTopId, sal_uIntPtr nBottmId, sal_uIntPtr aObjID );
            sal_uInt16      OptimizePos( SolIdMapper* pIdMapper, ObjectList* pObjLst, sal_uIntPtr nTopId, sal_uIntPtr nBottmId, sal_uIntPtr aObjID );
    virtual sal_Bool        ViewContent( ByteString& rObjectName );
    virtual sal_uInt16      CloseWindow();
    virtual void        ShowHelp();
    FloatingWindow*     GetTaskBarFrame() { return &maTaskBarFrame; }
    SoldepToolBox*      GetSoldepToolBox() { return &maToolBox; }

            sal_Bool        FindProject();
            void        Resize();
};

#endif
