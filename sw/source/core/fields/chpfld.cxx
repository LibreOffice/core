/*************************************************************************
 *
 *  $RCSfile: chpfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>        // SwChapterFieldType::ChangeExpansion()
#endif
#ifndef _PAM_HXX
#include <pam.hxx>          // fuer GetBodyTxtNode
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>       // fuer GetBodyTxtNode
#endif

#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif
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


void SwChapterField::ChangeExpansion( const SwFrm* pFrm,
                                      const SwTxtNode* pTxtNd,
                                      sal_Bool bSrchNum )
{
    ASSERT( pFrm, "in welchem Frame stehe ich denn?" )
    SwDoc* pDoc = (SwDoc*)pTxtNd->GetDoc();
    SwPosition aPos( pDoc->GetNodes().GetEndOfContent() );

    if( pFrm->IsInDocBody() )
        aPos.nNode = *pTxtNd;
    else if( 0 == (pTxtNd = GetBodyTxtNode( *pDoc, aPos, *pFrm )) )
        // kein TxtNode (Formatierung Kopf/Fusszeile)
        return;

    ASSERT( pTxtNd, "Wo steht das Feld" );
    pTxtNd = pTxtNd->FindOutlineNodeOfLevel( nLevel );
    if( pTxtNd )
    {
        if( bSrchNum )
        {
            const SwTxtNode* pONd = pTxtNd;
            do {
                if( pONd && pONd->GetTxtColl() )
                {
                    BYTE nPrevLvl = nLevel;
                    nLevel = GetRealLevel( pONd->GetTxtColl()->
                                            GetOutlineLevel() );
                    if( nPrevLvl < nLevel )
                        nLevel = nPrevLvl;
                    else if( SVX_NUM_NUMBER_NONE != pDoc->GetOutlineNumRule()
                            ->Get( nLevel ).eType )
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

        const SwNodeNum& rNum = *pTxtNd->GetOutlineNum();
        // nur die Nummer besorgen, ohne Pre-/Post-fixstrings
        sNumber = pDoc->GetOutlineNumRule()->MakeNumString( rNum, sal_False );

        if( NO_NUM > rNum.GetLevel() && !( NO_NUMLEVEL & rNum.GetLevel() ) )
        {
            const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get( rNum.GetLevel() );
            sPost = rNFmt.GetPostfix();
            sPre = rNFmt.GetPrefix();
        }
        else
            sPost = aEmptyStr, sPre = aEmptyStr;

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
BOOL   SwChapterField::QueryValue( com::sun::star::uno::Any& rAny, const String& rProperty ) const
{
    if(rProperty.EqualsAscii(UNO_NAME_LEVEL))
        rAny <<= (sal_Int8)nLevel;
    else if(rProperty.EqualsAscii(UNO_NAME_CHAPTER_FORMAT))
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
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return sal_True;
}
/*-----------------05.03.98 16:19-------------------

--------------------------------------------------*/
BOOL    SwChapterField::PutValue( const com::sun::star::uno::Any& rAny, const String& rProperty )
{
    if(rProperty.EqualsAscii(UNO_NAME_LEVEL))
    {
        sal_Int8 nTmp;
        rAny >>= nTmp;
        if(nTmp >= 0 && nTmp < MAXLEVEL)
            nLevel = nTmp;
        else
            return FALSE;
    }
    else if(rProperty.EqualsAscii(UNO_NAME_CHAPTER_FORMAT))
    {
        sal_Int16 nVal;
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
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return sal_True;
}
