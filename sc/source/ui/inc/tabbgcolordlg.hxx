/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef SC_TABBGCOLORDLG_HXX
#define SC_TABBGCOLORDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/valueset.hxx>

//------------------------------------------------------------------------

class ScTabBgColorDlg : public ModalDialog
{
public:
    ScTabBgColorDlg( Window* pParent,
                     const String& rTitle,
                     const String& rTabBgColorNoColorText,
                     const Color& rDefaultColor,
                     const rtl::OString& nHelpId );
    ~ScTabBgColorDlg();

    void GetSelectedColor( Color& rColor ) const;

private:
    class ScTabBgColorValueSet : public ValueSet
    {
    public:
        ScTabBgColorValueSet(Control* pParent, const ResId& rResId, ScTabBgColorDlg* pTabBgColorDlg);

        virtual void KeyInput( const KeyEvent& rKEvt );
    private:
        ScTabBgColorDlg* aTabBgColorDlg;
    };

    Control                 aBorderWin;
    ScTabBgColorValueSet    aTabBgColorSet;
    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;
    Color                   aTabBgColor;
    const String            aTabBgColorNoColorText;
    rtl::OString            msHelpId;

    void            FillColorValueSets_Impl();

    DECL_LINK( TabBgColorDblClickHdl_Impl, ValueSet* );
    DECL_LINK( TabBgColorOKHdl_Impl, OKButton* pBtn );
};

#endif // SC_TABBGCOLORDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
