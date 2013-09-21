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

#include <ctype.h>
#include <hintids.hxx>

#include <unotools/charclass.hxx>

#include <vcl/msgbox.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/acorrcfg.hxx>

#include <swwait.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <index.hxx>
#include <pam.hxx>
#include <edimp.hxx>
#include <fesh.hxx>
#include <swundo.hxx>       // for the UndoIds
#include <poolfmt.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <frminf.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <swtable.hxx>
#include <acorrect.hxx>
#include <shellres.hxx>
#include <section.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <comcore.hrc>
#include <numrule.hxx>

using namespace ::com::sun::star;

//JP 16.12.99: definition:
//      from pos cPosEnDash to cPosEmDash all chars changed to endashes,
//      from pos cPosEmDash to cPosEnd    all chars changed to emdashes
//      all other chars are changed to the user configuration

const sal_Unicode pBulletChar[6] = { '+', '*', '-', 0x2013, 0x2014, 0 };
const int cnPosEnDash = 2, cnPosEmDash = 4, cnPosEnd = 5;

const sal_Unicode cStarSymbolEnDash = 0x2013;
const sal_Unicode cStarSymbolEmDash = 0x2014;

SvxSwAutoFmtFlags* SwEditShell::pAutoFmtFlags = 0;

// Number of num-/bullet-paragraph templates. MAXLEVEL will soon be raised
// to x, but not the number of templates. (Artifact from <= 4.0)
const sal_uInt16 cnNumBullColls = 4;

class SwAutoFormat
{
    SvxSwAutoFmtFlags aFlags;
    SwPaM aDelPam;              // a Pam that can be used
    SwNodeIndex aNdIdx;         // the index on the current TextNode
    SwNodeIndex aEndNdIdx;      // index on the end of the area

    SwEditShell* pEditShell;
    SwDoc* pDoc;
    SwTxtNode* pAktTxtNd;       // the current TextNode
    SwTxtFrm* pAktTxtFrm;       // frame of the current TextNode
    CharClass* pCharClass;      // Character classification
    sal_uLong nEndNdIdx;        // for the percentage-display
    LanguageType eCharClassLang;

    sal_uInt16 nLastHeadLvl, nLastCalcHeadLvl;
    sal_uInt16 nLastEnumLvl, nLastCalcEnumLvl;
    sal_uInt16 nRedlAutoFmtSeqId;

    enum
    {
        NONE = 0,
        DELIM = 1,
        DIGIT = 2,
        CHG = 4,
        LOWER_ALPHA = 8,
        UPPER_ALPHA = 16,
        LOWER_ROMAN = 32,
        UPPER_ROMAN = 64,
        NO_DELIM = (DIGIT|LOWER_ALPHA|UPPER_ALPHA|LOWER_ROMAN|UPPER_ROMAN)
    };

    enum Format_Status
    {
        READ_NEXT_PARA,
        TST_EMPTY_LINE,
        TST_ALPHA_LINE,
        GET_ALL_INFO,
        IS_ONE_LINE,
        TST_ENUMERIC,
        TST_IDENT,
        TST_NEG_IDENT,
        TST_TXT_BODY,
        HAS_FMTCOLL,
        IS_ENDE
    } eStat;

    bool bEnde : 1;
    bool bEmptyLine : 1;
    bool bMoreLines : 1;

    // ------------- private methods -----------------------------
    void _GetCharClass( LanguageType eLang );
    CharClass& GetCharClass( LanguageType eLang ) const
    {
        if( !pCharClass || eLang != eCharClassLang )
        {
            SwAutoFormat* pThis = (SwAutoFormat*)this;
            pThis->_GetCharClass( eLang );
        }
        return *pCharClass;
    }

    bool IsSpace( const sal_Unicode c ) const
        { return (' ' == c || '\t' == c || 0x0a == c|| 0x3000 == c /* Jap. space */); }

    void SetColl( sal_uInt16 nId, bool bHdLineOrText = false );
    String GoNextPara();
    bool HasObjects( const SwNode& rNd );

    // TxtNode methods
    const SwTxtNode* GetNextNode() const;
    bool IsEmptyLine( const SwTxtNode& rNd ) const
        {   return rNd.GetTxt().isEmpty() ||
                rNd.GetTxt().getLength() == GetLeadingBlanks( rNd.GetTxt() ); }

    sal_Bool IsOneLine( const SwTxtNode& ) const;
    sal_Bool IsFastFullLine( const SwTxtNode& ) const;
    sal_Bool IsNoAlphaLine( const SwTxtNode&) const;
    sal_Bool IsEnumericChar( const SwTxtNode&) const;
    sal_Bool IsBlanksInString( const SwTxtNode&) const;
    sal_uInt16 CalcLevel( const SwTxtNode&, sal_uInt16 *pDigitLvl = 0 ) const;
    xub_StrLen GetBigIndent( xub_StrLen& rAktSpacePos ) const;

    String& DelLeadingBlanks( String& rStr ) const;
    String& DelTrailingBlanks( String& rStr ) const;
    xub_StrLen GetLeadingBlanks( const String& rStr ) const;
    xub_StrLen GetTrailingBlanks( const String& rStr ) const;

    bool IsFirstCharCapital( const SwTxtNode& rNd ) const;
    sal_uInt16 GetDigitLevel( const SwTxtNode& rTxtNd, xub_StrLen& rPos,
                            String* pPreFix = 0, String* pPostFix = 0,
                            String* pNumTypes = 0 ) const;
    /// get the FORMATED TextFrame
    SwTxtFrm* GetFrm( const SwTxtNode& rTxtNd ) const;

    void BuildIndent();
    void BuildText();
    void BuildTextIndent();
    void BuildEnum( sal_uInt16 nLvl, sal_uInt16 nDigitLevel );
    void BuildNegIndent( SwTwips nSpaces );
    void BuildHeadLine( sal_uInt16 nLvl );

    bool HasSelBlanks( SwPaM& rPam ) const;
    bool HasBreakAttr( const SwTxtNode& ) const;
    void DeleteSel( SwPaM& rPam );
    bool DeleteAktNxtPara( const String& rNxtPara );
    /// delete in the node start and/or end
    void DeleteAktPara( bool bStart = true, bool nEnd = true );
    void DelEmptyLine( bool bTstNextPara = true );
    /// when using multiline paragraphs delete the "left" and/or
    /// "right" margins
    void DelMoreLinesBlanks( bool bWithLineBreaks = false );
    /// delete the previous paragraph
    void DelPrevPara();
    /// execute AutoCorrect on current TextNode
    void AutoCorrect( xub_StrLen nSttPos = 0 );

    bool CanJoin( const SwTxtNode* pTxtNd ) const
    {
        return !bEnde && pTxtNd &&
             !IsEmptyLine( *pTxtNd ) &&
             !IsNoAlphaLine( *pTxtNd) &&
             !IsEnumericChar( *pTxtNd ) &&
             ((STRING_MAXLEN - 50 - pTxtNd->GetTxt().getLength()) >
                    pAktTxtNd->GetTxt().getLength()) &&
             !HasBreakAttr( *pTxtNd );
    }

    /// is a dot at the end ??
    bool IsSentenceAtEnd( const SwTxtNode& rTxtNd ) const;

    bool DoUnderline();
    bool DoTable();

    void _SetRedlineTxt( sal_uInt16 nId );
    bool SetRedlineTxt( sal_uInt16 nId )
        { if( aFlags.bWithRedlining )   _SetRedlineTxt( nId );  return true; }
    bool ClearRedlineTxt()
        { if( aFlags.bWithRedlining )   pDoc->SetAutoFmtRedlineComment(0);  return true; }

public:
    SwAutoFormat( SwEditShell* pEdShell, SvxSwAutoFmtFlags& rFlags,
                SwNodeIndex* pSttNd = 0, SwNodeIndex* pEndNd = 0 );
    ~SwAutoFormat() {
        delete pCharClass;
    }
};

const sal_Unicode* StrChr( const sal_Unicode* pSrc, sal_Unicode c )
{
    while( *pSrc && *pSrc != c )
        ++pSrc;
    return *pSrc ? pSrc : 0;
}

SwTxtFrm* SwAutoFormat::GetFrm( const SwTxtNode& rTxtNd ) const
{
    // get the Frame
    const SwCntntFrm *pFrm = rTxtNd.getLayoutFrm( pEditShell->GetLayout() );
    OSL_ENSURE( pFrm, "For Autoformat a Layout is needed" );
    if( aFlags.bAFmtByInput && !pFrm->IsValid() )
    {
        SwRect aTmpFrm( pFrm->Frm() );
        SwRect aTmpPrt( pFrm->Prt() );
        pFrm->Calc();
        if( pFrm->Frm() != aTmpFrm || pFrm->Prt() != aTmpPrt ||
            ( pFrm->IsTxtFrm() && !((SwTxtFrm*)pFrm)->Paint().IsEmpty() ) )
            pFrm->SetCompletePaint();
    }
    return ((SwTxtFrm*)pFrm)->GetFormatted();
}

void SwAutoFormat::_GetCharClass( LanguageType eLang )
{
    delete pCharClass;
    pCharClass = new CharClass( LanguageTag( eLang ));
    eCharClassLang = eLang;
}

