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


#ifndef _SVX_LIGHT3D_HXX_
#define _SVX_LIGHT3D_HXX_

#include <svx/dlgctl3d.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>

#include <vcl/field.hxx>
#include <svx/dlgctrl.hxx>

/*************************************************************************
|*
|* 3D TabDialog
|*
\************************************************************************/

class Svx3DTabDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:
            Svx3DTabDialog( Window* pParent, const SfxItemSet* pAttr );
            ~Svx3DTabDialog();
};

/*************************************************************************
|*
|* 3D SingleTabDialog
|*
\************************************************************************/
class Svx3DSingleTabDialog : public SfxSingleTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:
        Svx3DSingleTabDialog( Window* pParent, const SfxItemSet* pAttr );
        ~Svx3DSingleTabDialog();
};

#endif // _SVX_LIGHT3D_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
