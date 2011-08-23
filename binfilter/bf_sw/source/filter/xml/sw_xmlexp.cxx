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


#ifdef _MSC_VER
#pragma hdrstop
#endif

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
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _SVDMODEL_HXX 
#include <bf_svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX 
#include <bf_svx/svdpage.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#ifndef _XMLGRHLP_HXX
#include <bf_svx/xmlgrhlp.hxx>
#endif
#ifndef _XMLEOHLP_HXX
#include <bf_svx/xmleohlp.hxx>
#endif
#include <bf_svx/xmlgrhlp.hxx>
#endif
#ifndef _XMLEOHLP_HXX
#include <bf_svx/xmleohlp.hxx>
#endif
#ifndef _EEITEM_HXX
#include <bf_svx/eeitem.hxx>
#endif
#ifndef _SVDDEF_HXX
#include <bf_svx/svddef.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _SVX_XMLCNITM_HXX
#include <bf_svx/xmlcnitm.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <bf_xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX 
#include <bf_xmloff/xmluconv.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
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

#ifndef _COMPHELPER_PROCESSFACTORYHXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <bf_svx/forbiddencharacterstable.hxx>
#endif

#ifndef _FORBIDDEN_CHARACTERS_ENUM_HXX
#include <ForbiddenCharactersEnum.hxx>
#endif

// for locking SolarMutex: svapp + mutex
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif

#ifndef _XDEF_HXX
#include <bf_svx/xdef.hxx>
#endif

namespace binfilter {

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
using namespace ::com::sun::star::i18n;
using namespace ::binfilter::xmloff::token;

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

// #110680#
SwXMLExport::SwXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    sal_uInt16 nExportFlags) 
:	SvXMLExport( xServiceFactory, MAP_INCH, XML_TEXT, nExportFlags ),
#ifdef XML_CORE_API
    pCurPaM( 0 ),
    pOrigPaM( &rPaM ),
#endif
    pTableItemMapper( 0 ),
    pTableLines( 0 ),
#ifdef XML_CORE_API
    bExportWholeDoc( bExpWholeDoc ),
    bExportFirstTableOnly( bExpFirstTableOnly ),
#endif
    bBlock( sal_False ),
    bShowProgress( sal_True ),
    sNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
    sIsProtected(RTL_CONSTASCII_USTRINGPARAM("IsProtected")),
    sCell(RTL_CONSTASCII_USTRINGPARAM("Cell"))
{
    _InitItemExport();
}

#ifdef XML_CORE_API
// #110680#
SwXMLExport::SwXMLExport( 
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const Reference< XModel >& rModel, 
    SwPaM& rPaM,
    const OUString& rFileName,
    const Reference< XDocumentHandler > & rHandler,
    const Reference< XGraphicObjectResolver > & rEmbeddedGrfObjs,
    sal_Bool bExpWholeDoc, sal_Bool bExpFirstTableOnly,
    sal_Bool bShowProg ) 
:	SvXMLExport( xServiceFactory, rFileName, rHandler, rModel, rEmbeddedGrfObjs,
                 SW_MOD()->GetMetric( rPaM.GetDoc()->IsHTMLMode() ) ),
    pCurPaM( 0 ),
    pOrigPaM( &rPaM ),
    pTableItemMapper( 0 ),
    pTableLines( 0 ),
    bExportWholeDoc( bExpWholeDoc ),
    bExportFirstTableOnly( bExpFirstTableOnly ),
    bShowProgress( bShowProg ),
    sNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
    sIsProtected(RTL_CONSTASCII_USTRINGPARAM("IsProtected")),
    sCell(RTL_CONSTASCII_USTRINGPARAM("Cell"))
{
    _InitItemExport();
}
#endif

void SwXMLExport::setBlockMode()
{
    bBlock = sal_True;

}

