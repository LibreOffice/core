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
#ifndef INCLUDED_SFX2_SOURCE_INC_DOCUMENTFONTSDIALOG_HXX
#define INCLUDED_SFX2_SOURCE_INC_DOCUMENTFONTSDIALOG_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>

/**
 Tab page for document font settings in the document properties dialog.
*/
class SfxDocumentFontsPage: public SfxTabPage
{
public:
    SfxDocumentFontsPage( vcl::Window* parent, const SfxItemSet& set );
    virtual ~SfxDocumentFontsPage();
    virtual void dispose() override;
    static VclPtr<SfxTabPage> Create( vcl::Window* parent, const SfxItemSet* set );
protected:
    virtual bool FillItemSet( SfxItemSet* set ) override;
    virtual void Reset( const SfxItemSet* set ) override;
private:
    VclPtr<CheckBox> embedFontsCheckbox;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
