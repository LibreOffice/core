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

#ifndef INCLUDED_SVX_CONTDLG_HXX
#define INCLUDED_SVX_CONTDLG_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/childwin.hxx>
#include <svx/svxdllapi.h>

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

    SFX_DECL_CHILDWINDOW_WITHID( SvxContourDlgChildWindow );
};

#ifndef _REDUCED_ContourDlg_HXX_
#define _REDUCED INCLUDED_SVX_CONTDLG_HXX

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

    const Graphic&      GetGraphic() const;
    bool                IsGraphicChanged() const;

    PolyPolygon         GetPolyPolygon();

    const void*         GetEditingObject() const;

    void                Update( const Graphic& rGraphic, bool bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL, void* pEditingObj = NULL );

    static PolyPolygon  CreateAutoContour(  const Graphic& rGraphic,
                                            const Rectangle* pRect = NULL,
                                            const sal_uIntPtr nFlags = 0L );
};

#endif // _REDUCED INCLUDED_SVX_CONTDLG_HXX
#endif // INCLUDED_SVX_CONTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
