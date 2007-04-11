/*************************************************************************
#*    $Workfile:$
#*
#*    class Klassenname(n)
#*
#*    Beschreibung      DOKUNAME.DOC oder
#*                      Beschreibung des Moduls
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: vg $ $Date: 2007-04-11 21:38:22 $
#*    $Revision: 1.2 $
#*
#*    $Logfile:$
#*
#*    Copyright (c) 1989 - 2000, STAR DIVISION
#*
#*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/soldep/inc/soldep/graphwin.hxx,v 1.2 2007-04-11 21:38:22 vg Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.2.1  2007/02/09 16:15:15  vg
      #72503# get rid of hedabu procedure: Moving headers to soldep/inc/soldep and correspondent necessary changes

      Revision 1.2  2006/04/24 14:11:25  obo
      refactored version

      Revision 1.2.30.2  2006/03/14 15:31:51  obo
      save current state

      Revision 1.2.30.1  2006/01/06 11:14:52  obo
      tool bar

      Revision 1.2  2001/09/20 16:06:16  nf
      Support of scrollmouse

      Revision 1.1  2000/03/22 14:00:01  hjs
      scrollbars and directories in project-view


*************************************************************************/
#ifndef _FMRWRK_GRAPHWIN_HXX
#define _FMRWRK_GRAPHWIN_HXX

#include <svtools/scrwin.hxx>

class GraphWin : public ScrollableWindow
{
private:
    void* mpClass;

protected:
    Window aBufferWindow;

public:
    GraphWin( Window * pParent, void * pClass );
            void CalcSize();
    virtual void EndScroll( long nDeltaX, long nDeltaY );
    virtual void Resize();
    virtual void Command( const CommandEvent& rEvent);
    Window* GetBufferWindow(){ return &aBufferWindow; };
};

#define PIXELS( nLeft, nTop, nWidth, nHeight )\
    LogicToPixel( Point( nLeft, nTop ) ), LogicToPixel( Size( nWidth, nHeight ) )

#endif //  _FMRWRK_GRAPHWIN_HXX

