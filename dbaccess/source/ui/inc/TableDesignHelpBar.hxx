/*************************************************************************
 *
 *  $RCSfile: TableDesignHelpBar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#define DBAUI_TABLEDESIGNHELPBAR_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif

class MultiLineEdit;
namespace dbaui
{
    //==================================================================
    // Ableitung von TabPage ist ein Trick von TH,
    // um Aenderungen der Systemfarben zu bemerken (Bug #53905)
    class OTableDesignHelpBar : public TabPage
                                ,public IClipboardTest
    {
    private:
        String          m_sHelpText;
        MultiLineEdit*  m_pTextWin;
        USHORT          m_nDummy;

    protected:
        virtual void Resize();

    public:
        OTableDesignHelpBar( Window* pParent );
        virtual ~OTableDesignHelpBar();

        void SetHelpText( const String& rText );
        String GetHelpText() const { return m_sHelpText; }

        virtual long PreNotify( NotifyEvent& rNEvt );

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void copy();
        virtual void cut();
        virtual void paste();
    };

    //==================================================================
    class OFieldPropTabCtrl : public TabControl
    {
    public:
        OFieldPropTabCtrl( Window* pParent, WinBits nWinStyle = WB_STDTABCONTROL );
        virtual ~OFieldPropTabCtrl();

    };

}
#endif // DBAUI_TABLEDESIGNHELPBAR_HXX

