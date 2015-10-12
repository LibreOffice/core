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

#ifndef INCLUDED_CUI_SOURCE_INC_HEADERTABLISTBOX_HXX
#define INCLUDED_CUI_SOURCE_INC_HEADERTABLISTBOX_HXX

#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>


class MacroEventListBox : public Control
{
private:
    VclPtr<HeaderBar>               maHeaderBar;
    VclPtr<SvHeaderTabListBox>      maListBox;
protected:
    DECL_LINK_TYPED( HeaderEndDrag_Impl, HeaderBar*, void );
    virtual bool                Notify( NotifyEvent& rNEvt ) override;
public:
    MacroEventListBox( vcl::Window* pParent, WinBits nStyle );
    virtual ~MacroEventListBox();
    virtual void dispose() override;

    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;

    SvHeaderTabListBox& GetListBox()
    {
        return *maListBox.get();
    }

    HeaderBar& GetHeaderBar()
    {
        return *maHeaderBar.get();
    }

    void                        ConnectElements();/**< should be called after all manipulations on elements are done
                                                             calcs real sizes depending on sizes of this */
    void                        Show( bool bVisible = true, ShowFlags nFlags = ShowFlags::NONE );    ///< same meaning as Windows::Show()
    void                        Enable( bool bEnable = true, bool bChild = true );  ///< same meaning as Windows::Enable()
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
