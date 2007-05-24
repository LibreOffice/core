/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impoptimizer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-24 10:08:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef IMPOPTIMIZER_HXX
#define IMPOPTIMIZER_HXX

#ifndef _PPPOPTIMIZER_TOKEN_HXX
#include "pppoptimizertoken.hxx"
#endif
#ifndef OPTIMIZATIONSTATS_HXX
#include "optimizationstats.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTI_COMPONENT_FACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

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
