/*************************************************************************
 *
 *  $RCSfile: xmlexp.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-21 10:47:19 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLCNITM_HXX
#include <xmloff/xmlcnitm.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen wg. SW_MOD
#include <swmodule.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _XMLTEXTE_HXX
#include <xmltexte.hxx>
#endif
#ifndef _XMLEXP_HXX
#include <xmlexp.hxx>
#endif

#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORYHXX_
#include <comphelper/processfactory.hxx>
#endif


using namespace ::rtl;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;

#ifdef XML_CORE_API
void SwXMLExport::SetCurPaM( SwPaM& rPaM, sal_Bool bWhole, sal_Bool bTabOnly )
{
    if( !pCurPaM )
    {
        pCurPaM = new SwPaM( *rPaM.End(), *rPaM.Start() );
    }
    else
    {
        *pCurPaM->GetPoint() = *rPaM.Start();
        *pCurPaM->GetMark() = *rPaM.End();
    }

    // Set PaM to table/section start node if whole doc should be exported
    if( bWhole )
    {
        SwTableNode *pTblNd = pCurPaM->GetNode()->FindTableNode();
        if( pTblNd )
        {
            pCurPaM->GetPoint()->nNode = *pTblNd;

            if( bTabOnly )
                pCurPaM->GetMark()->nNode = *pTblNd->EndOfSectionNode();
        }

        SwSectionNode * pSectNd = pCurPaM->GetNode()->FindSectionNode();
        while( pSectNd )
        {
            pCurPaM->GetPoint()->nNode = *pSectNd;

            // SwSectionNode::FindSectionNode() returns the section node itself
            pSectNd = pSectNd->FindStartNode()->FindSectionNode();
        }
    }
}
#endif

SwXMLExport::SwXMLExport(sal_uInt16 nExportFlags) :
    SvXMLExport( MAP_INCH, sXML_text, nExportFlags ),
#ifdef XML_CORE_API
    pCurPaM( 0 ),
    pOrigPaM( &rPaM ),
#endif
    pTableItemMapper( 0 ),
    pTableLines( 0 ),
    nContentProgressStart( 0 ),
#ifdef XML_CORE_API
    bExportWholeDoc( bExpWholeDoc ),
    bExportFirstTableOnly( bExpFirstTableOnly ),
#endif
    bBlock( sal_False ),
    bShowProgress( sal_True ),
    sNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
    sCell(RTL_CONSTASCII_USTRINGPARAM("Cell"))
{
    _InitItemExport();
}

#ifdef XML_CORE_API
SwXMLExport::SwXMLExport( const Reference< XModel >& rModel, SwPaM& rPaM,
             const OUString& rFileName,
             const Reference< XDocumentHandler > & rHandler,
             const Reference< XGraphicObjectResolver > & rEmbeddedGrfObjs,
             sal_Bool bExpWholeDoc, sal_Bool bExpFirstTableOnly,
             sal_Bool bShowProg ) :
    SvXMLExport( rFileName, rHandler, rModel, rEmbeddedGrfObjs,
                 SW_MOD()->GetMetric( rPaM.GetDoc()->IsHTMLMode() ) ),
    pCurPaM( 0 ),
    pOrigPaM( &rPaM ),
    pTableItemMapper( 0 ),
    pTableLines( 0 ),
    nContentProgressStart( 0 ),
    bExportWholeDoc( bExpWholeDoc ),
    bExportFirstTableOnly( bExpFirstTableOnly ),
    bShowProgress( bShowProg ),
    sNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
    sCell(RTL_CONSTASCII_USTRINGPARAM("Cell"))
{
    _InitItemExport();
}
#endif

void SwXMLExport::setBlockMode()
{
    bBlock = sal_True;

}

sal_uInt32 SwXMLExport::exportDoc( const sal_Char *pClass )
{
    if( !GetModel().is() )
        return ERR_SWG_WRITE_ERROR;

    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();
    Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
    ASSERT( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xTextTunnel.is() )
        return ERR_SWG_WRITE_ERROR;

    SwXText *pText = (SwXText *)xTextTunnel->getSomething(
                                        SwXText::getUnoTunnelId() );
    ASSERT( pText, "SwXText missing" );
    if( !pText )
        return ERR_SWG_WRITE_ERROR;

    SwDoc *pDoc = pText->GetDoc();

    sal_Bool bExtended = sal_False;
    if( (getExportFlags() & (EXPORT_FONTDECLS|EXPORT_STYLES|
                             EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        GetTextParagraphExport()->SetBlockMode( bBlock );

        const SfxPoolItem* pItem;
        const SfxItemPool& rPool = pDoc->GetAttrPool();
        sal_uInt16 nItems = rPool.GetItemCount( RES_UNKNOWNATR_CONTAINER );
        for( sal_uInt16 i = 0; i < nItems; ++i )
        {
            if( 0 != (pItem = rPool.GetItem( RES_UNKNOWNATR_CONTAINER, i ) ) )
            {
                const SvXMLAttrContainerItem *pUnknown =
                            (const SvXMLAttrContainerItem *)pItem;
                if( (pUnknown->GetAttrCount() > 0) )
                {
                    sal_uInt16 nIdx = pUnknown->GetFirstNamespaceIndex();
                    while( USHRT_MAX != nIdx )
                    {
                        const OUString& rPrefix = pUnknown->GetPrefix( nIdx );
                        if( USHRT_MAX ==
                                    GetNamespaceMap().GetIndexByPrefix( rPrefix ) )
                        {
                            // Add namespace declaration for unknown attributes if
                            // there aren't existing ones for the prefix used by the
                            // attibutes
                            _GetNamespaceMap().Add( rPrefix,
                                                    pUnknown->GetNamespace( nIdx ),
                                                    XML_NAMESPACE_UNKNOWN );
                        }
                        nIdx = pUnknown->GetNextNamespaceIndex( nIdx );
                    }

                    bExtended = sal_True;
                }
            }
        }
    }

    MapUnit eUnit =
        SvXMLUnitConverter::GetMapUnit( SW_MOD()->GetMetric(pDoc->IsHTMLMode()) );
    if( GetMM100UnitConverter().getXMLMeasureUnit() != eUnit )
    {
        GetMM100UnitConverter().setXMLMeasureUnit( eUnit );
        pTwipUnitConv->setXMLMeasureUnit( eUnit );
    }

    SetExtended( bExtended );

#ifdef XML_CORE_API
    SetCurPaM( rPaM, bExportWholeDoc, bExportFirstTableOnly );
#endif

    SwDocStat aDocStat( pDoc->GetDocStat() );
    if( (getExportFlags() & EXPORT_META) != 0 )
    {
        // Update doc stat, so that correct values are exported and
        // the progress works correctly.
        if( aDocStat.bModified )
            pDoc->UpdateDocStat( aDocStat );

        SfxObjectShell* pObjSh = pDoc->GetDocShell();
        if( pObjSh )
            pObjSh->UpdateDocInfoForSave();     // update information
    }
    if( bShowProgress )
    {
        nContentProgressStart = (sal_Int32)aDocStat.nPara / 2;
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        pProgress->SetReference( nContentProgressStart + 2*aDocStat.nPara );
        pProgress->SetValue( 0 );
    }

    sal_Bool bSaveShowRedline = sal_False;
    Reference<XPropertySet> xPropSet;
    OUString sShowChanges(RTL_CONSTASCII_USTRINGPARAM("ShowChanges"));
    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        //Auf die Korrektheit der OrdNums sind wir schon angewiesen.
        SdrModel* pModel = pDoc->GetDrawModel();
        if( pModel )
            pModel->GetPage( 0 )->RecalcObjOrdNums();

        // switch redline mode
        xPropSet = Reference<XPropertySet>(GetModel(), UNO_QUERY);
        if (xPropSet.is())
        {
            // record old mode
            Any aAny = xPropSet->getPropertyValue(sShowChanges);
            bSaveShowRedline = *(sal_Bool*)aAny.getValue();

            // set show = false
            sal_Bool bTmp = sal_False;
            aAny.setValue(&bTmp, ::getBooleanCppuType());
            xPropSet->setPropertyValue(sShowChanges, aAny);
        }
    }

    // adjust document class (pClass)
    if (pDoc->IsGlobalDoc())
    {
        pClass = sXML_text_global;

        // additionally, we take care of the save-linked-sections-thingy
        bSaveLinkedSections = pDoc->IsGlblDocSaveLinks();
    }
    else if (pDoc->IsLabelDoc())
    {
        pClass = sXML_label;
    }
    else if (pDoc->IsHTMLMode())
    {
        pClass = sXML_online;
    }
    // else: keep default pClass that we received

     sal_uInt32 nRet = SvXMLExport::exportDoc( pClass );

    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        // switch redline mode back
        if (xPropSet.is())
        {
            Any aAny;
            aAny.setValue(&bSaveShowRedline, ::getBooleanCppuType());
            xPropSet->setPropertyValue(sShowChanges, aAny);
        }
    }

#ifdef XML_CORE_API
    if( pCurPaM )
    {
        while( pCurPaM->GetNext() != pCurPaM )
            delete pCurPaM->GetNext();
        delete pCurPaM;
        pCurPam = 0;
    }
#endif
    ASSERT( !pTableLines, "there are table columns infos left" );

    return nRet;
}

XMLTextParagraphExport* SwXMLExport::CreateTextParagraphExport()
{
    return new SwXMLTextParagraphExport( *this, *GetAutoStylePool().get() );
}

XMLShapeExport* SwXMLExport::CreateShapeExport()
{
    XMLShapeExport* pShapeExport = new XMLShapeExport( *this, XMLTextParagraphExport::CreateShapeExtPropMapper( *this ) );
    Reference < XDrawPageSupplier > xDPS( GetModel(), UNO_QUERY );
    if( xDPS.is() )
    {
         Reference < XShapes > xShapes( xDPS->getDrawPage(), UNO_QUERY );
        pShapeExport->seekShapes( xShapes );
    }

    return pShapeExport;
}

__EXPORT SwXMLExport::~SwXMLExport()
{
    _FinitItemExport();
}


void SwXMLExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::_ExportFontDecls();
}

#define NUM_EXPORTED_PROPERTIES 9
void SwXMLExport::GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps)
{
    Reference< XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
        return;
    Reference < XIndexContainer > xBox (xServiceFactory->createInstance
            (OUString( RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.document.IndexedPropertyValues") ) ), UNO_QUERY);
    if (!xBox.is() )
        return;

    sal_Int32 i=0;
    Any aAny;
#if 0
    for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
            pFrame;
            i++, pFrame = SfxViewFrame::GetNext(*pFrame ) )
    {
        Sequence < PropertyValue > aSequence;
        pFrame->GetViewShell()->ReadUserDataSequence( aSequence, sal_False );
        aAny <<= aSequence;
        xBox->insertByIndex(i, aAny);
    }
#endif
    aAny <<= Reference < XIndexAccess > ( xBox, UNO_QUERY );

    aProps.realloc( NUM_EXPORTED_PROPERTIES ); // Currently exporting 9 properties

    PropertyValue *pValue = aProps.getArray();
    sal_Int16 nIndex = 0;

    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "Views") );
    pValue[nIndex++].Value = aAny;

    if( !GetModel().is() )
    {
        aProps.realloc(nIndex);
        return;
    }

    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();
    Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
    ASSERT( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xTextTunnel.is() )
    {
        aProps.realloc(nIndex);
        return;
    }

    SwXText *pText = (SwXText *)xTextTunnel->getSomething(
                                        SwXText::getUnoTunnelId() );
    ASSERT( pText, "SwXText missing" );
    if( !pText )
    {
        aProps.realloc(nIndex);
        return;
    }

    SwDoc *pDoc = pText->GetDoc();
    const Rectangle &rRect = pDoc->GetDocShell()->SfxInPlaceObject::GetVisArea();

    aAny <<= rRect.Top();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaTop") );
    pValue[nIndex++].Value = aAny;

    aAny <<= rRect.Left();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaLeft") );
    pValue[nIndex++].Value = aAny;

    aAny <<= rRect.GetWidth();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaWidth") );
    pValue[nIndex++].Value = aAny;

    aAny <<= rRect.GetHeight();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaHeight") );
    pValue[nIndex++].Value = aAny;

    sal_Bool bShowInsert = sal_False, bShowDelete = sal_False;

    if (( REDLINE_SHOW_MASK & pDoc->GetRedlineMode()) | REDLINE_SHOW_INSERT )
        bShowInsert = sal_True;
    if (( REDLINE_SHOW_MASK & pDoc->GetRedlineMode()) | REDLINE_SHOW_DELETE )
        bShowDelete = sal_True;

    aAny <<= bShowInsert;
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowRedlineInsertions") );
    pValue[nIndex++].Value = aAny;

    aAny <<= bShowDelete;
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowRedlineDeletions") );
    pValue[nIndex++].Value = aAny;

    aAny <<= pDoc->IsHeadInBrowse();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowHeaderWhileBrowsing") );
    pValue[nIndex++].Value = aAny;

    aAny <<= pDoc->IsFootInBrowse();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowFooterWhileBrowsing") );
    pValue[nIndex++].Value = aAny;

    if ( nIndex < NUM_EXPORTED_PROPERTIES )
        aProps.realloc(nIndex);
}
#undef NUM_EXPORTED_PROPERTIES

void SwXMLExport::GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps)
{
}
void SwXMLExport::_ExportContent()
{
#ifdef XML_CORE_API
    // export field declarations
//  pTextFieldExport->ExportFieldDeclarations();

    // export all PaMs
    SwPaM *pPaM = pOrigPaM;
    sal_Bool bContinue = sal_True;
    do
    {
        // export PaM content
        ExportCurPaM( bExportWholeDoc );

        bContinue = pPaM->GetNext() != pOrigPaM;

        if( bContinue )
        {
            pPaM = (SwPaM *)pPaM->GetNext();
            SetCurPaM( *pPaM, bExportWholeDoc, bExportFirstTableOnly );
        }

    } while( bContinue );
#else

    // export forms
    Reference<XDrawPageSupplier> xDrawPageSupplier(GetModel(), UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        // export only if we actually have elements
        Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
        if (xPage.is())
        {
            Reference<XFormsSupplier> xFormSupp(xPage, UNO_QUERY);
            if (xFormSupp->getForms()->hasElements())
            {
                ::xmloff::OOfficeFormsExport aOfficeForms(*this);

                GetFormExport()->seekPage(xPage);
                GetFormExport()->exportForms(xPage);
            }
        }
    }

    GetTextParagraphExport()->exportTrackedChanges( sal_False );
    GetTextParagraphExport()->exportTextDeclarations();
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->exportFramesBoundToPage( bShowProgress );
    GetTextParagraphExport()->exportText( xText, bShowProgress );

#endif
}


//
// uno component registration
// helper functions for export service(s)
//

OUString SAL_CALL SwXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLExporter" ) );
}

Sequence< OUString > SAL_CALL SwXMLExport_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExport_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport(EXPORT_ALL);
}

OUString SAL_CALL SwXMLExportStyles_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLStylesExporter" ) );
}

Sequence< OUString > SAL_CALL SwXMLExportStyles_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportStyles_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportStyles_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport(
        EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES |
        EXPORT_FONTDECLS );
}

OUString SAL_CALL SwXMLExportContent_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLContentExporter" ) );
}

Sequence< OUString > SAL_CALL SwXMLExportContent_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportContent_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportContent_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport(
        EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_SCRIPTS |
        EXPORT_FONTDECLS );
}

OUString SAL_CALL SwXMLExportMeta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLMetaExporter" ) );
}

Sequence< OUString > SAL_CALL SwXMLExportMeta_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportMeta_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportMeta_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport(EXPORT_META);
}

OUString SAL_CALL SwXMLExportSettings_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLSettingsExporter" ) );
}

Sequence< OUString > SAL_CALL SwXMLExportSettings_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(SwXMLExportSettings_getImplementationName());
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL SwXMLExportSettings_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new SwXMLExport(EXPORT_SETTINGS);
}

const Sequence< sal_Int8 > & SwXMLExport::getUnoTunnelId() throw()
{
    static Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXMLExport::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return SvXMLExport::getSomething( rId );
}

#ifdef XML_CORE_API
void SwXMLExport::ExportCurPaM( sal_Bool bExportWholePaM )
{
    sal_Bool bFirstNode = sal_True;
    sal_Bool bExportWholeNode = bExportWholePaM;

    SwXMLNumRuleInfo aPrevNumInfo;
    SwXMLNumRuleInfo aNextNumInfo;

    while( pCurPaM->GetPoint()->nNode.GetIndex() <
                                pCurPaM->GetMark()->nNode.GetIndex() ||
           ( pCurPaM->GetPoint()->nNode.GetIndex() ==
                                pCurPaM->GetMark()->nNode.GetIndex() &&
             pCurPaM->GetPoint()->nContent.GetIndex() <=
                                pCurPaM->GetMark()->nContent.GetIndex() ) )
    {
        SwNode *pNd = pCurPaM->GetNode();

        aNextNumInfo.Set( *pNd );
        ExportListChange( aPrevNumInfo, aNextNumInfo );

        ASSERT( !(pNd->IsGrfNode() || pNd->IsOLENode()),
                "SwXMLExport::exportCurPaM: grf or OLE node unexpected" );
        if( pNd->IsTxtNode() )
        {
            SwTxtNode* pTxtNd = pNd->GetTxtNode();

            if( !bFirstNode )
                pCurPaM->GetPoint()->nContent.Assign( pTxtNd, 0 );

            ExportTxtNode( *pTxtNd, 0, STRING_LEN, bExportWholeNode );
        }
        else if( pNd->IsTableNode() )
        {
            ExportTable( *pNd->GetTableNode() );
        }
        else if( pNd->IsSectionNode() )
        {
            ExportSection( *pNd->GetSectionNode() );
        }
        else if( pNd == &pDoc->GetNodes().GetEndOfContent() )
            break;

        pCurPaM->GetPoint()->nNode++;   // next node

        sal_uInt32 nPos = pCurPaM->GetPoint()->nNode.GetIndex();

        // if not everything should be exported, the WriteAll flag must be
        // set for all but the first and last node anyway.
        bExportWholeNode = bExportWholePaM ||
                           nPos != pCurPaM->GetMark()->nNode.GetIndex();
        bFirstNode = sal_False;

        aPrevNumInfo = aNextNumInfo;
    }
    aNextNumInfo.Reset();
    ExportListChange( aPrevNumInfo, aNextNumInfo );
}
#endif
