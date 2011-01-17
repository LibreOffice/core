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
#ifndef _SD_MASTERLAYOUT_DIALOG_HXX
#define _SD_MASTERLAYOUT_DIALOG_HXX

#include <vcl/dialog.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>

#include "sdpage.hxx"

class SdDrawDocument;

namespace sd
{

class MasterLayoutDialog : public ModalDialog
{
private:
    SdDrawDocument* mpDoc;
    SdPage*         mpCurrentPage;

    FixedLine       maFLPlaceholders;
    CheckBox        maCBDate;
    CheckBox        maCBPageNumber;
    CheckBox        maCBHeader;
    CheckBox        maCBFooter;

    OKButton        maPBOK;
    CancelButton    maPBCancel;

    sal_Bool            mbOldHeader;
    sal_Bool            mbOldFooter;
    sal_Bool            mbOldDate;
    sal_Bool            mbOldPageNumber;

    void applyChanges();
    void remove( PresObjKind eKind );
    void create( PresObjKind eKind );

public:
    MasterLayoutDialog( Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~MasterLayoutDialog();

    virtual short Execute();
};

}

#endif // _SD_MASTERLAYOUT_DIALOG_HXX

