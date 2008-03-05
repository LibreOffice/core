/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CachedDataSequence.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:14:13 $
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
#ifndef _CHART_CACHEDDATASEQUENCE_HXX
#define _CHART_CACHEDDATASEQUENCE_HXX

// helper classes
#ifndef _CPPUHELPER_COMPBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#include "ServiceMacros.hxx"

// interfaces and types
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#include <com/sun/star/lang/XInitialization.hpp>
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XNUMERICALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XTEXTUALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif

#include <vector>

// ____________________
namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper7<
    ::com::sun::star::chart2::data::XDataSequence,
    ::com::sun::star::chart2::data::XNumericalDataSequence,
    ::com::sun::star::chart2::data::XTextualDataSequence,
    ::com::sun::star::util::XCloneable,
    ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::lang::XServiceInfo >
    CachedDataSequence_Base;
}

class CachedDataSequence :
        public ::comphelper::OMutexAndBroadcastHelper,
        public ::comphelper::OPropertyContainer,
        public ::comphelper::OPropertyArrayUsageHelper< CachedDataSequence >,
        public impl::CachedDataSequence_Base
{
public:
    /** constructs an empty sequence
     */
    CachedDataSequence();

    explicit CachedDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

    /** creates a sequence and initializes it with the given vector of floating
        point numbers
     */
    explicit CachedDataSequence( const ::std::vector< double > & rVector );

    /** creates a sequence and initializes it with the given vector of strings
     */
    explicit CachedDataSequence( const ::std::vector< ::rtl::OUString > & rVector );

    /** creates a sequence and initializes it with the given string.  This is
        especially useful for labels, which only have one element.
     */
    explicit CachedDataSequence( const ::rtl::OUString & rSingleText );

    /** creates a sequence and initializes it with the given vector of arbitrary
        content
     */
    explicit CachedDataSequence( const ::std::vector< ::com::sun::star::uno::Any > & rVector );

    /// Copy CTOR
    explicit CachedDataSequence( const CachedDataSequence & rSource );

    virtual ~CachedDataSequence();

    /** optimizes internal storage such that getData() is the fastest method for
        retrieving data.
    */
    void PreferMixedData();
    /** optimizes internal storage such that getNumericalData() is the fastest
        method for retrieving data.
     */
    void PreferNumericalData();
    /** optimizes internal storage such that getTextualData() is the fastest
        method for retrieving data.
     */
    void PreferTextualData();

    /** Allows setting the source-identifier.  This should only be called once.
        (Late initialization)
     */
//     void SetSourceIdentifier( const ::rtl::OUString & aId );

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( CachedDataSequence )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    // ____ XPropertySet ____
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

    // ____ XDataSequence ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSourceRangeRepresentation()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL generateLabel(
        ::com::sun::star::chart2::data::LabelOrigin nLabelOrigin )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XNumericalDataSequence ____
    /// @see ::com::sun::star::chart::data::XNumericalDataSequence
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getNumericalData() throw (::com::sun::star::uno::RuntimeException);

    // ____ XTextualDataSequence ____
    /// @see ::com::sun::star::chart::data::XTextualDataSequence
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getTextualData() throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments)
        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

    void fireModifyEvent();

    // <properties>
    sal_Int32                                       m_nNumberFormatKey;
    ::rtl::OUString                                 m_sRole;
    sal_Bool                                        m_bIsHidden;
    ::com::sun::star::uno::Sequence< sal_Int32 >    m_aHiddenValues;
    // </properties>

    enum DataType
    {
        NUMERICAL,
        TEXTUAL,
        MIXED
    };

    /** This method registers all properties.  It should be called by all
        constructors.
     */
    void registerProperties();

    /** is used by interface method getNumericalData() as well as
        PreferNumericalData().
     */
    ::com::sun::star::uno::Sequence< double > Impl_getNumericalData() const;
    /** is used by interface method getTextualData() as well as
        PreferTextualData().
     */
    ::com::sun::star::uno::Sequence< ::rtl::OUString > Impl_getTextualData() const;
    /** is used by interface method getData() as well as PreferMixedData().
     */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > Impl_getMixedData() const;

    /** is used by PreferNumericalData().
     */
    void Impl_setNumericalData( const ::com::sun::star::uno::Sequence< double > & rSeq );
    /** is used by PreferTextualData().
     */
    void Impl_setTextualData( const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rSeq );
    /** is used by PreferMixedData().
     */
    void Impl_setMixedData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & rSeq );

private:
    enum DataType                                       m_eCurrentDataType;

    ::com::sun::star::uno::Sequence< double >           m_aNumericalSequence;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTextualSequence;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >                    m_aMixedSequence;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >
        m_xModifyEventForwarder;
};

}  // namespace chart


// _CHART_CACHEDDATASEQUENCE_HXX
#endif
