/*************************************************************************
 *
 *  $RCSfile: ViewElementListProvider.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-13 15:15:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ViewElementListProvider.hxx"
#include "DrawModelWrapper.hxx"
#include "chartview/NumberFormatterWrapper.hxx"
#include "chartview/DataPointSymbolSupplier.hxx"
#include "macros.hxx"

/*
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
*/

//------------
//oldChartModelWrapper
/*
// header for class SfxPrinter
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
*/
// header for class FontList
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//------------
// header for class SvxShape
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
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

/*
//for creation of own number formatter
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
*/

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
//using namespace ::drafts::com::sun::star::chart2;

ViewElementListProvider::ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper
                                                 , NumberFormatterWrapper* pNumberFormatterWrapper )
                        : m_pDrawModelWrapper( pDrawModelWrapper )
                        , m_pFontList(NULL)
                        , m_pNumberFormatterWrapper(pNumberFormatterWrapper)
{
    DBG_ASSERT(m_pDrawModelWrapper,"A DrawModelWrapper is missing - maybe not all services can be provided");
    DBG_ASSERT(m_pNumberFormatterWrapper,"A Numberformatter is missing - maybe not all services can be provided");
}

ViewElementListProvider::~ViewElementListProvider()
{
    if(m_pFontList)
        delete m_pFontList;
}

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

//create chartspecific symbols for linecharts
SdrObjList* ViewElementListProvider::GetSymbolList() const
{
    static SdrObjList* m_pSymbolList = NULL;
    static uno::Reference< drawing::XShapes > m_xSymbols(NULL);//@todo this keeps the first drawinglayer alive ...
    try
    {
        if(!m_pSymbolList || !m_pSymbolList->GetObjCount())
        {
            //@todo use mutex

            //get factory from draw model
            uno::Reference< drawing::XDrawPagesSupplier > xDrawPagesSuplier( m_pDrawModelWrapper->getUnoModel(), uno::UNO_QUERY );
            uno::Reference< lang::XMultiServiceFactory > xShapeFactory( xDrawPagesSuplier, uno::UNO_QUERY );

            //create hidden draw page (target):
            uno::Reference< drawing::XDrawPages > xDrawPages( xDrawPagesSuplier->getDrawPages () );
            uno::Reference< drawing::XDrawPage >  xDrawPage( xDrawPages->insertNewByIndex ( 1 ) );
            uno::Reference<drawing::XShapes>      xTarget( xDrawPage, uno::UNO_QUERY );

            //create symbols via uno and convert to native sdr objects
            drawing::Direction3D aSymbolSize(1000,1000,1000);//this size is somehow arbitrary - nneds only to be big thus the dialog restricts the symbols to its own maximum value
            m_xSymbols =  DataPointSymbolSupplier::create2DSymbolList( xShapeFactory, xTarget, aSymbolSize );
            uno::Reference< lang::XUnoTunnel > xUnoTunnel( m_xSymbols, uno::UNO_QUERY );
            uno::Reference< lang::XTypeProvider > xTypeProvider( m_xSymbols, uno::UNO_QUERY );
            if(xUnoTunnel.is()&&xTypeProvider.is())
            {
                SvxShape* pSvxShape = reinterpret_cast<SvxShape*>(xUnoTunnel->getSomething( SvxShape::getUnoTunnelId() ));
                if(pSvxShape)
                {
                    SdrObject* pSdrObject = pSvxShape->GetSdrObject();
                    if(pSdrObject)
                        m_pSymbolList = pSdrObject->GetSubList();
                }
            }
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
    pView->SetMarkHdlHidden( TRUE );
    SdrPageView* pPageView = pView->ShowPage(pPage, Point());

    pObj=pObj->Clone();
    pPage->NbcInsertObject(pObj);
    pView->MarkObj(pObj,pPageView);
    if( pSymbolShapeProperties )
        pObj->SetItemSet(*pSymbolShapeProperties);

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

FontList* ViewElementListProvider::getFontList() const
{
    //was old chart:
    //SvxFontListItem* SfxObjectShell::.GetItem(SID_ATTR_CHAR_FONTLIST)

    if(!m_pFontList)
    {
        OutputDevice* pPrinter    = NULL;//getPrinter();
        OutputDevice* pDefaultOut = Application::GetDefaultDevice();    // #67730#
        m_pFontList = new FontList( pPrinter ? pPrinter    : pDefaultOut
                                , pPrinter ? pDefaultOut : NULL
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

SvNumberFormatter* ViewElementListProvider::GetOwnNumberFormatter() const
{
    if( m_pNumberFormatterWrapper )
        return m_pNumberFormatterWrapper->getSvNumberFormatter();
    return NULL;
    /*
        static SvNumberFormatter aNumberFormatter = SvNumberFormatter(
                ::comphelper::getProcessServiceFactory(),
                LANGUAGE_SYSTEM );
        return &aNumberFormatter;
    */
}
SvNumberFormatter* ViewElementListProvider::GetNumFormatter() const
{
    //@todo
    return GetOwnNumberFormatter();
}

//.............................................................................
} //namespace chart
//.............................................................................
