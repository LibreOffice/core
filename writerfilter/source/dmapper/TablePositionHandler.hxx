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

#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
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
    sal_Int32 m_nY;
    sal_Int32 m_nX;
    sal_Int32 m_nLeftFromText;
    sal_Int32 m_nRightFromText;
    sal_Int32 m_nTopFromText;
    sal_Int32 m_nBottomFromText;

    // Properties
    virtual void lcl_attribute(Id Name, Value& val) SAL_OVERRIDE;
    virtual void lcl_sprm(Sprm& sprm) SAL_OVERRIDE;

public:
    int getY() { return m_nY; }
    int getX() { return m_nX; }
    int getLeftFromText() { return m_nLeftFromText; }
    int getRightFromText() { return m_nRightFromText; }
    int getTopFromText() { return m_nTopFromText; }
    int getBottomFromText() { return m_nBottomFromText; }

    OUString getVertAnchor() { return m_aVertAnchor; }
    OUString getYSpec () { return m_aYSpec; }
    OUString getHorzAnchor () { return m_aHorzAnchor; }
    OUString getXSpec () { return m_aXSpec; }

    TablePositionHandler();
    virtual ~TablePositionHandler();

    /** Compute the UNO properties for the frame containing the table based
        on the received tokens.

        Note that the properties will need to be adjusted with the table
        properties before actually using them.
      */
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> getTablePosition() const;

    bool operator== (const TablePositionHandler& rHandler) const;
};

typedef boost::shared_ptr<TablePositionHandler> TablePositionHandlerPtr;
} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
