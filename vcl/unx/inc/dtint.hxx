/*************************************************************************
 *
 *  $RCSfile: dtint.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SV_DTINT_HXX
#define _SV_DTINT_HXX

#include <cstdio>
#include <dlfcn.h>

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#include <tools/color.hxx>

class SalFrame;
class SalBitmap;
class String;
class SalDisplay;
class FastItemInfo;

#ifndef _XLIB_H_
// forwards from X
struct Display;
struct XEvent;
#define Atom UINT32
#define XLIB_Window UINT32
#endif

#define XDND_PROTOCOL_VERSION 3

// NETBSD has no RTLD_GLOBAL
#ifndef RTLD_GLOBAL
#define DLOPEN_MODE (RTLD_LAZY)
#else
#define DLOPEN_MODE (RTLD_GLOBAL | RTLD_LAZY)
#endif

class DtIntegrator;

DECLARE_LIST( DtIntegratorList, DtIntegrator* );
DECLARE_LIST( DtSalFrameList, SalFrame* );
DECLARE_LIST( StringList, String* );

struct SystemLookInfo
{
    /** system foreground color */
    Color                           foreground;
    /** system background color */
    Color                           background;
    /** system foreground color for a selection */
    Color                           selectForeground;
    /** system background color for a selection */
    Color                           selectBackground;

    /** gradient for an active window */
    Color                           windowActiveStart;
    Color                           windowActiveEnd;
    /** border color for active window */
    Color                           activeBorder;
    /** text color for active window bar */
    Color                           activeForeground;
    /** gradient of an inactive window */
    Color                           windowInactiveStart;
    Color                           windowInactiveEnd;
    /** border color for inactive window */
    Color                           inactiveBorder;
    /** text color for inactive window bar */
    Color                           inactiveForeground;

    /** font to use for controls. Empty if not set. */
    String                          controlFont;
    /** font to use for dragbars. Empty if not set. */
    String                          windowFont;

    SystemLookInfo()
        {
            foreground.SetColor( COL_TRANSPARENT );
            background.SetColor( COL_TRANSPARENT );
            selectBackground.SetColor( COL_TRANSPARENT );
            selectForeground.SetColor( COL_TRANSPARENT );

            windowActiveStart.SetColor( COL_TRANSPARENT );
            windowActiveEnd.SetColor( COL_TRANSPARENT );
            activeBorder.SetColor( COL_TRANSPARENT );
            activeForeground.SetColor( COL_TRANSPARENT );

            windowInactiveStart.SetColor( COL_TRANSPARENT );
            windowInactiveEnd.SetColor( COL_TRANSPARENT );
            inactiveBorder.SetColor( COL_TRANSPARENT );
            inactiveForeground.SetColor( COL_TRANSPARENT );
        }
};

enum DtType {
    DtGeneric,
    DtCDE,
    DtKDE,
    DtGNOME,
    DtSCO,
    DtIRIX
};

enum DtDataType {
    DtTypeUrl,
    DtTypeText,
    DtTypeKnown
};

struct DtData
{
    DtDataType          meType;
    int                 mnX, mnY; // used for Drop data
    void*               mpFrame;  // dito
    int                 mnBytes;
    unsigned char*      mpBytes;
    unsigned char*      mpType;

    DtData() : meType( DtTypeText ), mnX( -1 ), mnY( -1 ), mpFrame( 0 ),
        mnBytes( 0 ), mpBytes( 0 ), mpType( NULL ) {}
    ~DtData()
    {
        if( mpBytes )
            delete mpBytes;
    }
};

enum DtDropAction
{
    DtDropNone = 0,
    DtDropCopy = 1,
    DtDropMove = 2,
    DtDropLink = 4,
    DtDropAny  = 7
};

struct DtDropQuery
{
    int             m_nX;
    int             m_nY;
    DtDropAction    m_eAction;
    void*           m_pFrame; // really a SalFrame*
};

class DtIntegrator
{
protected:
    enum DtDragState { DtDragNone = 0, DtDragging, DtWaitForStatus,
                       DtWaitForDataRequest };

    DtType              meType;
    Display*            mpDisplay;
    SalDisplay*         mpSalDisplay;
    SalFrame*           mpSalFrame;
    int                 mnRefCount;

    XLIB_Window         maSelectionWindow;
    Atom                maExPropertyAtom;

    Atom                mnXdndAware;
    Atom                mnXdndSelection;
    Atom                mnXdndEnter;
    Atom                mnXdndLeave;
    Atom                mnXdndStatus;
    Atom                mnXdndTypeList;
    Atom                mnXdndPosition;
    Atom                mnXdndDrop;
    Atom                mnXdndActionCopy;
    Atom                mnXdndActionMove;
    Atom                mnXdndActionLink;
    Atom                mnXdndActionAsk;
    Atom                mnXdndActionPrivate;
    Atom                mnXdndActionList;
    Atom                mnXdndActionDescription;
    Atom                mnXdndFinished;

    int                 mnDropDataTime;
    int                 mnLastDropX;
    int                 mnLastDropY;
    XLIB_Window         maDropSource;
    SalFrame*           mpDropTarget;
    StringList          maDropTypes;

