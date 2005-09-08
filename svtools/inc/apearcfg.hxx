/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apearcfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:08:54 $
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
#ifndef _SVT_APEARCFG_HXX
#define _SVT_APEARCFG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

class Application;

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
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
