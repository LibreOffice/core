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
#ifndef _SFX_STYLEDLG_HXX
#define _SFX_STYLEDLG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sfx2/tabdlg.hxx>

class SfxStyleSheetBase;

class SFX2_DLLPUBLIC SfxStyleDialog: public SfxTabDialog
{
private:
    SfxStyleSheetBase*          pStyle;
    DECL_DLLPRIVATE_LINK( CancelHdl, Button * );
protected:
    virtual const SfxItemSet*   GetRefreshedSet();

public:
#define ID_TABPAGE_MANAGESTYLES 1
    SfxStyleDialog( Window* pParent, const ResId& rResId, SfxStyleSheetBase&,
                    sal_Bool bFreeRes = sal_True, const String* pUserBtnTxt = 0 );

    ~SfxStyleDialog();

    SfxStyleSheetBase&          GetStyleSheet() { return *pStyle; }
    const SfxStyleSheetBase&    GetStyleSheet() const { return *pStyle; }

    virtual short               Ok();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
