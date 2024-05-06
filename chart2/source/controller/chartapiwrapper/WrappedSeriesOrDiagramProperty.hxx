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

#include <WrappedProperty.hxx>
#include "Chart2ModelContact.hxx"
#include <DataSeries.hxx>

#include <memory>
#include <utility>
#include <vector>

namespace com::sun::star::chart2 { class XDataSeries; }

namespace chart::wrapper
{

enum tSeriesOrDiagramPropertyType
{
    DATA_SERIES,
    DIAGRAM
};

//PROPERTYTYPE is the type of the outer property

template< typename PROPERTYTYPE >
class WrappedSeriesOrDiagramProperty : public WrappedProperty
{
public:
    virtual PROPERTYTYPE getValueFromSeries( const css::uno::Reference< css::beans::XPropertySet >& xSeriesPropertySet ) const =0;
    virtual void setValueToSeries( const css::uno::Reference< css::beans::XPropertySet >& xSeriesPropertySet, const PROPERTYTYPE & aNewValue ) const =0;

    explicit WrappedSeriesOrDiagramProperty( const OUString& rName, const css::uno::Any& rDefaulValue
        , std::shared_ptr<Chart2ModelContact> spChart2ModelContact
        , tSeriesOrDiagramPropertyType ePropertyType )
            : WrappedProperty(rName,OUString())
            , m_spChart2ModelContact(std::move(spChart2ModelContact))
            , m_aOuterValue(rDefaulValue)
            , m_aDefaultValue(rDefaulValue)
            , m_ePropertyType( ePropertyType )
    {
    }

    bool detectInnerValue( PROPERTYTYPE& rValue, bool& rHasAmbiguousValue ) const
    {
        rHasAmbiguousValue = false;
        if( m_ePropertyType != DIAGRAM || !m_spChart2ModelContact )
            return false;
        bool bHasDetectableInnerValue = false;
        rtl::Reference<Diagram> xDiagram = m_spChart2ModelContact->getDiagram();
        if (!xDiagram)
            return false;
        std::vector< rtl::Reference< DataSeries > > aSeriesVector =
            xDiagram->getDataSeries();
        for (auto const& series : aSeriesVector)
        {
            PROPERTYTYPE aCurValue = getValueFromSeries( series );
            if( !bHasDetectableInnerValue )
                rValue = aCurValue;
            else
            {
                if( rValue != aCurValue )
                {
                    rHasAmbiguousValue = true;
                    break;
                }
                else
                    rValue = aCurValue;
            }
            bHasDetectableInnerValue = true;
        }
        return bHasDetectableInnerValue;
    }
    void setInnerValue( PROPERTYTYPE aNewValue ) const
    {
        if( m_ePropertyType == DIAGRAM &&
            m_spChart2ModelContact )
        {
            std::vector< rtl::Reference< DataSeries > > aSeriesVector =
                m_spChart2ModelContact->getDiagram()->getDataSeries();
            for (auto const& series : aSeriesVector)
            {
                setValueToSeries( series, aNewValue );
            }
        }
    }
    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override
    {
        PROPERTYTYPE aNewValue = PROPERTYTYPE();
        if( ! (rOuterValue >>= aNewValue) )
            throw css::lang::IllegalArgumentException( u"statistic property requires different type"_ustr, nullptr, 0 );

        if( m_ePropertyType == DIAGRAM )
        {
            m_aOuterValue = rOuterValue;

            bool bHasAmbiguousValue = false;
            PROPERTYTYPE aOldValue = PROPERTYTYPE();
            if( detectInnerValue( aOldValue, bHasAmbiguousValue ) )
            {
                if( bHasAmbiguousValue || aNewValue != aOldValue )
                    setInnerValue( aNewValue );
            }
        }
        else
        {
            setValueToSeries( xInnerPropertySet, aNewValue );
        }
    }

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override
    {
        if( m_ePropertyType == DIAGRAM )
        {
            bool bHasAmbiguousValue = false;
            PROPERTYTYPE aValue = PROPERTYTYPE();
            if( detectInnerValue( aValue, bHasAmbiguousValue ) )
            {
                if(bHasAmbiguousValue)
                    m_aOuterValue = m_aDefaultValue;
                else
                    m_aOuterValue <<= aValue;
            }
            return m_aOuterValue;
        }
        else
        {
            css::uno::Any aRet( m_aDefaultValue );
            aRet <<= getValueFromSeries( xInnerPropertySet );
            return aRet;
        }
    }

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& /* xInnerPropertyState */ ) const override
    {
        return m_aDefaultValue;
    }

protected:
    std::shared_ptr< Chart2ModelContact >  m_spChart2ModelContact;
    mutable css::uno::Any         m_aOuterValue;
    css::uno::Any                 m_aDefaultValue;
    tSeriesOrDiagramPropertyType               m_ePropertyType;
};

} //namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