void SwAutoFormat::_SetRedlineTxt( sal_uInt16 nActionId )
{
    OUString sTxt;
    sal_uInt16 nSeqNo = 0;
    if( STR_AUTOFMTREDL_END > nActionId )
    {
        sTxt = ViewShell::GetShellRes()->GetAutoFmtNameLst()[ nActionId ];
        switch( nActionId )
        {
        case STR_AUTOFMTREDL_SET_NUMBULET:
        case STR_AUTOFMTREDL_DEL_MORELINES:

        // AutoCorrect actions
        case STR_AUTOFMTREDL_USE_REPLACE:
        case STR_AUTOFMTREDL_CPTL_STT_WORD:
        case STR_AUTOFMTREDL_CPTL_STT_SENT:
        case STR_AUTOFMTREDL_TYPO:
        case STR_AUTOFMTREDL_UNDER:
        case STR_AUTOFMTREDL_BOLD:
        case STR_AUTOFMTREDL_FRACTION:
        case STR_AUTOFMTREDL_DASH:
        case STR_AUTOFMTREDL_ORDINAL:
        case STR_AUTOFMTREDL_NON_BREAK_SPACE:
            nSeqNo = ++nRedlAutoFmtSeqId;
            break;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    else
        sTxt = "Action text is missing";
#endif

    pDoc->SetAutoFmtRedlineComment( &sTxt, nSeqNo );
}

String SwAutoFormat::GoNextPara()
{
    SwNode* pNewNd = 0;
    do {
        // has to be checked twice before and after incrementation
        if( aNdIdx.GetIndex() >= aEndNdIdx.GetIndex() )
        {
            bEnde = true;
            return aEmptyStr;
        }

        aNdIdx++;
        if( aNdIdx.GetIndex() >= aEndNdIdx.GetIndex() )
        {
            bEnde = true;
            return aEmptyStr;
        }
        else
            pNewNd = &aNdIdx.GetNode();

        // not a TextNode ->
        //      TableNode   : skip table
        //      NoTxtNode   : skip nodes
        //      EndNode     : at the end, terminate
        if( pNewNd->IsEndNode() )
        {
            bEnde = true;
            return aEmptyStr;
        }
        else if( pNewNd->IsTableNode() )
            aNdIdx = *pNewNd->EndOfSectionNode();
        else if( pNewNd->IsSectionNode() )
        {
            const SwSection& rSect = pNewNd->GetSectionNode()->GetSection();
            if( rSect.IsHiddenFlag() || rSect.IsProtectFlag() )
                aNdIdx = *pNewNd->EndOfSectionNode();
        }
    } while( !pNewNd->IsTxtNode() );

    if( !aFlags.bAFmtByInput )
        ::SetProgressState( aNdIdx.GetIndex() + nEndNdIdx - aEndNdIdx.GetIndex(),
                            pDoc->GetDocShell() );

    pAktTxtNd = (SwTxtNode*)pNewNd;
    pAktTxtFrm = GetFrm( *pAktTxtNd );
    return pAktTxtNd->GetTxt();
}

bool SwAutoFormat::HasObjects( const SwNode& rNd )
{
    // Is there something bound to the paragraph in the paragraph
    // like borders, DrawObjects, ...
    bool bRet = false;
    const SwFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < rFmts.size(); ++n )
    {
        const SwFmtAnchor& rAnchor = rFmts[ n ]->GetAnchor();
        if ((FLY_AT_PAGE != rAnchor.GetAnchorId()) &&
            rAnchor.GetCntntAnchor() &&
            &rAnchor.GetCntntAnchor()->nNode.GetNode() == &rNd )
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

const SwTxtNode* SwAutoFormat::GetNextNode() const
{
    if( aNdIdx.GetIndex()+1 >= aEndNdIdx.GetIndex() )
        return 0;
    return pDoc->GetNodes()[ aNdIdx.GetIndex() + 1 ]->GetTxtNode();
}

sal_Bool SwAutoFormat::IsOneLine( const SwTxtNode& rNd ) const
{
    SwTxtFrmInfo aFInfo( GetFrm( rNd ) );
    return aFInfo.IsOneLine();
}

sal_Bool SwAutoFormat::IsFastFullLine( const SwTxtNode& rNd ) const
{
    sal_Bool bRet = aFlags.bRightMargin;
    if( bRet )
    {
        SwTxtFrmInfo aFInfo( GetFrm( rNd ) );
        bRet = aFInfo.IsFilled( aFlags.nRightMargin );
    }
    return bRet;
}

sal_Bool SwAutoFormat::IsEnumericChar( const SwTxtNode& rNd ) const
{
    const OUString& rTxt = rNd.GetTxt();
    String sTmp( rTxt );
    xub_StrLen nBlnks = GetLeadingBlanks( sTmp );
    xub_StrLen nLen = rTxt.getLength() - nBlnks;
    if( !nLen )
        return sal_False;

    // -, +, * separated by blank ??
    if (2 < nLen && IsSpace(rTxt[nBlnks + 1]))
    {
        if (StrChr(pBulletChar, rTxt[nBlnks]))
            return sal_True;
        // Should there be a symbol font at the position?
        SwTxtFrmInfo aFInfo( GetFrm( rNd ) );
        if( aFInfo.IsBullet( nBlnks ))
            return sal_True;
    }

    // 1.) / 1. / 1.1.1 / (1). / (1) / ....
    return USHRT_MAX != GetDigitLevel( rNd, nBlnks );
}

sal_Bool SwAutoFormat::IsBlanksInString( const SwTxtNode& rNd ) const
{
    // Search more that 5 blanks/tabs in the string.
    String sTmp( rNd.GetTxt() );
    DelTrailingBlanks( DelLeadingBlanks( sTmp ));
    const sal_Unicode* pTmp = sTmp.GetBuffer();
    while( *pTmp )
    {
        if( IsSpace( *pTmp ) )
        {
            if( IsSpace( *++pTmp ))     // 2 spaces after each other
            {
                const sal_Unicode* pStt = pTmp;
                while( *pTmp && IsSpace( *++pTmp ))
                    ;
                if( 5 <= pTmp - pStt )
                    return sal_True;
            }
            else
                ++pTmp;
        }
        else
            ++pTmp;
    }
    return sal_False;
}

sal_uInt16 SwAutoFormat::CalcLevel( const SwTxtNode& rNd, sal_uInt16 *pDigitLvl ) const
{
    sal_uInt16 nLvl = 0, nBlnk = 0;
    const OUString& rTxt = rNd.GetTxt();
    if( pDigitLvl )
        *pDigitLvl = USHRT_MAX;

    if( RES_POOLCOLL_TEXT_MOVE == rNd.GetTxtColl()->GetPoolFmtId() )
    {
        if( aFlags.bAFmtByInput )
        {
            nLvl = rNd.GetAutoFmtLvl();
            ((SwTxtNode&)rNd).SetAutoFmtLvl( 0 );
            if( nLvl )
                return nLvl;
        }
        ++nLvl;
    }

    for (xub_StrLen n = 0, nEnd = rTxt.getLength(); n < nEnd; ++n)
    {
        switch (rTxt[n])
        {
        case ' ':   if( 3 == ++nBlnk )
                        ++nLvl, nBlnk = 0;
                    break;
        case '\t':  ++nLvl, nBlnk = 0;
                    break;
        default:
            if( pDigitLvl )
                // test 1.) / 1. / 1.1.1 / (1). / (1) / ....
                *pDigitLvl = GetDigitLevel( rNd, n );
            return nLvl;
        }
    }
    return nLvl;
}

xub_StrLen SwAutoFormat::GetBigIndent( xub_StrLen& rAktSpacePos ) const
{
    SwTxtFrmInfo aFInfo( GetFrm( *pAktTxtNd ) );
    const SwTxtFrm* pNxtFrm = 0;

    if( !bMoreLines )
    {
        const SwTxtNode* pNxtNd = GetNextNode();
        if( !CanJoin( pNxtNd ) || !IsOneLine( *pNxtNd ) )
            return 0;

        pNxtFrm = GetFrm( *pNxtNd );
    }

    return aFInfo.GetBigIndent( rAktSpacePos, pNxtFrm );
}

sal_Bool SwAutoFormat::IsNoAlphaLine( const SwTxtNode& rNd ) const
{
    const String& rStr = rNd.GetTxt();
    if( !rStr.Len() )
        return sal_False;
    // or better: determine via number of AlphaNum and !AlphaNum characters
    xub_StrLen nANChar = 0, nBlnk = 0;

    CharClass& rCC = GetCharClass( rNd.GetSwAttrSet().GetLanguage().GetLanguage() );
    for( xub_StrLen n = 0, nEnd = rStr.Len(); n < nEnd; ++n )
        if( IsSpace( rStr.GetChar( n ) ) )
            ++nBlnk;
        else if( rCC.isLetterNumeric( rStr, n ))
            ++nANChar;

    // If there are 75% of non-alphanumeric characters, then sal_True
    sal_uLong nLen = rStr.Len() - nBlnk;
    nLen = ( nLen * 3 ) / 4;            // long overflow, if the strlen > sal_uInt16
    return xub_StrLen(nLen) < (rStr.Len() - nANChar - nBlnk);
}

bool SwAutoFormat::DoUnderline()
{
    if( !aFlags.bSetBorder )
        return false;

    OUString const& rTxt(pAktTxtNd->GetTxt());
    int eState = 0;
    xub_StrLen nCnt = 0;
    while (nCnt < rTxt.getLength())
    {
        int eTmp = 0;
        switch (rTxt[nCnt])
        {
            case '-': eTmp = 1; break;
            case '_': eTmp = 2; break;
            case '=': eTmp = 3; break;
            case '*': eTmp = 4; break;
            case '~': eTmp = 5; break;
            case '#': eTmp = 6; break;
            default:
                return false;
        }
        if( 0 == eState )
            eState = eTmp;
        else if( eState != eTmp )
            return false;
        ++nCnt;
    }

    if( 2 < nCnt )
    {
        // then underline the previous paragraph if one exists
        DelEmptyLine( false );
        aDelPam.SetMark();
        aDelPam.GetMark()->nContent = 0;

        editeng::SvxBorderLine aLine;
        switch( eState )
        {
        case 1:         // single, 0,05 pt
            aLine.SetBorderLineStyle(table::BorderLineStyle::SOLID);
            aLine.SetWidth( DEF_LINE_WIDTH_0 );
            break;
        case 2:         // single, 1,0 pt
            aLine.SetBorderLineStyle(table::BorderLineStyle::SOLID);
            aLine.SetWidth( DEF_LINE_WIDTH_1 );
            break;
        case 3:         // double, 1,1 pt
            aLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
            aLine.SetWidth( DEF_LINE_WIDTH_0 );
            break;
        case 4:         // double, 4,5 pt
            aLine.SetBorderLineStyle(table::BorderLineStyle::THICKTHIN_SMALLGAP);
            aLine.SetWidth( DEF_LINE_WIDTH_1  );
            break;
        case 5:         // double, 6,0 pt
            aLine.SetBorderLineStyle(table::BorderLineStyle::THINTHICK_SMALLGAP);
            aLine.SetWidth( DEF_LINE_WIDTH_2 );
            break;
        case 6:         // double, 9,0 pt
            aLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
            aLine.SetWidth( DEF_LINE_WIDTH_2 );
            break;
        }
        SfxItemSet aSet(pDoc->GetAttrPool(),
                    RES_PARATR_CONNECT_BORDER, RES_PARATR_CONNECT_BORDER,
                    RES_BOX, RES_BOX,
                    0);
        aSet.Put( SwParaConnectBorderItem( sal_False ) );
        SvxBoxItem aBox( RES_BOX );
        aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
        aBox.SetDistance( 42 );     // ~0,75 mm
        aSet.Put(aBox);
        pDoc->InsertItemSet( aDelPam, aSet, 0 );

        aDelPam.DeleteMark();
    }
    return 2 < nCnt;
}

bool SwAutoFormat::DoTable()
{
    if( !aFlags.bCreateTable || !aFlags.bAFmtByInput ||
        pAktTxtNd->FindTableNode() )
        return false;

    const OUString& rTmp = pAktTxtNd->GetTxt();
    xub_StrLen nSttPlus = GetLeadingBlanks( rTmp );
    xub_StrLen nEndPlus = GetTrailingBlanks( rTmp );
    sal_Unicode cChar;

    if( 2 > nEndPlus - nSttPlus ||
        ( '+' != ( cChar = rTmp[nSttPlus]) && '|' != cChar ) ||
        ( '+' != ( cChar = rTmp[nEndPlus - 1]) && '|' != cChar ))
        return false;

    SwTxtFrmInfo aInfo( pAktTxtFrm );

    xub_StrLen n = nSttPlus;
    std::vector<sal_uInt16> aPosArr;

    while (n < rTmp.getLength())
    {
        switch (rTmp[n])
        {
        case '-':
        case '_':
        case '=':
        case ' ':
        case '\t':
            break;

        case '+':
        case '|':
            aPosArr.push_back( static_cast<sal_uInt16>(aInfo.GetCharPos(n)) );
            break;

        default:
            return false;
        }
        if( ++n == nEndPlus )
            break;
    }

    if( 1 < aPosArr.size() )
    {
        // get the text node's alignment
        sal_uInt16 nColCnt = aPosArr.size() - 1;
        SwTwips nSttPos = aPosArr[ 0 ];
        sal_Int16 eHori;
        switch( pAktTxtNd->GetSwAttrSet().GetAdjust().GetAdjust() )
        {
        case SVX_ADJUST_CENTER:     eHori = text::HoriOrientation::CENTER;    break;
        case SVX_ADJUST_RIGHT:      eHori = text::HoriOrientation::RIGHT;     break;

        default:
            if( nSttPos )
            {
                eHori = text::HoriOrientation::NONE;
                // then - as last - we need to add the current frame width into the array
                aPosArr.push_back( static_cast<sal_uInt16>(pAktTxtFrm->Frm().Width()) );
            }
            else
                eHori = text::HoriOrientation::LEFT;
            break;
        }

        // then create a table that matches the character
        DelEmptyLine();
        SwNodeIndex aIdx( aDelPam.GetPoint()->nNode );
        aDelPam.Move( fnMoveForward );
        pDoc->InsertTable( SwInsertTableOptions( tabopts::ALL_TBL_INS_ATTR , 1 ),
                           *aDelPam.GetPoint(), 1, nColCnt, eHori,
                           0, &aPosArr );
        aDelPam.GetPoint()->nNode = aIdx;
    }
    return 1 < aPosArr.size();
}

String& SwAutoFormat::DelLeadingBlanks( String& rStr ) const
{
    xub_StrLen nL;
    xub_StrLen n;

    for( nL = rStr.Len(), n = 0; n < nL && IsSpace( rStr.GetChar(n) ); ++n )
        ;
    if( n ) // no Spaces
        rStr.Erase( 0, n );
    return rStr;
}

String& SwAutoFormat::DelTrailingBlanks( String& rStr ) const
{
    xub_StrLen nL = rStr.Len(), n = nL;
    if( !nL )
        return rStr;

    while( --n && IsSpace( rStr.GetChar( n ) )  )
        ;
    if( n+1 != nL ) // no Spaces
        rStr.Erase( n+1 );
    return rStr;
}

xub_StrLen SwAutoFormat::GetLeadingBlanks( const String& rStr ) const
{
    xub_StrLen nL;
    xub_StrLen n;

    for( nL = rStr.Len(), n = 0; n < nL && IsSpace( rStr.GetChar( n ) ); ++n )
        ;
    return n;
}

xub_StrLen SwAutoFormat::GetTrailingBlanks( const String& rStr ) const
{
    xub_StrLen nL = rStr.Len(), n = nL;
    if( !nL )
        return 0;

    while( --n && IsSpace( rStr.GetChar( n ) )  )
        ;
    return ++n;
}

bool SwAutoFormat::IsFirstCharCapital( const SwTxtNode& rNd ) const
{
    const OUString& rTxt = rNd.GetTxt();
    for (xub_StrLen n = 0, nEnd = rTxt.getLength(); n < nEnd; ++n)
        if (!IsSpace(rTxt[n]))
        {
            CharClass& rCC = GetCharClass( rNd.GetSwAttrSet().
                                        GetLanguage().GetLanguage() );
            sal_Int32 nCharType = rCC.getCharacterType( rTxt, n );
            return CharClass::isLetterType( nCharType ) &&
                   0 != ( i18n::KCharacterType::UPPER &
                                                    nCharType );
        }
    return false;
}

sal_uInt16 SwAutoFormat::GetDigitLevel( const SwTxtNode& rNd, xub_StrLen& rPos,
        String* pPreFix, String* pPostFix, String* pNumTypes ) const
{
    // check for 1.) / 1. / 1.1.1 / (1). / (1) / ....
    const OUString& rTxt = rNd.GetTxt();
    xub_StrLen nPos = rPos;
    int eScan = NONE;

    sal_uInt16 nStart = 0;
    sal_uInt8 nDigitLvl = 0, nDigitCnt = 0;
    // count number of parenthesis to assure a sensible order is found
    sal_uInt16 nOpeningParentheses = 0;
    sal_uInt16 nClosingParentheses = 0;

    CharClass& rCC = GetCharClass( rNd.GetSwAttrSet().GetLanguage().GetLanguage() );

    while (nPos < rTxt.getLength() && nDigitLvl < MAXLEVEL - 1)
    {
        const sal_Unicode cCurrentChar = rTxt[nPos];
        if( ('0' <= cCurrentChar &&  '9' >= cCurrentChar) ||
            (0xff10 <= cCurrentChar &&  0xff19 >= cCurrentChar) )
        {
            if( eScan & DELIM )
            {
                if( eScan & CHG )   // not if it starts with a number
                {
                    ++nDigitLvl;
                    if( pPostFix )
                        *pPostFix += (sal_Unicode)1;
                }

                if( pNumTypes )
                    *pNumTypes += (sal_Unicode)('0' + SVX_NUM_ARABIC);

                eScan = eScan | CHG;
            }
            else if( pNumTypes && !(eScan & DIGIT) )
                *pNumTypes += (sal_Unicode)('0' + SVX_NUM_ARABIC);

            eScan &= ~DELIM;        // remvoe Delim
            if( 0 != (eScan & ~CHG) && DIGIT != (eScan & ~CHG))
                return USHRT_MAX;

            eScan |= DIGIT;         // add Digit
            if( 3 == ++nDigitCnt )  // more than 2 numbers are not an enum anymore
                return USHRT_MAX;

            nStart *= 10;
            nStart += cCurrentChar <= '9' ? cCurrentChar - '0' : cCurrentChar - 0xff10;
        }
        else if( rCC.isAlpha( rTxt, nPos ) )
        {
            bool bIsUpper =
                0 != ( i18n::KCharacterType::UPPER &
                                        rCC.getCharacterType( rTxt, nPos ));
            sal_Unicode cLow = rCC.lowercase(rTxt, nPos, 1)[0], cNumTyp;
            int eTmpScan;

            // Roman numbers are "mdclxvi". Since we want to start numbering with c or d more often,
            // convert first to characters and later to roman numbers if needed.
#ifdef WITH_ALPHANUM_AS_NUMFMT
            // detection of 'c' and 'd' a ROMAN numbering should not be done here
            if( 256 > cLow  &&( (eScan & (LOWER_ROMAN|UPPER_ROMAN))
                                    ? strchr( "mdclxvi", cLow )
                                    : strchr( "mlxvi", cLow ) ))
#else
            if( 256 > cLow  && ( strchr( "mdclxvi", cLow ) ))
#endif
            {
                if( bIsUpper )
                    cNumTyp = '0' + SVX_NUM_ROMAN_UPPER, eTmpScan = UPPER_ROMAN;
                else
                    cNumTyp = '0' + SVX_NUM_ROMAN_LOWER, eTmpScan = LOWER_ROMAN;
            }
            else if( bIsUpper )
                cNumTyp = '0' + SVX_NUM_CHARS_UPPER_LETTER, eTmpScan = UPPER_ALPHA;
            else
                cNumTyp = '0' + SVX_NUM_CHARS_LOWER_LETTER, eTmpScan = LOWER_ALPHA;


            // Switch to roman numbers (only for c/d!)
            if( 1 == nDigitCnt && ( eScan & (UPPER_ALPHA|LOWER_ALPHA) ) &&
                ( 3 == nStart || 4 == nStart) && 256 > cLow &&
                strchr( "mdclxvi", cLow ) &&
                (( eScan & UPPER_ALPHA ) ? (eTmpScan & (UPPER_ALPHA|UPPER_ROMAN))
                                         : (eTmpScan & (LOWER_ALPHA|LOWER_ROMAN))) )
            {
                sal_Unicode c = '0';
                nStart = 3 == nStart ? 100 : 500;
                if( UPPER_ALPHA == eTmpScan )
                    eTmpScan = UPPER_ROMAN, c += SVX_NUM_ROMAN_UPPER;
                else
                    eTmpScan = LOWER_ROMAN, c += SVX_NUM_ROMAN_LOWER;

                ( eScan &= ~(UPPER_ALPHA|LOWER_ALPHA)) |= eTmpScan;
                if( pNumTypes )
                    pNumTypes->SetChar( pNumTypes->Len() - 1, c );
            }

            if( eScan & DELIM )
            {
                if( eScan & CHG )   // not if it starts with a number
                {
                    ++nDigitLvl;
                    if( pPostFix )
                        *pPostFix += (sal_Unicode)1;
                }

                if( pNumTypes )
                    *pNumTypes += cNumTyp;
                eScan = eScan | CHG;
            }
            else if( pNumTypes && !(eScan & eTmpScan) )
                *pNumTypes += cNumTyp;

            eScan &= ~DELIM;        // remove Delim

            // if another type is set, stop here
            if( 0 != ( eScan & ~CHG ) && eTmpScan != ( eScan & ~CHG ))
                return USHRT_MAX;

            if( eTmpScan & (UPPER_ALPHA | LOWER_ALPHA) )
            {
                // allow characters only if they appear once
                return USHRT_MAX;
            }
            else
            {
                // roman numbers, check if valid characters
                sal_uInt16 nVal;
                bool bError = false;
                switch( cLow )
                {
                case 'm':   nVal = 1000; goto CHECK_ROMAN_1;
                case 'd':   nVal =  500; goto CHECK_ROMAN_5;
                case 'c':   nVal =  100; goto CHECK_ROMAN_1;
                case 'l':   nVal =   50; goto CHECK_ROMAN_5;
                case 'x':   nVal =   10; goto CHECK_ROMAN_1;
                case 'v':   nVal =    5; goto CHECK_ROMAN_5;

CHECK_ROMAN_1:
                    {
                        int nMod5 = nStart % (nVal * 5);
                        int nLast = nStart % nVal;
                        int n10 = nVal / 10;

                        if( nMod5 == ((3 * nVal) + n10 ) ||
                            nMod5 == ((4 * nVal) + n10 ) ||
                            nLast == n10 )
                            nStart = static_cast<sal_uInt16>(nStart + (n10 * 8));
                        else if( nMod5 == 0 ||
                                 nMod5 == (1 * nVal) ||
                                 nMod5 == (2 * nVal) )
                            nStart = nStart + nVal;
                        else
                            bError = true;
                    }
                    break;

CHECK_ROMAN_5:
                    {
                        if( ( nStart / nVal ) & 1 )
                            bError = true;
                        else
                        {
                            int nMod = nStart % nVal;
                            int n10 = nVal / 5;
                            if( n10 == nMod )
                                nStart = static_cast<sal_uInt16>(nStart + (3 * n10));
                            else if( 0 == nMod )
                                nStart = nStart + nVal;
                            else
                                bError = true;
                        }
                    }
                    break;

                case 'i':
                        if( nStart % 5 >= 3 )
                            bError = true;
                        else
                            nStart += 1;
                        break;

                default:
                    bError = true;
                }

                if( bError )
                    return USHRT_MAX;
            }
            eScan |= eTmpScan;          // add Digit
            ++nDigitCnt;
        }
        else if( (256 > cCurrentChar &&
                 strchr( ".)(", cCurrentChar )) ||
                 0x3002 == cCurrentChar /* Chinese trad. dot */||
                 0xff0e == cCurrentChar /* Japanese dot */||
                 0xFF08 == cCurrentChar /* opening bracket Chin./Jap.*/||
                 0xFF09 == cCurrentChar )/* closing bracket Chin./Jap. */
        {
            if(cCurrentChar == '(' || cCurrentChar == 0xFF09)
                nOpeningParentheses++;
            else if(cCurrentChar == ')'|| cCurrentChar == 0xFF08)
                nClosingParentheses++;
            // only if no numbers were read until here
            if( pPreFix && !( eScan & ( NO_DELIM | CHG )) )
                *pPreFix += rTxt[nPos];
            else if( pPostFix )
                *pPostFix += rTxt[nPos];

            if( NO_DELIM & eScan )
            {
                eScan |= CHG;
                if( pPreFix )
                    (*pPreFix += (sal_Unicode)1)
                              += OUString::number( nStart );
            }
            eScan &= ~NO_DELIM;     // remove Delim
            eScan |= DELIM;         // add Digit
            nDigitCnt = 0;
            nStart = 0;
        }
        else
            break;
        ++nPos;
    }
    if( !( CHG & eScan ) || rPos == nPos ||
        nPos == rTxt.getLength() || !IsSpace(rTxt[nPos]) ||
        (nOpeningParentheses > nClosingParentheses))
        return USHRT_MAX;

    if( (NO_DELIM & eScan) && pPreFix )     // do not forget the last one
        (*pPreFix += (sal_Unicode)1) += OUString::number( nStart );

    rPos = nPos;
    return nDigitLvl;       // 0 .. 9 (MAXLEVEL - 1)
}

void SwAutoFormat::SetColl( sal_uInt16 nId, bool bHdLineOrText )
{
    aDelPam.DeleteMark();
    aDelPam.GetPoint()->nNode = aNdIdx;
    aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, 0 );

    // keep hard tabs, alignment, language, hyphenation, DropCaps and nearly all frame attributes
    SfxItemSet aSet( pDoc->GetAttrPool(),
                        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
                        RES_PARATR_TABSTOP, RES_PARATR_DROP,
                        RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
                        RES_BACKGROUND, RES_SHADOW,
                        0 );

    if( pAktTxtNd->HasSwAttrSet() )
    {
        aSet.Put( *pAktTxtNd->GetpSwAttrSet() );
        // take HeaderLine/TextBody only if centered or right aligned, otherwise only justification
        SvxAdjustItem* pAdj;
        if( SFX_ITEM_SET == aSet.GetItemState( RES_PARATR_ADJUST,
                        sal_False, (const SfxPoolItem**)&pAdj ))
        {
            SvxAdjust eAdj = pAdj->GetAdjust();
            if( bHdLineOrText ? (SVX_ADJUST_RIGHT != eAdj &&
                                 SVX_ADJUST_CENTER != eAdj)
                              : SVX_ADJUST_BLOCK != eAdj )
                aSet.ClearItem( RES_PARATR_ADJUST );
        }
    }

    pDoc->SetTxtFmtCollByAutoFmt( *aDelPam.GetPoint(), nId, &aSet );
}

bool SwAutoFormat::HasSelBlanks( SwPaM& rPam ) const
{
    // Is there a Blank at the beginning or end?
    // Do not delete it, it will be inserted again.
    SwPosition * pPos = rPam.End();
    xub_StrLen nBlnkPos = pPos->nContent.GetIndex();
    SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
    if (nBlnkPos && nBlnkPos-- < pTxtNd->GetTxt().getLength() &&
        (' ' == pTxtNd->GetTxt()[nBlnkPos]))
        pPos->nContent--;
    else
    {
        pPos = rPam.GetPoint() == pPos ? rPam.GetMark() : rPam.GetPoint();
        nBlnkPos = pPos->nContent.GetIndex();
        pTxtNd = pPos->nNode.GetNode().GetTxtNode();
        if (nBlnkPos < pTxtNd->GetTxt().getLength() &&
            (' ' == pTxtNd->GetTxt()[nBlnkPos]))
            pPos->nContent++;
        else
            return false;
    }
    return true;
}

bool SwAutoFormat::HasBreakAttr( const SwTxtNode& rTxtNd ) const
{
    const SfxItemSet* pSet = rTxtNd.GetpSwAttrSet();
    if( !pSet )
        return false;

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False, &pItem )
        && SVX_BREAK_NONE != ((SvxFmtBreakItem*)pItem)->GetBreak() )
        return true;

    if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, sal_False, &pItem )
        && ((SwFmtPageDesc*)pItem)->GetPageDesc()
        && nsUseOnPage::PD_NONE != ((SwFmtPageDesc*)pItem)->GetPageDesc()->GetUseOn() )
        return true;
    return false;
}