sal_uInt32 SwXMLExport::exportDoc( enum XMLTokenEnum eClass )
{
    if( !GetModel().is() )
        return ERR_SWG_WRITE_ERROR;

    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();
    Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
    ASSERT( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xTextTunnel.is() )
        return ERR_SWG_WRITE_ERROR;

    // from here, we use core interfaces -> lock Solar-Mutex (#91949#)
    vos::OGuard aGuard(Application::GetSolarMutex());

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
        sal_uInt16 aWhichIds[5] = { RES_UNKNOWNATR_CONTAINER,
                                    RES_TXTATR_UNKNOWN_CONTAINER,
                                    SDRATTR_XMLATTRIBUTES,
                                    EE_PARA_XMLATTRIBS,
                                    EE_CHAR_XMLATTRIBS };

        sal_uInt16 nWhichIds = rPool.GetSecondaryPool() ? 5 : 2;
        for( sal_uInt16 j=0; j < nWhichIds; j++ )
        {
            sal_uInt16 nWhichId = aWhichIds[j];
            sal_uInt16 i=0, nItems = rPool.GetItemCount( nWhichId );
            for( i = 0; i < nItems; ++i )
            {
                if( 0 != (pItem = rPool.GetItem( nWhichId , i ) ) )
                {
                    const SvXMLAttrContainerItem *pUnknown = 
                                PTR_CAST( SvXMLAttrContainerItem, pItem );
                    ASSERT( pUnknown, "illegal attribute container item" );
                    if( pUnknown && (pUnknown->GetAttrCount() > 0) )
                    {
                        sal_uInt16 nIdx = pUnknown->GetFirstNamespaceIndex();
                        while( USHRT_MAX != nIdx )
                        {
                            const OUString& rPrefix =
                                pUnknown->GetPrefix( nIdx );
                            _GetNamespaceMap().Add( rPrefix,
                                                pUnknown->GetNamespace( nIdx ),
                                                XML_NAMESPACE_UNKNOWN );
                            nIdx = pUnknown->GetNextNamespaceIndex( nIdx );
                        }
                        bExtended = sal_True;
                    }
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

    SwDocStat aDocStat( pDoc->GetDocStat() );
    if( (getExportFlags() & EXPORT_META) != 0 )
    {
        // Update doc stat, so that correct values are exported and
        // the progress works correctly.
        if( aDocStat.bModified )
            pDoc->UpdateDocStat( aDocStat );

        SfxObjectShell* pObjSh = pDoc->GetDocShell();
        if( pObjSh )
            pObjSh->UpdateDocInfoForSave();		// update information
    }
    if( bShowProgress )
    {
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        if( XML_PROGRESS_REF_NOT_SET == pProgress->GetReference() )
        {
            // progress isn't initialized:
            // We assume that the whole doc is exported, and the following
            // durations:
            // - meta information: 2
            // - settings: 4 (TODO: not now!)
            // - styles (except page styles): 2
            // - page styles: 2 (TODO: not now!) + 2 for each paragraph
            // - paragraph: 2 (1 for automatic styles and one for content)

            // If required, update doc stat, so that
            // the progress works correctly.
            if( aDocStat.bModified )
                pDoc->UpdateDocStat( aDocStat );

            // count each item once, and then multiply by two to reach the 
            // figures given above
            // The styles in pDoc also count the default style that never 
            // gets exported -> subtract one.
            sal_Int32 nRef = 1;
            nRef += pDoc->GetCharFmts()->Count() - 1;
            nRef += pDoc->GetFrmFmts()->Count() - 1;
            nRef += pDoc->GetTxtFmtColls()->Count() - 1;
//			nRef += pDoc->GetPageDescCnt();
            nRef += aDocStat.nPara;
            pProgress->SetReference( 2*nRef );
            pProgress->SetValue( 0 );
        }
    }

    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        //Auf die Korrektheit der OrdNums sind wir schon angewiesen.
        SdrModel* pModel = pDoc->GetDrawModel();
        if( pModel )
            pModel->GetPage( 0 )->RecalcObjOrdNums();
    }

    // adjust document class (eClass)
    if (pDoc->IsGlobalDoc())
    {
        eClass = XML_TEXT_GLOBAL;

        // additionally, we take care of the save-linked-sections-thingy
        bSaveLinkedSections = pDoc->IsGlblDocSaveLinks();
    }
    else if (pDoc->IsLabelDoc())
    {
        eClass = XML_LABEL;
    }
    // else: keep default pClass that we received

    SvXMLGraphicHelper *pGraphicResolver = 0;
    if( !GetGraphicResolver().is() )
    {
        pGraphicResolver = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_WRITE );
        Reference< XGraphicObjectResolver > xGraphicResolver( pGraphicResolver );
        SetGraphicResolver( xGraphicResolver );
    }

    SvXMLEmbeddedObjectHelper *pEmbeddedResolver = 0;
    if( !GetEmbeddedResolver().is() )
    {
        SvPersist *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            pEmbeddedResolver = SvXMLEmbeddedObjectHelper::Create(
                                            *pPersist,
                                            EMBEDDEDOBJECTHELPER_MODE_WRITE );
            Reference< XEmbeddedObjectResolver > xEmbeddedResolver( pEmbeddedResolver );
            SetEmbeddedResolver( xEmbeddedResolver );
        }
    }

    // set redline mode if we export STYLES or CONTENT, unless redline
    // mode is taken care of outside (through info XPropertySet)
    sal_Bool bSaveRedline = 
        ( (getExportFlags() & (EXPORT_CONTENT|EXPORT_STYLES)) != 0 );
    if( bSaveRedline )
    {
        // if the info property set has a ShowChanges property,
        // then change tracking is taken care of on the outside,
        // so we don't have to!
        Reference<XPropertySet> rInfoSet = getExportInfo();
        if( rInfoSet.is() )
        {
            OUString sShowChanges( RTL_CONSTASCII_USTRINGPARAM("ShowChanges"));
            bSaveRedline = ! rInfoSet->getPropertySetInfo()->hasPropertyByName(
                                                                sShowChanges );
        }
    }
    sal_uInt16 nRedlineMode = 0;
    bSavedShowChanges = IsShowChanges( pDoc->GetRedlineMode() );
    if( bSaveRedline )
    {
        // now save and switch redline mode
        nRedlineMode = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode( 
            ( nRedlineMode & REDLINE_SHOW_MASK ) | REDLINE_INSERT );
    }

     sal_uInt32 nRet = SvXMLExport::exportDoc( eClass );

    // now we can restore the redline mode (if we changed it previously)
    if( bSaveRedline )
    {
        pDoc->SetRedlineMode( nRedlineMode );
    }
    

    if( pGraphicResolver )
        SvXMLGraphicHelper::Destroy( pGraphicResolver );
    if( pEmbeddedResolver )
        SvXMLEmbeddedObjectHelper::Destroy( pEmbeddedResolver );

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
    GetFontAutoStylePool();	// make sure the pool is created
    SvXMLExport::_ExportFontDecls();
}

