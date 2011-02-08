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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <com/sun/star/text/ChapterFormat.hpp>
#include <doc.hxx>
#include <frame.hxx>        // SwChapterFieldType::ChangeExpansion()
#include <pam.hxx>          // fuer GetBodyTxtNode
#include <ndtxt.hxx>
#include <chpfld.hxx>
#include <expfld.hxx>       // fuer GetBodyTxtNode
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif
#include <numrule.hxx>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung: SwChapterFieldType
 --------------------------------------------------------------------*/


SwChapterFieldType::SwChapterFieldType()
    : SwFieldType( RES_CHAPTERFLD )
{
}


SwFieldType* SwChapterFieldType::Copy() const
{
    return new SwChapterFieldType();
}


/*--------------------------------------------------------------------
    Beschreibung: Kapittelfeld
 --------------------------------------------------------------------*/


SwChapterField::SwChapterField(SwChapterFieldType* pTyp, sal_uInt32 nFmt)
    : SwField(pTyp, nFmt), nLevel( 0 )
{}


String SwChapterField::Expand() const
{
    String sStr( sNumber );
    switch( GetFormat() )
    {
        case CF_TITLE:      sStr = sTitle;  break;

        case CF_NUMBER:
        case CF_NUM_TITLE:  sStr.Insert( sPre, 0 );
                            sStr += sPost;
                            if( CF_NUM_TITLE == GetFormat() )
                                sStr += sTitle;
                            break;

        case CF_NUM_NOPREPST_TITLE: sStr += sTitle; break;
    }
    return sStr;
}


SwField* SwChapterField::Copy() const
{
    SwChapterField *pTmp =
        new SwChapterField((SwChapterFieldType*)GetTyp(), GetFormat());
    pTmp->nLevel = nLevel;
    pTmp->sTitle = sTitle;
    pTmp->sNumber = sNumber;
    pTmp->sPost = sPost;
    pTmp->sPre = sPre;

    return pTmp;
}

// --> OD 2008-02-14 #i53420#
//void SwChapterField::ChangeExpansion( const SwFrm* pFrm,
//                                      const SwTxtNode* pTxtNd,
//                                      sal_Bool bSrchNum )
//{
//    ASSERT( pFrm, "in welchem Frame stehe ich denn?" )
//    SwDoc* pDoc = (SwDoc*)pTxtNd->GetDoc();
//    SwPosition aPos( pDoc->GetNodes().GetEndOfContent() );

//    if( pFrm->IsInDocBody() )
//        aPos.nNode = *pTxtNd;
//    else if( 0 == (pTxtNd = GetBodyTxtNode( *pDoc, aPos, *pFrm )) )
//        // kein TxtNode (Formatierung Kopf/Fusszeile)
//        return;
//    ChangeExpansion(*pTxtNd, bSrchNum);
//}
void SwChapterField::ChangeExpansion(const SwFrm* pFrm,
                                      const SwCntntNode* pCntntNode,
                                      sal_Bool bSrchNum )
{
    ASSERT( pFrm, "in welchem Frame stehe ich denn?" )
    SwDoc* pDoc = (SwDoc*)pCntntNode->GetDoc();

    const SwTxtNode* pTxtNode = dynamic_cast<const SwTxtNode*>(pCntntNode);
    if ( !pTxtNode || !pFrm->IsInDocBody() )
    {
        SwPosition aDummyPos( pDoc->GetNodes().GetEndOfContent() );
        pTxtNode = GetBodyTxtNode( *pDoc, aDummyPos, *pFrm );
    }

    if ( pTxtNode )
    {
        ChangeExpansion( *pTxtNode, bSrchNum );
    }
}
// <--

