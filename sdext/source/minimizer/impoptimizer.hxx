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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_IMPOPTIMIZER_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_IMPOPTIMIZER_HXX

#include "pppoptimizertoken.hxx"
#include "optimizationstats.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


// - PDFExport -


class ImpOptimizer : public OptimizationStats
{
private:

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >    mxContext;
    com::sun::star::uno::Reference< com::sun::star::frame::XModel >             mxModel;
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >          mxStatusDispatcher;

    bool        mbJPEGCompression;
    sal_Int32   mnJPEGQuality;
    bool        mbRemoveCropArea;
    sal_Int32   mnImageResolution;
    bool        mbEmbedLinkedGraphics;
    bool        mbOLEOptimization;
    sal_Int32   mnOLEOptimizationType;
    OUString    maCustomShowName;
    bool        mbDeleteUnusedMasterPages;
    bool        mbDeleteHiddenSlides;
    bool        mbDeleteNotesPages;
    OUString    maSaveAsURL;
    OUString    maFilterName;
    bool        mbOpenNewDocument;

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > mxInformationDialog;

    bool Optimize();

public:

        ImpOptimizer( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rXFactory,
            const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel );
        ~ImpOptimizer();

        bool     Optimize( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rArguments );
        void     DispatchStatus();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
