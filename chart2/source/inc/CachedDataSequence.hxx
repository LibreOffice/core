/*************************************************************************
 *
 *  $RCSfile: CachedDataSequence.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART_CACHEDDATASEQUENCE_HXX
#define _CHART_CACHEDDATASEQUENCE_HXX

// helper classes
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

//
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XNUMERICALDATASEQUENCE_HPP_
#include <drafts/com/sun/star/chart2/XNumericalDataSequence.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTEXTUALDATASEQUENCE_HPP_
#include <drafts/com/sun/star/chart2/XTextualDataSequence.hpp>
#endif

#include <vector>

// ____________________
namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper3<
    ::drafts::com::sun::star::chart2::XNumericalDataSequence,
    ::drafts::com::sun::star::chart2::XTextualDataSequence,
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

    /** creates a sequence and initializes it with the given vector of floating
        point numbers
     */
    explicit CachedDataSequence( const ::std::vector< double > & rVector );

    /** creates a sequence and initializes it with the given vector of strings
     */
    explicit CachedDataSequence( const ::std::vector< ::rtl::OUString > & rVector );

    /** creates a sequence and initializes it with the given vector of arbitrary
        content
     */
    explicit CachedDataSequence( const ::std::vector< ::com::sun::star::uno::Any > & rVector );

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

    // ____ XNumericalDataSequence ____
    /// @see ::drafts::com::sun::star::chart::XNumericalDataSequence
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getNumericalData() throw (::com::sun::star::uno::RuntimeException);
    /// @see ::drafts::com::sun::star::chart::XNumericalDataSequence
//     virtual void SAL_CALL setNumericalData( const ::com::sun::star::uno::Sequence< double >& aData ) throw (::com::sun::star::uno::RuntimeException);

    // ____ XTextualDataSequence ____
    /// @see ::drafts::com::sun::star::chart::XTextualDataSequence
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getTextualData() throw (::com::sun::star::uno::RuntimeException);
    /// @see ::drafts::com::sun::star::chart::XNumericalDataSequence
//     virtual void SAL_CALL setTextualData( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aData ) throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSequence ____
    /// @see ::drafts::com::sun::star::chart::XDataSequence
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getData() throw (::com::sun::star::uno::RuntimeException);
    /// @see ::drafts::com::sun::star::chart::XDataSequence
    virtual ::rtl::OUString SAL_CALL getSourceIdentifier() throw (::com::sun::star::uno::RuntimeException);

    mutable ::osl::Mutex                  m_aMutex;

    // <properties>
    sal_Int32          m_nNumberFormatKey;
    ::rtl::OUString    m_sRole;
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
    ::rtl::OUString                                     m_sSourceIdentifier;

    enum DataType                                       m_eCurrentDataType;

    ::com::sun::star::uno::Sequence< double >           m_aNumericalSequence;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTextualSequence;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >                    m_aMixedSequence;

    sal_Bool                                            m_bIsHidden;
    ::com::sun::star::uno::Sequence< sal_Int32 >        m_aHiddenValues;
};

}  // namespace chart


// _CHART_CACHEDDATASEQUENCE_HXX
#endif
