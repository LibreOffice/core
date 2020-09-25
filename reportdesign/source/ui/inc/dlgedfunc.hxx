/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DLGEDFUNC_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DLGEDFUNC_HXX

#include <tools/gen.hxx>
#include <vcl/event.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/uno/XInterface.hpp>

class Timer;
class MouseEvent;
class Point;
class SdrTextObj;
class SdrObject;
namespace rptui
{

class OReportSection;
class OSectionView;


// DlgEdFunc


class DlgEdFunc /* : public LinkHdl */
{
    DlgEdFunc(const DlgEdFunc&) = delete;
    void operator =(const DlgEdFunc&) = delete;
protected:
    VclPtr<OReportSection> m_pParent;
    OSectionView&   m_rView;
    Timer           aScrollTimer;
    Point           m_aMDPos;
    css::uno::Reference<css::uno::XInterface> m_xOverlappingObj;
    SdrObject *     m_pOverlappingObj;
    Color           m_nOverlappedControlColor;
    Color           m_nOldColor;
    bool            m_bSelectionMode;
    bool            m_bUiActive;
    bool            m_bShowPropertyBrowser;

    DECL_LINK( ScrollTimeout, Timer *, void );
    void    ForceScroll( const Point& rPos );
    /** checks that no other object is overlapped.
    *
    * \param rMEvt
    */
    void    checkMovementAllowed(const MouseEvent& rMEvt);

    /** sets the correct mouse pointer when moving an object
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
    bool    isOnlyCustomShapeMarked() const;

    /** activate object if it is of type OBJ_OLE2
    */
    void    activateOle(SdrObject* _pObj);

    void checkTwoClicks(const MouseEvent& rMEvt);

public:
    DlgEdFunc( OReportSection* pParent );
    virtual ~DlgEdFunc();

    virtual bool MouseButtonDown( const MouseEvent& rMEvt );
    virtual bool MouseButtonUp( const MouseEvent& rMEvt );
    virtual bool MouseMove( const MouseEvent& rMEvt );

    /** checks if the keycode is known by the child windows
        @param  _rCode  the keycode
        @return <TRUE/> if the keycode is handled otherwise <FALSE/>
    */
    bool    handleKeyEvent(const KeyEvent& _rEvent);

    /** returns <TRUE/> if the mouse event is over an existing object
    *
    * \param rMEvt
    * \return <TRUE/> if overlapping, otherwise <FALSE/>
    */
    bool isOverlapping(const MouseEvent& rMEvt);
    void setOverlappedControlColor(Color _nColor);
    void stopScrollTimer();

    /** deactivate all ole object
    */
    void    deactivateOle(bool _bSelect = false);

    bool isUiActive() const { return m_bUiActive; }
protected:
    void colorizeOverlappedObject(SdrObject* _pOverlappedObj);
    void unColorizeOverlappedObj();


};


// DlgEdFuncInsert


class DlgEdFuncInsert : public DlgEdFunc
{
public:
    DlgEdFuncInsert( OReportSection* pParent );
    virtual ~DlgEdFuncInsert() override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
};


// DlgEdFuncSelect


class DlgEdFuncSelect : public DlgEdFunc
{
public:
    DlgEdFuncSelect( OReportSection* pParent );
    virtual ~DlgEdFuncSelect() override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
};

}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DLGEDFUNC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