/// Is there a dot at the end?
bool SwAutoFormat::IsSentenceAtEnd( const SwTxtNode& rTxtNd ) const
{
    const String& rStr = rTxtNd.GetTxt();
    xub_StrLen n = rStr.Len();
    if( !n )
        return true;

    while( --n && IsSpace( rStr.GetChar( n  ) ) )
        ;
    return '.' == rStr.GetChar( n );
}

/// Delete beginning and/or end in a node
void SwAutoFormat::DeleteAktPara( bool bStart, bool bEnd )
{
    if( aFlags.bAFmtByInput
        ? aFlags.bAFmtByInpDelSpacesAtSttEnd
        : aFlags.bAFmtDelSpacesAtSttEnd )
    {
        // delete blanks at the end of the current and at the beginning of the next one
        aDelPam.DeleteMark();
        aDelPam.GetPoint()->nNode = aNdIdx;
        xub_StrLen nPos(0);
        if( bStart && 0 != ( nPos = GetLeadingBlanks( pAktTxtNd->GetTxt() )))
        {
            aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, 0 );
            aDelPam.SetMark();
            aDelPam.GetPoint()->nContent = nPos;
            DeleteSel( aDelPam );
            aDelPam.DeleteMark();
        }
        if (bEnd && pAktTxtNd->GetTxt().getLength() !=
                    ( nPos = GetTrailingBlanks( pAktTxtNd->GetTxt() )) )
        {
            aDelPam.GetPoint()->nContent.Assign(
                    pAktTxtNd, pAktTxtNd->GetTxt().getLength());
            aDelPam.SetMark();
            aDelPam.GetPoint()->nContent = nPos;
            DeleteSel( aDelPam );
            aDelPam.DeleteMark();
        }
    }
}

