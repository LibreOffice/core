
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
#*    Letzte Aenderung  $Author: vg $ $Date: 2007-04-11 21:37:58 $
#*    $Revision: 1.2 $
#*
#*    $Logfile:$
#*
#*    Copyright (c) 1989 - 1996, STAR DIVISION
#*
#*************************************************************************/

#ifndef _FMRWRK_DEPWIN_HXX
#define _FMRWRK_DEPWIN_HXX

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#include <vcl/menu.hxx>
#include <svtools/scrwin.hxx>
#include <soldep/connctr.hxx>

class Depper;

class DepWin : public Window
{
private:
    ConnectorList   ConList;

    ObjectWin*      mpNewConWin;
    BOOL            mbStartNewCon;
    Point           maNewConStart;
    Point           maNewConEnd;
    ObjectWin*      mpSelectedProject;

public:
    PopupMenu*      mpPopup;
//  Depper*         mpDepperDontuseme;

                    DepWin( Window* pParent, WinBits nWinStyle );
                    ~DepWin();
    void            AddConnector( Connector* pNewCon );
    void            RemoveConnector( Connector* piOldCon );
    void            NewConnector( ObjectWin* pWin );
    ConnectorList*  GetConnectorList();
    void            ClearConnectorList() { ConList.Clear();}
    void            Paint( const Rectangle& rRect );
    void            MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rEvent);
//  void            Resize();
    void            MouseMove( const MouseEvent& rMEvt );
    BOOL            IsStartNewCon() { return mbStartNewCon; };
    void            SetPopupHdl( void* pHdl );
    void            SetSelectedProject( ObjectWin* object ) { mpSelectedProject = object; };
    ObjectWin*      GetSelectedProject() { return mpSelectedProject; };
//  DECL_LINK( PopupSelected, PopupMenu* );
};

#endif
