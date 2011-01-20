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

#ifndef SD_COPY_DLG_HXX
#define SD_COPY_DLG_HXX

#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svx/dlgctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

class XColorTable;

namespace sd {

class View;

/*************************************************************************
|*
|* Dialog zum Einstellen des Bildschirms
|*
\************************************************************************/
class CopyDlg
    : public SfxModalDialog
{
public:
    CopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
        XColorTable* pColTab, ::sd::View* pView );
    ~CopyDlg();

    void    GetAttr( SfxItemSet& rOutAttrs );
    DECL_LINK( Reset, void* );

private:
    FixedText           maFtCopies;
    NumericField        maNumFldCopies;
    ImageButton         maBtnSetViewData;

    FixedLine           maGrpMovement;
    FixedText           maFtMoveX;
    MetricField         maMtrFldMoveX;
    FixedText           maFtMoveY;
    MetricField         maMtrFldMoveY;
    FixedText           maFtAngle;
    MetricField         maMtrFldAngle;
    FixedLine           maGrpEnlargement;

    FixedText           maFtWidth;
    MetricField         maMtrFldWidth;
    FixedText           maFtHeight;
    MetricField         maMtrFldHeight;

    FixedLine           maGrpColor;
    FixedText           maFtStartColor;
    ColorLB             maLbStartColor;
    FixedText           maFtEndColor;
    ColorLB             maLbEndColor;

    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    PushButton          maBtnSetDefault;

    const SfxItemSet&   mrOutAttrs;
    XColorTable*        mpColorTab;
    Fraction            maUIScale;
    ::sd::View*         mpView;

    DECL_LINK( SelectColorHdl, void * );
    DECL_LINK( SetViewData, void * );
    DECL_LINK( SetDefault, void * );
};

} // end of namespace sd

#endif

