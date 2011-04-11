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


#ifndef _SD_DLGSNAP_HXX
#define _SD_DLGSNAP_HXX


#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <tools/fract.hxx>
#include <vcl/dialog.hxx>
#include "sdenumdef.hxx"
/************************************************************************/

class SfxItemSet;
namespace sd {
    class View;
}

/*************************************************************************
|*
|* Dialog zum Einstellen von Fanglinien und -punkten
|*
\************************************************************************/

class SdSnapLineDlg : public ModalDialog
{
private:
    FixedLine           aFlPos;
    FixedText           aFtX;
    MetricField         aMtrFldX;
    FixedText           aFtY;
    MetricField         aMtrFldY;
    FixedLine           aFlDir;
    ImageRadioButton    aRbPoint;
    ImageRadioButton    aRbVert;
    ImageRadioButton    aRbHorz;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    PushButton          aBtnDelete;
    long                nXValue;
    long                nYValue;
    FieldUnit           eUIUnit;
    Fraction            aUIScale;

    DECL_LINK( ClickHdl, Button * );

public:
    SdSnapLineDlg(Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView);

    void GetAttr(SfxItemSet& rOutAttrs);

    void HideRadioGroup();
    void HideDeleteBtn() { aBtnDelete.Hide(); }
    void SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY);
};



#endif      // _SD_DLGSNAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
