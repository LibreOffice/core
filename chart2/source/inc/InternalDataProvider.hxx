/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InternalDataProvider.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 17:14:46 $
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
#ifndef CHART2_INTERNALDATAPROVIDER_HXX
#define CHART2_INTERNALDATAPROVIDER_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATAPROVIDER_HPP_
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XINTERNALDATAPROVIDER_HPP_
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XRANGEXMLCONVERSION_HPP_
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _APPHELPER_SERVICEMACROS_HXX
#include "ServiceMacros.hxx"
#endif

#include "UncachedDataSequence.hxx"

#include <map>
#include <memory>

namespace chart
{

namespace impl
{
class InternalData;

typedef ::cppu::WeakImplHelper5<
        ::com::sun::star::chart2::XInternalDataProvider,
        ::com::sun::star::chart2::data::XRangeXMLConversion,
        ::com::sun::star::chart::XChartDataArray,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::lang::XServiceInfo >
    InternalDataProvider_Base;
}

/** Data provider that handles data internally.  This is used for charts with
    their own data.

    <p>The format for single ranges is "categories|label n|n" where n is a
    non-negative number. Meaning return all categories, the label of sequence n,
    or the data of sequence n.</p>

    <p>The format for a complete range is "all". (Do we need more than
    that?)</p>
 */
class InternalDataProvider :
        public impl::InternalDataProvider_Base
{
public:
    explicit InternalDataProvider();
    /// sets the internal data to the given data
    explicit InternalDataProvider(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XChartDataArray > & xDataToCopy );
    /// copies the data from the given data provider for all given used ranges
    explicit InternalDataProvider( const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::chart2::XChartDocument > & xChartDoc );
    // copy-CTOR
    explicit InternalDataProvider( const InternalDataProvider & rOther );
    virtual ~InternalDataProvider();

    void createDefaultData();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XInternalDataProvider ____
    virtual ::sal_Bool SAL_CALL hasDataByRangeRepresentation( const ::rtl::OUString& aRange )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
        getDataByRangeRepresentation( const ::rtl::OUString& aRange )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataByRangeRepresentation(
        const ::rtl::OUString& aRange,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aNewData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertSequence( ::sal_Int32 nAfterIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteSequence( ::sal_Int32 nAtIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertDataPointForAllSequences( ::sal_Int32 nAfterIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteDataPointForAllSequences( ::sal_Int32 nAtIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerDataSequenceForChanges(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSeq )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataProvider (base of XInternalDataProvider) ____
    virtual ::sal_Bool SAL_CALL createDataSourcePossible(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > SAL_CALL createDataSource(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL detectArguments(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const ::rtl::OUString& aRangeRepresentation )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
        const ::rtl::OUString& aRangeRepresentation )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelection > SAL_CALL getRangeSelection()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XRangeXMLConversion ____
    virtual ::rtl::OUString SAL_CALL convertRangeToXML(
        const ::rtl::OUString& aRangeRepresentation )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL convertRangeFromXML(
        const ::rtl::OUString& aXMLRange )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XChartDataArray ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setData(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRowDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getColumnDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnDescriptions(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartDataChangeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isNotANumber(
        double nNumber )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

private:
    impl::InternalData &  getInternalData();
    const impl::InternalData &  getInternalData() const;

    void addDataSequenceToMap(
        const ::rtl::OUString & rRangeRepresentation,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence > & xSequence );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >
        createDataSequenceAndAddToMap( const ::rtl::OUString & rRangeRepresentation,
                                       const ::rtl::OUString & rRole );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >
        createDataSequenceAndAddToMap( const ::rtl::OUString & rRangeRepresentation );
    void deleteMapReferences( const ::rtl::OUString & rRangeRepresentation );
    void adaptMapReferences(
        const ::rtl::OUString & rOldRangeRepresentation,
        const ::rtl::OUString & rNewRangeRepresentation );
    void increaseMapReferences( sal_Int32 nBegin, sal_Int32 nEnd );
    void decreaseMapReferences( sal_Int32 nBegin, sal_Int32 nEnd );

    typedef ::std::multimap< ::rtl::OUString,
            ::com::sun::star::uno::WeakReference< ::com::sun::star::chart2::data::XDataSequence > >
        tSequenceMap;
    typedef ::std::pair< tSequenceMap::iterator, tSequenceMap::iterator > tSequenceMapRange;
    typedef ::std::pair< tSequenceMap::const_iterator, tSequenceMap::const_iterator > tConstSequenceMapRange;

    /** cache for all sequences that have been returned.

        If the range-representation of a sequence changes and it is still
        referred to by some component (weak reference is valid), the range will
        be adapted.
     */
    mutable tSequenceMap m_aSequenceMap;
    mutable ::std::auto_ptr< impl::InternalData > m_apData;
    bool m_bDataInColumns;
};

} //  namespace chart

// CHART2_INTERNALDATAPROVIDER_HXX
#endif
