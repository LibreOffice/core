/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectHierarchy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:54:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_OBJECTHIERARCHY_HXX
#define CHART2_OBJECTHIERARCHY_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

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
    typedef ::rtl::OUString tCID;
    typedef ::std::vector< tCID > tChildContainer;

    /** @param bFlattenDiagram
            If <TRUE/>, the content of the diaram (data series, wall, floor,
            etc.) is treated as being at the same level as the diagram. (This is
            used for keyboard navigation).
     */
    explicit ObjectHierarchy(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        ExplicitValueProvider * pExplicitValueProvider = 0,
        bool bFlattenDiagram = false );
    ~ObjectHierarchy();

    static tCID      getRootNodeCID();
    static bool      isRootNode( const tCID & rCID );

    /// equal to getChildren( getRootNodeCID())
    tChildContainer  getTopLevelChildren() const;
    bool             hasChildren( const tCID & rParent ) const;
    tChildContainer  getChildren( const tCID & rParent ) const;

    tChildContainer  getSiblings( const tCID & rNode ) const;

    /// The result is empty, if the node cannot be found in the tree
    tCID             getParent( const tCID & rNode ) const;
    /// @returns -1, if no parent can be determined
    sal_Int32        getIndexInParent( const tCID & rNode ) const;

private:

    ::std::auto_ptr< impl::ImplObjectHierarchy > m_apImpl;
};

class ObjectKeyNavigation
{
public:
    explicit ObjectKeyNavigation( const ObjectHierarchy::tCID & rCurrentCID,
                                  const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart2::XChartDocument > & xChartDocument,
                                  ExplicitValueProvider * pExplicitValueProvider = 0 );

    bool handleKeyEvent( const ::com::sun::star::awt::KeyEvent & rEvent );
    ObjectHierarchy::tCID getCurrentSelection() const;

private:
    void setCurrentSelection( const ObjectHierarchy::tCID & rCID );
    bool first();
    bool last();
    bool next();
    bool previous();
    bool up();
    bool down();
    bool veryFirst();
    bool veryLast();

    ObjectHierarchy::tCID m_aCurrentCID;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
    ExplicitValueProvider * m_pExplicitValueProvider;
    bool m_bStepDownInDiagram;
};

} //  namespace chart

// CHART2_OBJECTHIERARCHY_HXX
#endif