void SwAutoFormat::DeleteSel( SwPaM& rDelPam )
{
    if( aFlags.bWithRedlining )
    {
        // Add to Shell-Cursor-Ring so that DelPam will be moved as well!
        SwPaM* pShCrsr = pEditShell->_GetCrsr();
        SwPaM aTmp( *pAktTxtNd, 0, pShCrsr );

        Ring *pPrev = rDelPam.GetPrev();
        rDelPam.MoveRingTo( pShCrsr );

        pEditShell->DeleteSel( rDelPam );

        // and remove Pam again:
        Ring *p, *pNext = (Ring*)&rDelPam;
        do {
            p = pNext;
            pNext = p->GetNext();
            p->MoveTo( &rDelPam );
        } while( p != pPrev );

        aNdIdx = aTmp.GetPoint()->nNode;
        pAktTxtNd = aNdIdx.GetNode().GetTxtNode();
    }
    else
        pEditShell->DeleteSel( rDelPam );
}

bool SwAutoFormat::DeleteAktNxtPara( const String& rNxtPara )
{
    // delete blanks at the end of the current and at the beginning of the next one
    aDelPam.DeleteMark();
    aDelPam.GetPoint()->nNode = aNdIdx;
    aDelPam.GetPoint()->nContent.Assign( pAktTxtNd,
                    GetTrailingBlanks( pAktTxtNd->GetTxt() ) );
    aDelPam.SetMark();

    aDelPam.GetPoint()->nNode++;
    SwTxtNode* pTNd = aDelPam.GetNode()->GetTxtNode();
    if( !pTNd )
    {
        // then delete only up to end of the paragraph
        aDelPam.GetPoint()->nNode--;
        aDelPam.GetPoint()->nContent = pAktTxtNd->GetTxt().getLength();
    }
    else
        aDelPam.GetPoint()->nContent.Assign( pTNd,
                            GetLeadingBlanks( rNxtPara ));

    // Is there a Blank at the beginning or end?
    // Do not delete it, it will be inserted again.
    bool bHasBlnks = HasSelBlanks( aDelPam );

    if( *aDelPam.GetPoint() != *aDelPam.GetMark() )
        DeleteSel( aDelPam );
    aDelPam.DeleteMark();

    return !bHasBlnks;
}

void SwAutoFormat::DelEmptyLine( bool bTstNextPara )
{
    SetRedlineTxt( STR_AUTOFMTREDL_DEL_EMPTY_PARA );
    // delete blanks in empty paragraph
    aDelPam.DeleteMark();
    aDelPam.GetPoint()->nNode = aNdIdx;
    aDelPam.GetPoint()->nContent.Assign(
            pAktTxtNd, pAktTxtNd->GetTxt().getLength() );
    aDelPam.SetMark();

    aDelPam.GetMark()->nNode--;
    SwTxtNode* pTNd = aDelPam.GetNode( sal_False )->GetTxtNode();
    if( pTNd )
        // first use the previous text node
        aDelPam.GetMark()->nContent.Assign(pTNd, pTNd->GetTxt().getLength());
    else if( bTstNextPara )
    {
        // then try the next (at the beginning of a Doc, table cells, borders, ...)
        aDelPam.GetMark()->nNode += 2;
        pTNd = aDelPam.GetNode( sal_False )->GetTxtNode();
        if( pTNd )
        {
            aDelPam.GetMark()->nContent.Assign( pTNd, 0 );
            aDelPam.GetPoint()->nContent = 0;
        }
    }
    else
    {
        aDelPam.GetMark()->nNode = aNdIdx;
        aDelPam.GetMark()->nContent = 0;
        pTNd = pAktTxtNd;
    }
    if( pTNd )
        DeleteSel( aDelPam );

    aDelPam.DeleteMark();
    ClearRedlineTxt();
}

void SwAutoFormat::DelMoreLinesBlanks( bool bWithLineBreaks )
{
    if( aFlags.bAFmtByInput
        ? aFlags.bAFmtByInpDelSpacesBetweenLines
        : aFlags.bAFmtDelSpacesBetweenLines )
    {
        // delete all blanks on the left and right of the indentation
        aDelPam.DeleteMark();
        aDelPam.GetPoint()->nNode = aNdIdx;
        aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, 0 );

        SwTxtFrmInfo aFInfo( pAktTxtFrm );
        aFInfo.GetSpaces( aDelPam, !aFlags.bAFmtByInput || bWithLineBreaks );

        SwPaM* pNxt;
        do {
            pNxt = (SwPaM*)aDelPam.GetNext();
            if( pNxt->HasMark() && *pNxt->GetPoint() != *pNxt->GetMark() )
            {
                bool bHasBlnks = HasSelBlanks( *pNxt );
                DeleteSel( *pNxt );
                if( !bHasBlnks )
                {
                    pDoc->InsertString( *pNxt, OUString(' ') );
                }
            }

            if( pNxt == &aDelPam )
                break;
            delete pNxt;
        } while( true );

        aDelPam.DeleteMark();
    }
}

// delete the previous paragraph
void SwAutoFormat::DelPrevPara()
{
    aDelPam.DeleteMark();
    aDelPam.GetPoint()->nNode = aNdIdx;
    aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, 0 );
    aDelPam.SetMark();

    aDelPam.GetPoint()->nNode--;
    SwTxtNode* pTNd = aDelPam.GetNode()->GetTxtNode();
    if( pTNd )
    {
        // erstmal den vorherigen Textnode benutzen.
        aDelPam.GetPoint()->nContent.Assign(pTNd, pTNd->GetTxt().getLength());
        DeleteSel( aDelPam );
    }
    aDelPam.DeleteMark();
}

void SwAutoFormat::BuildIndent()
{
    SetRedlineTxt( STR_AUTOFMTREDL_SET_TMPL_INDENT );

    // read all succeeding paragraphs that belong to this indentation
    bool bBreak = true;
    if( bMoreLines )
        DelMoreLinesBlanks( true );
    else
        bBreak = !IsFastFullLine( *pAktTxtNd ) ||
                IsBlanksInString( *pAktTxtNd ) ||
                IsSentenceAtEnd( *pAktTxtNd );
    SetColl( RES_POOLCOLL_TEXT_IDENT );
    if( !bBreak )
    {
        SetRedlineTxt( STR_AUTOFMTREDL_DEL_MORELINES );
        const SwTxtNode* pNxtNd = GetNextNode();
        if( pNxtNd && !bEnde )
        {
            do {
                bBreak = !IsFastFullLine( *pNxtNd ) ||
                        IsBlanksInString( *pNxtNd ) ||
                        IsSentenceAtEnd( *pNxtNd );
                if( DeleteAktNxtPara( pNxtNd->GetTxt() ))
                {
                    pDoc->InsertString( aDelPam, OUString(' ') );
                }
                if( bBreak )
                    break;
                pNxtNd = GetNextNode();
            } while( CanJoin( pNxtNd ) &&
                    !CalcLevel( *pNxtNd ) );
        }
    }
    DeleteAktPara( true, true );
    AutoCorrect();
}

void SwAutoFormat::BuildTextIndent()
{
    SetRedlineTxt( STR_AUTOFMTREDL_SET_TMPL_TEXT_INDENT);
    // read all succeeding paragraphs that belong to this indentation
    bool bBreak = true;
    if( bMoreLines )
        DelMoreLinesBlanks( true );
    else
        bBreak = !IsFastFullLine( *pAktTxtNd ) ||
                    IsBlanksInString( *pAktTxtNd ) ||
                    IsSentenceAtEnd( *pAktTxtNd );

    if( aFlags.bAFmtByInput )
        pAktTxtNd->SetAutoFmtLvl( (sal_uInt8)CalcLevel( *pAktTxtNd ) );

    SetColl( RES_POOLCOLL_TEXT_MOVE );
    if( !bBreak )
    {
        SetRedlineTxt( STR_AUTOFMTREDL_DEL_MORELINES );
        const SwTxtNode* pNxtNd = GetNextNode();
        while(  CanJoin( pNxtNd ) &&
                CalcLevel( *pNxtNd ) )
        {
            bBreak = !IsFastFullLine( *pNxtNd ) || IsBlanksInString( *pNxtNd ) ||
                    IsSentenceAtEnd( *pNxtNd );
            if( DeleteAktNxtPara( pNxtNd->GetTxt() ) )
            {
                pDoc->InsertString( aDelPam, OUString(' ') );
            }
            if( bBreak )
                break;
            pNxtNd = GetNextNode();
        }
    }
    DeleteAktPara( true, true );
    AutoCorrect();
}

void SwAutoFormat::BuildText()
{
    SetRedlineTxt( STR_AUTOFMTREDL_SET_TMPL_TEXT );
    // read all succeeding paragraphs that belong to this text without indentation
    bool bBreak = true;
    if( bMoreLines )
        DelMoreLinesBlanks();
    else
        bBreak = !IsFastFullLine( *pAktTxtNd ) ||
                    IsBlanksInString( *pAktTxtNd ) ||
                    IsSentenceAtEnd( *pAktTxtNd );
    SetColl( RES_POOLCOLL_TEXT, true );
    if( !bBreak )
    {
        SetRedlineTxt( STR_AUTOFMTREDL_DEL_MORELINES );
        const SwTxtNode* pNxtNd = GetNextNode();
        while(  CanJoin( pNxtNd ) &&
                !CalcLevel( *pNxtNd ) )
        {
            bBreak = !IsFastFullLine( *pNxtNd ) || IsBlanksInString( *pNxtNd ) ||
                    IsSentenceAtEnd( *pNxtNd );
            if( DeleteAktNxtPara( pNxtNd->GetTxt() ) )
            {
                pDoc->InsertString( aDelPam, OUString(' ') );
            }
            if( bBreak )
                break;
            const SwTxtNode* pCurrNode = pNxtNd;
            pNxtNd = GetNextNode();
            if(!pNxtNd || pCurrNode == pNxtNd)
                break;
        }
    }
    DeleteAktPara( true, true );
    AutoCorrect();
}

