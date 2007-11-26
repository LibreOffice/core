/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: webconninfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 16:41:00 $
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
#ifndef _SVX_WEBCONNINFO_HXX
#define _SVX_WEBCONNINFO_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>

//........................................................................
namespace svx
{
//........................................................................

    class PasswordTable : public SvxSimpleTable
    {
    public:
        PasswordTable( Window* pParent, const ResId& rResId );

        void InsertHeaderItem( USHORT nColumn, const String& rText, HeaderBarItemBits nBits );
        void ResetTabs();
        void Resort( bool bForced );
    };

    //====================================================================
    //= class WebConnectionIfoDialog
    //====================================================================
    class WebConnectionInfoDialog : public ModalDialog
    {
    private:
        FixedInfo           m_aNeverShownFI;
        PasswordTable       m_aPasswordsLB;
        PushButton          m_aRemoveBtn;
        PushButton          m_aRemoveAllBtn;
        PushButton          m_aChangeBtn;
        FixedLine           m_aButtonsFL;
        CancelButton        m_aCloseBtn;
        HelpButton          m_aHelpBtn;

    DECL_LINK( HeaderBarClickedHdl, SvxSimpleTable* );
    DECL_LINK( RemovePasswordHdl, PushButton* );
    DECL_LINK( RemoveAllPasswordsHdl, PushButton* );
    DECL_LINK( ChangePasswordHdl, PushButton* );
    DECL_LINK( EntrySelectedHdl, void* );

    void FillPasswordList();

    public:
        WebConnectionInfoDialog( Window* pParent );
        ~WebConnectionInfoDialog();
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // #ifndef _SVX_WEBCONNINFO_HXX

