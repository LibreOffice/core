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

#include <vcl/weld.hxx>
#include <svl/zforlist.hxx>
#include <swdllapi.h>

class SwView;

class SW_DLLPUBLIC SwNumFormatBase
{
protected:
    sal_Int32           nStdEntry;
    sal_uInt32          nDefFormat;
    SvNumFormatType     nCurrFormatType;
    LanguageType        eCurLanguage;
    bool                bOneArea;
    bool                mbCurrFormatTypeNeedsInit;
    bool                bShowLanguageControl; //determine whether the language control has
                                              //to be shown in the number format dialog
    bool                bUseAutomaticLanguage;//determine whether language is automatically assigned
public:
    SwNumFormatBase();

    void SetAutomaticLanguage(bool bSet) { bUseAutomaticLanguage = bSet; }
    bool IsAutomaticLanguage()const { return bUseAutomaticLanguage; }
    SvNumFormatType GetFormatType() const { return nCurrFormatType; }
    LanguageType GetCurLanguage() const { return eCurLanguage;}
    void SetLanguage(LanguageType eSet)  { eCurLanguage = eSet; }
    void SetShowLanguageControl(bool bSet) { bShowLanguageControl = bSet; }
    SAL_DLLPRIVATE static double GetDefValue(const SvNumFormatType nFormatType);
    void SetOneArea(bool bOnlyOne) { bOneArea = bOnlyOne; }

    void SetFormatType(const SvNumFormatType nFormatType);
    void SetDefFormat(const sal_uInt32 nDefFormat);
    virtual sal_uInt32 GetFormat() const = 0;

    virtual void Init();
    void CallSelectHdl();

    virtual void clear();
    virtual int get_count() const = 0;
    virtual int get_active() const = 0;
    virtual OUString get_id(int nPos) const = 0;
    virtual OUString get_text(int nPos) const = 0;
    virtual weld::Widget& get_widget() const = 0;
    virtual void append(const OUString& rId, const OUString& rText) = 0;
    virtual void append_text(const OUString& rText) = 0;
    virtual void insert_text(int nPos, const OUString& rText) = 0;
    virtual void set_active(int nPos) = 0;
    virtual void set_id(int nPos, const OUString& rId) = 0;
    virtual ~SwNumFormatBase() {}
};

class SW_DLLPUBLIC NumFormatListBox : public SwNumFormatBase
{
    std::unique_ptr<weld::ComboBox> mxControl;

    DECL_DLLPRIVATE_LINK( SelectHdl, weld::ComboBox&, void );

    virtual void Init() override;

public:
    NumFormatListBox(std::unique_ptr<weld::ComboBox> xControl);

    virtual sal_uInt32 GetFormat() const override;

    virtual void clear() override;
    virtual int get_count() const override { return mxControl->get_count(); }
    virtual int get_active() const override { return mxControl->get_active(); }
    virtual OUString get_id(int nPos) const override { return mxControl->get_id(nPos); }
    virtual OUString get_text(int nPos) const override { return mxControl->get_text(nPos); }
    virtual weld::Widget& get_widget() const override { return *mxControl; }
    virtual void append(const OUString& rId, const OUString& rText) override { mxControl->append(rId, rText); }
    virtual void append_text(const OUString& rText) override { mxControl->append_text(rText); }
    virtual void insert_text(int nPos, const OUString& rText) override { mxControl->insert_text(nPos, rText); }
    virtual void set_active(int nPos) override { mxControl->set_active(nPos); }
    virtual void set_id(int nPos, const OUString& rId) override { mxControl->set_id(nPos, rId); }
    void            show() { mxControl->show(); }
    void            hide() { mxControl->hide(); }

    void            set_sensitive(bool bSensitive) { mxControl->set_sensitive(bSensitive); }
    void            connect_changed(const Link<weld::ComboBox&, void>& rLink) { mxControl->connect_changed(rLink); }
};

class SW_DLLPUBLIC SwNumFormatTreeView : public SwNumFormatBase
{
    std::unique_ptr<weld::TreeView> mxControl;

    DECL_DLLPRIVATE_LINK( SelectHdl, weld::TreeView&, void );

    virtual void Init() override;

public:
    SwNumFormatTreeView(std::unique_ptr<weld::TreeView> xControl);

    virtual sal_uInt32 GetFormat() const override;

    virtual void clear() override;
    virtual int get_count() const override { return mxControl->n_children(); }
    virtual int get_active() const override { return mxControl->get_selected_index(); }
    virtual OUString get_id(int nPos) const override { return mxControl->get_id(nPos); }
    virtual OUString get_text(int nPos) const override { return mxControl->get_text(nPos); }
    virtual weld::Widget& get_widget() const override { return *mxControl; }
    virtual void append(const OUString& rId, const OUString& rText) override { mxControl->append(rId, rText); }
    virtual void append_text(const OUString& rText) override { mxControl->append_text(rText); }
    virtual void insert_text(int nPos, const OUString& rText) override { mxControl->insert_text(nPos, rText); }
    virtual void set_active(int nPos) override { mxControl->select(nPos); }
    virtual void set_id(int nPos, const OUString& rId) override { mxControl->set_id(nPos, rId); }
    OUString        get_selected_text() const { return mxControl->get_selected_text(); }
    bool            get_visible() const { return mxControl->get_visible(); }
    int             get_selected_index() const { return mxControl->get_selected_index(); }
    void            set_visible(bool bVisible) { mxControl->set_visible(bVisible); }
    void            select(int nPos) { mxControl->select(nPos); }
    void            connect_row_activated(const Link<weld::TreeView&, bool>& rLink) { mxControl->connect_row_activated(rLink); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
