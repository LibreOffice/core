/*************************************************************************
 *
 *  $RCSfile: xmlexp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-12 17:30:28 $
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

#ifndef _NODE_HXX //autogen wg. SwTableNode
#include <node.hxx>
#endif
#ifndef _NDTXT_HXX //autogen wg. SwTxtNode
#include <ndtxt.hxx>
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

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
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

#ifndef _XMLOFF_XMLASTPL_HXX
#include <xmloff/xmlastpl.hxx>
#endif

#ifndef _XMLOFF_TXTFLDE_HXX
#include <xmloff/txtflde.hxx>
#endif

#ifndef _XMLOFF_XMLNUMFE_HXX
#include <xmloff/xmlnumfe.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#ifndef _XMLOFF_TXTPARAE_HXX
#include <xmloff/txtparae.hxx>
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include <xmloff/xmlaustp.hxx>
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT
#include <xmloff/XMLTextMasterPageExport.hxx>
#endif

#ifndef _XMLNUME_HXX
#include <xmlnume.hxx>
#endif
#ifndef _XMLFMTE_HXX
#include <xmlfmte.hxx>
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
    pParaItemMapper( 0 ),
#endif
    pTableItemMapper( 0 ),
#ifdef XML_CORE_API
    pItemSetAutoStylePool( new SwXMLAutoStylePool ),
    pListElements( 0 ),
    pExportedLists( 0 ),
#endif
    pTableLines( 0 ),
    bExportWholeDoc( bExpWholeDoc ),
    bExportFirstTableOnly( bExpFirstTableOnly ),
    bShowProgress( bShowProg )
#ifdef XML_CORE_API
    , pTextFieldExport( 0 ),
    pNumberFormatExport( 0 )
#endif
{
//  _GetNamespaceMap().AddAtIndex( XML_NAMESPACE_TEXT, sXML_np_text,
//                                 sXML_n_text, XML_NAMESPACE_TEXT );
//  _GetNamespaceMap().AddAtIndex( XML_NAMESPACE_TABLE, sXML_np_table,
//                                 sXML_n_table, XML_NAMESPACE_TABLE );

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

    if( bShowProgress )
        ::StartProgress( STR_STATSTR_W4WWRITE, 0, pDoc->GetNodes().Count(),
                         pDoc->GetDocShell() );

#ifdef XML_CORE_API
    Reference< util::XNumberFormatsSupplier > xNumFmtSupp(
        pDoc->GetDocShell()->GetBaseModel(), UNO_QUERY);
    pNumberFormatExport = new SvXMLNumFmtExport( rHandler, xNumFmtSupp );

    pTextFieldExport = new XMLTextFieldExport( *this );
#endif
    xMasterPageExport = new XMLTextMasterPageExport( *this );

    SfxObjectShell* pObjSh = pDoc->GetDocShell();
    if( pObjSh )
        pObjSh->UpdateDocInfoForSave();     // update information
}

#ifndef XML_CORE_API
XMLTextParagraphExport* SwXMLExport::CreateTextParagraphExport()
{
    return new SwXMLTextParagraphExport( *this, *GetAutoStylePool().get() );
}
#endif

__EXPORT SwXMLExport::~SwXMLExport()
{
#ifdef XML_CORE_API
    delete pItemSetAutoStylePool;
#endif

#ifdef XML_CORE_API
    delete pTextFieldExport;
    delete pNumberFormatExport;
#endif

    if( bShowProgress )
        ::EndProgress( pDoc->GetDocShell() );

#ifdef XML_CORE_API
    if( pCurPaM )
    {
        while( pCurPaM->GetNext() != pCurPaM )
            delete pCurPaM->GetNext();
        delete pCurPaM;
    }
#endif
    _FinitItemExport();
#ifdef XML_CORE_API
    _FinitNumRuleExport();
#endif
    ASSERT( !pTableLines, "there are table columns infos left" );
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

    GetTextParagraphExport()->exportFramesBoundToPage();
    GetTextParagraphExport()->exportText( xText );
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
        if( bShowProgress )
            ::SetProgressState( nPos, pDoc->GetDocShell() );

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


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlexp.cxx,v 1.2 2000-10-12 17:30:28 mib Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:15:00  hr
      initial import

      Revision 1.35  2000/09/18 16:05:05  willem.vandorp
      OpenOffice header added.

      Revision 1.34  2000/09/18 11:58:02  mib
      text frames/graphics import and export continued

      Revision 1.33  2000/08/15 11:55:56  kz
      #65293# del. 2. Parameter

      Revision 1.32  2000/08/10 10:22:15  mib
      #74404#: Adeptions to new XSL/XLink working draft

      Revision 1.31  2000/08/02 14:52:39  mib
      text export continued

      Revision 1.30  2000/07/31 09:42:35  mib
      text export continued

      Revision 1.29  2000/07/27 08:06:33  mib
      text import continued

      Revision 1.28  2000/07/26 05:11:20  mib
      text import/export continued

      Revision 1.27  2000/07/24 12:01:21  dvo
      - field declarations now exported at content beginning

      Revision 1.26  2000/07/24 10:19:02  dvo
      - textfield export for XML_CORE_API

      Revision 1.25  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.24  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.23  2000/05/03 12:08:05  mib
      unicode

      Revision 1.22  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.21  2000/02/11 14:40:57  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.20  2000/02/07 10:03:28  mib
      #70271#: tables

      Revision 1.19  2000/01/20 10:03:15  mib
      #70271#: Lists reworked

      Revision 1.18  2000/01/12 15:00:22  mib
      #70271#: lists

      Revision 1.17  2000/01/06 15:08:27  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.16  1999/12/06 14:49:49  mib
      #70271#: office:version attribute

      Revision 1.15  1999/12/06 11:41:33  mib
      #70258#: Container item for unkown attributes

      Revision 1.14  1999/11/26 11:08:59  mib
      progress, export-flags

      Revision 1.13  1999/11/22 15:52:34  os
      headers added

      Revision 1.12  1999/11/17 20:07:53  nn
      document language

      Revision 1.11  1999/11/12 14:50:28  mib
      meta import and export reactivated

      Revision 1.10  1999/11/10 15:08:09  mib
      Import now uses XMLItemMapper

      Revision 1.9  1999/11/09 15:40:29  mib
      Using XMLItemMapper for export

      Revision 1.8  1999/11/05 19:43:18  nn
      _ExportMeta

      Revision 1.7  1999/10/26 13:34:30  mib
      removed 'using namespace' from header files

      Revision 1.6  1999/10/08 11:47:09  mib
      moved some file to SVTOOLS/SVX

      Revision 1.5  1999/09/22 11:56:47  mib
      string -> wstring

      Revision 1.4  1999/08/17 14:28:22  MIB
      namespace map methods renamed consistently


      Rev 1.3   17 Aug 1999 16:28:22   MIB
   namespace map methods renamed consistently

      Rev 1.2   13 Aug 1999 16:21:14   MIB
   new base class XMLExport, styles and sections

      Rev 1.1   12 Aug 1999 18:05:20   MIB
   Export ofSvxFontItem, SvxFontHeightItem and SvxLRSpaceItem

      Rev 1.0   12 Aug 1999 12:28:54   MIB
   Initial revision.

*************************************************************************/

