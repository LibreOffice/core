/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/text/ChapterFormat.hpp>
#include <doc.hxx>
#include <frame.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <chpfld.hxx>
#include <expfld.hxx>
#include <unofldmid.h>
#include <numrule.hxx>

using namespace ::com::sun::star;

namespace
{

OUString removeControlChars(const OUString& sIn)
{
    OUStringBuffer aBuf(sIn.replace('\n', ' '));
    sal_Int32 nLen = aBuf.getLength();
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if (aBuf[i] < ' ')
        {
            sal_Int32 j = i+1;
            while (j<nLen && aBuf[j]<' ') ++j;
            aBuf.remove(i, j-i);
            nLen = aBuf.getLength();
        }
    }
    return aBuf.makeStringAndClear();
}

}

SwChapterFieldType::SwChapterFieldType()
    : SwFieldType( SwFieldIds::Chapter )
{
}

SwFieldType* SwChapterFieldType::Copy() const
{
    return new SwChapterFieldType();
}

// chapter field

SwChapterField::SwChapterField(SwChapterFieldType* pTyp, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat), nLevel( 0 )
{}

OUString SwChapterField::Expand() const
{
    switch( GetFormat() )
    {
        case CF_TITLE:
            return sTitle;
        case CF_NUMBER:
            return sPre + sNumber + sPost;
        case CF_NUM_TITLE:
            return sPre + sNumber + sPost + sTitle;
        case CF_NUM_NOPREPST_TITLE:
            return sNumber + sTitle;
    }
    // CF_NUMBER_NOPREPST
    return sNumber;
}

SwField* SwChapterField::Copy() const
{
    SwChapterField *pTmp =
        new SwChapterField(static_cast<SwChapterFieldType*>(GetTyp()), GetFormat());
    pTmp->nLevel = nLevel;
    pTmp->sTitle = sTitle;
    pTmp->sNumber = sNumber;
    pTmp->sPost = sPost;
    pTmp->sPre = sPre;

    return pTmp;
}

// #i53420#
void SwChapterField::ChangeExpansion(const SwFrame & rFrame,
                                      const SwContentNode* pContentNode,
                                      bool bSrchNum )
{
    SwDoc* pDoc = const_cast<SwDoc*>(pContentNode->GetDoc());

    const SwTextNode* pTextNode = dynamic_cast<const SwTextNode*>(pContentNode);
    if (!pTextNode || !rFrame.IsInDocBody())
    {
        SwPosition aDummyPos( pDoc->GetNodes().GetEndOfContent() );
        pTextNode = GetBodyTextNode( *pDoc, aDummyPos, rFrame );
    }

    if ( pTextNode )
    {
        ChangeExpansion( *pTextNode, bSrchNum );
    }
}

void SwChapterField::ChangeExpansion(const SwTextNode &rTextNd, bool bSrchNum)
{
    sNumber.clear();
    sTitle.clear();
    sPost.clear();
    sPre.clear();

    SwDoc* pDoc = const_cast<SwDoc*>(rTextNd.GetDoc());
    const SwTextNode *pTextNd = rTextNd.FindOutlineNodeOfLevel( nLevel );
    if( pTextNd )
    {
        if( bSrchNum )
        {
            const SwTextNode* pONd = pTextNd;
            do {
                if( pONd && pONd->GetTextColl() )
                {
                    sal_uInt8 nPrevLvl = nLevel;

                    OSL_ENSURE( pONd->GetAttrOutlineLevel() >= 0 && pONd->GetAttrOutlineLevel() <= MAXLEVEL,
                            "<SwChapterField::ChangeExpansion(..)> - outline node with inconsistent outline level. Serious defect." );
                    nLevel = static_cast<sal_uInt8>(pONd->GetAttrOutlineLevel());

                    if( nPrevLvl < nLevel )
                        nLevel = nPrevLvl;
                    else if( SVX_NUM_NUMBER_NONE != pDoc->GetOutlineNumRule()
                            ->Get( nLevel ).GetNumberingType() )
                    {
                        pTextNd = pONd;
                        break;
                    }

                    if( !nLevel-- )
                        break;
                    pONd = pTextNd->FindOutlineNodeOfLevel( nLevel );
                }
                else
                    break;
            } while( true );
        }

        // get the number without Pre-/Post-fixstrings

        if ( pTextNd->IsOutline() )
        {
            // correction of refactoring done by cws swnumtree:
            // retrieve numbering string without prefix and suffix strings
            // as stated in the above german comment.
            sNumber = pTextNd->GetNumString( false );

            SwNumRule* pRule( pTextNd->GetNumRule() );
            if ( pTextNd->IsCountedInList() && pRule )
            {
                int nListLevel = pTextNd->GetActualListLevel();
                if (nListLevel < 0)
                    nListLevel = 0;
                if (nListLevel >= MAXLEVEL)
                    nListLevel = MAXLEVEL - 1;

                const SwNumFormat& rNFormat = pRule->Get(nListLevel);
                sPost = rNFormat.GetSuffix();
                sPre = rNFormat.GetPrefix();
            }
        }
        else
        {
            sNumber = "??";
        }

        sTitle = removeControlChars(pTextNd->GetExpandText(0, -1, false, false, false));

    }
}

bool SwChapterField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BYTE1:
        rAny <<= static_cast<sal_Int8>(nLevel);
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
        assert(false);
    }
    return true;
}

bool SwChapterField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_BYTE1:
    {
        sal_Int8 nTmp = 0;
        rAny >>= nTmp;
        if(nTmp >= 0 && nTmp < MAXLEVEL)
            nLevel = nTmp;
        else
            bRet = false;
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

                default:        SetFormat(CF_NUM_TITLE);
            }
        }
        break;

    default:
        assert(false);
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
