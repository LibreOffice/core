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
#include <vcl/weld.hxx>
#include <svl/zforlist.hxx>
#include "swdllapi.h"

class SwView;

class SW_DLLPUBLIC NumFormatListBox : public ListBox
{
    short               nCurrFormatType;
    sal_Int32           nStdEntry;
    bool                bOneArea;
    sal_uInt32          nDefFormat;
    LanguageType        eCurLanguage;
    bool                bShowLanguageControl; //determine whether the language control has
                                              //to be shown in the number format dialog
    bool                bUseAutomaticLanguage;//determine whether language is automatically assigned

    DECL_DLLPRIVATE_LINK( SelectHdl, ListBox&, void );

    SAL_DLLPRIVATE static double   GetDefValue(const short nFormatType);
    SAL_DLLPRIVATE void            Init();

public:
    NumFormatListBox(vcl::Window* pWin, WinBits nStyle);

    virtual ~NumFormatListBox() override;

    void            Clear();

    void     SetOneArea(bool bOnlyOne) { bOneArea = bOnlyOne; }

    void            SetFormatType(const short nFormatType);
    short    GetFormatType() const { return nCurrFormatType; }
    void            SetDefFormat(const sal_uInt32 nDefFormat);
    sal_uInt32      GetFormat() const;

    LanguageType GetCurLanguage() const { return eCurLanguage;}
    void                SetLanguage(LanguageType eSet)  { eCurLanguage = eSet;}

    void            SetAutomaticLanguage(bool bSet){bUseAutomaticLanguage = bSet;}
    bool            IsAutomaticLanguage()const {return bUseAutomaticLanguage;}

    void            SetShowLanguageControl(bool bSet){bShowLanguageControl = bSet;}
};

class SW_DLLPUBLIC NumFormatComboBoxText
{
    Weld::Dialog& m_rDialog;
    std::unique_ptr<Weld::ComboBoxText> m_xComboBoxText;
    Link<NumFormatComboBoxText&, void> maSelectHdl;
    short               nCurrFormatType;
    sal_Int32           nStdEntry;
    bool                bOneArea;
    sal_uInt32          nDefFormat;
    LanguageType        eCurLanguage;
    bool                bShowLanguageControl; //determine whether the language control has
                                              //to be shown in the number format dialog
    bool                bUseAutomaticLanguage;//determine whether language is automatically assigned

    DECL_DLLPRIVATE_LINK(SelectHdl, NumFormatComboBoxText&, void);
    DECL_LINK(Changed, Weld::ComboBoxText&, void);

    SAL_DLLPRIVATE static double   GetDefValue(const short nFormatType);
    SAL_DLLPRIVATE void            Init();

public:
    NumFormatComboBoxText(Weld::Dialog& rDialog, Weld::ComboBoxText* pComboBoxText);

    void            Clear();

    void     SetOneArea(bool bOnlyOne) { bOneArea = bOnlyOne; }

    void            SetFormatType(const short nFormatType);
    short    GetFormatType() const { return nCurrFormatType; }
    void            SetDefFormat(const sal_uInt32 nDefFormat);
    sal_uInt32      GetFormat() const;

    LanguageType GetCurLanguage() const { return eCurLanguage;}
    void                SetLanguage(LanguageType eSet)  { eCurLanguage = eSet;}

    void            SetAutomaticLanguage(bool bSet){bUseAutomaticLanguage = bSet;}
    bool            IsAutomaticLanguage()const {return bUseAutomaticLanguage;}

    void            SetShowLanguageControl(bool bSet){bShowLanguageControl = bSet;}

    void set_sensitive(bool bSensitive)
    {
        m_xComboBoxText->set_sensitive(bSensitive);
    }

    const Link<NumFormatComboBoxText&, void>& GetSelectHdl() const { return maSelectHdl; }
    void SetSelectHdl(const Link<NumFormatComboBoxText&, void>& rLink) { maSelectHdl = rLink; }

    sal_Int32 GetSelectEntryPos() const { return m_xComboBoxText->get_active(); }
    sal_Int32 GetEntryCount() const { return m_xComboBoxText->get_count(); }
    OUString GetEntry(sal_Int32 nPos) const { return m_xComboBoxText->get_text(nPos); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
