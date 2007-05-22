/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DialogModel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:28:19 $
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
#ifndef CHART2_DIALOGMODEL_HXX
#define CHART2_DIALOGMODEL_HXX

#include "TimerTriggeredControllerLock.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star { namespace chart2 {
    class XDataSeriesContainer;
    class XDataSeries;
    class XChartType;
    class XChartTypeTemplate;
    struct InterpretedData;
    namespace data {
        class XDataProvider;
        class XLabeledDataSequence;
    }
}}}}

namespace chart
{

class RangeSelectionHelper;

class DialogModel
{
public:
    explicit DialogModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    ~DialogModel();

    typedef ::std::pair<
                ::rtl::OUString,
                ::std::pair< ::com::sun::star::uno::Reference<
                                 ::com::sun::star::chart2::XDataSeries >,
                             ::com::sun::star::uno::Reference<
                                 ::com::sun::star::chart2::XChartType > > >
        tSeriesWithChartTypeByName;

    typedef ::std::map< ::rtl::OUString, ::rtl::OUString >
        tRolesWithRanges;

    void setTemplate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartTypeTemplate > & xTemplate );

    ::boost::shared_ptr< RangeSelectionHelper >
        getRangeSelectionHelper() const throw();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >
        getChartModel() const throw();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataProvider >
        getDataProvider() const throw();

    ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeriesContainer > >
        getAllDataSeriesContainers() const throw();

    ::std::vector< tSeriesWithChartTypeByName >
        getAllDataSeriesWithLabel() const throw();

    tRolesWithRanges getRolesWithRanges(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::rtl::OUString & aRoleOfSequenceForLabel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType > & xChartType );

    enum eMoveDirection
    {
        MOVE_DOWN,
        MOVE_UP
    };

    void moveSeries( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::chart2::XDataSeries > & xSeries,
                     eMoveDirection eDirection );

    /// @return the newly inserted series
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > insertSeriesAfter(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDataSeries > & xSeries,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XChartType > & xChartType,
                bool bCreateDataCachedSequences = false ) throw();

    void deleteSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType > & xChartType ) throw();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence >
        getCategories() const throw();

    void setCategories( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::data::XLabeledDataSequence > & xCategories );

    ::rtl::OUString getCategoriesRange() const throw();

    bool isCategoryDiagram() const throw();

    void detectArguments(
        ::rtl::OUString & rOutRangeString,
        bool & rOutUseColumns, bool & rOutFirstCellAsLabel, bool & rOutHasCategories ) const throw();

    bool allArgumentsForRectRangeDetected() const;

    bool setData( const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::beans::PropertyValue > & rArguments ) throw();

    void startControllerLockTimer();

    /** Applies the content of xSource to xDestination.  As a result
        xDestination is a copy of xSource, but maintains its identity.
        (Something like a flat assignment operator)
     */
    static void restoreModel(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xSource,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xDestination );

    static ::rtl::OUString ConvertRoleFromInternalToUI( const ::rtl::OUString & rRoleString );
    static ::rtl::OUString ConvertRoleFromUIToInternal( const ::rtl::OUString & rRoleString );
    static ::rtl::OUString GetRoleDataLabel();

    // pass a role string (not translated) and get an index that serves for
    // relative ordering, to get e.g. x-values and y-values in the right order
    static sal_Int32 GetRoleIndexForSorting( const ::rtl::OUString & rInternalRoleString );

    static bool isSeriesValid(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::rtl::OUString & aRoleOfSequenceForLabel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType > & xChartType );

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >
        m_xChartDocument;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >
        m_xBackupChartDocument;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartTypeTemplate >
        m_xTemplate;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext >
        m_xContext;

    mutable ::boost::shared_ptr< RangeSelectionHelper >
        m_spRangeSelectionHelper;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

private:
    void createBackup();

    void applyInterpretedData(
        const ::com::sun::star::chart2::InterpretedData & rNewData,
        const ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > > & rSeriesToReUse,
        bool bSetStyles );

    sal_Int32 countSeries() const;
};

} //  namespace chart

// CHART2_DIALOGMODEL_HXX
#endif
