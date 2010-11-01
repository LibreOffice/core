/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

//.............................................................................
namespace chart
{
//.............................................................................

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

    XColorTable*    GetColorTable() const;
    XDashList*      GetDashList() const;
    XLineEndList*   GetLineEndList() const;
    XGradientList*  GetGradientList() const;
    XHatchList*     GetHatchList() const;
    XBitmapList*    GetBitmapList() const;

    SdrObject* getNamedSdrObject( const rtl::OUString& rName );
    static SdrObject* getNamedSdrObject( const String& rName, SdrObjList* pObjList );

    static bool removeShape( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape );
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
