
/*************************************************************************
#*
#*    $Workfile:$
#*
#*    class Klassenname(n)
#*
#*    Implementation    SOURCE.CXX (nur bei HXX-Files)
#*
#*    Beschreibung      DOKUNAME.DOC oder
#*                      Beschreibung des Moduls
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: vg $ $Date: 2007-04-11 21:37:40 $
#*    $Revision: 1.2 $
#*
#*    $Logfile:$
#*
#*    Copyright (c) 1989 - 1996, STAR DIVISION
#*
#*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/soldep/inc/soldep/connctr.hxx,v 1.2 2007-04-11 21:37:40 vg Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.2.1  2007/02/09 16:15:15  vg
      #72503# get rid of hedabu procedure: Moving headers to soldep/inc/soldep and correspondent necessary changes

      Revision 1.2  2006/04/24 14:11:25  obo
      refactored version

      Revision 1.5.30.3  2006/03/14 15:31:51  obo
      save current state

      Revision 1.5.30.2  2006/01/26 15:11:57  obo
      fix repaint problem in hide mode

      Revision 1.5.30.1  2006/01/06 11:11:51  obo
      hide mode

      Revision 1.5  2001/09/14 14:59:21  nf
      Support of layers

      Revision 1.3  2000/11/02 10:31:36  hjs
      unicode first shot

      Revision 1.2  1999/06/11 17:20:42  hjs
      now it's alpha


*************************************************************************/


#ifndef _FMRWRK_CONNCTR_HXX
#define _FMRWRK_CONNCTR_HXX


#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#include <bootstrp/listmacr.hxx>
#include <tools/stream.hxx>

class DepWin;
class ObjectWin;

static Rectangle aEmptyRect( Point( 0, 0), Point( 0, 0));

class Connector
{
    ObjectWin *mpStartWin, *mpEndWin;
    ULONG mnStartId, mnEndId;
    Point mStart, mEnd, mCenter;
    DepWin* mpParent;

    BOOL bVisible;
    BOOL mbHideIndependend;
    static BOOL msbHideMode;


public:
    double len;
    Connector( DepWin* pParent, WinBits nWinStyle );
    ~Connector();

    void    Initialize( ObjectWin* pStartWin, ObjectWin* pEndWin, BOOL bVis = FALSE );
    Point   GetMiddle();
    void    Paint( const Rectangle& rRect );
    void    UpdatePosition( ObjectWin* pWin, BOOL bPaint = TRUE );
    USHORT  Save( SvFileStream& rOutFile );
    USHORT  Load( SvFileStream& rInFile );
    ULONG   GetStartId(){ return mnStartId; };
    ULONG   GetEndId(){ return mnEndId; };
    ObjectWin*  GetStartWin(){ return mpStartWin; };
    ObjectWin*  GetEndWin(){ return mpEndWin; };
    ObjectWin*  GetOtherWin( ObjectWin* pWin );
    ULONG   GetOtherId( ULONG nId );
    ULONG   GetLen();
    BOOL    IsStart( ObjectWin* pWin );
    void    SetHideIndependend( BOOL bHide) { mbHideIndependend = bHide; };

    BOOL    IsVisible() { return bVisible; }
    void    SetVisibility( BOOL visible ) { bVisible = visible; }
    void    UpdateVisibility();
    void    SetHideMode(BOOL bHide) { msbHideMode = bHide; };
};

DECLARE_LIST( ConnectorList, Connector* )

#endif
