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
#ifndef _CHART2_DRAWMODELWRAPPER_HXX
#define _CHART2_DRAWMODELWRAPPER_HXX

//----
#include <svx/svdmodel.hxx>
// header for class SdrObject
#include <svx/svdobj.hxx>

//----
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "chartviewdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTVIEW DrawModelWrapper : private SdrModel
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory > m_xMCF;
    SfxItemPool* m_pChartItemPool;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > m_xMainDrawPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > m_xHiddenDrawPage;

    std::auto_ptr< OutputDevice > m_apRefDevice;

    //no default constructor
    DrawModelWrapper();

public:
    SAL_DLLPRIVATE DrawModelWrapper(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext );
    SAL_DLLPRIVATE virtual ~DrawModelWrapper();

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getShapeFactory();

    // the main page will contain the normal view objects
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > getMainDrawPage();
    SAL_DLLPRIVATE void clearMainDrawPage();

    // the extra page is not visible, but contains some extras like the symbols for data points
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > getHiddenDrawPage();

    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage );

    SAL_DLLPRIVATE void lockControllers();
    SAL_DLLPRIVATE void unlockControllers();

    /// tries to get an OutputDevice from the XParent of the model to use as reference device
    SAL_DLLPRIVATE void attachParentReferenceDevice(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel );

    OutputDevice* getReferenceDevice() const;

    SfxItemPool&            GetItemPool();
    SAL_DLLPRIVATE const SfxItemPool&      GetItemPool() const;

    SAL_DLLPRIVATE virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        createUnoModel();
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        getUnoModel();
    SdrModel& getSdrModel();

    XColorListRef     GetColorList() const;
    XDashListRef      GetDashList() const;
    XLineEndListRef   GetLineEndList() const;
    XGradientListRef  GetGradientList() const;
    XHatchListRef     GetHatchList() const;
    XBitmapListRef    GetBitmapList() const;

    SdrObject* getNamedSdrObject( const OUString& rName );
    static SdrObject* getNamedSdrObject( const String& rName, SdrObjList* pObjList );

    static bool removeShape( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape );
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
