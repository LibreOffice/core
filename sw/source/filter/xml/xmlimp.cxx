/*************************************************************************
 *
 *  $RCSfile: xmlimp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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

#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_I18NMAP_HXX
#include <xmloff/i18nmap.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include <xmloff/txtimp.hxx>
#endif


#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

sal_Char __READONLY_DATA sXML_np__text[] = "text";
sal_Char __READONLY_DATA sXML_np__table[] = "table";

//----------------------------------------------------------------------------

enum SwXMLDocTokens
{
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_OFFICE_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aDocTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_styles,     XML_TOK_DOC_STYLES     },
    { XML_NAMESPACE_OFFICE, sXML_automatic_styles, XML_TOK_DOC_AUTOSTYLES   },
    { XML_NAMESPACE_OFFICE, sXML_master_styles,  XML_TOK_DOC_MASTERSTYLES   },
    { XML_NAMESPACE_OFFICE, sXML_meta,       XML_TOK_DOC_META       },
    { XML_NAMESPACE_OFFICE, sXML_body,       XML_TOK_DOC_BODY       },
    XML_TOKEN_MAP_END
};

class SwXMLDocContext_Impl : public SvXMLImportContext
{
    const SwXMLImport& GetSwImport() const
        { return (const SwXMLImport&)GetImport(); }
    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

public:

    SwXMLDocContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SwXMLDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const Reference< xml::sax::XAttributeList > & xAttrList );
};

SwXMLDocContext_Impl::SwXMLDocContext_Impl( SwXMLImport& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const Reference< xml::sax::XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

SwXMLDocContext_Impl::~SwXMLDocContext_Impl()
{
}

SvXMLImportContext *SwXMLDocContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetDocElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_DOC_STYLES:
        pContext = GetSwImport().CreateStylesContext( rLocalName, xAttrList,
                                                      sal_False );
        break;
    case XML_TOK_DOC_AUTOSTYLES:
        pContext = GetSwImport().CreateStylesContext( rLocalName, xAttrList,
                                                      sal_True );
        break;
//  case XML_TOK_DOC_USESTYLES:
//      pContext = GetSwImport().CreateUseStylesContext( rLocalName,
//                                                       xAttrList );
//      break;
    case XML_TOK_DOC_MASTERSTYLES:
//      pContext = GetSwImport().CreateMasterStylesContext( rLocalName,
//                                                       xAttrList );
        break;
    case XML_TOK_DOC_META:
        pContext = GetSwImport().CreateMetaContext( rLocalName );
        break;
    case XML_TOK_DOC_BODY:
        pContext = GetSwImport().CreateBodyContext( rLocalName );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );


    return pContext;
}

//----------------------------------------------------------------------------

const SvXMLTokenMap& SwXMLImport::GetDocElemTokenMap()
{
    if( !pDocElemTokenMap )
        pDocElemTokenMap = new SvXMLTokenMap( aDocTokenMap );

    return *pDocElemTokenMap;
}

SvXMLImportContext *SwXMLImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_OFFICE==nPrefix &&
        rLocalName.compareToAscii( sXML_document ) == 0 )
        pContext = new SwXMLDocContext_Impl( *this, nPrefix, rLocalName,
                                             xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

SwXMLImport::SwXMLImport(
        SwDoc& rDoc, const SwPaM& rPaM,
        sal_Bool bLDoc, sal_Bool bInsertMode, sal_uInt16 nStyleFamMask,
        const ::com::sun::star::uno::Reference<
                 ::com::sun::star::frame::XModel > & rModel ) :
    SvXMLImport( rModel ),
    bLoadDoc( bLDoc ),
    bInsert( bInsertMode ),
    nStyleFamilyMask( nStyleFamMask ),
    pDoc( &rDoc ),
    pPaM( new SwPaM( *rPaM.GetPoint() )  ),
#ifdef XML_CORE_API
    pI18NMap( new SvI18NMap ),
    pUnusedNumRules( 0 ),
#endif
    pDocElemTokenMap( 0 ),
#ifdef XML_CORE_API
    pBodyElemTokenMap( 0 ),
    pStyleStylesElemTokenMap( 0 ),
    pTextPElemTokenMap( 0 ),
    pTextPAttrTokenMap( 0 ),
    pTextListBlockAttrTokenMap( 0 ),
    pTextListBlockElemTokenMap( 0 ),
#endif
    pTableElemTokenMap( 0 ),
#ifdef XML_CORE_API
    pParaItemMapper( 0 ),
#endif
    pTableItemMapper( 0 ),
    pSttNdIdx( 0 ),
    bAutoStylesValid( sal_False )
{
//  GetNamespaceMap().AddAtIndex( XML_NAMESPACE_TEXT, sXML_np__text,
//                                sXML_n_text, XML_NAMESPACE_TEXT );
//  GetNamespaceMap().AddAtIndex( XML_NAMESPACE_TABLE, sXML_np__table,
//                                sXML_n_table, XML_NAMESPACE_TABLE );

    _InitItemImport();

    if( !IsStylesOnlyMode() )
    {
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( IsInsertMode() )
        {
            const SwPosition* pPos = pPaM->GetPoint();

            // Split once and remember the node that has been splitted.
            pDoc->SplitNode( *pPos );
            *pSttNdIdx = pPos->nNode.GetIndex()-1;

            // Split again.
            pDoc->SplitNode( *pPos );

            // Insert all content into the new node
            pPaM->Move( fnMoveBackward );
            pDoc->SetTxtFmtColl( *pPaM,
                             pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD) );
        }
    }
    Reference < XTextRange > xTextRange =
        CreateTextRangeFromPosition( pDoc, *rPaM.GetPoint(), 0 );
    Reference < XText > xText = xTextRange->getText();
    Reference < XTextCursor > xTextCursor =
        xText->createTextCursorByRange( xTextRange );
    GetTextImport()->SetCursor( xTextCursor );
}

SwXMLImport::~SwXMLImport()
{
    GetTextImport()->ResetCursor();

    if( !IsStylesOnlyMode() )
    {
        if( IsInsertMode() && pSttNdIdx->GetIndex() )
        {
            // If we are in insert mode, join the splitted node that is in front
            // of the new content with the first new node.
            SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
            SwNodeIndex aNxtIdx( *pSttNdIdx );
            if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ))
            {
                xub_StrLen nStt = pTxtNode->GetTxt().Len();

                // If the PaM points to the first new node, move the PaM to the
                // end of the previous node.
                if( pPaM->GetPoint()->nNode == aNxtIdx )
                {
                    pPaM->GetPoint()->nNode = *pSttNdIdx;
                    pPaM->GetPoint()->nContent.Assign( pTxtNode, nStt );
                }

#ifndef PRODUCT
                // !!! This should be impossible !!!!
                ASSERT( pSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound( sal_True ).nNode.GetIndex(),
                        "PaM.Bound1 point to new node " );
                ASSERT( pSttNdIdx->GetIndex()+1 !=
                                        pPaM->GetBound( sal_False ).nNode.GetIndex(),
                        "PaM.Bound2 points to new node" );

                if( pSttNdIdx->GetIndex()+1 ==
                                        pPaM->GetBound( sal_True ).nNode.GetIndex() )
                {
                    sal_uInt16 nCntPos =
                            pPaM->GetBound( sal_True ).nContent.GetIndex();
                    pPaM->GetBound( sal_True ).nContent.Assign( pTxtNode,
                            pTxtNode->GetTxt().Len() + nCntPos );
                }
                if( pSttNdIdx->GetIndex()+1 ==
                                pPaM->GetBound( sal_False ).nNode.GetIndex() )
                {
                    sal_uInt16 nCntPos =
                            pPaM->GetBound( sal_False ).nContent.GetIndex();
                    pPaM->GetBound( sal_False ).nContent.Assign( pTxtNode,
                            pTxtNode->GetTxt().Len() + nCntPos );
                }
#endif
                // If the first new node isn't empty, convert  the node's text
                // attributes into hints. Otherwise, set the new node's paragraph
                // style at the previous (empty) node.
                SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
                if( pTxtNode->GetTxt().Len() )
                    pDelNd->FmtToTxtAttr( pTxtNode );
                else
                    pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
                pTxtNode->JoinNext();
            }
        }

        SwPosition* pPos = pPaM->GetPoint();
        if( !pPos->nContent.GetIndex() )
        {
            SwTxtNode* pCurrNd;
            sal_uInt32 nNodeIdx = pPos->nNode.GetIndex();

            if( !IsInsertMode() )
            {
                if( pDoc->GetNodes()[nNodeIdx -1]->IsCntntNode() )
                {
                    SwCntntNode* pCNd = pPaM->GetCntntNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() )
                    {
                        pPaM->GetBound(sal_True).nContent.Assign( 0, 0 );
                        pPaM->GetBound(sal_False).nContent.Assign( 0, 0 );
                        pDoc->GetNodes().Delete( pPaM->GetPoint()->nNode );
                    }
                }
            }
            else if( 0 != (pCurrNd = pDoc->GetNodes()[nNodeIdx]->GetTxtNode()) )
            {
                if( pCurrNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pNextNd->JoinPrev();
                }
                else if( !pCurrNd->GetTxt().Len() )
                {
                    pPos->nContent.Assign( 0, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode, 1 );
                    pPaM->Move( fnMoveBackward );
                }
            }
        }
        else if( IsInsertMode() )
        {
            pPaM->Move( fnMoveForward, fnGoNode );
            SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTxtNode && pTxtNode->CanJoinPrev( &aPrvIdx ) &&
                *pSttNdIdx <= aPrvIdx )
            {
                // In fact, we should do an JoinNext here, but the Cursor and
                // many other stuff is registered to the current node. That for
                // the node should remain and we do an JoinPrev here.

                // Convert paragraph attributes into hints and set paragraph
                // style at the next node.
                SwTxtNode* pPrev = aPrvIdx.GetNode().GetTxtNode();
                pTxtNode->ChgFmtColl( pPrev->GetTxtColl() );
                pTxtNode->FmtToTxtAttr( pPrev );
                pTxtNode->SwCntntNode::ResetAllAttr();

                if( pPrev->GetpSwAttrSet() )
                    pTxtNode->SwCntntNode::SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPaM->GetBound(sal_True).nNode.GetNode() == pPrev )
                    pPaM->GetBound(sal_True).nContent.Assign( pTxtNode, 0 );
                if( &pPaM->GetBound(sal_False).nNode.GetNode() == pPrev )
                    pPaM->GetBound(sal_False).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }
    }

#ifdef XML_CORE_API
    RemoveUnusedNumRules();
#endif

    delete pSttNdIdx;
    delete pPaM;
    delete pDocElemTokenMap;
#ifdef XML_CORE_API
    delete pI18NMap;
    delete pBodyElemTokenMap;
    delete pTextPElemTokenMap;
    delete pTextPAttrTokenMap;
    delete pStyleStylesElemTokenMap;
    delete pTextListBlockAttrTokenMap;
    delete pTextListBlockElemTokenMap;
#endif
    delete pTableElemTokenMap;
    _FinitItemImport();
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlimp.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.30  2000/09/18 16:05:06  willem.vandorp
      OpenOffice header added.

      Revision 1.29  2000/08/29 07:33:47  mib
      text import continued

      Revision 1.28  2000/08/24 11:16:41  mib
      text import continued

      Revision 1.27  2000/08/10 10:22:16  mib
      #74404#: Adeptions to new XSL/XLink working draft

      Revision 1.26  2000/07/31 09:42:35  mib
      text export continued

      Revision 1.25  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.24  2000/07/07 13:58:36  mib
      text styles using StarOffice API

      Revision 1.23  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.22  2000/05/15 16:53:17  jp
      Changes for Unicode

      Revision 1.21  2000/05/03 12:08:05  mib
      unicode

      Revision 1.20  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.19  2000/02/17 14:40:30  mib
      #70271#: XML table import

      Revision 1.17  2000/01/27 08:59:02  mib
      #70271#: outline numbering

      Revision 1.16  2000/01/20 10:03:16  mib
      #70271#: Lists reworked

      Revision 1.15  2000/01/06 15:08:27  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.14  1999/11/26 11:12:51  mib
      loading of styles only and insert mode

      Revision 1.13  1999/11/22 15:53:39  mib
      split/join nodes correctly

      Revision 1.12  1999/11/12 11:43:03  mib
      using item mapper, part iii

      Revision 1.11  1999/11/10 15:08:09  mib
      Import now uses XMLItemMapper

      Revision 1.10  1999/11/01 11:38:50  mib
      List style import

      Revision 1.9  1999/10/15 12:39:11  mib
      moved styles element from style to office namespace

      Revision 1.8  1999/10/08 11:47:49  mib
      moved some file to SVTOOLS/SVX

      Revision 1.7  1999/10/05 14:31:14  hr
      #65293#: removed redundant const

      Revision 1.6  1999/09/28 10:46:58  mib
      memory leak

      Revision 1.5  1999/09/23 11:54:16  mib
      i18n, token maps and hard paragraph attributes

      Revision 1.4  1999/09/22 11:57:14  mib
      string -> wstring

      Revision 1.3  1999/08/19 12:57:42  MIB
      attribute import added


      Rev 1.2   19 Aug 1999 14:57:42   MIB
   attribute import added

      Rev 1.1   18 Aug 1999 17:05:20   MIB
   Style import

      Rev 1.0   17 Aug 1999 16:32:52   MIB
   Initial revision.

*************************************************************************/

