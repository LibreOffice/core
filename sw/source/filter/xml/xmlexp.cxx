/*************************************************************************
 *
 *  $RCSfile: xmlexp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-23 14:42:37 $
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
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // ...Percent()
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>      // ResId fuer Statusleiste
#endif
#ifndef _SHL_HXX //autogen wg. SHL_WRITER
#include <tools/shl.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen wg. SW_MOD
#include <swmodule.hxx>
#endif

#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include "docstat.hxx"
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

#ifndef _XMLTEXTE_HXX
#include <xmltexte.hxx>
#endif
#ifndef _XMLEXP_HXX
#include <xmlexp.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;

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

SwXMLExport::SwXMLExport( const Reference< XModel >& rModel, SwPaM& rPaM,
             const OUString& rFileName,
             const Reference< xml::sax::XDocumentHandler > & rHandler,
             sal_Bool bExpWholeDoc, sal_Bool bExpFirstTableOnly,
             sal_Bool bShowProg ) :
    SvXMLExport( rFileName, rHandler, rModel,
                 SW_MOD()->GetMetric( rPaM.GetDoc()->IsHTMLMode() ) ),
    pDoc( rPaM.GetDoc() ),
#ifdef XML_CORE_API
    pCurPaM( 0 ),
    pOrigPaM( &rPaM ),
#endif
    pTableItemMapper( 0 ),
    pTableLines( 0 ),
    nContentProgressStart( 0 ),
    bExportWholeDoc( bExpWholeDoc ),
    bExportFirstTableOnly( bExpFirstTableOnly ),
    bShowProgress( bShowProg ),
    sNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
    sCell(RTL_CONSTASCII_USTRINGPARAM("Cell"))
{
    const SfxPoolItem* pItem;
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    sal_uInt16 nItems = rPool.GetItemCount( RES_UNKNOWNATR_CONTAINER );
    sal_Bool bExtended = sal_False;
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

    SetExtended( bExtended );

#ifdef XML_CORE_API
    SetCurPaM( rPaM, bExportWholeDoc, bExportFirstTableOnly );
#endif

    _InitItemExport();

    // Update doc stat, so that correct values are exported and
    // the progress works correctly.
    SwDocStat aDocStat( GetDoc().GetDocStat() );
    if( aDocStat.bModified )
        GetDoc().UpdateDocStat( aDocStat
#if SUPD < 614
                 ,0
#endif
                );
    if( bShowProgress )
    {
//      ::StartProgress( STR_STATSTR_W4WWRITE, 0, pDoc->GetNodes().Count(),
//                       pDoc->GetDocShell() );
        nContentProgressStart = (sal_Int32)aDocStat.nPara / 2;
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        pProgress->SetReference( nContentProgressStart + 2*aDocStat.nPara );
        pProgress->SetValue( 0 );
    }

    SfxObjectShell* pObjSh = pDoc->GetDocShell();
    if( pObjSh )
        pObjSh->UpdateDocInfoForSave();     // update information
}

XMLTextParagraphExport* SwXMLExport::CreateTextParagraphExport()
{
    return new SwXMLTextParagraphExport( *this, *GetAutoStylePool().get() );
}

XMLShapeExport* SwXMLExport::CreateShapeExport()
{
    return new XMLShapeExport( *this, XMLTextParagraphExport::CreateShapeExtPropMapper( *this ) );
}

__EXPORT SwXMLExport::~SwXMLExport()
{
//  if( bShowProgress )
//      ::EndProgress( pDoc->GetDocShell() );

#ifdef XML_CORE_API
    if( pCurPaM )
    {
        while( pCurPaM->GetNext() != pCurPaM )
            delete pCurPaM->GetNext();
        delete pCurPaM;
    }
#endif
    _FinitItemExport();
    ASSERT( !pTableLines, "there are table columns infos left" );
}


void SwXMLExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::_ExportFontDecls();
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
    GetTextParagraphExport()->exportTextDeclarations();
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->exportFramesBoundToPage( bShowProgress );
    GetTextParagraphExport()->exportText( xText, bShowProgress );
#endif
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
//      if( bShowProgress )
//          ::SetProgressState( nPos, pDoc->GetDocShell() );

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
