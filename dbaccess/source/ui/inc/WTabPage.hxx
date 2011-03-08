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

#ifndef DBAUI_WIZ_TABBPAGE_HXX
#define DBAUI_WIZ_TABBPAGE_HXX

#include <vcl/tabpage.hxx>

namespace dbaui
{
    // ========================================================
    // Wizard Page
    // ========================================================
    class OCopyTableWizard;
    class OWizardPage       : public TabPage
    {
    protected:
        OCopyTableWizard*           m_pParent;
        sal_Bool                    m_bFirstTime;   // Page wird das erste mal gerufen ; should be set in the reset methode

        OWizardPage( Window* pParent, const ResId& rResId );// : TabPage(pParent,rResId),m_pParent((OCopyTableWizard*)pParent),m_bFirstTime(sal_True) {};

    public:
        virtual void        Reset ( )           = 0;
        virtual sal_Bool    LeavePage()         = 0;
        virtual String      GetTitle() const    = 0;

        sal_Bool            IsFirstTime() const { return m_bFirstTime; }
    };
}
#endif // DBAUI_WIZ_TABBPAGE_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
