/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WNameMatch.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:15:41 $
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
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#define DBAUI_WIZ_NAMEMATCHING_HXX

#ifndef DBAUI_WIZ_TABBPAGE_HXX
#include "WTabPage.hxx"
#endif
#ifndef _DBAUI_MARKTREE_HXX_
#include "marktree.hxx"
#endif
#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif

namespace dbaui
{
    // ========================================================
    // columns are at root only no children
    // ========================================================
    class OColumnTreeBox : public OMarkableTreeListBox
    {
        sal_Bool m_bReadOnly;
    protected:
        virtual void InitEntry(SvLBoxEntry* pEntry, const String& rStr, const Image& rImg1, const Image& rImg2);

    public:
        OColumnTreeBox( Window* pParent, WinBits nWinStyle=0 );
        OColumnTreeBox( Window* pParent, const ResId& rResId );

        void FillListBox( const ODatabaseExport::TColumnVector& _rList);
        void SetReadOnly(sal_Bool _bRo=sal_True) { m_bReadOnly = _bRo; }
        virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );

    private:
        using OMarkableTreeListBox::Select;
    };

    // ========================================================
    // Wizard Page: OWizNameMatching
    // Name matching for data appending
    // ========================================================
    class OWizNameMatching : public OWizardPage
    {
        FixedText           m_FT_TABLE_LEFT;
        FixedText           m_FT_TABLE_RIGHT;
        OColumnTreeBox      m_CTRL_LEFT;    // left side
        OColumnTreeBox      m_CTRL_RIGHT;   // right side
        ImageButton         m_ibColumn_up;
        ImageButton         m_ibColumn_down;
        ImageButton         m_ibColumn_up_right;
        ImageButton         m_ibColumn_down_right;
        PushButton          m_pbAll;
        PushButton          m_pbNone;
        String              m_sSourceText;
        String              m_sDestText;

        sal_Bool            m_bAttrsChanged;

        DECL_LINK( ButtonClickHdl, Button * );
        DECL_LINK( RightButtonClickHdl, Button * );
        DECL_LINK( AllNoneClickHdl, Button * );
        DECL_LINK( TableListClickHdl, SvTreeListBox* );
        DECL_LINK( TableListRightSelectHdl, SvTreeListBox* );

    public:
        virtual void            Reset ( );
        virtual void            ActivatePage();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const ;

        OWizNameMatching(Window* pParent);
        virtual ~OWizNameMatching();

    };
}
#endif // DBAUI_WIZ_NAMEMATCHING_HXX



