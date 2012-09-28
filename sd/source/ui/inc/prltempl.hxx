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


#ifndef SD_PRLTEMPL_HXX
#define SD_PRLTEMPL_HXX

#include "sdresid.hxx"
#include <sfx2/tabdlg.hxx>
#include <svx/tabarea.hxx>

#include "prlayout.hxx" // fuer enum PresentationObjects


class SfxObjectShell;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;

/*************************************************************************
|*
|* Vorlagen-Tab-Dialog
|*
\************************************************************************/
class SdPresLayoutTemplateDlg : public SfxTabDialog
{
private:
    const SfxObjectShell*   mpDocShell;

    XColorListRef         pColorTab;
    XGradientListRef      pGradientList;
    XHatchListRef         pHatchingList;
    XBitmapListRef        pBitmapList;
    XDashListRef          pDashList;
    XLineEndListRef       pLineEndList;

    sal_uInt16              nPageType;
    sal_uInt16              nDlgType;
    sal_uInt16              nPos;

    ChangeType          nColorTableState;
    ChangeType          nBitmapListState;
    ChangeType          nGradientListState;
    ChangeType          nHatchingListState;

    PresentationObjects ePO;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

    // fuers Maping mit dem neuen SvxNumBulletItem
    SfxItemSet aInputSet;
    SfxItemSet* pOutSet;
    const SfxItemSet* pOrgSet;

    sal_uInt16 GetOutlineLevel() const;

    using SfxTabDialog::GetOutputItemSet;

public:
    SdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool );
    ~SdPresLayoutTemplateDlg();

    const SfxItemSet* GetOutputItemSet() const;
};


#endif // SD_PRLTEMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