#define NUM_EXPORTED_VIEW_SETTINGS 11
void SwXMLExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    Reference< XMultiServiceFactory > xServiceFactory =
            ::legacy_binfilters::getLegacyProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
        return;

    aProps.realloc( NUM_EXPORTED_VIEW_SETTINGS );
     // Currently exporting 9 properties
    PropertyValue *pValue = aProps.getArray();
    sal_Int32 nIndex = 0;

    Reference < XIndexContainer > xBox (xServiceFactory->createInstance
            (OUString( RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.document.IndexedPropertyValues") ) ), UNO_QUERY);
    if (xBox.is() )
    {
#if 0
        Any aAny;
        sal_Int32 i=0;
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(); 
                pFrame;
                i++, pFrame = SfxViewFrame::GetNext(*pFrame ) )
        {
            Sequence < PropertyValue > aSequence;
            pFrame->GetViewShell()->WriteUserDataSequence( aSequence, sal_False );
            aAny <<= aSequence;
            xBox->insertByIndex(i, aAny);
        }
#endif
        pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "Views") );
        pValue[nIndex++].Value <<= Reference < XIndexAccess > ( xBox, UNO_QUERY );
    }

    Reference < XText > xText;
    SwXText *pText = 0;

    if( GetModel().is() )
    {
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        xText = xTextDoc->getText();
        Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
        ASSERT( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
        if( xTextTunnel.is() )
        {
            pText = (SwXText *)xTextTunnel->getSomething(
                                                SwXText::getUnoTunnelId() );
            ASSERT( pText, "SwXText missing" );
        }
    }

    if( !pText )
    {
        aProps.realloc(nIndex);
        return;
    }

    SwDoc *pDoc = pText->GetDoc();
    const Rectangle rRect =
        pDoc->GetDocShell()->SfxInPlaceObject::GetVisArea();
    sal_Bool bTwip = pDoc->GetDocShell()->SfxInPlaceObject::GetMapUnit ( ) == MAP_TWIP;

    ASSERT( bTwip || ( pDoc->GetDocShell()->SfxInPlaceObject::GetMapUnit() == MAP_100TH_MM ),
        "Map unit for visible area is neither in TWIPS nor in 100th mm!" );

    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaTop") );
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.Top() ) : rRect.Top();

    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaLeft") );
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.Left() ) : rRect.Left();

    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaWidth") );
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.GetWidth() ) : rRect.GetWidth();

    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ViewAreaHeight") );
    pValue[nIndex++].Value <<= bTwip ? TWIP_TO_MM100 ( rRect.GetHeight() ) : rRect.GetHeight();

    // "show redline mode" cannot simply be read from the document
    // since it gets changed during execution. If it's in the info
    // XPropertySet, we take it from there.
    sal_Bool bShowRedlineChanges = bSavedShowChanges;
    Reference<XPropertySet> xInfoSet( getExportInfo() );
    if ( xInfoSet.is() )
    {
        OUString sShowChanges( RTL_CONSTASCII_USTRINGPARAM( "ShowChanges" ));
        if( xInfoSet->getPropertySetInfo()->hasPropertyByName( sShowChanges ) )
        {
            bShowRedlineChanges = *(sal_Bool*) xInfoSet->
                                   getPropertyValue( sShowChanges ).getValue();
        }
    }

    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowRedlineChanges") );
    pValue[nIndex++].Value.setValue( &bShowRedlineChanges, ::getBooleanCppuType() );

    sal_Bool bShowHead = pDoc->IsHeadInBrowse();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowHeaderWhileBrowsing") );
    pValue[nIndex++].Value.setValue( &bShowHead, ::getBooleanCppuType() );

    sal_Bool bShowFoot =  pDoc->IsFootInBrowse();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "ShowFooterWhileBrowsing") );
    pValue[nIndex++].Value.setValue( &bShowFoot, ::getBooleanCppuType() );

    sal_Bool bInBrowse =  pDoc->IsBrowseMode();
    pValue[nIndex].Name = OUString( RTL_CONSTASCII_USTRINGPARAM ( "InBrowseMode") );
    pValue[nIndex++].Value.setValue( &bInBrowse, ::getBooleanCppuType() );

    if ( nIndex < NUM_EXPORTED_VIEW_SETTINGS )
        aProps.realloc(nIndex);
}
#undef NUM_EXPORTED_VIEW_SETTINGS

