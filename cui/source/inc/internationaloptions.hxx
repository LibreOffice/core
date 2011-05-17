/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
