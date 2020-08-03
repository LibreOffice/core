/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>

namespace dbaui
{

/**
 * Dialog to set such properties of a query as distinct values and limit
 * It can be opened from Edit menu in Query Design View
 */
class QueryPropertiesDialog : public weld::GenericDialogController
{

public:

    QueryPropertiesDialog(
        weld::Window* pParent, const bool bDistinct, const sal_Int64 nLimit );
    virtual ~QueryPropertiesDialog() override;
    bool getDistinct() const
    {
        return m_xRB_Distinct->get_active();
    }

    sal_Int64 getLimit() const;

private:

    std::unique_ptr<weld::RadioButton> m_xRB_Distinct;
    std::unique_ptr<weld::RadioButton> m_xRB_NonDistinct;
    std::unique_ptr<weld::ComboBox> m_xLB_Limit;
};

} ///dbaui namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