void SwAutoFormat::BuildEnum( sal_uInt16 nLvl, sal_uInt16 nDigitLevel )
{
    SetRedlineTxt( STR_AUTOFMTREDL_SET_NUMBULET );

    bool bBreak = true;

    // first, determine current indentation and frame width
    SwTwips nFrmWidth = pAktTxtFrm->Prt().Width();;
    SwTwips nLeftTxtPos;
    {
        sal_Int32 nPos(0);
        while (nPos < pAktTxtNd->GetTxt().getLength() &&
               IsSpace(pAktTxtNd->GetTxt()[nPos]))
        {
            ++nPos;
        }

        SwTxtFrmInfo aInfo( pAktTxtFrm );
        nLeftTxtPos = aInfo.GetCharPos(nPos);
        nLeftTxtPos -= pAktTxtNd->GetSwAttrSet().GetLRSpace().GetLeft();
    }

    if( bMoreLines )
        DelMoreLinesBlanks();
    else
        bBreak = !IsFastFullLine( *pAktTxtNd ) ||
                    IsBlanksInString( *pAktTxtNd ) ||
                    IsSentenceAtEnd( *pAktTxtNd );
    sal_Bool bRTL = pEditShell->IsInRightToLeftText();
    DeleteAktPara( true, true );

    bool bChgBullet = false, bChgEnum = false;
    xub_StrLen nAutoCorrPos = 0;

    // if numbering is set, get the current one
    SwNumRule aRule( pDoc->GetUniqueNumRuleName(),
                     // #i89178#
                     numfunc::GetDefaultPositionAndSpaceMode() );

    const SwNumRule* pCur = 0;
    if( aFlags.bSetNumRule && 0 != (pCur = pAktTxtNd->GetNumRule()) )
        aRule = *pCur;

    // replace bullet character with defined one
    const String& rStr = pAktTxtNd->GetTxt();
    xub_StrLen nTxtStt = 0, nOrigTxtStt = 0;
    const sal_Unicode* pFndBulletChr;
    if( aFlags.bChgEnumNum &&
        2 < rStr.Len() &&
        0 != ( pFndBulletChr = StrChr( pBulletChar, rStr.GetChar( nTxtStt ) ))
        && IsSpace( rStr.GetChar( nTxtStt + 1 ) ) )
    {
        if( aFlags.bAFmtByInput )
        {
            if( aFlags.bSetNumRule )
            {
                SwCharFmt* pCFmt = pDoc->GetCharFmtFromPool(
                                            RES_POOLCHR_BUL_LEVEL );
                bChgBullet = true;
                // Was the format already somewhere adjusted?
                if( !aRule.GetNumFmt( nLvl ) )
                {
                    int nBulletPos = pFndBulletChr - pBulletChar;
                    sal_Unicode cBullChar;
                    const Font* pBullFnt( 0 );
                    if( nBulletPos < cnPosEnDash )
                    {
                        cBullChar = aFlags.cBullet;
                        pBullFnt = &aFlags.aBulletFont;
                    }
                    else
                    {
                        cBullChar = nBulletPos < cnPosEmDash
                                        ? cStarSymbolEnDash
                                        : cStarSymbolEmDash;
                        // #i63395#
                        // Only apply user defined default bullet font
                        if ( numfunc::IsDefBulletFontUserDefined() )
                        {
                            pBullFnt = &numfunc::GetDefBulletFont();
                        }
                    }

                    sal_uInt16 nAbsPos = lBullIndent;
                    sal_uInt16 nSpaceSteps = nLvl
                                            ? sal_uInt16(nLeftTxtPos / nLvl)
                                            : lBullIndent;
                    for( sal_uInt8 n = 0; n < MAXLEVEL; ++n, nAbsPos = nAbsPos + nSpaceSteps )
                    {
                        SwNumFmt aFmt( aRule.Get( n ) );
                        aFmt.SetBulletFont( pBullFnt );
                        aFmt.SetBulletChar( cBullChar );
                        aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                        // #i93908# clear suffix for bullet lists
                        aFmt.SetPrefix(OUString());
                        aFmt.SetSuffix(OUString());
                        aFmt.SetFirstLineOffset( lBullFirstLineOffset );
                        aFmt.SetAbsLSpace( nAbsPos );
                        if( !aFmt.GetCharFmt() )
                            aFmt.SetCharFmt( pCFmt );
                        if( bRTL )
                            aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );

                        aRule.Set( n, aFmt );

                        if( n == nLvl &&
                            nFrmWidth < ( nSpaceSteps * MAXLEVEL ) )
                            nSpaceSteps = static_cast<sal_uInt16>(( nFrmWidth - nLeftTxtPos ) /
                                                ( MAXLEVEL - nLvl ));
                    }
                }
            }
        }
        else
        {
            bChgBullet = true;
            SetColl( static_cast<sal_uInt16>(RES_POOLCOLL_BUL_LEVEL1 + ( std::min( nLvl, cnNumBullColls ) * 4 )) );
        }
    }
    else
    {
        // Then it is a numbering

        //JP 21.11.97: The NumLevel is either the DigitLevel or, if the latter is not existent or 0,
        //             it is determined by the indentation level.

        String aPostFix, aPreFix, aNumTypes;
        if( USHRT_MAX != ( nDigitLevel = GetDigitLevel( *pAktTxtNd, nTxtStt,
                                        &aPreFix, &aPostFix, &aNumTypes )) )
        {
            bChgEnum = true;

            // Level 0 and Indentation, determine level by left indentation and default NumIndent
            if( !nDigitLevel && nLeftTxtPos )
                nLvl = std::min( sal_uInt16( nLeftTxtPos / lNumIndent ),
                            sal_uInt16( MAXLEVEL - 1 ) );
            else
                nLvl = nDigitLevel;
        }

        if( bChgEnum && aFlags.bSetNumRule )
        {
            if( !pCur )         // adjust NumRule if it is new
            {
                SwCharFmt* pCFmt = pDoc->GetCharFmtFromPool(
                                            RES_POOLCHR_NUM_LEVEL );
                if( !nDigitLevel )
                {
                    SwNumFmt aFmt( aRule.Get( nLvl ) );
                    aFmt.SetStart( static_cast<sal_uInt16>(aPreFix.GetToken( 1,
                                            (sal_Unicode)1 ).ToInt32()));
                    aFmt.SetPrefix( aPreFix.GetToken( 0, (sal_Unicode)1 ));
                    aFmt.SetSuffix( aPostFix.GetToken( 0, (sal_Unicode)1 ));
                    aFmt.SetIncludeUpperLevels( 0 );

                    if( !aFmt.GetCharFmt() )
                        aFmt.SetCharFmt( pCFmt );

                    if( aNumTypes.Len() )
                        aFmt.SetNumberingType(aNumTypes.GetChar( 0 ) - '0');

                    if( bRTL )
                        aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );
                    aRule.Set( nLvl, aFmt );
                }
                else
                {
                    sal_uInt16 nSpaceSteps = nLvl ? sal_uInt16(nLeftTxtPos / nLvl) : 0;
                    sal_uInt8 n;
                    for( n = 0; n <= nLvl; ++n )
                    {
                        SwNumFmt aFmt( aRule.Get( n ) );

                        aFmt.SetStart( static_cast<sal_uInt16>(aPreFix.GetToken( n+1,
                                                    (sal_Unicode)1 ).ToInt32() ));
                        if( !n )
                            aFmt.SetPrefix( aPreFix.GetToken( n, (sal_Unicode)1 ));
                        aFmt.SetSuffix( aPostFix.GetToken( n, (sal_Unicode)1 ));
                        aFmt.SetIncludeUpperLevels( MAXLEVEL );
                        if( n < aNumTypes.Len() )
                            aFmt.SetNumberingType((aNumTypes.GetChar( n ) - '0'));

                        aFmt.SetAbsLSpace( sal_uInt16( nSpaceSteps * n )
                                            + lNumIndent );

                        if( !aFmt.GetCharFmt() )
                            aFmt.SetCharFmt( pCFmt );
                        if( bRTL )
                            aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );

                        aRule.Set( n, aFmt );
                    }

                    // Does it fit completely into the frame?
                    bool bDefStep = nFrmWidth < (nSpaceSteps * MAXLEVEL);
                    for( ; n < MAXLEVEL; ++n )
                    {
                        SwNumFmt aFmt( aRule.Get( n ) );
                        aFmt.SetIncludeUpperLevels( MAXLEVEL );
                        if( bDefStep )
                            aFmt.SetAbsLSpace( sal_uInt16( (nLeftTxtPos +
                                SwNumRule::GetNumIndent(static_cast<sal_uInt8>(n-nLvl)))));
                        else
                            aFmt.SetAbsLSpace( sal_uInt16( nSpaceSteps * n )
                                                + lNumIndent );
                        aRule.Set( n, aFmt );
                    }
                }
            }
        }
        else if( !aFlags.bAFmtByInput )
            SetColl( static_cast<sal_uInt16>(RES_POOLCOLL_NUM_LEVEL1 + ( std::min( nLvl, cnNumBullColls ) * 4 ) ));
        else
            bChgEnum = false;
    }

    if( bChgEnum || bChgBullet )
    {
        aDelPam.DeleteMark();
        aDelPam.GetPoint()->nNode = aNdIdx;

        if( aFlags.bSetNumRule )
        {
            if( aFlags.bAFmtByInput )
            {
                aDelPam.SetMark();
                aDelPam.GetMark()->nNode++;
                aDelPam.GetNode(sal_False)->GetTxtNode()->SetAttrListLevel( nLvl );
            }

            pAktTxtNd->SetAttrListLevel(nLvl);
            pAktTxtNd->SetNumLSpace( true );

            // start new list
            pDoc->SetNumRule( aDelPam, aRule, true );
            aDelPam.DeleteMark();

            aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, 0 );
        }
        else
            aDelPam.GetPoint()->nContent.Assign( pAktTxtNd,
                        bChgEnum ? (nTxtStt - nOrigTxtStt) : 0 );
        aDelPam.SetMark();

        if( bChgBullet )
            nTxtStt += 2;

        while( nTxtStt < rStr.Len() && IsSpace( rStr.GetChar( nTxtStt ) ))
            nTxtStt++;

        aDelPam.GetPoint()->nContent = nTxtStt - nOrigTxtStt;
        DeleteSel( aDelPam );

        if( !aFlags.bSetNumRule )
        {
            String sChgStr = OUString('\t');
            if( bChgBullet )
                sChgStr.Insert( aFlags.cBullet, 0 );
            pDoc->InsertString( aDelPam, sChgStr );

            SfxItemSet aSet( pDoc->GetAttrPool(), aTxtNodeSetRange );
            if( bChgBullet )
            {
                aDelPam.GetPoint()->nContent = 0;
                aDelPam.SetMark();
                aDelPam.GetMark()->nContent = 1;
                SetAllScriptItem( aSet,
                     SvxFontItem( aFlags.aBulletFont.GetFamily(),
                                  aFlags.aBulletFont.GetName(),
                                  aFlags.aBulletFont.GetStyleName(),
                                  aFlags.aBulletFont.GetPitch(),
                                  aFlags.aBulletFont.GetCharSet(),
                                  RES_CHRATR_FONT ) );
                pDoc->SetFmtItemByAutoFmt( aDelPam, aSet );
                aDelPam.DeleteMark();
                nAutoCorrPos = 2;
                aSet.ClearItem();
            }
            SvxTabStopItem aTStops( RES_PARATR_TABSTOP );    aTStops.Insert( SvxTabStop( 0 ));
            aSet.Put( aTStops );
            pDoc->SetFmtItemByAutoFmt( aDelPam, aSet );
        }
    }

    if( bBreak )
    {
        AutoCorrect( nAutoCorrPos );       /* Offset due to Bullet + Tab */
        return;
    }

    const SwTxtNode* pNxtNd = GetNextNode();
    while( CanJoin( pNxtNd ) &&
            nLvl == CalcLevel( *pNxtNd ) )
    {
        SetRedlineTxt( STR_AUTOFMTREDL_DEL_MORELINES );
        bBreak = !IsFastFullLine( *pNxtNd ) || IsBlanksInString( *pNxtNd ) ||
                IsSentenceAtEnd( *pNxtNd );
        if( DeleteAktNxtPara( pNxtNd->GetTxt() ) )
        {
            pDoc->InsertString( aDelPam, OUString(' ') );
        }
        if( bBreak )
            break;
        const SwTxtNode* pCurrNode = pNxtNd;
        pNxtNd = GetNextNode();
        if(!pNxtNd || pCurrNode == pNxtNd)
            break;
    }
    DeleteAktPara( false, true );
    AutoCorrect( nAutoCorrPos );
}

