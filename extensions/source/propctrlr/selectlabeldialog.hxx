/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selectlabeldialog.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-14 11:32:23 $
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

#ifndef _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    // OSelectLabelDialog
    //========================================================================
    class OSelectLabelDialog
            :public ModalDialog
            ,public PcrClient
    {
        FixedText       m_aMainDesc;
        SvTreeListBox   m_aControlTree;
        CheckBox        m_aNoAssignment;
        FixedLine       m_aSeparator;
        OKButton        m_aOk;
        CancelButton    m_aCancel;

        ImageList       m_aModelImages;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xControlModel;
        ::rtl::OUString m_sRequiredService;
        Image           m_aRequiredControlImage;
        SvLBoxEntry*    m_pInitialSelection;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xInitialLabelControl;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xSelectedControl;
        SvLBoxEntry*    m_pLastSelected;
        sal_Bool        m_bHaveAssignableControl;

    public:
        OSelectLabelDialog(Window* pParent, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xControlModel);
        ~OSelectLabelDialog();

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  GetSelected() const { return m_aNoAssignment.IsChecked() ? ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > () : m_xSelectedControl; }

    protected:
        sal_Int32 InsertEntries(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xContainer, SvLBoxEntry* pContainerEntry);

        DECL_LINK(OnEntrySelected, SvTreeListBox*);
        DECL_LINK(OnNoAssignmentClicked, Button*);
    };

//............................................................................
}   // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_

