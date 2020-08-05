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

#include <sal/config.h>

#include <map>

#include "ChangingResource.hxx"
#include "ThreeDHelper.hxx"

#include <com/sun/star/chart2/CurveStyle.hpp>
#include <vcl/weld.hxx>

namespace com::sun::star::beans
{
class XPropertySet;
}
namespace com::sun::star::chart2
{
class XChartDocument;
}
namespace com::sun::star::chart2
{
class XChartTypeTemplate;
}
namespace com::sun::star::lang
{
class XMultiServiceFactory;
}

class ValueSet;

namespace chart
{
enum GlobalStackMode
{
    GlobalStackMode_NONE,
    GlobalStackMode_STACK_Y,
    GlobalStackMode_STACK_Y_PERCENT,
    GlobalStackMode_STACK_Z
};

class ChartTypeParameter
{
public:
    ChartTypeParameter(sal_Int32 nSubTypeIndex, bool bXAxisWithValues = false, bool b3DLook = false,
                       GlobalStackMode eStackMode = GlobalStackMode_NONE, bool _bSymbols = true,
                       bool _bLines = true,
                       css::chart2::CurveStyle eCurveStyle = css::chart2::CurveStyle_LINES);
    ChartTypeParameter();

    bool mapsToSameService(const ChartTypeParameter& rParameter) const;
    bool mapsToSimilarService(const ChartTypeParameter& rParameter,
                              sal_Int32 nTheHigherTheLess) const;

    sal_Int32 nSubTypeIndex; //starting with 1

    bool bXAxisWithValues;
    bool b3DLook;
    bool bSymbols;
    bool bLines;

    GlobalStackMode eStackMode;
    css::chart2::CurveStyle eCurveStyle;

    sal_Int32 nCurveResolution;
    sal_Int32 nSplineOrder;

    sal_Int32 nGeometry3D;

    ThreeDLookScheme eThreeDLookScheme;
    bool bSortByXValues;

    bool mbRoundedEdge;
};

typedef std::map<OUString, ChartTypeParameter> tTemplateServiceChartTypeParameterMap;

class ChartTypeDialogController : public ChangingResource
{
public:
    ChartTypeDialogController();
    virtual ~ChartTypeDialogController() override;

    virtual OUString getName() = 0;
    virtual OUString getImage() = 0;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const = 0;
    virtual void fillSubTypeList(ValueSet& rSubTypeList, const ChartTypeParameter& rParameter);

    virtual bool shouldShow_3DLookControl() const;
    virtual bool shouldShow_StackingControl() const;
    virtual bool shouldShow_SplineControl() const;
    virtual bool shouldShow_GeometryControl() const;
    virtual bool shouldShow_SortByXValuesResourceGroup() const;

    virtual void showExtraControls(weld::Builder* pBuilder);
    virtual void hideExtraControls() const;
    virtual void
    fillExtraControls(const css::uno::Reference<css::chart2::XChartDocument>& xChartModel,
                      const css::uno::Reference<css::beans::XPropertySet>& xTemplateProps) const;
    /// @throws css::uno::RuntimeException
    virtual void setTemplateProperties(
        const css::uno::Reference<css::beans::XPropertySet>& xTemplateProps) const;

    bool isSubType(const OUString& rServiceName);
    ChartTypeParameter getChartTypeParameterForService(
        const OUString& rServiceName,
        const css::uno::Reference<css::beans::XPropertySet>& xTemplateProps);
    virtual void adjustSubTypeAndEnableControls(
        ChartTypeParameter&
            rParameter); //if you have different counts of subtypes you may need to adjust the index
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter);
    virtual void adjustParameterToMainType(ChartTypeParameter& rParameter);
    OUString getServiceNameForParameter(const ChartTypeParameter& rParameter) const;
    void commitToModel(const ChartTypeParameter& rParameter,
                       const css::uno::Reference<css::chart2::XChartDocument>& xChartModel);
    css::uno::Reference<css::chart2::XChartTypeTemplate> getCurrentTemplate(
        const ChartTypeParameter& rParameter,
        const css::uno::Reference<css::lang::XMultiServiceFactory>& xTemplateManager) const;

protected:
    bool bSupportsXAxisWithValues;
    bool bSupports3D;
};

class ColumnOrBarChartDialogController_Base : public ChartTypeDialogController
{
public:
    ColumnOrBarChartDialogController_Base();
    virtual ~ColumnOrBarChartDialogController_Base() override;

    virtual bool shouldShow_3DLookControl() const override;
    virtual bool shouldShow_GeometryControl() const override;

    virtual void adjustSubTypeAndEnableControls(ChartTypeParameter& rParameter) override;
};

class ColumnChartDialogController : public ColumnOrBarChartDialogController_Base
{
public:
    ColumnChartDialogController();
    virtual ~ColumnChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
};

class BarChartDialogController : public ColumnOrBarChartDialogController_Base
{
public:
    BarChartDialogController();
    virtual ~BarChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
};

class PieChartDialogController : public ChartTypeDialogController
{
public:
    PieChartDialogController();
    virtual ~PieChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;

    virtual bool shouldShow_3DLookControl() const override;
};

class LineChartDialogController : public ChartTypeDialogController
{
public:
    LineChartDialogController();
    virtual ~LineChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToMainType(ChartTypeParameter& rParameter) override;

    virtual bool shouldShow_StackingControl() const override;
    virtual bool shouldShow_SplineControl() const override;
};

class XYChartDialogController : public ChartTypeDialogController
{
public:
    XYChartDialogController();
    virtual ~XYChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;

    virtual bool shouldShow_SplineControl() const override;
    virtual bool shouldShow_SortByXValuesResourceGroup() const override;
};

class AreaChartDialogController : public ChartTypeDialogController
{
public:
    AreaChartDialogController();
    virtual ~AreaChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToMainType(ChartTypeParameter& rParameter) override;

    virtual bool shouldShow_3DLookControl() const override;
};

class NetChartDialogController : public ChartTypeDialogController
{
public:
    NetChartDialogController();
    virtual ~NetChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;

    virtual bool shouldShow_StackingControl() const override;
};

class StockChartDialogController : public ChartTypeDialogController
{
public:
    StockChartDialogController();
    virtual ~StockChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;
};

class CombiColumnLineChartDialogController : public ChartTypeDialogController
{
public:
    CombiColumnLineChartDialogController();

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;

    virtual void showExtraControls(weld::Builder* pBuilder) override;
    virtual void hideExtraControls() const override;
    virtual void fillExtraControls(
        const css::uno::Reference<css::chart2::XChartDocument>& xChartModel,
        const css::uno::Reference<css::beans::XPropertySet>& xTemplateProps) const override;

    virtual void setTemplateProperties(
        const css::uno::Reference<css::beans::XPropertySet>& xTemplateProps) const override;

private:
    DECL_LINK(ChangeLineCountHdl, weld::SpinButton&, void);

private:
    std::unique_ptr<weld::Label> m_xFT_NumberOfLines;
    std::unique_ptr<weld::SpinButton> m_xMF_NumberOfLines;
};

class BubbleChartDialogController : public ChartTypeDialogController
{
public:
    BubbleChartDialogController();
    virtual ~BubbleChartDialogController() override;

    virtual OUString getName() override;
    virtual OUString getImage() override;
    virtual const tTemplateServiceChartTypeParameterMap& getTemplateMap() const override;
    virtual void fillSubTypeList(ValueSet& rSubTypeList,
                                 const ChartTypeParameter& rParameter) override;
    virtual void adjustParameterToSubType(ChartTypeParameter& rParameter) override;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
