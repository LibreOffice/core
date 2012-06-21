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

#ifndef _OFFAPP_INTERNATIONALOPTIONS_HXX_
#define _OFFAPP_INTERNATIONALOPTIONS_HXX_

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

namespace offapp
{
    class InternationalOptionsPage : public SfxTabPage
    {
    private:
        struct IMPL;
        IMPL*               m_pImpl;

    protected:
                            InternationalOptionsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        virtual sal_Bool        FillItemSet( SfxItemSet& _rSet );
        virtual void        Reset( const SfxItemSet& _rSet );

    public:
        virtual             ~InternationalOptionsPage();

        static SfxTabPage*  CreateSd( Window* _pParent, const SfxItemSet& _rAttrSet );
        static SfxTabPage*  CreateSc( Window* _pParent, const SfxItemSet& _rAttrSet );
    };

}   // /namespace offapp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
