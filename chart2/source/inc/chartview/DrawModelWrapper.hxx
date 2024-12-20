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

#include <svx/svdmodel.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>
#include <chartview/chartviewdllapi.hxx>

namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::frame { class XModel; }

class SdrObjList;
class SdrObject;

namespace chart
{

class UNLESS_MERGELIBS(OOO_DLLPUBLIC_CHARTVIEW) DrawModelWrapper final : private SdrModel
{
private:
    rtl::Reference<SvxDrawPage> m_xMainDrawPage;
    rtl::Reference<SvxDrawPage> m_xHiddenDrawPage;
    rtl::Reference<SfxItemPool> m_xChartItemPool;
    VclPtr<OutputDevice> m_pRefDevice;

public:
    SAL_DLLPRIVATE DrawModelWrapper();
    SAL_DLLPRIVATE virtual ~DrawModelWrapper() override;

    css::uno::Reference< css::lang::XMultiServiceFactory > getShapeFactory();

    // the main page will contain the normal view objects
    const rtl::Reference<SvxDrawPage> & getMainDrawPage();
    SAL_DLLPRIVATE void clearMainDrawPage();

    // the extra page is not visible, but contains some extras like the symbols for data points
    const rtl::Reference<SvxDrawPage> & getHiddenDrawPage();

    static rtl::Reference<SvxShapeGroupAnyD>
         getChartRootShape( const rtl::Reference<SvxDrawPage>& xPage );

    SAL_DLLPRIVATE void lockControllers();
    SAL_DLLPRIVATE void unlockControllers();

    OutputDevice* getReferenceDevice() const;

    SfxItemPool&            GetItemPool();

    SAL_DLLPRIVATE virtual css::uno::Reference< css::frame::XModel >
        createUnoModel() override;
    SAL_DLLPRIVATE const css::uno::Reference< css::frame::XModel > &
        getUnoModel();
    SdrModel& getSdrModel();

    XColorListRef     GetColorList() const;
    XDashListRef      GetDashList() const;
    XLineEndListRef   GetLineEndList() const;
    XGradientListRef  GetGradientList() const;
    XHatchListRef     GetHatchList() const;
    XBitmapListRef    GetBitmapList() const;
    XPatternListRef   GetPatternList() const;

    SdrObject* getNamedSdrObject( const OUString& rName );
    static SdrObject* getNamedSdrObject( const OUString& rName, SdrObjList const * pObjList );

    static bool removeShape( const rtl::Reference<SvxShape>& xShape );

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
