/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewElementListProvider.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:51:19 $
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
#include "precompiled_chart2.hxx"
#include "ViewElementListProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "chartview/DataPointSymbolSupplier.hxx"
#include "macros.hxx"
#include "DrawViewWrapper.hxx"

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#ifndef _SVX_XPROPERTYTABLE_HXX
#include <svx/XPropertyTable.hxx>
#endif
#ifndef _SVX_UNOFILL_HXX_
#include <svx/unofill.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif

// header for class NameOrIndex
#ifndef _SVX_XIT_HXX
#include <svx/xit.hxx>
#endif
// header for class XFillBitmapItem
#ifndef _SVX_XBTMPIT_HXX
#include <svx/xbtmpit.hxx>
#endif
#ifndef _SVX_XFLFTRIT_HXX
#include <svx/xflftrit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XFLHTIT_HXX
#include <svx/xflhtit.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX
#include <svx/xflgrit.hxx>
#endif
// header for class XLineStartItem
#ifndef _SVX_XLNSTIT_HXX
#include <svx/xlnstit.hxx>
#endif
// header for class XLineEndItem
#ifndef _SVX_XLNEDIT_HXX
#include <svx/xlnedit.hxx>
#endif

//------------
//oldChartModelWrapper

// header for class SfxItemPool
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
// header for class FontList
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//------------
// header for class SdrObject
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif


//---------------
//for creation of a symbol Graphic
// header for class VirtualDevice
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
// header for class SdrView
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
//---------------

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

ViewElementListProvider::ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper )
                        : m_pDrawModelWrapper( pDrawModelWrapper )
                        , m_pFontList(NULL)
{
}

ViewElementListProvider::~ViewElementListProvider()
{
    if(m_pFontList)
        delete m_pFontList;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

XColorTable*   ViewElementListProvider::GetColorTable() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetColorTable();
    return NULL;
}
XDashList*     ViewElementListProvider::GetDashList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetDashList();
    return NULL;
}
XLineEndList*  ViewElementListProvider::GetLineEndList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetLineEndList();
    return NULL;
}
XGradientList* ViewElementListProvider::GetGradientList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetGradientList();
    return NULL;
}
XHatchList*    ViewElementListProvider::GetHatchList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetHatchList();
    return NULL;
}
XBitmapList*   ViewElementListProvider::GetBitmapList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetBitmapList();
    return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//create chartspecific symbols for linecharts
SdrObjList* ViewElementListProvider::GetSymbolList() const
{
    SdrObjList* m_pSymbolList = NULL;
    uno::Reference< drawing::XShapes > m_xSymbols(NULL);//@todo this keeps the first drawinglayer alive ...
    try
    {
        if(!m_pSymbolList || !m_pSymbolList->GetObjCount())
        {
            //@todo use mutex

            //get shape factory
            uno::Reference< lang::XMultiServiceFactory > xShapeFactory( m_pDrawModelWrapper->getShapeFactory() );

            //get hidden draw page (target):
            uno::Reference<drawing::XShapes> xTarget( m_pDrawModelWrapper->getHiddenDrawPage(), uno::UNO_QUERY );

            //create symbols via uno and convert to native sdr objects
            drawing::Direction3D aSymbolSize(220,220,0); // should be 250, but 250 -> 280 ??
            m_xSymbols =  DataPointSymbolSupplier::create2DSymbolList( xShapeFactory, xTarget, aSymbolSize );

            SdrObject* pSdrObject = DrawViewWrapper::getSdrObject( uno::Reference< drawing::XShape >( m_xSymbols, uno::UNO_QUERY ) );
            if(pSdrObject)
                m_pSymbolList = pSdrObject->GetSubList();
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
    return m_pSymbolList;
}

Graphic ViewElementListProvider::GetSymbolGraphic( sal_Int32 nStandardSymbol, const SfxItemSet* pSymbolShapeProperties ) const
{
    SdrObjList* pSymbolList = this->GetSymbolList();
    if( !pSymbolList->GetObjCount() )
        return Graphic();
    if(nStandardSymbol<0)
        nStandardSymbol*=-1;
    if( nStandardSymbol >= static_cast<sal_Int32>(pSymbolList->GetObjCount()) )
        nStandardSymbol %= pSymbolList->GetObjCount();
    SdrObject* pObj = pSymbolList->GetObj(nStandardSymbol);

    VirtualDevice aVDev;
    aVDev.SetMapMode(MapMode(MAP_100TH_MM));
    SdrModel* pModel = new SdrModel();
    pModel->GetItemPool().FreezeIdRanges();
    SdrPage* pPage = new SdrPage( *pModel, FALSE );
    pPage->SetSize(Size(1000,1000));
    pModel->InsertPage( pPage, 0 );
    SdrView* pView = new SdrView( pModel, &aVDev );
    pView->hideMarkHandles();
    SdrPageView* pPageView = pView->ShowSdrPage(pPage);

    pObj=pObj->Clone();
    pPage->NbcInsertObject(pObj);
    pView->MarkObj(pObj,pPageView);
    if( pSymbolShapeProperties )
        pObj->SetMergedItemSet(*pSymbolShapeProperties);

    GDIMetaFile aMeta(pView->GetAllMarkedMetaFile());

    Graphic aGraph(aMeta);
    Size aSize = pObj->GetSnapRect().GetSize();
    aGraph.SetPrefSize(aSize);
    aGraph.SetPrefMapMode(MAP_100TH_MM);

    pView->UnmarkAll();
    pObj=pPage->RemoveObject(0);
    if(pObj)
        delete pObj;
    delete pView;
    delete pModel;

    return aGraph;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FontList* ViewElementListProvider::getFontList() const
{
    //was old chart:
    //SvxFontListItem* SfxObjectShell::.GetItem(SID_ATTR_CHAR_FONTLIST)

    if(!m_pFontList)
    {
        OutputDevice* pRefDev    = m_pDrawModelWrapper ? m_pDrawModelWrapper->getReferenceDevice() : NULL;
        OutputDevice* pDefaultOut = Application::GetDefaultDevice();    // #67730#
        m_pFontList = new FontList( pRefDev ? pRefDev    : pDefaultOut
                                , pRefDev ? pDefaultOut : NULL
                                , FALSE );
    }
    return m_pFontList;
}

/*
SfxPrinter* ObjectPropertiesDialogParameter::getPrinter()
{
    //was old chart:
    //SfxPrinter* SchChartDocShell::GetPrinter()

    // OLE-Objekt: kein Printer anlegen ??? see old chart: :UpdateTablePointers
    //@todo get printer from calc or other container
    //return NULL;

    SfxPrinter* pPrinter = NULL;
    bool bOwnPrinter = true;
    if (!pPrinter)
    {
        SfxBoolItem aItem(SID_PRINTER_NOTFOUND_WARN, TRUE);
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet(GetPool(),
                                          SID_PRINTER_NOTFOUND_WARN,
                                          SID_PRINTER_NOTFOUND_WARN, 0);
        pSet->Put(aItem);
        pPrinter = new SfxPrinter(pSet); //@todo ->need to remember and delete
        bOwnPrinter = TRUE;

        MapMode aMapMode = pPrinter->GetMapMode();
        aMapMode.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMapMode);

        if (pChDoc)
        {
            if (pPrinter != pChDoc->GetRefDevice())
                pChDoc->SetRefDevice(pPrinter);

            if (pPrinter != pChDoc->GetOutliner()->GetRefDevice())
                pChDoc->GetOutliner()->SetRefDevice(pPrinter);
        }
    }
    return pPrinter;
}
*/

//.............................................................................
} //namespace chart
//.............................................................................
