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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUMFMTLB_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUMFMTLB_HXX

#include <vcl/lstbox.hxx>
#include <svl/zforlist.hxx>
#include <swdllapi.h>

class SwView;

class SW_DLLPUBLIC NumFormatListBox : public ListBox
{
    SvNumFormatType     nCurrFormatType;
    bool                mbCurrFormatTypeNeedsInit;
    sal_Int32           nStdEntry;
    bool                bOneArea;
    sal_uInt32          nDefFormat;
    LanguageType        eCurLanguage;
    bool                bShowLanguageControl; //determine whether the language control has
                                              //to be shown in the number format dialog
    bool                bUseAutomaticLanguage;//determine whether language is automatically assigned

    DECL_DLLPRIVATE_LINK( SelectHdl, ListBox&, void );

    SAL_DLLPRIVATE void            Init();

public:
    NumFormatListBox(vcl::Window* pWin, WinBits nStyle);

    virtual ~NumFormatListBox() override;

    void            Clear();

    void     SetOneArea(bool bOnlyOne) { bOneArea = bOnlyOne; }

    void            SetFormatType(const SvNumFormatType nFormatType);
    SvNumFormatType GetFormatType() const { return nCurrFormatType; }
    void            SetDefFormat(const sal_uInt32 nDefFormat);
    sal_uInt32      GetFormat() const;

    LanguageType GetCurLanguage() const { return eCurLanguage;}
    void                SetLanguage(LanguageType eSet)  { eCurLanguage = eSet;}

    void            SetAutomaticLanguage(bool bSet){bUseAutomaticLanguage = bSet;}
    bool            IsAutomaticLanguage()const {return bUseAutomaticLanguage;}

    void            SetShowLanguageControl(bool bSet){bShowLanguageControl = bSet;}

    SAL_DLLPRIVATE static double   GetDefValue(const SvNumFormatType nFormatType);
};

class SW_DLLPUBLIC SwNumFormatListBox
{
    SvNumFormatType     nCurrFormatType;
    bool                mbCurrFormatTypeNeedsInit;
    sal_Int32           nStdEntry;
    sal_uInt32          nDefFormat;
    LanguageType        eCurLanguage;
    bool                bUseAutomaticLanguage;//determine whether language is automatically assigned

    std::unique_ptr<weld::ComboBox> mxControl;

    DECL_DLLPRIVATE_LINK( SelectHdl, weld::ComboBox&, void );

    SAL_DLLPRIVATE void            Init();

public:
    SwNumFormatListBox(std::unique_ptr<weld::ComboBox> xControl);

    ~SwNumFormatListBox();

    void            clear();
    void            show() { mxControl->show(); }
    void            hide() { mxControl->hide(); }

    void            SetFormatType(const SvNumFormatType nFormatType);
    void            SetDefFormat(const sal_uInt32 nDefFormat);
    sal_uInt32      GetFormat() const;

    void            CallSelectHdl();

    void            set_sensitive(bool bSensitive) { mxControl->set_sensitive(bSensitive); }
    void            connect_changed(const Link<weld::ComboBox&, void>& rLink) { mxControl->connect_changed(rLink); }
    weld::ComboBox& get_widget() const { return *mxControl; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