void SwChapterField::ChangeExpansion(const SwTxtNode &rTxtNd, sal_Bool bSrchNum)
{
    SwDoc* pDoc = (SwDoc*)rTxtNd.GetDoc();
    const SwTxtNode *pTxtNd = rTxtNd.FindOutlineNodeOfLevel( nLevel );
    if( pTxtNd )
    {
        if( bSrchNum )
        {
            const SwTxtNode* pONd = pTxtNd;
            do {
                if( pONd && pONd->GetTxtColl() )
                {
                    sal_uInt8 nPrevLvl = nLevel;

                    // --> OD 2008-04-02 #refactorlists#
//                    nLevel = GetRealLevel( pONd->GetTxtColl()->
//                                            GetOutlineLevel() );
                    //ASSERT( pONd->GetOutlineLevel() >= 0 && pONd->GetOutlineLevel() < MAXLEVEL,   //#outline level,zhaojianwei
                    //        "<SwChapterField::ChangeExpansion(..)> - outline node with inconsistent outline level. Serious defect -> please inform OD." );
                    //nLevel = static_cast<sal_uInt8>(pONd->GetOutlineLevel());
                    ASSERT( pONd->GetAttrOutlineLevel() >= 0 && pONd->GetAttrOutlineLevel() <= MAXLEVEL,
                            "<SwChapterField::ChangeExpansion(..)> - outline node with inconsistent outline level. Serious defect -> please inform OD." );
                    nLevel = static_cast<sal_uInt8>(pONd->GetAttrOutlineLevel());                           //<-end,zhaojianwei
                    // <--

                    if( nPrevLvl < nLevel )
                        nLevel = nPrevLvl;
                    else if( SVX_NUM_NUMBER_NONE != pDoc->GetOutlineNumRule()
                            ->Get( nLevel ).GetNumberingType() )
                    {
                        pTxtNd = pONd;
                        break;
                    }

                    if( !nLevel-- )
                        break;
                    pONd = pTxtNd->FindOutlineNodeOfLevel( nLevel );
                }
                else
                    break;
            } while( sal_True );
        }

        // nur die Nummer besorgen, ohne Pre-/Post-fixstrings

        if ( pTxtNd->IsOutline() )
        {
            // --> OD 2005-11-17 #128041#
            // correction of refactoring done by cws swnumtree:
            // retrieve numbering string without prefix and suffix strings
            // as stated in the above german comment.
            sNumber = pTxtNd->GetNumString( false );
            // <--

            SwNumRule* pRule( pTxtNd->GetNumRule() );
            if ( pTxtNd->IsCountedInList() && pRule )
            {
                const SwNumFmt& rNFmt = pRule->Get( static_cast<sal_uInt16>(pTxtNd->GetActualListLevel()) );
                sPost = rNFmt.GetSuffix();
                sPre = rNFmt.GetPrefix();
            }
            else
                sPost = aEmptyStr, sPre = aEmptyStr;
        }
        else
        {
            sPost = aEmptyStr;
            sPre = aEmptyStr;
            sNumber = String("??", RTL_TEXTENCODING_ASCII_US);
        }

        sTitle = pTxtNd->GetExpandTxt();

        for( xub_StrLen i = 0; i < sTitle.Len(); ++i )
            if( ' ' > sTitle.GetChar( i ) )
                sTitle.Erase( i--, 1 );
    }
    else
    {
        sNumber = aEmptyStr;
        sTitle = aEmptyStr;
        sPost = aEmptyStr;
        sPre = aEmptyStr;
    }
}

/*-----------------05.03.98 16:19-------------------

--------------------------------------------------*/
sal_Bool SwChapterField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BYTE1:
        rAny <<= (sal_Int8)nLevel;
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nRet;
            switch( GetFormat() )
            {
                case CF_NUMBER: nRet = text::ChapterFormat::NUMBER; break;
                case CF_TITLE:  nRet = text::ChapterFormat::NAME; break;
                case CF_NUMBER_NOPREPST:
                    nRet = text::ChapterFormat::DIGIT;
                break;
                case CF_NUM_NOPREPST_TITLE:
                    nRet = text::ChapterFormat::NO_PREFIX_SUFFIX;
                break;
                case CF_NUM_TITLE:
                default:        nRet = text::ChapterFormat::NAME_NUMBER;
            }
            rAny <<= nRet;
        }
        break;

    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*-----------------05.03.98 16:19-------------------

--------------------------------------------------*/
sal_Bool SwChapterField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    sal_Bool bRet = sal_True;
    switch( nWhichId )
    {
    case FIELD_PROP_BYTE1:
    {
        sal_Int8 nTmp = 0;
        rAny >>= nTmp;
        if(nTmp >= 0 && nTmp < MAXLEVEL)
            nLevel = nTmp;
        else
            bRet = sal_False;
        break;
    }

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nVal = 0;
            rAny >>= nVal;
            switch( nVal )
            {
                case text::ChapterFormat::NAME: SetFormat(CF_TITLE); break;
                case text::ChapterFormat::NUMBER:  SetFormat(CF_NUMBER); break;
                case text::ChapterFormat::NO_PREFIX_SUFFIX:
                            SetFormat(CF_NUM_NOPREPST_TITLE);
                break;
                case text::ChapterFormat::DIGIT:
                        SetFormat(CF_NUMBER_NOPREPST);
                break;
                //case text::ChapterFormat::NAME_NUMBER:
                default:        SetFormat(CF_NUM_TITLE);
            }
        }
        break;

    default:
        DBG_ERROR("illegal property");
        bRet = sal_False;
    }
    return bRet;
}
