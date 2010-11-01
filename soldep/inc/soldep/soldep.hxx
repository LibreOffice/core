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

#ifndef _FMRWRK_SOLDEP_HXX
#define _FMRWRK_SOLDEP_HXX

#define SOURCEROOT "SOLARSRC"

#include <tools/string.hxx>

#include <soldep/depper.hxx>

#define SOURCEROOT "SOLARSRC"

class ObjectWin;

class SolDep : public Depper
{
    BOOL            mbBServer;          //call from build server
    ULONG           mnMinDynXOffs;
    ULONG           mnLevelOffset;
    ULONG           mnXOffset;
    ObjWinList*     mpTravellerList;

    String      msSourceName;
    ByteString  msVersionMajor;
    ByteString  msVersionMinor;
    ByteString  msProject;
    ObjectWin*  mpFocusWin;
    BOOL        mbIsHide;

    GenericInformationList *mpStandLst;

    DECL_LINK( ChildWindowEventListener, VclSimpleEvent* );
    DECL_LINK( ToolSelect, SoldepToolBox* );

    SolarFileList* GetPrjListFromDir();

    Point       CalcPos( USHORT nSet, USHORT nIndex );
    ULONG       CalcXOffset( ULONG nObjectsToFit );
    double      CalcDistSum( ObjWinList* pObjList, DistType eDistType = TOPDOWN );
    USHORT      Impl_Traveller( ObjectWin* pWin, USHORT nDepth );
    double      Impl_PermuteMin( ObjWinList& rObjList, Point* pPosArray, ObjWinList& rResultList,
                    double dMinDist, ULONG nStart, ULONG nSize, DistType eDisType = TOPDOWN );
    USHORT      Load( const ByteString& rFileName );
    void        WriteToErrorFile();
    BOOL        MarkObjects( ObjectWin* pObjectWin );
    void        InitContextMenueMainWnd();
    void        InitContextMenuePrjViewWnd(DepWin* pWin);
protected:
    /// will be called for any VclWindowEvent events broadcasted by our VCL window
    virtual void ProcessChildWindowEvent( const VclWindowEvent& _rVclWindowEvent );
     inline bool isAlive() const        { return NULL != mpProcessWin; }


public:
    SolDep( Window* pBaseWindow );
    ~SolDep();
    BOOL                IsHideMode() { return mbIsHide;};
    void                ToggleHideDependency();

    virtual ULONG       GetStart(SolIdMapper* pIdMapper, ObjectList* pObjList);
            ULONG       GetStartPrj(SolIdMapper* pIdMapper, ObjectList* pObjList);
    virtual USHORT      ReadSource( BOOL bUpdater = FALSE );
    virtual USHORT      WriteSource();
    virtual USHORT      OpenSource();
    BOOL                GetVersion();
    void                Init();
    void                Init( ByteString &rVersion, GenericInformationList *pVersionList = NULL );
    BOOL                InitPrj( ByteString& rListName );
//        using Depper::AddObject;
    virtual ULONG       AddObject( ByteString& rBodyText, BOOL Interact=TRUE );
            ULONG       AddPrjObject( ByteString& rBodyText, BOOL Interact=TRUE );
    virtual ObjectWin*  RemoveObject( USHORT nId, BOOL bDelete = TRUE );
    virtual void        RemoveAllObjects( ObjectList* pObjLst );
    virtual USHORT      AddConnector( ObjectWin* pStartWin, ObjectWin* pEndWin );
            USHORT      AddConnectorPrjView( ObjectWin* pStartWin, ObjectWin* pEndWin );
    virtual USHORT      RemoveConnector( ObjectWin* pStartWin, ObjectWin* pEndWin );
            USHORT      RemoveConnectorPrjView( ObjectWin* pStartWin, ObjectWin* pEndWin );
            USHORT      AutoArrange( SolIdMapper* pIdMapper, ObjectList* pObjLst, ULONG nTopId, ULONG nBottmId, ULONG aObjID );
            USHORT      OptimizePos( SolIdMapper* pIdMapper, ObjectList* pObjLst, ULONG nTopId, ULONG nBottmId, ULONG aObjID );
    virtual BOOL        ViewContent( ByteString& rObjectName );
    virtual USHORT      CloseWindow();
    virtual void        ShowHelp();
    FloatingWindow*     GetTaskBarFrame() { return &maTaskBarFrame; }
    SoldepToolBox*      GetSoldepToolBox() { return &maToolBox; }

            BOOL        FindProject();
            void        Resize();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
