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
#pragma once

#include <ObjectIdentifier.hxx>

namespace com::sun::star::awt { struct KeyEvent; }
namespace com::sun::star::chart2 { class XChartDocument; }

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
    typedef std::vector< ObjectIdentifier > tChildContainer;

    /** @param bFlattenDiagram
            If <TRUE/>, the content of the diagram (data series, wall, floor,
            etc.) is treated as being at the same level as the diagram. (This is
            used for keyboard navigation).
     */
    explicit ObjectHierarchy(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument,
        ExplicitValueProvider * pExplicitValueProvider,
        bool bFlattenDiagram = false,
        bool bOrderingForElementSelector = false );
    ~ObjectHierarchy();

    static ObjectIdentifier      getRootNodeOID();
    static bool      isRootNode( const ObjectIdentifier& rOID );

    /// equal to getChildren( getRootNodeOID())
    tChildContainer  getTopLevelChildren() const;
    bool             hasChildren( const ObjectIdentifier& rParent ) const;
    tChildContainer  getChildren( const ObjectIdentifier& rParent ) const;

    tChildContainer  getSiblings( const ObjectIdentifier& rNode ) const;

    /// The result is empty, if the node cannot be found in the tree
    ObjectIdentifier             getParent( const ObjectIdentifier& rNode ) const;
    /// @returns -1, if no parent can be determined
    sal_Int32        getIndexInParent( const ObjectIdentifier& rNode ) const;

private:

    std::unique_ptr< impl::ImplObjectHierarchy > m_apImpl;
};

class ObjectKeyNavigation
{
public:
    explicit ObjectKeyNavigation( const ObjectIdentifier & rCurrentOID,
                                  const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument,
                                  ExplicitValueProvider * pExplicitValueProvider );

    bool handleKeyEvent( const css::awt::KeyEvent & rEvent );
    const ObjectIdentifier& getCurrentSelection() const { return m_aCurrentOID;}

private:
    void setCurrentSelection( const ObjectIdentifier& rOID );
    bool first();
    bool last();
    bool next();
    bool previous();
    bool up();
    bool down();
    bool veryFirst();
    bool veryLast();

    ObjectIdentifier m_aCurrentOID;
    css::uno::Reference< css::chart2::XChartDocument > m_xChartDocument;
    ExplicitValueProvider * m_pExplicitValueProvider;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
