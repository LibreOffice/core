/*************************************************************************
 *
 *  $RCSfile: xmlimp.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-14 16:35:00 $
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
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
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
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTSHAPEIMPORTHELPER_HXX_
#include <xmloff/XMLTextShapeImportHelper.hxx>
#endif
#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_HXX_
#include <xmloff/XMLFontStylesContext.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include "unocrsr.hxx"
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;

sal_Char __READONLY_DATA sXML_np__text[] = "text";
sal_Char __READONLY_DATA sXML_np__table[] = "table";

//----------------------------------------------------------------------------

enum SwXMLDocTokens
{
    XML_TOK_DOC_FONTDECLS,
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_DOC_SCRIPT,
    XML_TOK_OFFICE_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aDocTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_font_decls, XML_TOK_DOC_FONTDECLS  },
    { XML_NAMESPACE_OFFICE, sXML_styles,     XML_TOK_DOC_STYLES     },
    { XML_NAMESPACE_OFFICE, sXML_automatic_styles, XML_TOK_DOC_AUTOSTYLES   },
    { XML_NAMESPACE_OFFICE, sXML_master_styles,  XML_TOK_DOC_MASTERSTYLES   },
    { XML_NAMESPACE_OFFICE, sXML_meta,       XML_TOK_DOC_META       },
    { XML_NAMESPACE_OFFICE, sXML_body,       XML_TOK_DOC_BODY       },
    { XML_NAMESPACE_OFFICE, sXML_script,     XML_TOK_DOC_SCRIPT     },
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
    // process document class
    // global-text, online are handled via document shell;
    // we only handle label documents
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
             sLocalName.equalsAsciiL(sXML_class, sizeof(sXML_class)-1) )
        {
            if (xAttrList->getValueByIndex(nAttr).equalsAsciiL(
                sXML_label, sizeof(sXML_label)-1))
            {
                // OK, we need to set label mode. To do this, tunnel
                // to get the SwDoc, then set label mode.

                Reference<XText> xText(GetImport().GetModel(), UNO_QUERY);
                Reference<XUnoTunnel> xTunnel(
                    GetImport().GetTextImport()->GetText(), UNO_QUERY);
                DBG_ASSERT(xTunnel.is(), "I can't get the Tunnel");
                SwXText* pText = (SwXText*)xTunnel->getSomething(
                    SwXText::getUnoTunnelId());
                if (NULL != pText)
                {
                    SwDoc* pDoc = pText->GetDoc();
                    if (NULL != pDoc)
                        pDoc->SetLabelDoc();
                }
            }
        }
    }
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
    case XML_TOK_DOC_FONTDECLS:
        pContext = GetSwImport().CreateFontDeclsContext( rLocalName,
                                                             xAttrList );
        break;
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
        pContext = GetSwImport().CreateMasterStylesContext( rLocalName,
                                                            xAttrList );
        break;
    case XML_TOK_DOC_META:
        pContext = GetSwImport().CreateMetaContext( rLocalName );
        break;
    case XML_TOK_DOC_SCRIPT:
        pContext = GetSwImport().CreateScriptContext( rLocalName );
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

SwXMLImport::SwXMLImport() :
    bLoadDoc( sal_True ),
    bInsert( sal_False ),
    bBlock( sal_False ),
    nStyleFamilyMask( SFX_STYLE_FAMILY_ALL ),
    pDocElemTokenMap( 0 ),
    pTableElemTokenMap( 0 ),
    pTableItemMapper( 0 ),
    pSttNdIdx( 0 ),
    bProgressValid( sal_False ),
    bShowProgress( sal_True ),
    nProgress( 0 )
{
    _InitItemImport();

}

#ifdef XML_CORE_API
SwXMLImport::SwXMLImport(
        SwDoc& rDoc, const SwPaM& rPaM,
        sal_Bool bLDoc, sal_Bool bInsertMode, sal_uInt16 nStyleFamMask,
        const ::com::sun::star::uno::Reference<
                 ::com::sun::star::frame::XModel > & rModel,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XGraphicObjectResolver > & rEGO,
        SvStorage *pPkg ) :
    SvXMLImport( rModel, rEGO ),
    bLoadDoc( bLDoc ),
    bInsert( bInsertMode ),
    nStyleFamilyMask( nStyleFamMask ),
    pDocElemTokenMap( 0 ),
    pTableElemTokenMap( 0 ),
    pTableItemMapper( 0 ),
    pSttNdIdx( 0 ),
    bProgressValid( sal_False ),
    bShowProgress( sal_True ),
    nProgress( 0 ),
    xPackage( pPkg )
{
    _InitItemImport();

    Reference < XTextRange > xTextRange =
        SwXTextRange::CreateTextRangeFromPosition( &rDoc, *rPaM.GetPoint(), 0 );
    Reference < XText > xText = xTextRange->getText();
    Reference < XTextCursor > xTextCursor =
        xText->createTextCursorByRange( xTextRange );
    GetTextImport()->SetCursor( xTextCursor );
}
#endif

SwXMLImport::~SwXMLImport()
{
    delete pDocElemTokenMap;
    delete pTableElemTokenMap;
    _FinitItemImport();
}

void SwXMLImport::setTextInsertMode(
         const Reference< XTextRange > & rInsertPos )
{
    bInsert = sal_True;

    Reference < XText > xText = rInsertPos->getText();
    Reference < XTextCursor > xTextCursor =
        xText->createTextCursorByRange( rInsertPos );
    GetTextImport()->SetCursor( xTextCursor );
}

void SwXMLImport::setStyleInsertMode( sal_uInt16 nFamilies,
                                      sal_Bool bOverwrite )
{
    bInsert = !bOverwrite;
    nStyleFamilyMask = nFamilies;
    bLoadDoc = sal_False;
}

void SwXMLImport::setBlockMode( )
{
    bBlock = sal_True;
}


const Sequence< sal_Int8 > & SwXMLImport::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXMLImport::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return SvXMLImport::getSomething( rId );
}

void SwXMLImport::startDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    DBG_ASSERT( GetModel().is(), "model is missing" );
    if( !GetModel().is() )
        return;

    // There only is a text cursor by now if we are in insert mode. In any
    // other case we have to create one at the start of the document.
    Reference < XTextCursor > xTextCursor = GetTextImport()->GetCursor();
    if( !xTextCursor.is() )
    {
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();
        xTextCursor = xText->createTextCursor();
        GetTextImport()->SetCursor( xTextCursor );
    }

    Reference<XUnoTunnel> xCrsrTunnel( xTextCursor, UNO_QUERY );
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xCrsrTunnel.is() )
        return;
    SwXTextCursor *pTxtCrsr =
                (SwXTextCursor*)xCrsrTunnel->getSomething(
                                            SwXTextCursor::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    if( !pTxtCrsr )
        return;

    SwDoc *pDoc = pTxtCrsr->GetDoc();
    ASSERT( pDoc, "SwDoc missing" );
    if( !pDoc )
        return;

    if( !IsStylesOnlyMode() )
    {
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( IsInsertMode() )
        {
            Reference<XUnoTunnel> xCrsrTunnel( GetTextImport()->GetCursor(),
                                               UNO_QUERY);
            ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
            SwXTextCursor *pTxtCrsr =
                    (SwXTextCursor*)xCrsrTunnel->getSomething(
                                            SwXTextCursor::getUnoTunnelId() );
            ASSERT( pTxtCrsr, "SwXTextCursor missing" );
            SwPaM *pPaM = pTxtCrsr->GetCrsr();
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

    // We need a draw model to be able to set the z order
    pDoc->MakeDrawModel();
}

void SwXMLImport::endDocument( void )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    if( !IsStylesOnlyMode() )
    {
        Reference<XUnoTunnel> xCrsrTunnel( GetTextImport()->GetCursor(),
                                              UNO_QUERY);
        ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
        SwXTextCursor *pTxtCrsr =
                (SwXTextCursor*)xCrsrTunnel->getSomething(
                                            SwXTextCursor::getUnoTunnelId() );
        ASSERT( pTxtCrsr, "SwXTextCursor missing" );
        SwPaM *pPaM = pTxtCrsr->GetCrsr();
        if( IsInsertMode() && pSttNdIdx->GetIndex() )
        {
            // If we are in insert mode, join the splitted node that is in front
            // of the new content with the first new node. Or in other words:
            // Revert the first split node.
            SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
            SwNodeIndex aNxtIdx( *pSttNdIdx );
            if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
            {
                // If the PaM points to the first new node, move the PaM to the
                // end of the previous node.
                if( pPaM->GetPoint()->nNode == aNxtIdx )
                {
                    pPaM->GetPoint()->nNode = *pSttNdIdx;
                    pPaM->GetPoint()->nContent.Assign( pTxtNode,
                                            pTxtNode->GetTxt().Len() );
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
                // attributes into hints. Otherwise, set the new node's
                // paragraph style at the previous (empty) node.
                SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
                if( pTxtNode->GetTxt().Len() )
                    pDelNd->FmtToTxtAttr( pTxtNode );
                else
                    pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
                pTxtNode->JoinNext();
            }
        }

        SwPosition* pPos = pPaM->GetPoint();
        DBG_ASSERT( !pPos->nContent.GetIndex(), "last paragraph isn't empty" );
        if( !pPos->nContent.GetIndex() )
        {
            SwTxtNode* pCurrNd;
            sal_uInt32 nNodeIdx = pPos->nNode.GetIndex();
            SwDoc *pDoc = pPaM->GetDoc();

            DBG_ASSERT( pPos->nNode.GetNode().IsCntntNode(),
                        "insert position is not a content node" );
            if( !IsInsertMode() )
            {
                // If we're not in insert mode, the last node is deleted.
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
                // Id we're in insert mode, the empty node is joined with
                // the next and the previous one.
                if( pCurrNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPaM->SetMark(); pPaM->DeleteMark();
                    pNextNd->JoinPrev();

                    if( pNextNd->CanJoinPrev(/* &pPos->nNode*/ ) )
                    {
//                      SwTxtNode* pPrevNd = pPos->nNode.GetNode().GetTxtNode();
//                      pPos->nContent.Assign( pPrevNd, 0 );
//                      pPaM->SetMark(); pPaM->DeleteMark();
//                      pPrevNd->JoinNext();
                        pNextNd->JoinPrev();
                    }
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
#ifdef WE_ARE_SURE_WE_DONT_NEED_THIS_CODE_ANY_LONGER_REMOVE_IT
        else if( IsInsertMode() )
        {
            // We're always appending a paragarph at the end of the document,
            // so this code should enver be executed!
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
#endif
    }

    GetTextImport()->ResetCursor();

    delete pSttNdIdx;
    pSttNdIdx = 0;
}


