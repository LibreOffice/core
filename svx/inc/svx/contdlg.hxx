/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:42:03 $
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

#ifndef _CONTDLG_HXX_
#define _CONTDLG_HXX_

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Float
|*
\************************************************************************/

class Graphic;

class SVX_DLLPUBLIC SvxContourDlgChildWindow : public SfxChildWindow
{
 public:

    SvxContourDlgChildWindow( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxContourDlgChildWindow );

    static void UpdateContourDlg( const Graphic& rGraphic, BOOL bGraphicLinked,
                                  const PolyPolygon* pPolyPoly = NULL,
                                  void* pEditingObj = NULL );
};

#ifndef _REDUCED_ContourDlg_HXX_
#define _REDUCED_CONTDLG_HXX_

class SvxSuperContourDlg;

/*************************************************************************
|*
|*
|*
\************************************************************************/

class SvxContourDlgItem : public SfxControllerItem
{
    SvxSuperContourDlg& rDlg;

protected:

    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

public:

                        SvxContourDlgItem( USHORT nId, SvxSuperContourDlg& rDlg, SfxBindings& rBindings );
};

/*************************************************************************
|*
|*
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxContourDlg : public SfxFloatingWindow
{
    using Window::Update;

    SvxSuperContourDlg* pSuperClass;

//#if 0 // _SOLAR__PRIVATE

protected:

    void                SetSuperClass( SvxSuperContourDlg& rSuperClass ) { pSuperClass = &rSuperClass; }

//#endif // __PRIVATE

public:

                        SvxContourDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                       Window* pParent, const ResId& rResId );
                        ~SvxContourDlg();

    void                SetExecState( BOOL bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( BOOL bLinked );
    const Graphic&      GetGraphic() const;
    BOOL                IsGraphicChanged() const;

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    PolyPolygon         GetPolyPolygon();

    void                SetEditingObject( void* pObj );
    const void*         GetEditingObject() const;

    void                Update( const Graphic& rGraphic, BOOL bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL, void* pEditingObj = NULL );

    static PolyPolygon  CreateAutoContour(  const Graphic& rGraphic,
                                            const Rectangle* pRect = NULL,
                                            const ULONG nFlags = 0L );
    static void         ScaleContour( PolyPolygon& rContour, const Graphic& rGraphic,
                                      const MapUnit eUnit, const Size& rDisplaySize );
};

/*************************************************************************
|*
|* Defines
|*
\************************************************************************/

#define SVXCONTOURDLG() ( (SvxContourDlg*) ( SfxViewFrame::Current()->GetChildWindow(   \
                          SvxContourDlgChildWindow::GetChildWindowId() )->  \
                          GetWindow() ) )

#endif // _REDUCED_CONTDLG_HXX_
#endif // _CONTDLG_HXX_