    Link                maDropFinishHdl;
    Link                maQueryDropHdl;
    Link                maBeginDropHdl;

    DtDragState         meDragState;
    int                 mnLastDragX;
    int                 mnLastDragY;
    int                 mnLastDragTimestamp;
    int                 mnWaitTimestamp;
    Link                maQueryDragDataHdl;
    XLIB_Window         maDragSource;
    XLIB_Window         maDragTarget;
    StringList          maDragTypes;

    DtIntegrator( SalFrame* );

    DtSalFrameList      maDropzones;

    Link                maClipboardChangedHdl;
    Atom                mnClipboardAtom;
    Atom                mnTargetsAtom;
    Atom                mnCompoundAtom;
    DtData*             mpLastData;

    virtual void ImplRegisterDropzone( SalFrame* );
    virtual void ImplUnregisterDropzone( SalFrame* );
    virtual void ImplHandleXEvent( XEvent* );

    BOOL LaunchProcess( const String&, const String& rDirectory = String() );

    static DtIntegratorList aIntegratorList;
    static String           aHomeDir;

    // helper functions
    XLIB_Window GetXdndAwareWindowBeneathPointer( int& rVersion, XEvent* );
    void SendXdndLeave();
    void SendXdndEnter();
    void SendXdndPosition( XEvent* );
    void CheckXdndTimeout( int );

public:
    static DtIntegrator* CreateDtIntegrator( SalFrame* );

    static void HandleXEvent( XEvent* );

    virtual ~DtIntegrator();

    virtual BOOL    StartProcess( String&, String&, const String& rDir = String() );

    // functions for Clipboard
    inline Link             SetClipboardChangedHdl( const Link& );
    const Link&             GetClipboardChangedHdl()
        { return maClipboardChangedHdl; }
    void                    Copy( DtData* );    // copy to system
    DtData*                 Paste();            // copy from system
    BOOL                    CheckUnxClipboardChanged();

    // functions for Dnd
    void                    RegisterDropzone( SalFrame* );
    void                    UnregisterDropzone( SalFrame* );
    DtDropAction            ExecuteDrag( const StringList &, SalFrame* pFrame );
    DtData* DropFinish( const String& rType );
    StringList& GetDropTypes() { return maDropTypes; }

    const Link& GetDropFinishHdl() { return maDropFinishHdl; }
    Link SetDropFinishHdl( const Link& rNewLink )
        {
            Link aRet = maDropFinishHdl;
            maDropFinishHdl = rNewLink;
            return aRet;
        }
    const Link& GetQueryDropHdl() { return maDropFinishHdl; }
    Link SetQueryDropHdl( const Link& rNewLink )
        {
            Link aRet = maQueryDropHdl;
            maQueryDropHdl = rNewLink;
            return aRet;
        }
    const Link& GetBeginDropHdl() { return maBeginDropHdl; }
    Link SetBeginDropHdl( const Link& rNewLink )
        {
            Link aRet = maBeginDropHdl;
            maBeginDropHdl = rNewLink;
            return aRet;
        }
    const Link& GetQueryDragDataHdl() { return maQueryDragDataHdl; }
    Link SetQueryDragDataHdl( const Link& rNewLink )
        {
            Link aRet = maQueryDragDataHdl;
            maQueryDragDataHdl = rNewLink;
            return aRet;
        }

    // SystemLook
    virtual BOOL GetSystemLook( SystemLookInfo& rInfo );

    DtType          GetDtType() { return meType; }
    SalFrame*       GetFrame() { return mpSalFrame; }
    SalDisplay*     GetSalDisplay() { return mpSalDisplay; }
    Display*        GetDisplay() { return mpDisplay; }

    void Acquire() { mnRefCount++; }
    inline void Release();
};

inline void DtIntegrator::Release()
{
    mnRefCount--;
    if( ! mnRefCount )
    {
        aIntegratorList.Remove( this );
        delete this;
    }
}

inline Link DtIntegrator::SetClipboardChangedHdl( const Link& rLink )
{
    Link aOldLink = maClipboardChangedHdl;
    maClipboardChangedHdl = rLink;
    return aOldLink;
}

// helper funktions for dynamic loading
extern BOOL bSymbolLoadFailed;

inline void* _LoadSymbol( void* pLibrary, char* pSymbolname )
{
    void *pRet = dlsym( pLibrary, pSymbolname );
    if( ! pRet )
    {
        fprintf( stderr, "Could not load symbol %s: %s\n",
                 pSymbolname, dlerror() );
        bSymbolLoadFailed = TRUE;
    }
    return pRet;
}
inline void* _LoadLibrary( char* pLibname )
{
    bSymbolLoadFailed = FALSE;
    void *pRet = dlopen( pLibname, DLOPEN_MODE );
    if( ! pRet )
    {
#ifdef DEBUG
        fprintf( stderr, "%s could not be opened: %s\n",
                 pLibname, dlerror() );
#endif
        bSymbolLoadFailed = TRUE;
    }
    return pRet;
}

#endif
