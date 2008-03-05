/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InsertFunctions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:12:28 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_reportdesign.hxx"

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//------------------------------------------------------------------------

#include <toolkit/helper/vclunohelper.hxx>
#include <sot/exchange.hxx>
#include <svtools/globalnameitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/stritem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/pfiledlg.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <svtools/moduleoptions.hxx>
#include <svtools/insdlg.hxx>
#include <svtools/soerr.hxx>
#include <svx/svxdlg.hxx>
#include <sot/clsids.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdmodel.hxx>

#include <cppuhelper/component_context.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/data/DatabaseDataProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

using namespace ::com::sun::star;

#include "InsertFunctions.hxx"
#include "RptObject.hxx"

namespace rptui
{
//------------------------------------------------------------------------
void InitializeChart( const uno::Reference< frame::XModel>& _xModel,
                      const uno::Reference < embed::XEmbeddedObject >& xObj)
{
    //ScDocShell* pDocShell = pViewData->GetDocShell();
    //ScDocument* pScDoc = pDocShell->GetDocument();

    //rtl::OUString aRangeString( rRangeParam );
    //if ( !aRangeString.getLength() )
    //{
    //    SCCOL nCol1 = 0;
    //    SCROW nRow1 = 0;
    //    SCTAB nTab1 = 0;
    //    SCCOL nCol2 = 0;
    //    SCROW nRow2 = 0;
    //    SCTAB nTab2 = 0;

    //    ScMarkData& rMark = pViewData->GetMarkData();
    //    if ( !rMark.IsMarked() )
    //        pViewData->GetView()->MarkDataArea( TRUE );

    //    if ( pViewData->GetSimpleArea( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 ) )
    //    {
    //        PutInOrder( nCol1, nCol2 );
    //        PutInOrder( nRow1, nRow2 );
    //        if ( nCol2>nCol1 || nRow2>nRow1 )
    //        {
    //            ScDocument* pDoc = pViewData->GetDocument();
    //            pDoc->LimitChartArea( nTab1, nCol1,nRow1, nCol2,nRow2 );

    //            String aStr;
    //            ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    //            aRange.Format( aStr, SCR_ABS_3D, pScDoc );
    //            aRangeString = aStr;
    //        }
    //    }
    //}

    //if ( rRangeParam.getLength() )
    {
        // connect to Calc data (if no range string, leave chart alone, with its own data)

        uno::Reference< chart2::data::XDataReceiver > xReceiver;
        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is())
            xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
        OSL_ASSERT( xReceiver.is());
        if( xReceiver.is() )
        {
            // lock the model to suppress any internal updates
            uno::Reference< frame::XModel > xChartModel( xReceiver, uno::UNO_QUERY );
            if( xChartModel.is() )
                xChartModel->lockControllers();

            uno::Reference< lang::XMultiServiceFactory> xFac(_xModel,uno::UNO_QUERY_THROW);
            uno::Reference< chart2::data::XDatabaseDataProvider > xDataProvider( xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.data.DataProvider"))),uno::UNO_QUERY);
            xReceiver->attachDataProvider( xDataProvider.get() );

            uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( _xModel, uno::UNO_QUERY );
            xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

            uno::Sequence< beans::PropertyValue > aArgs( 4 );
            aArgs[0] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
                uno::makeAny( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("all")) ), beans::PropertyState_DIRECT_VALUE );
            aArgs[1] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("HasCategories"), -1,
                uno::makeAny( sal_True ), beans::PropertyState_DIRECT_VALUE );
            aArgs[2] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
                uno::makeAny( sal_False ), beans::PropertyState_DIRECT_VALUE );
            aArgs[3] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("DataRowSource"), -1,
                uno::makeAny( chart::ChartDataRowSource_COLUMNS ), beans::PropertyState_DIRECT_VALUE );
            xReceiver->setArguments( aArgs );

            if( xChartModel.is() )
                xChartModel->unlockControllers();
        }
    }
}
// -----------------------------------------------------------------------------
} // namespace rptui
// -----------------------------------------------------------------------------

