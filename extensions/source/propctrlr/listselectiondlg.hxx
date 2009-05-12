/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listselectiondlg.hxx,v $
 * $Revision: 1.4 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= ListSelectionDialog
    //====================================================================
    class ListSelectionDialog : public ModalDialog
    {
    private:
        FixedText       m_aLabel;
        ListBox         m_aEntries;
        OKButton        m_aOK;
        CancelButton    m_aCancel;
        HelpButton      m_aHelp;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xListBox;
        ::rtl::OUString m_sPropertyName;

    public:
        ListSelectionDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxListBox,
            const ::rtl::OUString& _rPropertyName,
            const String& _rPropertyUIName
        );

        // Dialog overridables
        virtual short   Execute();

    private:
        void    initialize( );
        void    commitSelection();

        void    fillEntryList   ( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rListEntries );

        void    selectEntries   ( const ::com::sun::star::uno::Sequence< sal_Int16 >& /* [in ] */ _rSelection );
        void    collectSelection(       ::com::sun::star::uno::Sequence< sal_Int16 >& /* [out] */ _rSelection );
    };

//........................................................................
} // namespacepcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