void SwAutoFormat::BuildNegIndent( SwTwips nSpaces )
{
    SetRedlineTxt( STR_AUTOFMTREDL_SET_TMPL_NEG_INDENT );
    // Test of contraposition (n words, divided by spaces/tabs, with same indentation in 2nd line)

    // read all succeeding paragraphs that belong to this enumeration
    bool bBreak = true;
    xub_StrLen nSpacePos, nTxtPos = GetBigIndent( nSpacePos );
    if( bMoreLines )
        DelMoreLinesBlanks( true );
    else
        bBreak = !IsFastFullLine( *pAktTxtNd ) ||
                    ( !nTxtPos && IsBlanksInString( *pAktTxtNd )) ||
                    IsSentenceAtEnd( *pAktTxtNd );

    SetColl( static_cast<sal_uInt16>( nTxtPos
                ? RES_POOLCOLL_CONFRONTATION
                : RES_POOLCOLL_TEXT_NEGIDENT ) );

    if( nTxtPos )
    {
        const OUString& rStr = pAktTxtNd->GetTxt();
        bool bInsTab = true;

        if ('\t' == rStr[nSpacePos+1]) // leave tab alone
        {
            --nSpacePos;
            bInsTab = false;
        }

        xub_StrLen nSpaceStt = nSpacePos;
        while (nSpaceStt && IsSpace(rStr[--nSpaceStt]))
            ;
        ++nSpaceStt;

        if (bInsTab && '\t' == rStr[nSpaceStt]) // leave tab alone
        {
            ++nSpaceStt;
            bInsTab = false;
        }


        aDelPam.DeleteMark();
        aDelPam.GetPoint()->nNode = aNdIdx;
        aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, nSpacePos );

        // delete old Spaces, etc.
        if( nSpaceStt < nSpacePos )
        {
            aDelPam.SetMark();
            aDelPam.GetMark()->nContent = nSpaceStt;
            DeleteSel( aDelPam );
            if( bInsTab )
            {
                pDoc->InsertString( aDelPam, OUString('\t') );
            }
        }
    }

    if( !bBreak )
    {
        SetRedlineTxt( STR_AUTOFMTREDL_DEL_MORELINES );
        SwTxtFrmInfo aFInfo( pAktTxtFrm );
        const SwTxtNode* pNxtNd = GetNextNode();
        while(  CanJoin( pNxtNd ) &&
                20 < std::abs( (long)(nSpaces - aFInfo.SetFrm(
                                GetFrm( *pNxtNd ) ).GetLineStart() ))
            )
        {
            bBreak = !IsFastFullLine( *pNxtNd ) ||
                    IsBlanksInString( *pNxtNd ) ||
                    IsSentenceAtEnd( *pNxtNd );
            if( DeleteAktNxtPara( pNxtNd->GetTxt() ) )
            {
                pDoc->InsertString( aDelPam, OUString(' ') );
            }
            if( bBreak )
                break;
            pNxtNd = GetNextNode();
        }
    }
    DeleteAktPara( true, true );
    AutoCorrect();
}

void SwAutoFormat::BuildHeadLine( sal_uInt16 nLvl )
{
    if( aFlags.bWithRedlining )
    {
        OUString sTxt(ViewShell::GetShellRes()->GetAutoFmtNameLst()[
                                    STR_AUTOFMTREDL_SET_TMPL_HEADLINE ] );
        sTxt = sTxt.replaceAll( "$(ARG1)", OUString::number( nLvl + 1 ) );
        pDoc->SetAutoFmtRedlineComment( &sTxt );
    }

    SetColl( static_cast<sal_uInt16>(RES_POOLCOLL_HEADLINE1 + nLvl ), true );
    if( aFlags.bAFmtByInput )
    {
        SwTxtFmtColl& rNxtColl = pAktTxtNd->GetTxtColl()->GetNextTxtFmtColl();

        DelPrevPara();

        DeleteAktPara( true, false );
        DeleteAktNxtPara( aEmptyStr );

        aDelPam.DeleteMark();
        aDelPam.GetPoint()->nNode = aNdIdx.GetIndex() + 1;
        aDelPam.GetPoint()->nContent.Assign( aDelPam.GetCntntNode(), 0 );
        pDoc->SetTxtFmtColl( aDelPam, &rNxtColl );
    }
    else
    {
        DeleteAktPara( true, true );
        AutoCorrect();
    }
}

/// Start autocorrection for the current TextNode
void SwAutoFormat::AutoCorrect( xub_StrLen nPos )
{
    SvxAutoCorrect* pATst = SvxAutoCorrCfg::Get().GetAutoCorrect();
    long aSvxFlags = pATst->GetFlags( );
    bool bReplaceQuote = ( aSvxFlags & ChgQuotes ) > 0;
    bool bReplaceSglQuote = ( aSvxFlags & ChgSglQuotes ) > 0;

    if( aFlags.bAFmtByInput ||
        (!aFlags.bAutoCorrect && !bReplaceQuote && !bReplaceSglQuote &&
        !aFlags.bCptlSttSntnc && !aFlags.bCptlSttWrd &&
        !aFlags.bChgOrdinalNumber &&
        !aFlags.bChgToEnEmDash && !aFlags.bSetINetAttr &&
        !aFlags.bChgWeightUnderl && !aFlags.bAddNonBrkSpace) )
        return;

    const OUString* pTxt = &pAktTxtNd->GetTxt();
    if (nPos >= pTxt->getLength())
        return;

    bool bGetLanguage = aFlags.bChgOrdinalNumber ||
                        aFlags.bChgToEnEmDash || aFlags.bSetINetAttr ||
                        aFlags.bCptlSttWrd || aFlags.bCptlSttSntnc ||
                        aFlags.bAddNonBrkSpace;


    aDelPam.DeleteMark();
    aDelPam.GetPoint()->nNode = aNdIdx;
    aDelPam.GetPoint()->nContent.Assign( pAktTxtNd, 0 );

    SwAutoCorrDoc aACorrDoc( *pEditShell, aDelPam );

    SwTxtFrmInfo aFInfo( 0 );

    sal_Int32 nSttPos, nLastBlank = nPos;
    sal_Bool bFirst = aFlags.bCptlSttSntnc, bFirstSent = bFirst;
    sal_Unicode cChar = 0;

    CharClass& rAppCC = GetAppCharClass();

    do {
        while (nPos < pTxt->getLength() && IsSpace(cChar = (*pTxt)[nPos]))
            ++nPos;
        if (nPos == pTxt->getLength())
            break;      // das wars

        if( ( ( bReplaceQuote && '\"' == cChar ) ||
              ( bReplaceSglQuote && '\'' == cChar ) ) &&
            (!nPos || ' ' == (*pTxt)[nPos-1]))
        {
            // --------------------------------------
            // beachte: Sonderfall Symbolfonts !!!
            if( !aFInfo.GetFrm() )
                aFInfo.SetFrm( GetFrm( *pAktTxtNd ) );
            if( !aFInfo.IsBullet( nPos ))
            {
                SetRedlineTxt( STR_AUTOFMTREDL_TYPO );
                aDelPam.GetPoint()->nContent = nPos;
                bool bSetHardBlank = false;

                String sReplace( pATst->GetQuote( aACorrDoc,
                                    nPos, cChar, sal_True ));

                aDelPam.SetMark();
                aDelPam.GetPoint()->nContent = nPos+1;
                if( 2 == sReplace.Len() && ' ' == sReplace.GetChar( 1 ))
                {
                    sReplace.Erase( 1 );
                    bSetHardBlank = true;
                }
                pDoc->ReplaceRange( aDelPam, sReplace, false );

                if( aFlags.bWithRedlining )
                {
                    aNdIdx = aDelPam.GetPoint()->nNode;
                    pAktTxtNd = aNdIdx.GetNode().GetTxtNode();
                    pTxt = &pAktTxtNd->GetTxt();
                    aDelPam.SetMark();
                    aFInfo.SetFrm( 0 );
                }

                nPos += sReplace.Len() - 1;
                aDelPam.DeleteMark();
                if( bSetHardBlank )
                {
                    pDoc->InsertString( aDelPam, OUString(CHAR_HARDBLANK) );
                    ++nPos;
                }
            }
        }

        bool bCallACorr = false;
        int bBreak = 0;
        if (nPos && IsSpace((*pTxt)[nPos-1]))
            nLastBlank = nPos;
        for (nSttPos = nPos; !bBreak && nPos < pTxt->getLength(); ++nPos)
            switch (cChar = (*pTxt)[nPos])
            {
            case '\"':
            case '\'':
                if( ( cChar == '\"' && bReplaceQuote ) || ( cChar == '\'' && bReplaceSglQuote ) )
                {
                    // consider Symbolfonts!
                    if( !aFInfo.GetFrm() )
                        aFInfo.SetFrm( GetFrm( *pAktTxtNd ) );
                    if( !aFInfo.IsBullet( nPos ))
                    {
                        SetRedlineTxt( STR_AUTOFMTREDL_TYPO );
                        bool bSetHardBlank = false;
                        aDelPam.GetPoint()->nContent = nPos;
                        String sReplace( pATst->GetQuote( aACorrDoc,
                                                    nPos, cChar, sal_False ));

                        if( 2 == sReplace.Len() && ' ' == sReplace.GetChar( 0 ))
                        {
                            sReplace.Erase( 0, 1 );
                            bSetHardBlank = true;
                        }

                        aDelPam.SetMark();
                        aDelPam.GetPoint()->nContent = nPos+1;
                        pDoc->ReplaceRange( aDelPam, sReplace, false );

                        if( aFlags.bWithRedlining )
                        {
                            aNdIdx = aDelPam.GetPoint()->nNode;
                            pAktTxtNd = aNdIdx.GetNode().GetTxtNode();
                            pTxt = &pAktTxtNd->GetTxt();
                            aDelPam.SetMark();
                            aDelPam.DeleteMark();
                            aFInfo.SetFrm( 0 );
                        }

                        nPos += sReplace.Len() - 1;
                        aDelPam.DeleteMark();

                        if( bSetHardBlank )
                        {
                            aDelPam.GetPoint()->nContent = nPos;
                            pDoc->InsertString( aDelPam, OUString(CHAR_HARDBLANK) );
                            aDelPam.GetPoint()->nContent = ++nPos;
                        }
                    }
                }
                break;
            case '*':
            case '_':
                if( aFlags.bChgWeightUnderl )
                {
                    // consider Symbolfonts!
                    if( !aFInfo.GetFrm() )
                        aFInfo.SetFrm( GetFrm( *pAktTxtNd ) );
                    if( !aFInfo.IsBullet( nPos ))
                    {
                        SetRedlineTxt( '*' == cChar
                                            ? STR_AUTOFMTREDL_BOLD
                                            : STR_AUTOFMTREDL_UNDER );

                        sal_Unicode cBlank = nSttPos ? (*pTxt)[nSttPos - 1] : 0;
                        aDelPam.GetPoint()->nContent = nPos;

                        if( pATst->FnChgWeightUnderl( aACorrDoc, *pTxt,
                                                            nSttPos, nPos ))
                        {
                            if( aFlags.bWithRedlining )
                            {
                                aNdIdx = aDelPam.GetPoint()->nNode;
                                pAktTxtNd = aNdIdx.GetNode().GetTxtNode();
                                pTxt = &pAktTxtNd->GetTxt();
                                aDelPam.SetMark();
                                aDelPam.DeleteMark();
                                aFInfo.SetFrm( 0 );
                            }
                            //#125102# in case of the mode REDLINE_SHOW_DELETE the ** are still contained in pTxt
                            if(0 == (pDoc->GetRedlineMode() & nsRedlineMode_t::REDLINE_SHOW_DELETE))
                                nPos = aDelPam.GetPoint()->nContent.GetIndex() - 1;
                            // Was a character deleted before starting?
                            if (cBlank && cBlank != (*pTxt)[nSttPos - 1])
                                --nSttPos;
                        }
                    }
                }
                break;
            case '/':
                if ( aFlags.bAddNonBrkSpace )
                {
                    LanguageType eLang = (bGetLanguage && pAktTxtNd)
                                           ? pAktTxtNd->GetLang( nSttPos )
                                           : LANGUAGE_SYSTEM;

                    SetRedlineTxt( STR_AUTOFMTREDL_NON_BREAK_SPACE );
                    if ( pATst->FnAddNonBrkSpace( aACorrDoc, *pTxt, nSttPos, nPos, eLang ) )
                        --nPos;
                }
                break;

            case '.':
            case '!':
            case '?':
                if( aFlags.bCptlSttSntnc )
                    bFirstSent = sal_True;
            default:
                if( !( rAppCC.isLetterNumeric( *pTxt, nPos )
                        || '/' == cChar )) //  '/' should not be a word separator (e.g. '1/2' needs to be handled as one word for replacement)
                {
                    --nPos;     // revert ++nPos which was decremented in for loop
                    ++bBreak;
                }
                break;
            }

        if( nPos == nSttPos )
        {
            if (++nPos == pTxt->getLength())
                bCallACorr = true;
        }
        else
            bCallACorr = true;


        if( bCallACorr )
        {
            bCallACorr = false;
            aDelPam.GetPoint()->nContent = nPos;
            SetRedlineTxt( STR_AUTOFMTREDL_USE_REPLACE );
            if( aFlags.bAutoCorrect &&
                aACorrDoc.ChgAutoCorrWord( nSttPos, nPos, *pATst, 0 ) )
            {
                nPos = aDelPam.GetPoint()->nContent.GetIndex();

                if( aFlags.bWithRedlining )
                {
                    aNdIdx = aDelPam.GetPoint()->nNode;
                    pAktTxtNd = aNdIdx.GetNode().GetTxtNode();
                    pTxt = &pAktTxtNd->GetTxt();
                    aDelPam.SetMark();
                    aDelPam.DeleteMark();
                }

                continue;       // do not check further
            }

            LanguageType eLang = (bGetLanguage && pAktTxtNd)
                                           ? pAktTxtNd->GetLang( nSttPos )
                                           : LANGUAGE_SYSTEM;

            if ( aFlags.bAddNonBrkSpace )
            {
                SetRedlineTxt( STR_AUTOFMTREDL_NON_BREAK_SPACE );
                pATst->FnAddNonBrkSpace( aACorrDoc, *pTxt, nSttPos, nPos, eLang );
            }

            if( ( aFlags.bChgOrdinalNumber &&
                    SetRedlineTxt( STR_AUTOFMTREDL_ORDINAL ) &&
                    pATst->FnChgOrdinalNumber( aACorrDoc, *pTxt, nSttPos, nPos, eLang ) ) ||
                ( aFlags.bChgToEnEmDash &&
                    SetRedlineTxt( STR_AUTOFMTREDL_DASH ) &&
                    pATst->FnChgToEnEmDash( aACorrDoc, *pTxt, nSttPos, nPos, eLang ) ) ||
                ( aFlags.bSetINetAttr &&
                    (nPos == pTxt->getLength() || IsSpace((*pTxt)[nPos])) &&
                    SetRedlineTxt( STR_AUTOFMTREDL_DETECT_URL ) &&
                    pATst->FnSetINetAttr( aACorrDoc, *pTxt, nLastBlank, nPos, eLang ) ) )
                    nPos = aDelPam.GetPoint()->nContent.GetIndex();
            else
            {
                // two capital letters at the beginning of a word?
                if( aFlags.bCptlSttWrd )
                {
                    SetRedlineTxt( STR_AUTOFMTREDL_CPTL_STT_WORD );
                    pATst->FnCptlSttWrd( aACorrDoc, *pTxt, nSttPos, nPos, eLang );
                }
                // capital letter at the beginning of a sentence?
                if( aFlags.bCptlSttSntnc && bFirst )
                {
                    SetRedlineTxt( STR_AUTOFMTREDL_CPTL_STT_SENT );
                    pATst->FnCptlSttSntnc( aACorrDoc, *pTxt, sal_True, nSttPos, nPos, eLang);
                    bFirst = sal_False;
                }

                bFirst = bFirstSent;
                bFirstSent = sal_False;

                if( aFlags.bWithRedlining )
                {
                    aNdIdx = aDelPam.GetPoint()->nNode;
                    pAktTxtNd = aNdIdx.GetNode().GetTxtNode();
                    pTxt = &pAktTxtNd->GetTxt();
                    aDelPam.SetMark();
                    aDelPam.DeleteMark();
                }
            }
        }
    } while (nPos < pTxt->getLength());
    ClearRedlineTxt();
}

