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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONDEDIT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONDEDIT_HXX

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svx/dbaexchange.hxx>

template <typename X> class ConditionEdit;

template <class T> class ConditionEditDropTarget final : public DropTargetHelper
{
private:
    ConditionEdit<T>& m_rEdit;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent&) override
    {
        return svx::OColumnTransferable::canExtractColumnDescriptor(
                   GetDataFlavorExVector(), ColumnTransferFormatFlags::COLUMN_DESCRIPTOR)
                   ? DND_ACTION_COPY
                   : DND_ACTION_NONE;
    }
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        sal_Int8 nRet = DND_ACTION_NONE;
        if (m_rEdit.GetDropEnable())
        {
            TransferableDataHelper aData(rEvt.maDropEvent.Transferable);

            const DataFlavorExVector& rVector = aData.GetDataFlavorExVector();
            if (svx::OColumnTransferable::canExtractColumnDescriptor(
                    rVector, ColumnTransferFormatFlags::COLUMN_DESCRIPTOR))
            {
                svx::ODataAccessDescriptor aColDesc
                    = svx::OColumnTransferable::extractColumnDescriptor(aData);
                OUString sDBName;
                bool bBrackets = m_rEdit.GetBrackets();
                if (bBrackets)
                    sDBName += "[";
                OUString sTmp = aColDesc.getDataSource();
                sDBName += sTmp + ".";

                aColDesc[svx::DataAccessDescriptorProperty::Command] >>= sTmp;
                sDBName += sTmp + ".";

                aColDesc[svx::DataAccessDescriptorProperty::ColumnName] >>= sTmp;
                sDBName += sTmp;
                if (bBrackets)
                    sDBName += "]";

                m_rEdit.get_widget().set_text(sDBName);
                nRet = DND_ACTION_COPY;
            }
        }
        return nRet;
    }

public:
    ConditionEditDropTarget(ConditionEdit<T>& rEdit)
        : DropTargetHelper(rEdit.get_widget().get_drop_target())
        , m_rEdit(rEdit)
    {
    }
};

//< weld::Entry >
template <class T> class ConditionEdit
{
    std::unique_ptr<T> m_xControl;
    ConditionEditDropTarget<T> m_aDropTargetHelper;
    bool m_bBrackets, m_bEnableDrop;

public:
    ConditionEdit(std::unique_ptr<T> xControl)
        : m_xControl(std::move(xControl))
        , m_aDropTargetHelper(*this)
        , m_bBrackets(true)
        , m_bEnableDrop(true)
    {
    }

    OUString get_text() const { return m_xControl->get_text(); }
    void set_text(const OUString& rText) { m_xControl->set_text(rText); }
    void set_visible(bool bVisible) { m_xControl->set_visible(bVisible); }
    void set_accessible_name(const OUString& rName) { m_xControl->set_accessible_name(rName); }
    void save_value() { m_xControl->save_value(); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }
    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    void connect_changed(const Link<T&, void>& rLink) { m_xControl->connect_changed(rLink); }
    void replace_selection(const OUString& rText) { m_xControl->replace_selection(rText); }
    void hide() { m_xControl->hide(); }
    T& get_widget() { return *m_xControl; }

    OUString get_buildable_name() const { return m_xControl->get_buildable_name(); }
    void set_buildable_name(const OUString& rId) { m_xControl->set_buildable_name(rId); }

    void ShowBrackets(bool bShow) { m_bBrackets = bShow; }
    bool GetBrackets() const { return m_bBrackets; }
    void SetDropEnable(bool bFlag) { m_bEnableDrop = bFlag; }
    bool GetDropEnable() const { return m_bEnableDrop; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
