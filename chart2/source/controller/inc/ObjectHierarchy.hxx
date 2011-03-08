/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_OBJECTHIERARCHY_HXX
#define CHART2_OBJECTHIERARCHY_HXX

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
            If <TRUE/>, the content of the diaram (data series, wall, floor,
            etc.) is treated as being at the same level as the diagram. (This is
            used for keyboard navigation).
     */
    explicit ObjectHierarchy(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        ExplicitValueProvider * pExplicitValueProvider = 0,
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

    ::std::auto_ptr< impl::ImplObjectHierarchy > m_apImpl;
};

class ObjectKeyNavigation
{
public:
    explicit ObjectKeyNavigation( const ObjectHierarchy::tOID & rCurrentOID,
                                  const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart2::XChartDocument > & xChartDocument,
                                  ExplicitValueProvider * pExplicitValueProvider = 0 );

    bool handleKeyEvent( const ::com::sun::star::awt::KeyEvent & rEvent );
    ObjectHierarchy::tOID getCurrentSelection() const;

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

// CHART2_OBJECTHIERARCHY_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