SwAutoFormat::SwAutoFormat( SwEditShell* pEdShell, SvxSwAutoFmtFlags& rFlags,
                            SwNodeIndex* pSttNd, SwNodeIndex* pEndNd )
    : aFlags( rFlags ),
    aDelPam( pEdShell->GetDoc()->GetNodes().GetEndOfExtras() ),
    aNdIdx( pEdShell->GetDoc()->GetNodes().GetEndOfExtras(), +1 ),
    aEndNdIdx( pEdShell->GetDoc()->GetNodes().GetEndOfContent() ),
    pEditShell( pEdShell ),
    pDoc( pEdShell->GetDoc() ),
    pAktTxtNd( 0 ), pAktTxtFrm( 0 ),
    pCharClass( 0 ),
    nRedlAutoFmtSeqId( 0 )
{
    OSL_ENSURE( (pSttNd && pEndNd) || (!pSttNd && !pEndNd),
            "Got no area" );

    if( aFlags.bSetNumRule && !aFlags.bAFmtByInput )
        aFlags.bSetNumRule = sal_False;

    bool bReplaceStyles = !aFlags.bAFmtByInput || aFlags.bReplaceStyles;

    const SwTxtNode* pNxtNd = 0;
    bool bNxtEmpty = false;
    sal_Bool bNxtAlpha = sal_False;
    sal_uInt16 nNxtLevel = 0;

    // set area for autoformatting
    if( pSttNd )
    {
        aNdIdx = *pSttNd;
        aNdIdx--;           // for GoNextPara, one paragraph prior to that
        aEndNdIdx = *pEndNd;
        aEndNdIdx++;

        // check the previous TextNode
        pNxtNd = aNdIdx.GetNode().GetTxtNode();
        bEmptyLine = !pNxtNd ||
                    IsEmptyLine( *pNxtNd ) ||
                    IsNoAlphaLine( *pNxtNd );
    }
    else
        bEmptyLine = true;      // at document beginning

    bEnde = false;

    // set value for percentage display
    nEndNdIdx = aEndNdIdx.GetIndex();

    if( !aFlags.bAFmtByInput )
        ::StartProgress( STR_STATSTR_AUTOFORMAT, aNdIdx.GetIndex(),
                         nEndNdIdx = aEndNdIdx.GetIndex(),
                         pDoc->GetDocShell() );

    RedlineMode_t eRedlMode = pDoc->GetRedlineMode(), eOldMode = eRedlMode;
    if( aFlags.bWithRedlining )
    {
        pDoc->SetAutoFmtRedline( sal_True );
        eRedlMode = (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT);
    }
    else
      eRedlMode = (RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_IGNORE);
    pDoc->SetRedlineMode( eRedlMode );

    // save undo state (might be turned off)
    bool const bUndoState = pDoc->GetIDocumentUndoRedo().DoesUndo();

    // If multiple lines, then do not merge with next paragraph
    bMoreLines = false;

    nLastCalcHeadLvl = nLastCalcEnumLvl = 0;
    nLastHeadLvl = nLastEnumLvl = USHRT_MAX;
    sal_uInt16 nLevel = 0;
    sal_uInt16 nDigitLvl = 0;

    // set defaults
    SwTxtFrmInfo aFInfo( 0 );

    // This is the automat for autoformatting
    eStat = READ_NEXT_PARA;
    while( !bEnde )
    {
        switch( eStat )
        {
        case READ_NEXT_PARA:
            {
                GoNextPara();
                eStat = bEnde ? IS_ENDE : TST_EMPTY_LINE;
            }
            break;

        case TST_EMPTY_LINE:
            if( IsEmptyLine( *pAktTxtNd ) )
            {
                if( aFlags.bDelEmptyNode && !HasObjects( *pAktTxtNd ) )
                {
                    bEmptyLine = true;
                    sal_uLong nOldCnt = pDoc->GetNodes().Count();
                    DelEmptyLine();
                    // Was there really a deletion of a node?
                    if( nOldCnt != pDoc->GetNodes().Count() )
                        aNdIdx--;       // do not skip the next paragraph
                }
                eStat = READ_NEXT_PARA;
            }
            else
                eStat = TST_ALPHA_LINE;
            break;

        case TST_ALPHA_LINE:
            if( IsNoAlphaLine( *pAktTxtNd ))
            {
                // recognize a table definition +---+---+
                if( aFlags.bAFmtByInput && aFlags.bCreateTable && DoTable() )
                {
                    //JP 30.09.96: DoTable() builds on PopCrsr and MoveCrsr after AutoFormat!
                    pEdShell->Pop( sal_False );
                    *pEdShell->GetCrsr() = aDelPam;
                    pEdShell->Push();

                    eStat = IS_ENDE;
                    break;
                }

                // Check for 3 "---" or "===". In this case, the previous paragraph should be
                // underlined and the current be deleted!
                if( !DoUnderline() && bReplaceStyles )
                {
                    SetColl( RES_POOLCOLL_STANDARD, true );
                    bEmptyLine = true;
                }
                eStat = READ_NEXT_PARA;
            }
            else
                eStat = GET_ALL_INFO;
            break;

        case GET_ALL_INFO:
            {
                if( pAktTxtNd->GetNumRule() )
                {
                    // do nothing in numbering, go to next
                    bEmptyLine = false;
                    eStat = READ_NEXT_PARA;
                    // delete all blanks at beginning/end and in between
                    //JP 29.04.98: first only "all in between"
                    DelMoreLinesBlanks( false );
                    break;
                }

                aFInfo.SetFrm( pAktTxtFrm );

                // so far: if there were templates assigned, keep these and go to next node
                sal_uInt16 nPoolId = pAktTxtNd->GetTxtColl()->GetPoolFmtId();
                if( IsPoolUserFmt( nPoolId )
                        ? !aFlags.bChgUserColl
                        : ( RES_POOLCOLL_STANDARD != nPoolId &&
                           ( !aFlags.bAFmtByInput ||
                            (RES_POOLCOLL_TEXT_MOVE != nPoolId &&
                             RES_POOLCOLL_TEXT != nPoolId )) ))
                {
                    eStat = HAS_FMTCOLL;
                    break;
                }

                // check for hard spaces or LRSpaces set by the template
                if( IsPoolUserFmt( nPoolId ) ||
                    RES_POOLCOLL_STANDARD == nPoolId )
                {
                    short nSz;
                    SvxLRSpaceItem* pLRSpace;
                    if( SFX_ITEM_SET == pAktTxtNd->GetSwAttrSet().
                        GetItemState( RES_LR_SPACE, sal_True,
                                        (const SfxPoolItem**)&pLRSpace ) &&
                        ( 0 != (nSz = pLRSpace->GetTxtFirstLineOfst()) ||
                            0 != pLRSpace->GetTxtLeft() ) )
                    {
                        // exception: numbering/enumation can have an indentation
                        if( IsEnumericChar( *pAktTxtNd ))
                        {
                            nLevel = CalcLevel( *pAktTxtNd, &nDigitLvl );
                            if( nLevel >= MAXLEVEL )
                                nLevel = MAXLEVEL-1;
                            BuildEnum( nLevel, nDigitLvl );
                            eStat = READ_NEXT_PARA;
                            break;
                        }

                        // never merge (maybe only indent as exception)
                        bMoreLines = true;

                        if( bReplaceStyles )
                        {
                            // then use one of our templates
                            if( 0 < nSz )           // positive 1st line indentation
                                BuildIndent();
                            else if( 0 > nSz )      // negative 1st line indentation
                                BuildNegIndent( aFInfo.GetLineStart() );
                            else if( pLRSpace->GetTxtLeft() )   // is indentation
                                BuildTextIndent();
                        }
                        eStat = READ_NEXT_PARA;
                        break;
                    }
                }

                nLevel = CalcLevel( *pAktTxtNd, &nDigitLvl );
                bMoreLines = !IsOneLine( *pAktTxtNd );
                pNxtNd = GetNextNode();
                if( pNxtNd )
                {
                    bNxtEmpty = IsEmptyLine( *pNxtNd );
                    bNxtAlpha = IsNoAlphaLine( *pNxtNd );
                    nNxtLevel = CalcLevel( *pNxtNd );

                    if( !bEmptyLine && HasBreakAttr( *pAktTxtNd ) )
                        bEmptyLine = true;
                    if( !bNxtEmpty && HasBreakAttr( *pNxtNd ) )
                        bNxtEmpty = true;

                }
                else
                {
                    bNxtEmpty = false;
                    bNxtAlpha = sal_False;
                    nNxtLevel = 0;
                }
                eStat = !bMoreLines ? IS_ONE_LINE : TST_ENUMERIC;
            }
            break;

        case IS_ONE_LINE:
            {
                eStat = TST_ENUMERIC;
                if( !bReplaceStyles )
                    break;

                String sClrStr( pAktTxtNd->GetTxt() );

                if( !DelLeadingBlanks( sClrStr ).Len() )
                {
                    bEmptyLine = true;
                    eStat = READ_NEXT_PARA;
                    break;      // read next paragraph
                }

                // check if headline
                if( !bEmptyLine || !IsFirstCharCapital( *pAktTxtNd ) ||
                    IsBlanksInString( *pAktTxtNd ) )
                    break;

                bEmptyLine = false;
                String sEndClrStr( sClrStr );
                xub_StrLen nLen = DelTrailingBlanks( sEndClrStr ).Len();

                // not, then check if headline
                if( ':' == sEndClrStr.GetChar( nLen - 1 ) )
                {
                    {
                        BuildHeadLine( 2 );
                        eStat = READ_NEXT_PARA;
                        break;
                    }
                }
                else if( 256 <= sEndClrStr.GetChar( nLen-1 ) ||
                         !strchr( ",.;", sEndClrStr.GetChar( nLen-1 )) )
                {
                    if( bNxtEmpty || bNxtAlpha
                        || ( pNxtNd && IsEnumericChar( *pNxtNd ))

                        )
                    {

                        // one level below?
                        if( nLevel >= MAXLEVEL )
                            nLevel = MAXLEVEL-1;

                        if( USHRT_MAX == nLastHeadLvl )
                            nLastHeadLvl = 0;
                        else if( nLastCalcHeadLvl < nLevel )
                        {
                            if( nLastHeadLvl+1 < MAXLEVEL )
                                ++nLastHeadLvl;
                        }
                        // one level above?
                        else if( nLastCalcHeadLvl > nLevel )
                        {
                            if( nLastHeadLvl )
                                --nLastHeadLvl;
                        }
                        nLastCalcHeadLvl = nLevel;

                        if( aFlags.bAFmtByInput )
                            BuildHeadLine( nLevel );
                        else
                            BuildHeadLine( nLastHeadLvl );
                        eStat = READ_NEXT_PARA;
                        break;
                    }
                }
            }
            break;

        case TST_ENUMERIC:
            {
                bEmptyLine = false;
                if( IsEnumericChar( *pAktTxtNd ))
                {
                    if( nLevel >= MAXLEVEL )
                        nLevel = MAXLEVEL-1;
                    BuildEnum( nLevel, nDigitLvl );
                    eStat = READ_NEXT_PARA;
                }
                else if( bReplaceStyles )
                    eStat = nLevel ? TST_IDENT : TST_NEG_IDENT;
                else
                    eStat = READ_NEXT_PARA;
            }
            break;

        case TST_IDENT:
            // Spaces at the beginning, check again for indentation
            if( bMoreLines && nLevel )
            {
                SwTwips nSz = aFInfo.GetFirstIndent();
                if( 0 < nSz )           // positive 1st line indentation
                    BuildIndent();
                else if( 0 > nSz )      // negative 1st line indentation
                    BuildNegIndent( aFInfo.GetLineStart() );
                else                    // is indentation
                    BuildTextIndent();
                eStat = READ_NEXT_PARA;
            }
            else if( nLevel && pNxtNd && !bEnde &&
                     !bNxtEmpty && !bNxtAlpha && !nNxtLevel &&
                     !IsEnumericChar( *pNxtNd ) )
            {
                // is an indentation
                BuildIndent();
                eStat = READ_NEXT_PARA;
            }
            else
                eStat = TST_TXT_BODY;
            break;

        case TST_NEG_IDENT:
            // no spaces at the beginning, check again for negative indentation
            {
                if( bMoreLines && !nLevel )
                {
                    SwTwips nSz = aFInfo.GetFirstIndent();
                    if( 0 < nSz )           // positive 1st line indentation
                        BuildIndent();
                    else if( 0 > nSz )      // negative 1st line indentation
                        BuildNegIndent( aFInfo.GetLineStart() );
                    else                    // is _no_ indentation
                        BuildText();
                    eStat = READ_NEXT_PARA;
                }
                else if( !nLevel && pNxtNd && !bEnde &&
                         !bNxtEmpty && !bNxtAlpha && nNxtLevel &&
                         !IsEnumericChar( *pNxtNd ) )
                {
                    // is a negative indentation
                    BuildNegIndent( aFInfo.GetLineStart() );
                    eStat = READ_NEXT_PARA;
                }
                else
                    eStat = TST_TXT_BODY;
            }
            break;

        case TST_TXT_BODY:
            {
                if( bMoreLines )
                {
                    SwTwips nSz = aFInfo.GetFirstIndent();
                    if( 0 < nSz )           // positive 1st line indentation
                        BuildIndent();
                    else if( 0 > nSz )      // negative 1st line indentation
                        BuildNegIndent( aFInfo.GetLineStart() );
                    else if( nLevel )       // is indentation
                        BuildTextIndent();
                    else
                        BuildText();
                }
                else if( nLevel )
                    BuildTextIndent();
                else
                    BuildText();
                eStat = READ_NEXT_PARA;
            }
            break;

        case HAS_FMTCOLL:
            {
                // so far: if there were templates assigned, keep these and go to next node
                bEmptyLine = false;
                eStat = READ_NEXT_PARA;
                // delete all blanks at beginning/end and in between
                //JP 29.04.98: first only "all in between"
                DelMoreLinesBlanks( false );

                // handle hard attributes
                if( pAktTxtNd->HasSwAttrSet() )
                {
                    short nSz;
                    SvxLRSpaceItem* pLRSpace;
                    if( bReplaceStyles &&
                        SFX_ITEM_SET == pAktTxtNd->GetSwAttrSet().
                        GetItemState( RES_LR_SPACE, sal_False,
                                        (const SfxPoolItem**)&pLRSpace ) &&
                        ( 0 != (nSz = pLRSpace->GetTxtFirstLineOfst()) ||
                            0 != pLRSpace->GetTxtLeft() ) )
                    {
                        // then use one of our templates
                        if( 0 < nSz )           // positive 1st line indentation
                            BuildIndent();
                        else if( 0 > nSz )      // negative 1st line indentation
                        {
                            BuildNegIndent( aFInfo.GetLineStart() );
                        }
                        else if( pLRSpace->GetTxtLeft() )   // is indentation
                            BuildTextIndent();
                        else
                            BuildText();
                    }
                }
            }
            break;

        case IS_ENDE:
            bEnde = true;
            break;
        }
    }

    if( aFlags.bWithRedlining )
        pDoc->SetAutoFmtRedline( sal_False );
    pDoc->SetRedlineMode( eOldMode );

    // restore undo (in case it has been changed)
    pDoc->GetIDocumentUndoRedo().DoUndo(bUndoState);

    // disable display of percentage again
    if( !aFlags.bAFmtByInput )
        ::EndProgress( pDoc->GetDocShell() );
}

