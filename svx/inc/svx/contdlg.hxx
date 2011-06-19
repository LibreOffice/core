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

#ifndef _CONTDLG_HXX_
#define _CONTDLG_HXX_

#include <sfx2/basedlgs.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/childwin.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Derivation from SfxChildWindow as "container" for Float
|*
\************************************************************************/

class Graphic;

class SVX_DLLPUBLIC SvxContourDlgChildWindow : public SfxChildWindow
{
 public:

    SvxContourDlgChildWindow( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxContourDlgChildWindow );

    static void UpdateContourDlg( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                  const PolyPolygon* pPolyPoly = NULL,
                                  void* pEditingObj = NULL );
};

#ifndef _REDUCED_ContourDlg_HXX_
#define _REDUCED_CONTDLG_HXX_

class SvxSuperContourDlg;

class SvxContourDlgItem : public SfxControllerItem
{
    SvxSuperContourDlg& rDlg;

protected:

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

public:

                        SvxContourDlgItem( sal_uInt16 nId, SvxSuperContourDlg& rDlg, SfxBindings& rBindings );
};

class SVX_DLLPUBLIC SvxContourDlg : public SfxFloatingWindow
{
    using Window::Update;

    SvxSuperContourDlg* pSuperClass;


protected:

    void                SetSuperClass( SvxSuperContourDlg& rSuperClass ) { pSuperClass = &rSuperClass; }

public:

                        SvxContourDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                       Window* pParent, const ResId& rResId );
                        ~SvxContourDlg();

    void                SetExecState( sal_Bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( sal_Bool bLinked );
    const Graphic&      GetGraphic() const;
    sal_Bool                IsGraphicChanged() const;

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    PolyPolygon         GetPolyPolygon();

    void                SetEditingObject( void* pObj );
    const void*         GetEditingObject() const;

    void                Update( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL, void* pEditingObj = NULL );

    static PolyPolygon  CreateAutoContour(  const Graphic& rGraphic,
                                            const Rectangle* pRect = NULL,
                                            const sal_uIntPtr nFlags = 0L );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
