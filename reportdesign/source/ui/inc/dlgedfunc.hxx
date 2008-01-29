#ifndef _REPORT_RPTUIFUNC_HXX
#define _REPORT_RPTUIFUNC_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedfunc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:49:17 $
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

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

class Timer;
class MouseEvent;
class Point;
class SdrTextObj;
class SdrObject;
namespace rptui
{

class OReportSection;
class OSectionView;

//============================================================================
// DlgEdFunc
//============================================================================

class DlgEdFunc /* : public LinkHdl */
{
    DlgEdFunc(const DlgEdFunc&);
    void operator =(const DlgEdFunc&);
protected:
    OReportSection* m_pParent;
    OSectionView*   m_pView;
    Timer           aScrollTimer;
    Point           m_aMDPos;

    DECL_LINK( ScrollTimeout, Timer * );
    void    ForceScroll( const Point& rPos );
    /** checks that no other object is overlapped.
    *
    * \param rMEvt
    */
    void    checkMovementAllowed(const MouseEvent& rMEvt);

    /** sets the correct mouse pointer when moving a object
    *
    * \param rMEvt
    * \return <TRUE/> when the pointer was already set.
    */
    bool    setMovementPointer(const MouseEvent& rMEvt);

    bool    isRectangleHit(const MouseEvent& rMEvt);
    /**
        returns true, as long as only customshapes in the marked list,
        custom shapes can drop every where
    */
    bool    isOnlyCustomShapeMarked();
public:
    DlgEdFunc( OReportSection* pParent );
    virtual ~DlgEdFunc();

    virtual BOOL MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL MouseButtonUp( const MouseEvent& rMEvt );
    virtual BOOL MouseMove( const MouseEvent& rMEvt );

    /** checks if the keycode is known by the child windows
        @param  _rCode  the keycode
        @return <TRUE/> if the keycode is handled otherwise <FALSE/>
    */
    virtual sal_Bool    handleKeyEvent(const KeyEvent& _rEvent);

    /** returns <TRUE/> if the mouse event is over an existing object
    *
    * \param rMEvt
    * \return <TRUE/> if overlapping, otherwise <FALSE/>
    */
    bool isOverlapping(const MouseEvent& rMEvt);
    void setOverlappedControlColor(sal_Int32 _nColor);
    void stopScrollTimer();
protected:
    void colorizeOverlappedObject(SdrObject* _pOverlappedObj);
    void unColorizeOverlappedObj();

    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> m_xOverlappingObj;
    SdrObject *m_pOverlappingObj;
    sal_Int32 m_nOverlappedControlColor;
    sal_Int32 m_nOldColor;

    bool m_bSelectionMode;
};

//============================================================================
// DlgEdFuncInsert
//============================================================================

class DlgEdFuncInsert : public DlgEdFunc
{
public:
    DlgEdFuncInsert( OReportSection* pParent );
    ~DlgEdFuncInsert();

    virtual BOOL MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL MouseButtonUp( const MouseEvent& rMEvt );
    virtual BOOL MouseMove( const MouseEvent& rMEvt );
};

//============================================================================
// DlgEdFuncSelect
//============================================================================

class DlgEdFuncSelect : public DlgEdFunc
{
public:
    DlgEdFuncSelect( OReportSection* pParent );
    ~DlgEdFuncSelect();

    virtual BOOL MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL MouseButtonUp( const MouseEvent& rMEvt );
    virtual BOOL MouseMove( const MouseEvent& rMEvt );

    void SetInEditMode(SdrTextObj* _pTextObj,const MouseEvent& rMEvt, BOOL bQuickDrag);
};

}
#endif //_REPORT_RPTUIFUNC_HXX