void SwEditShell::AutoFormat( const SvxSwAutoFmtFlags* pAFlags )
{
    SwWait* pWait = 0;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( UNDO_AUTOFORMAT );

    SvxSwAutoFmtFlags aAFFlags;     // use default values or add params?
    if( pAFlags )
    {
        aAFFlags = *pAFlags;
        if( !aAFFlags.bAFmtByInput )
            pWait = new SwWait( *GetDoc()->GetDocShell(), sal_True );
    }

    SwPaM* pCrsr = GetCrsr();
    // There are more than one or a selection is open
    if( pCrsr->GetNext() != pCrsr || pCrsr->HasMark() )
    {
        FOREACHPAM_START(GetCrsr())
            if( PCURCRSR->HasMark() )
            {
                SwAutoFormat aFmt( this, aAFFlags, &PCURCRSR->Start()->nNode,
                                     &PCURCRSR->End()->nNode );
            }
        FOREACHPAM_END()
    }
    else
    {
        SwAutoFormat aFmt( this, aAFFlags );
    }

    EndUndo( UNDO_AUTOFORMAT );
    EndAllAction();

    delete pWait;
}

void SwEditShell::AutoFmtBySplitNode()
{
    SET_CURR_SHELL( this );
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() == pCrsr && pCrsr->Move( fnMoveBackward, fnGoNode ) )
    {
        StartAllAction();
        StartUndo( UNDO_AUTOFORMAT );

        bool bRange = false;
        pCrsr->SetMark();
        SwIndex* pCntnt = &pCrsr->GetMark()->nContent;
        if( pCntnt->GetIndex() )
        {
            *pCntnt = 0;
            bRange = true;
        }
        else
        {
            // then go one node backwards
            SwNodeIndex aNdIdx( pCrsr->GetMark()->nNode, -1 );
            SwTxtNode* pTxtNd = aNdIdx.GetNode().GetTxtNode();
            if (pTxtNd && !pTxtNd->GetTxt().isEmpty())
            {
                pCntnt->Assign( pTxtNd, 0 );
                pCrsr->GetMark()->nNode = aNdIdx;
                bRange = true;
            }
        }

        if( bRange )
        {
            Push();     // save cursor

            SvxSwAutoFmtFlags aAFFlags = *GetAutoFmtFlags(); // use default values so far

            SwAutoFormat aFmt( this, aAFFlags, &pCrsr->GetMark()->nNode,
                                    &pCrsr->GetPoint()->nNode );

            //JP 30.09.96: DoTable() builds on PopCrsr and MoveCrsr!
            Pop( sal_False );
            pCrsr = GetCrsr();
        }
        pCrsr->DeleteMark();
        pCrsr->Move( fnMoveForward, fnGoNode );

        EndUndo( UNDO_AUTOFORMAT );
        EndAllAction();
    }
}

SvxSwAutoFmtFlags* SwEditShell::GetAutoFmtFlags()
{
    if (!pAutoFmtFlags)
        pAutoFmtFlags = new SvxSwAutoFmtFlags;

    return pAutoFmtFlags;
}

void SwEditShell::SetAutoFmtFlags(SvxSwAutoFmtFlags * pFlags)
{
    SvxSwAutoFmtFlags* pEditFlags = GetAutoFmtFlags();

    pEditFlags->bSetNumRule     = pFlags->bSetNumRule;
    pEditFlags->bChgEnumNum     = pFlags->bChgEnumNum;
    pEditFlags->bSetBorder      = pFlags->bSetBorder;
    pEditFlags->bCreateTable    = pFlags->bCreateTable;
    pEditFlags->bReplaceStyles  = pFlags->bReplaceStyles;
    pEditFlags->bAFmtByInpDelSpacesAtSttEnd =
                                    pFlags->bAFmtByInpDelSpacesAtSttEnd;
    pEditFlags->bAFmtByInpDelSpacesBetweenLines =
                                    pFlags->bAFmtByInpDelSpacesBetweenLines;

    //JP 15.12.98: copy BulletChar and Font into "normal" ones
    //             because AutoFormat can only work with the latter!
    pEditFlags->cBullet             = pFlags->cByInputBullet;
    pEditFlags->aBulletFont         = pFlags->aByInputBulletFont;
    pEditFlags->cByInputBullet      = pFlags->cByInputBullet;
    pEditFlags->aByInputBulletFont  = pFlags->aByInputBulletFont;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
