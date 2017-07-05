/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_TABLEPOSITIONHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_TABLEPOSITIONHANDLER_HXX

#include "LoggedResources.hxx"
#include <memory>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace writerfilter
{
namespace dmapper
{

/// Handler for floating table positioning
class TablePositionHandler
    : public LoggedProperties
{
    OUString m_aVertAnchor;
    OUString m_aYSpec;
    OUString m_aHorzAnchor;
    OUString m_aXSpec;
    sal_Int32 m_nY = 0;
    sal_Int32 m_nX = 0;
    sal_Int32 m_nLeftFromText = 0;
    sal_Int32 m_nRightFromText = 0;
    sal_Int32 m_nTopFromText = 0;
    sal_Int32 m_nBottomFromText = 0;

    // Properties
    void lcl_attribute(Id Name, Value& val) override;
    void lcl_sprm(Sprm& sprm) override;

public:
    sal_Int32 getY()
    {
        return m_nY;
    }
    sal_Int32 getX()
    {
        return m_nX;
    }
    sal_Int32 getLeftFromText()
    {
        return m_nLeftFromText;
    }
    sal_Int32 getRightFromText()
    {
        return m_nRightFromText;
    }
    sal_Int32 getTopFromText()
    {
        return m_nTopFromText;
    }
    sal_Int32 getBottomFromText()
    {
        return m_nBottomFromText;
    }

    const OUString& getVertAnchor()
    {
        return m_aVertAnchor;
    }
    const OUString& getYSpec()
    {
        return m_aYSpec;
    }
    const OUString& getHorzAnchor()
    {
        return m_aHorzAnchor;
    }
    const OUString& getXSpec()
    {
        return m_aXSpec;
    }

    TablePositionHandler();
    ~TablePositionHandler() override;

    /** Compute the UNO properties for the frame containing the table based
        on the received tokens.

        Note that the properties will need to be adjusted with the table
        properties before actually using them.
      */
    css::uno::Sequence<css::beans::PropertyValue> getTablePosition() const;

    bool operator== (const TablePositionHandler& rHandler) const;
};

using TablePositionHandlerPtr = std::shared_ptr<TablePositionHandler>;
} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