// Locally derive XMLTextShapeImportHelper, so we can take care of the
// form import This is Writer, but not text specific, so it should go
// here!
class SvTextShapeImportHelper : public XMLTextShapeImportHelper
{
    // hold own reference form import helper, because the SvxImport
    // stored in the superclass, from whom we originally got the
    // reference, is already destroyed when we want to use it in the
    // destructor
    UniReference< ::xmloff::OFormLayerXMLImport > rFormImport;

public:

    SvTextShapeImportHelper(SvXMLImport& rImp);
    virtual ~SvTextShapeImportHelper();
};

SvTextShapeImportHelper::SvTextShapeImportHelper(SvXMLImport& rImp) :
    XMLTextShapeImportHelper(rImp)
{
    if (rImp.GetFormImport().is())
    {
        Reference<drawing::XDrawPageSupplier> xSupplier(rImp.GetModel(),
                                                        UNO_QUERY);
        if (xSupplier.is())
        {
            rImp.GetFormImport()->startPage(xSupplier->getDrawPage());
            rFormImport = rImp.GetFormImport();
        }
    }
}

SvTextShapeImportHelper::~SvTextShapeImportHelper()
{
    rFormImport->endPage();
}


XMLShapeImportHelper* SwXMLImport::CreateShapeImport()
{
    return new SvTextShapeImportHelper( *this );
}

