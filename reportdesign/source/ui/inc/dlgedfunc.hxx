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
#ifndef _REPORT_RPTUIFUNC_HXX
#define _REPORT_RPTUIFUNC_HXX

#include <vcl/timer.hxx>

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
    OSectionView&   m_rView;
    Timer           aScrollTimer;
    Point           m_aMDPos;
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface> m_xOverlappingObj;
    SdrObject *     m_pOverlappingObj;
    sal_Int32       m_nOverlappedControlColor;
    sal_Int32       m_nOldColor;
    bool            m_bSelectionMode;
    bool            m_bUiActive;
    bool            m_bShowPropertyBrowser;

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

    /** activate object if it is of type OBJ_OLE2
    */
    void    activateOle(SdrObject* _pObj);

    void checkTwoCklicks(const MouseEvent& rMEvt);

public:
    DlgEdFunc( OReportSection* pParent );
    virtual ~DlgEdFunc();

    virtual sal_Bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual sal_Bool MouseMove( const MouseEvent& rMEvt );

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

    /** deactivate all ole object
    */
    void    deactivateOle(bool _bSelect = false);

    inline bool isUiActive() const { return m_bUiActive; }
protected:
    void colorizeOverlappedObject(SdrObject* _pOverlappedObj);
    void unColorizeOverlappedObj();


};

//============================================================================
// DlgEdFuncInsert
//============================================================================

class DlgEdFuncInsert : public DlgEdFunc
{
public:
    DlgEdFuncInsert( OReportSection* pParent );
    ~DlgEdFuncInsert();

    virtual sal_Bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual sal_Bool MouseMove( const MouseEvent& rMEvt );
};

//============================================================================
// DlgEdFuncSelect
//============================================================================

class DlgEdFuncSelect : public DlgEdFunc
{
public:
    DlgEdFuncSelect( OReportSection* pParent );
    ~DlgEdFuncSelect();

    virtual sal_Bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual sal_Bool MouseMove( const MouseEvent& rMEvt );

    void SetInEditMode(SdrTextObj* _pTextObj,const MouseEvent& rMEvt, sal_Bool bQuickDrag);
};

}
#endif //_REPORT_RPTUIFUNC_HXX
