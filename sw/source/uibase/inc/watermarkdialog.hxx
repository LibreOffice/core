/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_WATERMARKDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_WATERMARKDIALOG_HXX

#include <sfx2/bindings.hxx>
#include <vcl/layout.hxx>

class SwWatermarkDialog : public ModelessDialog
{
public:
    SwWatermarkDialog( vcl::Window* pParent );
    virtual ~SwWatermarkDialog() override;
    virtual void dispose() override;

    void update();

private:
    DECL_LINK( CheckBoxHdl, Button*, void );
    DECL_LINK( OKButtonHdl, Button*, void );

    VclPtr<VclGrid> m_pTextGrid;
    VclPtr<CheckBox> m_pEnableWatermarkCB;
    VclPtr<Edit> m_pTextInput;
    VclPtr<PushButton> m_pOKButton;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
