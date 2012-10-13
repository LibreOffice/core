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

#ifndef _MACROPG_IMPL_HXX
#define _MACROPG_IMPL_HXX

class _SvxMacroTabPage_Impl
{
public:
                                    _SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet );
                                    ~_SvxMacroTabPage_Impl();

    FixedText*                      pAssignFT;
    PushButton*                     pAssignPB;
    PushButton*                     pAssignComponentPB;
    PushButton*                     pDeletePB;
    Image*                          pMacroImg;
    Image*                          pComponentImg;
    String*                         pStrEvent;
    String*                         pAssignedMacro;
    _HeaderTabListBox*              pEventLB;
    sal_Bool                            bReadOnly;
    sal_Bool                            bIDEDialogMode;
};

class AssignComponentDialog : public ModalDialog
{
private:
    FixedText       maMethodLabel;
    Edit            maMethodEdit;
    OKButton        maOKButton;
    CancelButton    maCancelButton;
    HelpButton      maHelpButton;

    ::rtl::OUString maURL;

    DECL_LINK(ButtonHandler, void *);

public:
    AssignComponentDialog( Window * pParent, const ::rtl::OUString& rURL );
    ~AssignComponentDialog();

    ::rtl::OUString getURL( void ) const
        { return maURL; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
