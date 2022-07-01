/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/itemset.hxx>
#include <vcl/weld.hxx>

namespace chart
{
class DataTablePropertiesResources final
{
private:
    std::unique_ptr<weld::CheckButton> m_xCbHorizontalBorder;
    std::unique_ptr<weld::CheckButton> m_xCbVerticalBorder;
    std::unique_ptr<weld::CheckButton> m_xCbOutilne;
    std::unique_ptr<weld::CheckButton> m_xCbKeys;

public:
    DataTablePropertiesResources(weld::Builder& rBuilder);

    void initFromItemSet(SfxItemSet const& rInAttrs);
    bool writeToItemSet(SfxItemSet& rOutAttrs) const;
    void setChecksSensitive(bool bSensitive);

    bool getHorizontalBorder() { return m_xCbHorizontalBorder->get_active(); }
    void setHorizontalBorder(bool bActive) { m_xCbHorizontalBorder->set_active(bActive); }

    bool getVerticalBorder() { return m_xCbVerticalBorder->get_active(); }
    void setVerticalBorder(bool bActive) { m_xCbVerticalBorder->set_active(bActive); }

    bool getOutline() { return m_xCbOutilne->get_active(); }
    void setOutline(bool bActive) { m_xCbOutilne->set_active(bActive); }

    bool getKeys() { return m_xCbKeys->get_active(); }
    void setKeys(bool bActive) { m_xCbKeys->set_active(bActive); }
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