SvXMLImportContext *SwXMLImport::CreateFontDeclsContext(
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    XMLFontStylesContext *pFSContext =
            new XMLFontStylesContext( *this, XML_NAMESPACE_OFFICE,
                                      rLocalName, xAttrList,
                                      gsl_getSystemTextEncoding() );
    SetFontDecls( pFSContext );
    return pFSContext;
}

void SwXMLImport::SetProgressRef( sal_Int32 nParagraphs )
{
    if( bShowProgress && !bProgressValid )
    {
        if( !nParagraphs )
            nParagraphs = 250;  // We assume 250 paragarphs here

        // We assume that
        // - 5% are taken reading meta information and fonts
        // - 5% are taken reading styles
        // - 5% are taken reading automatic styles
        // - 85% are taken reading the rest of the document
        // The 85% correspond exactly to the number of paragarphs
        // contained in the document
        nProgressRef = (nParagraphs *100) / 85;
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        if( pProgress )
            pProgress->SetReference( nProgressRef );
        bProgressValid = sal_True;
    }
}

void SwXMLImport::ShowProgress( sal_Int32 nPercent )
{
    if( bShowProgress)
    {
        if( !bProgressValid )
            SetProgressRef( 0 );

        nProgress = nPercent * nProgressRef / 100;
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        if( pProgress )
            pProgress->SetValue( nProgress );
    }
}

uno::Sequence< OUString > SAL_CALL SwXMLImport_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.sax.importer.Writer" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SwXMLImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SwXMLImport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SwXMLImport_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SwXMLImport;
}
