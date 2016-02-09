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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SELECTLABELDIALOG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SELECTLABELDIALOG_HXX

#include <vcl/fixed.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
#include <vcl/dialog.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "modulepcr.hxx"


namespace pcr
{


    // OSelectLabelDialog

    class OSelectLabelDialog
            :public ModalDialog
            ,public PcrClient
    {
        VclPtr<FixedText>       m_pMainDesc;
        VclPtr<SvTreeListBox>   m_pControlTree;
        VclPtr<CheckBox>        m_pNoAssignment;

        ImageList               m_aModelImages;
        css::uno::Reference< css::beans::XPropertySet >   m_xControlModel;
        OUString m_sRequiredService;
        Image                   m_aRequiredControlImage;
        SvTreeListEntry*        m_pInitialSelection;
        css::uno::Reference< css::beans::XPropertySet >   m_xInitialLabelControl;

        css::uno::Reference< css::beans::XPropertySet >   m_xSelectedControl;
        SvTreeListEntry*        m_pLastSelected;
        bool                    m_bHaveAssignableControl;

    public:
        OSelectLabelDialog(vcl::Window* pParent, css::uno::Reference< css::beans::XPropertySet >  _xControlModel);
        virtual ~OSelectLabelDialog();
        virtual void dispose() override;

        css::uno::Reference< css::beans::XPropertySet >  GetSelected() const { return m_pNoAssignment->IsChecked() ? css::uno::Reference< css::beans::XPropertySet > () : m_xSelectedControl; }

    protected:
        sal_Int32 InsertEntries(const css::uno::Reference< css::uno::XInterface >& _xContainer, SvTreeListEntry* pContainerEntry);

        DECL_LINK_TYPED(OnEntrySelected, SvTreeListBox*, void);
        DECL_LINK_TYPED(OnNoAssignmentClicked, Button*, void);
    };


}   // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SELECTLABELDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
