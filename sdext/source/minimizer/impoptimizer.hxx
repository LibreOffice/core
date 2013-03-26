/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef IMPOPTIMIZER_HXX
#define IMPOPTIMIZER_HXX

#include "pppoptimizertoken.hxx"
#include "optimizationstats.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#ifndef _COM_SUN_STAR_LANG_XMULTI_COMPONENT_FACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
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
