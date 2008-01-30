/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WColumnSelect.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:48:21 $
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
#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#define DBAUI_WIZ_COLUMNSELECT_HXX

#include "WTabPage.hxx"
#include "WCopyTable.hxx"

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <comphelper/stl_types.hxx>

namespace dbaui
{
    class OFieldDescription;

    // ========================================================
    // Wizard Page: OWizColumnSelect
    // ========================================================

    class OWizColumnSelect : public OWizardPage
    {

        FixedLine               m_flColumns;
        MultiListBox            m_lbOrgColumnNames; // left side
        ImageButton             m_ibColumn_RH;
        ImageButton             m_ibColumns_RH;
        ImageButton             m_ibColumn_LH;
        ImageButton             m_ibColumns_LH;
        MultiListBox            m_lbNewColumnNames; // right side


        DECL_LINK( ButtonClickHdl, Button * );
        DECL_LINK( ListDoubleClickHdl, MultiListBox * );


        void clearListBox(MultiListBox& _rListBox);
        void fillColumns(       ListBox* pRight,
                                ::std::vector< ::rtl::OUString> &_rRightColumns);

        void createNewColumn(   ListBox* _pListbox,
                                OFieldDescription* _pSrcField,
                                ::std::vector< ::rtl::OUString>& _rRightColumns,
                                const ::rtl::OUString&  _sColumnName,
                                const ::rtl::OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::TStringMixEqualFunctor& _aCase);

        void moveColumn(        ListBox* _pRight,
                                ListBox* _pLeft,
                                ::std::vector< ::rtl::OUString>& _rRightColumns,
                                const ::rtl::OUString&  _sColumnName,
                                const ::rtl::OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::TStringMixEqualFunctor& _aCase);

        void enableButtons();


        USHORT adjustColumnPosition(ListBox* _pLeft,
                                    const ::rtl::OUString&  _sColumnName,
                                    ODatabaseExport::TColumnVector::size_type nCurrentPos,
                                    const ::comphelper::TStringMixEqualFunctor& _aCase);

    public:
        virtual void            Reset ( );
        virtual void            ActivatePage();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const ;

        OWizColumnSelect(Window* pParent);
        virtual ~OWizColumnSelect();
    };
}
#endif // DBAUI_WIZ_COLUMNSELECT_HXX



