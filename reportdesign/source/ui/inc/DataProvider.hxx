/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:08:23 $
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

#ifndef RPT_DATAPROVIDER_HXX
#define RPT_DATAPROVIDER_HXX

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATAPROVIDER_HPP_
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XRANGEXMLCONVERSION_HPP_
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASOURCE_HPP_
#include <com/sun/star/chart2/data/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XTEXTUALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XNUMERICALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_DATASEQUENCEROLE_HPP_
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
// #ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
// #include <com/sun/star/lang/XUnoTunnel.hpp>
// #endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#include <map>
#include <list>

// DataProvider ==============================================================
class DatabaseDataProvider : public
                ::cppu::WeakImplHelper3<
                    ::com::sun::star::chart2::data::XDataProvider,
                    ::com::sun::star::chart2::data::XRangeXMLConversion,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit DatabaseDataProvider( ScDocument* pDoc );
    virtual ~DatabaseDataProvider();

private:
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataProvider ---------------------------------------------------------

    virtual ::sal_Bool SAL_CALL createDataSourcePossible(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSource > SAL_CALL createDataSource(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > SAL_CALL detectArguments(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource )
            throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const ::rtl::OUString& aRangeRepresentation )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
            const ::rtl::OUString& aRangeRepresentation )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelection > SAL_CALL getRangeSelection()
        throw (::com::sun::star::uno::RuntimeException);

/*    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > SAL_CALL getNumberFormatsSupplier()
        throw (::com::sun::star::uno::RuntimeException);*/

    // XRangeXMLConversion ---------------------------------------------------

    virtual ::rtl::OUString SAL_CALL convertRangeToXML( const ::rtl::OUString& sRangeRepresentation )
        throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual ::rtl::OUString SAL_CALL convertRangeFromXML( const ::rtl::OUString& sXMLRange )
        throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);
private:
};


// DataSource ================================================================

class ScChart2DataSource : public
                ::cppu::WeakImplHelper2<
                    ::com::sun::star::chart2::data::XDataSource,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataSource( ScDocument* pDoc);
    virtual ~ScChart2DataSource();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataSource -----------------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL
        getDataSequences() throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

    // implementation

    void AddLabeledSequence(const com::sun::star::uno::Reference < com::sun::star::chart2::data::XLabeledDataSequence >& xNew);

private:

    ScDocument*                 m_pDocument;
    typedef std::list < com::sun::star::uno::Reference< com::sun::star::chart2::data::XLabeledDataSequence > >  LabeledList;
    LabeledList                 m_aLabeledSequences;

};


// LabeledDataSequence =======================================================

class ScChart2LabeledDataSequence : public
                ::cppu::WeakImplHelper4<
                    ::com::sun::star::chart2::data::XLabeledDataSequence,
                    ::com::sun::star::util::XCloneable,
                    ::com::sun::star::util::XModifyBroadcaster,
                    ::com::sun::star::lang::XServiceInfo >,
                SfxListener
{
public:

    explicit ScChart2LabeledDataSequence( ScDocument* pDoc );
    virtual ~ScChart2LabeledDataSequence();

    // SfxListener -----------------------------------------------------------

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XLabeledDataSequence --------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL getValues()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setValues(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSequence )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL getLabel()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLabel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xSequence )
        throw (::com::sun::star::uno::RuntimeException);

    // XCloneable ------------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster ----------------------------------------------------

    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >   m_aData;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >   m_aLabel;
    ScDocument*                                               m_pDocument;
};

// DataSequence ==============================================================

class ScChart2DataSequence : public
                ::cppu::WeakImplHelper7<
                    ::com::sun::star::chart2::data::XDataSequence,
                    ::com::sun::star::chart2::data::XTextualDataSequence,
                    ::com::sun::star::chart2::data::XNumericalDataSequence,
                    ::com::sun::star::util::XCloneable,
                    ::com::sun::star::util::XModifyBroadcaster,
                    ::com::sun::star::beans::XPropertySet,
