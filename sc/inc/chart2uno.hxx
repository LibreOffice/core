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

#ifndef INCLUDED_SC_INC_CHART2UNO_HXX
#define INCLUDED_SC_INC_CHART2UNO_HXX

#include "cellsuno.hxx"
#include "rangelst.hxx"
#include "externalrefmgr.hxx"
#include "types.hxx"
#include "chartlis.hxx"

#include <svl/lstner.hxx>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XSheetDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/DataSequenceRole.hpp>
#include <com/sun/star/chart2/XTimeBased.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase8.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>

#include <list>
#include <unordered_set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class ScDocument;

// DataProvider
class ScChart2DataProvider : public
                ::cppu::WeakImplHelper5<
                    ::com::sun::star::chart2::data::XDataProvider,
                    ::com::sun::star::chart2::data::XSheetDataProvider,
                    ::com::sun::star::chart2::data::XRangeXMLConversion,
                    ::com::sun::star::beans::XPropertySet,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:

    explicit ScChart2DataProvider( ScDocument* pDoc );
    virtual ~ScChart2DataProvider();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    // XDataProvider
    virtual sal_Bool SAL_CALL createDataSourcePossible(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSource > SAL_CALL createDataSource(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > SAL_CALL detectArguments(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
            const OUString& aRangeRepresentation )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray( const OUString& aRole, const OUString& aRangeRepresentation )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelection > SAL_CALL getRangeSelection()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSheetDataProvider
    virtual sal_Bool SAL_CALL createDataSequenceByFormulaTokensPossible(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& aTokens )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
        SAL_CALL createDataSequenceByFormulaTokens(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& aTokens )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XRangeXMLConversion
    virtual OUString SAL_CALL convertRangeToXML( const OUString& sRangeRepresentation )
        throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception ) SAL_OVERRIDE;

    virtual OUString SAL_CALL convertRangeFromXML( const OUString& sXMLRange )
        throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception ) SAL_OVERRIDE;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo> SAL_CALL
        getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& xListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XVetoableChangeListener>& rListener)
        throw( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:

    ScDocument*                 m_pDocument;
    SfxItemPropertySet          m_aPropSet;
    bool                        m_bIncludeHiddenCells;
};

#endif // INCLUDED_SC_INC_CHART2UNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
