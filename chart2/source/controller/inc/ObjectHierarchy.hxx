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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_OBJECTHIERARCHY_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_OBJECTHIERARCHY_HXX

#include "ObjectIdentifier.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>

#include <memory>
#include <vector>

namespace chart
{

class ExplicitValueProvider;

namespace impl
{
class ImplObjectHierarchy;
}

class ObjectHierarchy
{
public:
    typedef ObjectIdentifier tOID;
    typedef ::std::vector< tOID > tChildContainer;

    /** @param bFlattenDiagram
            If <TRUE/>, the content of the diagram (data series, wall, floor,
            etc.) is treated as being at the same level as the diagram. (This is
            used for keyboard navigation).
     */
    explicit ObjectHierarchy(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        ExplicitValueProvider * pExplicitValueProvider = nullptr,
        bool bFlattenDiagram = false,
        bool bOrderingForElementSelector = false );
    ~ObjectHierarchy();

    static tOID      getRootNodeOID();
    static bool      isRootNode( const tOID& rOID );

    /// equal to getChildren( getRootNodeOID())
    tChildContainer  getTopLevelChildren() const;
    bool             hasChildren( const tOID& rParent ) const;
    tChildContainer  getChildren( const tOID& rParent ) const;

    tChildContainer  getSiblings( const tOID& rNode ) const;

    /// The result is empty, if the node cannot be found in the tree
    tOID             getParent( const tOID& rNode ) const;
    /// @returns -1, if no parent can be determined
    sal_Int32        getIndexInParent( const tOID& rNode ) const;

private:

    ::std::unique_ptr< impl::ImplObjectHierarchy > m_apImpl;
};

class ObjectKeyNavigation
{
public:
    explicit ObjectKeyNavigation( const ObjectHierarchy::tOID & rCurrentOID,
                                  const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart2::XChartDocument > & xChartDocument,
                                  ExplicitValueProvider * pExplicitValueProvider = nullptr );

    bool handleKeyEvent( const ::com::sun::star::awt::KeyEvent & rEvent );
    ObjectHierarchy::tOID getCurrentSelection() const { return m_aCurrentOID;}

private:
    void setCurrentSelection( const ObjectHierarchy::tOID& rOID );
    bool first();
    bool last();
    bool next();
    bool previous();
    bool up();
    bool down();
    bool veryFirst();
    bool veryLast();

    ObjectHierarchy::tOID m_aCurrentOID;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
    ExplicitValueProvider * m_pExplicitValueProvider;
    bool m_bStepDownInDiagram;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_OBJECTHIERARCHY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
