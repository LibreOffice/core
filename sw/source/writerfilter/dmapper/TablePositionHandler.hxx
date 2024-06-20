/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "LoggedResources.hxx"

namespace com::sun::star::beans
{
struct PropertyValue;
}

namespace writerfilter::dmapper
{
/// Handler for floating table positioning
class TablePositionHandler : public LoggedProperties
{
    OUString m_aVertAnchor{ u"margin"_ustr };
    OUString m_aYSpec;
    OUString m_aHorzAnchor{ u"text"_ustr };
    OUString m_aXSpec;
    sal_Int32 m_nY = 0;
    sal_Int32 m_nX = 0;
    sal_Int32 m_nLeftFromText = 0;
    sal_Int32 m_nRightFromText = 0;
    sal_Int32 m_nTopFromText = 0;
    sal_Int32 m_nBottomFromText = 0;
    Id m_nTableOverlap = 0;

    // Properties
    void lcl_attribute(Id nId, Value& rVal) override;
    void lcl_sprm(Sprm& sprm) override;

public:
    void setTableOverlap(Id nTableOverlap) { m_nTableOverlap = nTableOverlap; }

    TablePositionHandler();
    ~TablePositionHandler() override;

    /** Compute the UNO properties for the frame containing the table based
        on the received tokens.

        Note that the properties will need to be adjusted with the table
        properties before actually using them.
      */
    css::uno::Sequence<css::beans::PropertyValue> getTablePosition() const;

    bool operator==(const TablePositionHandler& rHandler) const;
};

using TablePositionHandlerPtr = tools::SvRef<TablePositionHandler>;
} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
