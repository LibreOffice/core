/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Chart2ModelContact.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:16:44 $
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
#ifndef CHART_CHART2MODELCONTACT_HXX
#define CHART_CHART2MODELCONTACT_HXX

#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLE_HPP_
#include <com/sun/star/chart2/XTitle.hpp>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <map>

namespace chart
{
class ExplicitValueProvider;

namespace wrapper
{

class Chart2ModelContact
{
public:
    Chart2ModelContact( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~Chart2ModelContact();

public:
    void setModel( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xChartModel );
    void clear();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel > getChartModel() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument > getChart2Document() const;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram > getChart2Diagram() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > getDrawPage();

    /** get the current values calculated for an axis in the current view in
        case properties are 'auto'.
     */
    sal_Bool getExplicitValuesForAxis(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis > & xAxis,
        ::com::sun::star::chart2::ExplicitScaleData &  rOutExplicitScale,
        ::com::sun::star::chart2::ExplicitIncrementData & rOutExplicitIncrement );

    sal_Int32 getExplicitNumberFormatKeyForAxis(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );

    /** returns an element with name rElementName found in the list corresponding to the type PropertyName
    (e.g. if rTableTypePropertyName == "FillGradientName" and rElementName == "Gradient 3", then this method will return the corresponding gradient )
    */
    ::com::sun::star::uno::Any GetListElementByName(
        const ::rtl::OUString & rElementName, const ::rtl::OUString & rTableTypePropertyName );

    /** search the list corresponding to rTableTypePropertyName for an element that has the same values as rElement.
    returns the found name.
    */
    ::rtl::OUString GetNameOfListElement(
        const ::com::sun::star::uno::Any& rElementValue, const ::rtl::OUString & rTableTypePropertyName );


    /** Returns the size of the page in logic coordinates.  This value is used
        for setting an appropriate "ReferencePageSize" for FontHeights.
     */
    ::com::sun::star::awt::Size GetPageSize() const;

    /** Returns the size of the diagram object in logic coordinates.  This value
        is used for setting an appropriate "ReferenceDiagramSize" for FontHeights.
     */
    ::com::sun::star::awt::Size GetDiagramSize() const;

    /** Returns the size of the diagram object in logic coordinates inclusive
        the space reserved for axis titles.
     */
    ::com::sun::star::awt::Size GetDiagramSizeInclusive() const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetDiagramPosition() const;

    /** Returns the position of the diagram in logic coordinates inclusive
        the space reserved for axis titles.
     */
    ::com::sun::star::awt::Point GetDiagramPositionInclusive() const;

    /** Returns the size of the object in logic coordinates.
     */
    ::com::sun::star::awt::Size GetLegendSize() const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetLegendPosition() const;

    /** Returns the size of the object in logic coordinates.
     */
    ::com::sun::star::awt::Size GetTitleSize( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XTitle > & xTitle ) const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetTitlePosition( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XTitle > & xTitle ) const;


    /** Returns the size of the object in logic coordinates.
     */
    ::com::sun::star::awt::Size GetAxisSize( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XAxis > & xAxis ) const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetAxisPosition( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XAxis > & xAxis ) const;

private: //methods
    ExplicitValueProvider* getExplicitValueProvider() const;
    ::com::sun::star::awt::Rectangle GetDiagramRectangleInclusive() const;

public: //member
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

private: //member
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XModel >   m_xChartModel;

    mutable ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XUnoTunnel >        m_xChartView;

    typedef std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > > tTableMap;//GradientTable, HatchTable etc.
    tTableMap   m_aTableMap;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHART2MODELCONTACT_HXX
#endif
