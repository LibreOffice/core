/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: browserpage.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_

#include <vcl/tabpage.hxx>
#include "browserlistbox.hxx"

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OBrowserPage
    //========================================================================
    class OBrowserPage : public TabPage
    {
    private:
        OBrowserListBox     m_aListBox;

    protected:
        virtual void Resize();
        virtual void StateChanged(StateChangedType nType);

    public:
        OBrowserPage(Window* pParent, WinBits nWinStyle = 0);
        ~OBrowserPage();

        sal_Int32 getMinimumWidth();
        sal_Int32 getMinimumHeight();

              OBrowserListBox& getListBox();
        const OBrowserListBox& getListBox() const;
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_


