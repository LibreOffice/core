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

#include <sal/types.h>

#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <svx/svxdllapi.h>
#include <tools/poly.hxx>

class SfxBindings;
class SfxModule;

/*************************************************************************
|*
|* Derivation from SfxChildWindow as "container" for Float
|*
\************************************************************************/

class Graphic;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxContourDlgChildWindow final : public SfxChildWindow
{
public:
    SvxContourDlgChildWindow( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo const * );

    SFX_DECL_CHILDWINDOW_WITHID( SvxContourDlgChildWindow );
};

class SvxSuperContourDlg;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxContourDlg final : public SfxModelessDialogController
{
    std::unique_ptr<SvxSuperContourDlg> m_xImpl;

public:

    SvxContourDlg(SfxBindings *pBindings, SfxChildWindow *pCW, weld::Window* pParent);
    virtual             ~SvxContourDlg() override;

    const Graphic&      GetGraphic() const;
    bool                IsGraphicChanged() const;

    tools::PolyPolygon  GetPolyPolygon();

    const void*         GetEditingObject() const;

    void                Update( const Graphic& rGraphic, bool bGraphicLinked,
                                const tools::PolyPolygon* pPolyPoly, void* pEditingObj );

    static tools::PolyPolygon  CreateAutoContour(  const Graphic& rGraphic,
                                            const tools::Rectangle* pRect = nullptr );
};

#endif // INCLUDED_SVX_CONTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