//                     ::com::sun::star::lang::XUnoTunnel,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataSequence( ScDocument* pDoc,
            const com::sun::star::uno::Reference< com::sun::star::chart2::data::XDataProvider >& xDP,
            const ScRangeListRef& rRangeList );
    virtual ~ScChart2DataSequence();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataSequence ---------------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL getData() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSourceRangeRepresentation()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
        SAL_CALL generateLabel(::com::sun::star::chart2::data::LabelOrigin nOrigin)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // XNumericalDataSequence --------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< double >
        SAL_CALL getNumericalData(  ) throw (::com::sun::star::uno::RuntimeException);

    // XTextualDataSequence --------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
        SAL_CALL getTextualData(  ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet ----------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValue(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const ::rtl::OUString& rPropertyName)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XCloneable ------------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster ----------------------------------------------------

    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel ------------------------------------------------------------

//  virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
//                                  sal_Int8 >& aIdentifier )
//                              throw(::com::sun::star::uno::RuntimeException);

//  static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
//  static ScChart2DataSequence* getImplementation( const com::sun::star::uno::Reference<
//                                  com::sun::star::uno::XInterface> xObj );

    // Implementation --------------------------------------------------------

    ScRangeListRef GetRangeList() { return m_xRanges; }

    void    RefChanged();
    DECL_LINK( ValueListenerHdl, SfxHint* );

private:

    // properties
    ::com::sun::star::chart2::data::DataSequenceRole  m_aRole;
    sal_Bool                    m_bHidden;
    // internals
    ScRangeListRef              m_xRanges;
    sal_Int64                   m_nObjectId;
    ScDocument*                 m_pDocument;
    com::sun::star::uno::Reference < com::sun::star::chart2::data::XDataProvider > m_xDataProvider;
    SfxItemPropertySet          m_aPropSet;

    ScLinkListener*             m_pValueListener;
    sal_Bool                    m_bGotDataChangedHint;
    XModifyListenerArr_Impl     m_aValueListeners;
};

// DataSequence ==============================================================

class ScChart2EmptyDataSequence : public
                ::cppu::WeakImplHelper6<
                    ::com::sun::star::chart2::data::XDataSequence,
                    ::com::sun::star::chart2::data::XTextualDataSequence,
                    ::com::sun::star::util::XCloneable,
                    ::com::sun::star::util::XModifyBroadcaster,
                    ::com::sun::star::beans::XPropertySet,
//                     ::com::sun::star::lang::XUnoTunnel,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2EmptyDataSequence( ScDocument* pDoc,
            const com::sun::star::uno::Reference< com::sun::star::chart2::data::XDataProvider >& xDP,
            const ScRangeListRef& rRangeList, sal_Bool bColumn );
    virtual ~ScChart2EmptyDataSequence();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XDataSequence ---------------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL getData() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSourceRangeRepresentation()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
        SAL_CALL generateLabel(::com::sun::star::chart2::data::LabelOrigin nOrigin)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // XTextualDataSequence --------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
        SAL_CALL getTextualData(  ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet ----------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setPropertyValue(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const ::rtl::OUString& rPropertyName)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener(
            const ::rtl::OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XCloneable ------------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster ----------------------------------------------------

    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo ----------------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel ------------------------------------------------------------

//  virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
//                                  sal_Int8 >& aIdentifier )
//                              throw(::com::sun::star::uno::RuntimeException);

//  static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
//  static ScChart2DataSequence* getImplementation( const com::sun::star::uno::Reference<
//                                  com::sun::star::uno::XInterface> xObj );

    // Implementation --------------------------------------------------------

    ScRangeListRef GetRangeList() { return m_xRanges; }

private:

    // properties
    ::com::sun::star::chart2::data::DataSequenceRole  m_aRole;
    sal_Bool                    m_bHidden;
    // internals
    ScRangeListRef              m_xRanges;
    ScDocument*                 m_pDocument;
    com::sun::star::uno::Reference < com::sun::star::chart2::data::XDataProvider > m_xDataProvider;
    SfxItemPropertySet          m_aPropSet;
    sal_Bool                    m_bColumn; // defines the orientation to create the right labels

};

#endif // RPT_DATAPROVIDER_HXX
