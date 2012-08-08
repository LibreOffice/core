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
#ifndef _SVX_SRCHXTRA_HXX
#define _SVX_SRCHXTRA_HXX

#include <vcl/field.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/tabdlg.hxx>

#include <svx/checklbx.hxx>
#include <svx/srchdlg.hxx>

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchFormatDialog : public SfxTabDialog
{
public:
    SvxSearchFormatDialog( Window* pParent, const SfxItemSet& rSet );
    ~SvxSearchFormatDialog();

protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

private:
    FontList*       pFontList;
};

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchAttributeDialog : public ModalDialog
{
public:
    SvxSearchAttributeDialog( Window* pParent, SearchAttrItemList& rLst,
                              const sal_uInt16* pWhRanges );

private:
    FixedText           aAttrFL;
    SvxCheckListBox     aAttrLB;

    OKButton            aOKBtn;
    CancelButton        aEscBtn;
    HelpButton          aHelpBtn;

    SearchAttrItemList& rList;

    DECL_LINK(OKHdl, void *);
};

// class SvxSearchSimilarityDialog ---------------------------------------

class SvxSearchSimilarityDialog : public ModalDialog
{
private:
    FixedLine           aFixedLine;
    FixedText           aOtherTxt;
    NumericField        aOtherFld;
    FixedText           aLongerTxt;
    NumericField        aLongerFld;
    FixedText           aShorterTxt;
    NumericField        aShorterFld;
    CheckBox            aRelaxBox;

    OKButton            aOKBtn;
    CancelButton        aEscBtn;
    HelpButton          aHelpBtn;

public:
    SvxSearchSimilarityDialog(  Window* pParent,
                                sal_Bool bRelax,
                                sal_uInt16 nOther,
                                sal_uInt16 nShorter,
                                sal_uInt16 nLonger );

    sal_uInt16  GetOther()      { return (sal_uInt16)aOtherFld.GetValue(); }
    sal_uInt16  GetShorter()    { return (sal_uInt16)aShorterFld.GetValue(); }
    sal_uInt16  GetLonger()     { return (sal_uInt16)aLongerFld.GetValue(); }
    sal_Bool    IsRelaxed()     { return aRelaxBox.IsChecked(); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
