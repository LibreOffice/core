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

#ifndef _SD_INSPAGOB_HXX
#define _SD_INSPAGOB_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include "sdtreelb.hxx"

class SdDrawDocument;

//------------------------------------------------------------------------

class SdInsertPagesObjsDlg : public ModalDialog
{
private:
    SdPageObjsTLB           aLbTree;
    CheckBox                aCbxLink;
    CheckBox                aCbxMasters;
    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;

    SfxMedium*              pMedium;
    const SdDrawDocument*   mpDoc;
    const String&           rName;

    void                    Reset();
    DECL_LINK( SelectObjectHdl, void * );

public:
                SdInsertPagesObjsDlg( Window* pParent,
                                const SdDrawDocument* pDoc,
                                SfxMedium* pSfxMedium,
                                const String& rFileName );
                ~SdInsertPagesObjsDlg();

    List*       GetList( sal_uInt16 nType );
    sal_Bool        IsLink();
    sal_Bool        IsRemoveUnnessesaryMasterPages() const;
};


#endif // _SD_INSPAGOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
