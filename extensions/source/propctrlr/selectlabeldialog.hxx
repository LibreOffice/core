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

#include <vcl/weld.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "modulepcr.hxx"

namespace pcr
{
    // OSelectLabelDialog
    class OSelectLabelDialog final : public weld::GenericDialogController
    {
        css::uno::Reference< css::beans::XPropertySet >   m_xControlModel;
        OUString m_sRequiredService;
        OUString m_aRequiredControlImage;
        std::unique_ptr<weld::TreeIter> m_xInitialSelection;
        // the entry datas of the listbox entries
        std::vector<std::unique_ptr<css::uno::Reference<css::beans::XPropertySet>>> m_xUserData;
        css::uno::Reference< css::beans::XPropertySet >   m_xInitialLabelControl;

        css::uno::Reference< css::beans::XPropertySet >   m_xSelectedControl;
        std::unique_ptr<weld::TreeIter> m_xLastSelected;
        bool m_bLastSelected;
        bool m_bHaveAssignableControl;

        std::unique_ptr<weld::Label> m_xMainDesc;
        std::unique_ptr<weld::TreeView> m_xControlTree;
        std::unique_ptr<weld::CheckButton> m_xNoAssignment;

    public:
        OSelectLabelDialog(weld::Window* pParent, css::uno::Reference< css::beans::XPropertySet > const & _xControlModel);
        virtual ~OSelectLabelDialog() override;

        css::uno::Reference< css::beans::XPropertySet >  GetSelected() const { return m_xNoAssignment->get_active() ? css::uno::Reference< css::beans::XPropertySet > () : m_xSelectedControl; }

    private:
        sal_Int32 InsertEntries(const css::uno::Reference< css::uno::XInterface >& _xContainer, weld::TreeIter& rContainerEntry);

        DECL_LINK(OnEntrySelected, weld::TreeView&, void);
        DECL_LINK(OnNoAssignmentClicked, weld::ToggleButton&, void);
    };
}   // namespace pcr

#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SELECTLABELDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
