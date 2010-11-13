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
#ifndef _SVT_APEARCFG_HXX
#define _SVT_APEARCFG_HXX

#include "svtools/svtdllapi.h"
#include "tools/solar.h"
#include <unotools/configitem.hxx>

class Application;

typedef enum {
    LookStardivision = 0,
    LookMotif,
    LookWindows,
    LookOSTwo,
    LookMacintosh
} SystemLook;

typedef enum {
    SnapToButton = 0,
    SnapToMiddle,
    NoSnap
} SnapType;

typedef enum { // MUST match the order chosen in ListBox LB_DRAG_MODE in optgdlg.src
    DragFullWindow,
    DragFrame,
    DragSystemDep
} DragMode;


class SVT_DLLPUBLIC SvtTabAppearanceCfg : public utl::ConfigItem
{
    short           nLookNFeel          ;
    short           nDragMode           ;
    short           nScaleFactor        ;
    short           nSnapMode           ;
    short           nMiddleMouse;
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    short           nAAMinPixelHeight   ;
#endif

    BOOL            bMenuMouseFollow        ;
    BOOL            bSingleLineTabCtrl      ;
    BOOL            bColoredTabCtrl         ;
#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    BOOL            bFontAntialiasing       ;
#endif

    static sal_Bool  bInitialized ;

    SVT_DLLPRIVATE const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();

public:
    SvtTabAppearanceCfg( );
    ~SvtTabAppearanceCfg( );

    virtual void    Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);

    USHORT      GetLookNFeel () const { return nLookNFeel; }
    void        SetLookNFeel ( USHORT nSet );

    USHORT      GetDragMode  () const { return nDragMode; }
    void        SetDragMode  ( USHORT nSet );

    USHORT      GetScaleFactor () const { return nScaleFactor; }
    void        SetScaleFactor ( USHORT nSet );

    USHORT      GetSnapMode () const { return nSnapMode; }
    void        SetSnapMode ( USHORT nSet );

    USHORT      GetMiddleMouseButton () const { return nMiddleMouse; }
    void        SetMiddleMouseButton ( USHORT nSet );

    void        SetApplicationDefaults ( Application* pApp );

    void        SetMenuMouseFollow(BOOL bSet) {bMenuMouseFollow = bSet; SetModified();}
    BOOL        IsMenuMouseFollow() const{return bMenuMouseFollow;}

    void        SetSingleLineTabCtrl(BOOL bSet) {bSingleLineTabCtrl = bSet; SetModified();}
    BOOL        IsSingleLineTabCtrl()const {return   bSingleLineTabCtrl;}

#if defined( UNX ) || defined ( FS_PRIV_DEBUG )
    void        SetFontAntiAliasing( BOOL bSet )    { bFontAntialiasing = bSet; SetModified(); }
    BOOL        IsFontAntiAliasing() const { return bFontAntialiasing; }

    USHORT      GetFontAntialiasingMinPixelHeight( ) const { return nAAMinPixelHeight; }
    void        SetFontAntialiasingMinPixelHeight( USHORT _nMinHeight ) { nAAMinPixelHeight = _nMinHeight; SetModified(); }
#endif

    void        SetColoredTabCtrl(BOOL bSet)   {bColoredTabCtrl = bSet; SetModified();};
    BOOL        IsColoredTabCtrl()const {return     bColoredTabCtrl;}

    static sal_Bool IsInitialized()  { return bInitialized; }
    static void    SetInitialized() { bInitialized = sal_True; }
};

#endif //  _OFA_APEARCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
