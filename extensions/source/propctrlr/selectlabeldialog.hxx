/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_

#include <vcl/fixed.hxx>
#include <svtools/treelistbox.hxx>
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
