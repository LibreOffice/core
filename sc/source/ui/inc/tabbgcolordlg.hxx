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
                     const OString& nHelpId );
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
    OString            msHelpId;

    void            FillColorValueSets_Impl();

    DECL_LINK(TabBgColorDblClickHdl_Impl, void *);
    DECL_LINK(TabBgColorOKHdl_Impl, void *);
};

#endif // SC_TABBGCOLORDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