void SwXMLExport::GetConfigurationSettings( Sequence < PropertyValue >& rProps)
{
    Reference< XMultiServiceFactory > xFac( GetModel(), UNO_QUERY );
    if( xFac.is() )
    {
        Reference< XPropertySet > xProps( xFac->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.Settings" ) ) ), UNO_QUERY );
        if( xProps.is() )
            SvXMLUnitConverter::convertPropertySet( rProps, xProps );
    }	
}

void SwXMLExport::_ExportContent()
{
    // export forms
    Reference<XDrawPageSupplier> xDrawPageSupplier(GetModel(), UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        // export only if we actually have elements
        Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
        if (xPage.is())
        {
            // #103597# prevent export of form controls which are embedded in
            // mute sections
            Reference<XIndexAccess> xIAPage( xPage, UNO_QUERY );
            GetTextParagraphExport()->PreventExportOfControlsInMuteSections( 
                xIAPage, GetFormExport() );

            Reference<XFormsSupplier> xFormSupp(xPage, UNO_QUERY);
            if (xFormSupp->getForms()->hasElements())
            {
                ::binfilter::xmloff::OOfficeFormsExport aOfficeForms(*this);

                GetFormExport()->seekPage(xPage);
                GetFormExport()->exportForms(xPage);
            }
        }
    }

    Reference<XPropertySet> xPropSet(GetModel(), UNO_QUERY);
    if (xPropSet.is())
    {
        OUString sTwoDigitYear(RTL_CONSTASCII_USTRINGPARAM("TwoDigitYear"));

        Any aAny = xPropSet->getPropertyValue( sTwoDigitYear );
        aAny <<= (sal_Int16)1930;

        sal_Int16 nYear;
        aAny >>= nYear;
        if (nYear != 1930 )
        {
            ::rtl::OUStringBuffer sBuffer;
            GetMM100UnitConverter().convertNumber(sBuffer, nYear);
            AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, sBuffer.makeStringAndClear());
            SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, sal_True, sal_True);
        }
    }

    GetTextParagraphExport()->exportTrackedChanges( sal_False );
    GetTextParagraphExport()->exportTextDeclarations();
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->exportFramesBoundToPage( bShowProgress );
    GetTextParagraphExport()->exportText( xText, bShowProgress );
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
    // #110680#
    // return (cppu::OWeakObject*)new SwXMLExport(EXPORT_ALL);
    return (cppu::OWeakObject*)new SwXMLExport( rSMgr, EXPORT_ALL );
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
    // #110680#
    //return (cppu::OWeakObject*)new SwXMLExport( 
    //	EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES | 
    //	EXPORT_FONTDECLS );
    return (cppu::OWeakObject*)new SwXMLExport( 
        rSMgr,
        EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES | EXPORT_FONTDECLS );
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
    // #110680#
    //return (cppu::OWeakObject*)new SwXMLExport(
    //	EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_SCRIPTS | 
    //	EXPORT_FONTDECLS );
    return (cppu::OWeakObject*)new SwXMLExport(
        rSMgr,
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
    // #110680#
    // return (cppu::OWeakObject*)new SwXMLExport(EXPORT_META);
    return (cppu::OWeakObject*)new SwXMLExport( rSMgr, EXPORT_META );
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
    // #110680#
    // return (cppu::OWeakObject*)new SwXMLExport(EXPORT_SETTINGS);
    return (cppu::OWeakObject*)new SwXMLExport( rSMgr, EXPORT_SETTINGS );
}

const Sequence< sal_Int8 > & SwXMLExport::getUnoTunnelId() throw()
{
    static Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
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


// XServiceInfo
// override empty method from parent class
OUString SAL_CALL SwXMLExport::getImplementationName() 
    throw(RuntimeException)
{
    switch( getExportFlags() )
    {
        case EXPORT_ALL:
            return SwXMLExport_getImplementationName();
            break;
        case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS):
            return SwXMLExportStyles_getImplementationName();
            break;
        case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
            return SwXMLExportContent_getImplementationName();
            break;
        case EXPORT_META:
            return SwXMLExportMeta_getImplementationName();
            break;
        case EXPORT_SETTINGS:
            return SwXMLExportSettings_getImplementationName();
            break;
        default:
            // generic name for 'unknown' cases
            return OUString( RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.Writer.SwXMLExport" ) );
            break;
    }
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

        pCurPaM->GetPoint()->nNode++; 	// next node

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
}
