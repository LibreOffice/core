/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listselectiondlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:16:59 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
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

