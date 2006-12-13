/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: browserpage.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:56:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERPAGE_HXX_

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLISTBOX_HXX_
#include "browserlistbox.hxx"
#endif

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


