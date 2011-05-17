/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_

#include <vcl/fixed.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
#include <vcl/dialog.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "modulepcr.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
