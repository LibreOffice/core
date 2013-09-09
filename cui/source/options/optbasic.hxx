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

#ifndef _SVX_OPTBASIC_HXX
#define _SVX_OPTBASIC_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <sfx2/tabdlg.hxx>

class SvxBasicIDEOptionsPage: public SfxTabPage
{
private:
    CheckBox* pCodeCompleteChk;
    CheckBox* pAutocloseProcChk;
    CheckBox* pAutocloseParenChk;
    CheckBox* pAutocloseQuotesChk;
    CheckBox* pAutoCorrectChk;
    CheckBox* pUseExtendedTypesChk;

    void LoadConfig();

public:
    SvxBasicIDEOptionsPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxBasicIDEOptionsPage();

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rSet );
    virtual sal_Bool FillItemSet( SfxItemSet& rSet );
    virtual void Reset( const SfxItemSet& rSet );
    virtual void FillUserData();
};


#endif //_SVX_OPTBASIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
