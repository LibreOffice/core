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

#ifndef INCLUDED_SVX_PAGESIZEHELPER_HXX
#define INCLUDED_SVX_PAGESIZEHELPER_HXX

#include <i18nutil/paper.hxx>
#include <svx/svxdllapi.h>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>

enum class PaperSizeApp
{
    Std,
    Draw
};

class SVX_DLLPUBLIC PaperSizeListBox : public ListBox
{
public:
    PaperSizeListBox( vcl::Window* pParent );

    void FillPaperSizeEntries( PaperSizeApp eApp );
    void SetSelection(  Paper eSize  );
    Paper GetSelection() const;

    Size GetOptimalSize() const override;
};

class SVX_DLLPUBLIC SvxPaperSizeListBox
{
private:
    std::unique_ptr<weld::ComboBoxText> m_xControl;
public:
    SvxPaperSizeListBox(std::unique_ptr<weld::ComboBoxText> pControl);

    void FillPaperSizeEntries(PaperSizeApp eApp);
    void SetSelection(Paper eSize);
    Paper GetSelection() const;

    void connect_changed(const Link<weld::ComboBoxText&, void>& rLink) { m_xControl->connect_changed(rLink); }
    void clear() { m_xControl->clear(); }
    void save_value() { return m_xControl->save_value(); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }

    weld::ComboBoxText& get_widget() const { return *m_xControl; }
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
