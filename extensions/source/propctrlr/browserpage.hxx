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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERPAGE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERPAGE_HXX

#include <vcl/tabpage.hxx>
#include "browserlistbox.hxx"


namespace pcr
{



    //= OBrowserPage

    class OBrowserPage : public TabPage
    {
    private:
        OBrowserListBox     m_aListBox;

    protected:
        virtual void Resize() SAL_OVERRIDE;
        virtual void StateChanged(StateChangedType nType) SAL_OVERRIDE;

    public:
        OBrowserPage(vcl::Window* pParent, WinBits nWinStyle = 0);
        virtual ~OBrowserPage();

        sal_Int32 getMinimumWidth();
        sal_Int32 getMinimumHeight();

              OBrowserListBox& getListBox() { return m_aListBox; }
        const OBrowserListBox& getListBox() const { return m_aListBox; }
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERPAGE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
