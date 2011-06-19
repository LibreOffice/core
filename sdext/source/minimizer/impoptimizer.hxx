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

#ifndef IMPOPTIMIZER_HXX
#define IMPOPTIMIZER_HXX

#include "pppoptimizertoken.hxx"
#include "optimizationstats.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class Point;
class Size;

// -------------
// - PDFExport -
// -------------

class ImpOptimizer : public OptimizationStats
{
private:

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >    mxMSF;
    com::sun::star::uno::Reference< com::sun::star::frame::XModel >             mxModel;
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >          mxStatusDispatcher;

    sal_Bool        mbJPEGCompression;
    sal_Int32       mnJPEGQuality;
    sal_Bool        mbRemoveCropArea;
    sal_Int32       mnImageResolution;
    sal_Bool        mbEmbedLinkedGraphics;
    sal_Bool        mbOLEOptimization;
    sal_Int32       mnOLEOptimizationType;
    rtl::OUString   maCustomShowName;
    sal_Bool        mbDeleteUnusedMasterPages;
    sal_Bool        mbDeleteHiddenSlides;
    sal_Bool        mbDeleteNotesPages;
    rtl::OUString   maSaveAsURL;
    rtl::OUString   maFilterName;
    sal_Bool        mbOpenNewDocument;

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > mxInformationDialog;

    sal_Bool Optimize();

public:

        ImpOptimizer( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rXFactory,
            const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel );
        ~ImpOptimizer();

        sal_Bool Optimize( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rArguments );
        void     DispatchStatus();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
