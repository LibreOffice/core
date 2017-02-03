/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WrappedGL3DProperties.hxx"
#include "Chart2ModelContact.hxx"
#include "FastPropertyIdRanges.hxx"
#include <unonames.hxx>
#include <WrappedProperty.hxx>
#include <DiagramHelper.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>

using namespace com::sun::star;

namespace chart { namespace wrapper {

namespace {

enum
{
    PROP_GL3DCHARTTYPE_ROUNDED_EDGE = FAST_PROPERTY_ID_START_GL_3D
};

class WrappedGL3DProperty : public WrappedProperty
{
    uno::Any maDefault;
    std::shared_ptr<Chart2ModelContact> mpModelContact;

private:
    uno::Reference<chart2::XChartType> getChartType() const
    {
        uno::Reference<chart2::XDiagram> xDiagram = mpModelContact->getChart2Diagram();
        uno::Sequence<uno::Reference<chart2::XChartType> > aCTs =
            DiagramHelper::getChartTypesFromDiagram(xDiagram);

        for (sal_Int32 i = 0; i < aCTs.getLength(); ++i)
        {
            uno::Reference<chart2::XChartType> xThisCT = aCTs[i];
            if (xThisCT->getChartType() == "com.sun.star.chart2.GL3DBarChartType")
                // Found the right chart type.
                return xThisCT;
        }

        return uno::Reference<chart2::XChartType>();
    }

public:
    WrappedGL3DProperty( const OUString& rInName, const OUString& rOutName, const uno::Any& rDefault, const std::shared_ptr<Chart2ModelContact>& pContact ) :
        WrappedProperty(rInName, rOutName), maDefault(rDefault), mpModelContact(pContact) {}

    virtual uno::Any getPropertyValue( const uno::Reference<beans::XPropertySet>& /*xInnerPS*/ ) const override
    {
        uno::Reference<chart2::XChartType> xCT = getChartType();
        if (!xCT.is())
            return uno::Any();

        try
        {
            uno::Reference<beans::XPropertySet> xPS(xCT, uno::UNO_QUERY_THROW);
            return xPS->getPropertyValue(CHART_UNONAME_ROUNDED_EDGE);
        }
        catch ( const uno::Exception& )  {}

        return uno::Any();
    };

    virtual void setPropertyValue(
        const uno::Any& rOutValue, const uno::Reference<beans::XPropertySet>& /*xInnerPS*/ ) const override
    {
        uno::Reference<chart2::XChartType> xCT = getChartType();
        if (!xCT.is())
            return;

        try
        {
            uno::Reference<beans::XPropertySet> xPS(xCT, uno::UNO_QUERY_THROW);
            return xPS->setPropertyValue(CHART_UNONAME_ROUNDED_EDGE, rOutValue);
        }
        catch ( const uno::Exception& )  {}
    }

    virtual void setPropertyToDefault( const uno::Reference<beans::XPropertyState>& /*xInnerPropState*/ ) const override
    {
        uno::Reference<chart2::XChartType> xCT = getChartType();
        if (!xCT.is())
            return;

        try
        {
            uno::Reference<beans::XPropertySet> xPS(xCT, uno::UNO_QUERY_THROW);
            return xPS->setPropertyValue(CHART_UNONAME_ROUNDED_EDGE, maDefault);
        }
        catch ( const uno::Exception& )  {}
    }

    virtual uno::Any getPropertyDefault( const uno::Reference<beans::XPropertyState>& /*xInnerPS*/ ) const override
    {
        return maDefault;
    }

    virtual beans::PropertyState getPropertyState( const uno::Reference<beans::XPropertyState>& /*xInnerPS*/ ) const override
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
};

}

void WrappedGL3DProperties::addProperties( std::vector<css::beans::Property> & rOutProps )
{
    rOutProps.push_back(
        beans::Property(
            CHART_UNONAME_ROUNDED_EDGE,
            PROP_GL3DCHARTTYPE_ROUNDED_EDGE,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT
        )
    );
}

void WrappedGL3DProperties::addWrappedProperties(
    std::vector<WrappedProperty*>& rList, const std::shared_ptr<Chart2ModelContact>& pChart2ModelContact )
{
    rList.push_back(
        new WrappedGL3DProperty(
            CHART_UNONAME_ROUNDED_EDGE, CHART_UNONAME_ROUNDED_EDGE, uno::Any(false), pChart2ModelContact));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
