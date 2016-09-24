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

#include <comphelper/processfactory.hxx>
#include <editeng/acorrcfg.hxx>
#include <o3tl/any.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include <svl/urihelper.hxx>

#include <editeng/svxacorr.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/instance.hxx>

using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;


SvxAutoCorrCfg::SvxAutoCorrCfg() :
    aBaseConfig(*this),
    aSwConfig(*this),
    bFileRel(true),
    bNetRel(true),
    bAutoTextTip(true),
    bAutoTextPreview(false),
    bAutoFmtByInput(true),
    bSearchInAllCategories(false)
{
    SvtPathOptions aPathOpt;
    OUString sSharePath, sUserPath, sAutoPath( aPathOpt.GetAutoCorrectPath() );

    sSharePath = sAutoPath.getToken(0, ';');
    sUserPath = sAutoPath.getToken(1, ';');

    //fdo#67743 ensure the userdir exists so that any later attempt to copy the
    //shared autocorrect file into the user dir will succeed
    ::ucbhelper::Content aContent;
    Reference < ucb::XCommandEnvironment > xEnv;
    ::utl::UCBContentHelper::ensureFolder(comphelper::getProcessComponentContext(), xEnv, sUserPath, aContent);

    OUString* pS = &sSharePath;
    for( sal_uInt16 n = 0; n < 2; ++n, pS = &sUserPath )
    {
        INetURLObject aPath( *pS );
        aPath.insertName("acor");
        *pS = aPath.GetMainURL(INetURLObject::DECODE_TO_IURI);
    }
    pAutoCorrect = new SvxAutoCorrect( sSharePath, sUserPath );

    aBaseConfig.Load(true);
    aSwConfig.Load(true);
}

SvxAutoCorrCfg::~SvxAutoCorrCfg()
{
    delete pAutoCorrect;
}

void SvxAutoCorrCfg::SetAutoCorrect(SvxAutoCorrect *const pNew)
{
    if (pNew != pAutoCorrect)
    {
        if (pNew && (pAutoCorrect->GetFlags() != pNew->GetFlags()))
        {
            aBaseConfig.SetModified();
            aSwConfig.SetModified();
        }
        delete pAutoCorrect;
        pAutoCorrect = pNew;
    }
}

Sequence<OUString>  SvxBaseAutoCorrCfg::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Exceptions/TwoCapitalsAtStart",        //  0
        "Exceptions/CapitalAtStartSentence",    //  1
        "UseReplacementTable",                  //  2
        "TwoCapitalsAtStart",                   //  3
        "CapitalAtStartSentence",               //  4
        "ChangeUnderlineWeight",                //  5
        "SetInetAttribute",                     //  6
        "ChangeOrdinalNumber",                  //  7
        "AddNonBreakingSpace",                  //  8
        "ChangeDash",                           //  9
        "RemoveDoubleSpaces",                   // 10
        "ReplaceSingleQuote",                   // 11
        "SingleQuoteAtStart",                   // 12
        "SingleQuoteAtEnd",                     // 13
        "ReplaceDoubleQuote",                   // 14
        "DoubleQuoteAtStart",                   // 15
        "DoubleQuoteAtEnd",                     // 16
        "CorrectAccidentalCapsLock"             // 17
    };
    const int nCount = 18;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

void SvxBaseAutoCorrCfg::Load(bool bInit)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    if(bInit)
        EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        long nFlags = 0;        // default all off
        sal_Int32 nTemp = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= SaveWordCplSttLst;
                    break;//"Exceptions/TwoCapitalsAtStart",
                    case  1:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= SaveWordWrdSttLst;
                    break;//"Exceptions/CapitalAtStartSentence",
                    case  2:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= Autocorrect;
                    break;//"UseReplacementTable",
                    case  3:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= CapitalStartWord;
                    break;//"TwoCapitalsAtStart",
                    case  4:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= CapitalStartSentence;
                    break;//"CapitalAtStartSentence",
                    case  5:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= ChgWeightUnderl;
                    break;//"ChangeUnderlineWeight",
                    case  6:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= SetINetAttr;
                    break;//"SetInetAttribute",
                    case  7:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= ChgOrdinalNumber;
                    break;//"ChangeOrdinalNumber",
                    case 8:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                             nFlags |= AddNonBrkSpace;
                    break;//"AddNonBreakingSpace"
                    case  9:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= ChgToEnEmDash;
                    break;//"ChangeDash",
                    case 10:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= IgnoreDoubleSpace;
                    break;//"RemoveDoubleSpaces",
                    case 11:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= ChgSglQuotes;
                    break;//"ReplaceSingleQuote",
                    case 12:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetStartSingleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;//"SingleQuoteAtStart",
                    case 13:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetEndSingleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;//"SingleQuoteAtEnd",
                    case 14:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= ChgQuotes;
                    break;//"ReplaceDoubleQuote",
                    case 15:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetStartDoubleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;//"DoubleQuoteAtStart",
                    case 16:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetEndDoubleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;//"DoubleQuoteAtEnd"
                    case 17:
                        if(*o3tl::doAccess<bool>(pValues[nProp]))
                            nFlags |= CorrectCapsLock;
                    break;//"CorrectAccidentalCapsLock"
                }
            }
        }
        if( nFlags )
            rParent.pAutoCorrect->SetAutoCorrFlag( nFlags );
        rParent.pAutoCorrect->SetAutoCorrFlag( ( 0xffff & ~nFlags ), false );

    }
}

SvxBaseAutoCorrCfg::SvxBaseAutoCorrCfg(SvxAutoCorrCfg& rPar) :
    utl::ConfigItem(OUString("Office.Common/AutoCorrect")),
    rParent(rPar)
{
}

SvxBaseAutoCorrCfg::~SvxBaseAutoCorrCfg()
{
}

void SvxBaseAutoCorrCfg::ImplCommit()
{
    const long nFlags = rParent.pAutoCorrect->GetFlags();
    PutProperties(
        GetPropertyNames(),
        {css::uno::Any((nFlags & SaveWordCplSttLst) != 0),
            // "Exceptions/TwoCapitalsAtStart"
         css::uno::Any((nFlags & SaveWordWrdSttLst) != 0),
            // "Exceptions/CapitalAtStartSentence"
         css::uno::Any((nFlags & Autocorrect) != 0), // "UseReplacementTable"
         css::uno::Any((nFlags & CapitalStartWord) != 0),
            // "TwoCapitalsAtStart"
         css::uno::Any((nFlags & CapitalStartSentence) != 0),
            // "CapitalAtStartSentence"
         css::uno::Any((nFlags & ChgWeightUnderl) != 0),
            // "ChangeUnderlineWeight"
         css::uno::Any((nFlags & SetINetAttr) != 0), // "SetInetAttribute"
         css::uno::Any((nFlags & ChgOrdinalNumber) != 0),
            // "ChangeOrdinalNumber"
         css::uno::Any((nFlags & AddNonBrkSpace) != 0), // "AddNonBreakingSpace"
         css::uno::Any((nFlags & ChgToEnEmDash) != 0), // "ChangeDash"
         css::uno::Any((nFlags & IgnoreDoubleSpace) != 0),
            // "RemoveDoubleSpaces"
         css::uno::Any((nFlags & ChgSglQuotes) != 0), // "ReplaceSingleQuote"
         css::uno::Any(sal_Int32(rParent.pAutoCorrect->GetStartSingleQuote())),
            // "SingleQuoteAtStart"
         css::uno::Any(sal_Int32(rParent.pAutoCorrect->GetEndSingleQuote())),
            // "SingleQuoteAtEnd"
         css::uno::Any((nFlags & ChgQuotes) != 0), // "ReplaceDoubleQuote"
         css::uno::Any(sal_Int32(rParent.pAutoCorrect->GetStartDoubleQuote())),
            // "DoubleQuoteAtStart"
         css::uno::Any(sal_Int32(rParent.pAutoCorrect->GetEndDoubleQuote())),
            // "DoubleQuoteAtEnd"
         css::uno::Any((nFlags & CorrectCapsLock) != 0)});
            // "CorrectAccidentalCapsLock"
}

void SvxBaseAutoCorrCfg::Notify( const Sequence<OUString>& /* aPropertyNames */)
{
    Load(false);
}

Sequence<OUString>  SvxSwAutoCorrCfg::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Text/FileLinks",                             // 0
        "Text/InternetLinks",                         // 1
        "Text/ShowPreview",                           // 2
        "Text/ShowToolTip",                           // 3
        "Text/SearchInAllCategories",                                   // 4
        "Format/Option/UseReplacementTable",                            // 5
        "Format/Option/TwoCapitalsAtStart",                             // 6
        "Format/Option/CapitalAtStartSentence",                         // 7
        "Format/Option/ChangeUnderlineWeight",                          // 8
        "Format/Option/SetInetAttribute",                               // 9
        "Format/Option/ChangeOrdinalNumber",                            //10
        "Format/Option/AddNonBreakingSpace",                            //11
        "Format/Option/ChangeDash",                                     //12
        "Format/Option/DelEmptyParagraphs",                             //13
        "Format/Option/ReplaceUserStyle",                               //14
        "Format/Option/ChangeToBullets/Enable",                         //15
        "Format/Option/ChangeToBullets/SpecialCharacter/Char",          //16
        "Format/Option/ChangeToBullets/SpecialCharacter/Font",          //17
        "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",    //18
        "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",   //19
        "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",     //20
        "Format/Option/CombineParagraphs",                              //21
        "Format/Option/CombineValue",                                   //22
        "Format/Option/DelSpacesAtStartEnd",                            //23
        "Format/Option/DelSpacesBetween",                               //24
        "Format/ByInput/Enable",                                        //25
        "Format/ByInput/ChangeDash",                                    //26
        "Format/ByInput/ApplyNumbering/Enable",                         //27
        "Format/ByInput/ChangeToBorders",                               //28
        "Format/ByInput/ChangeToTable",                                 //29
        "Format/ByInput/ReplaceStyle",                                  //30
        "Format/ByInput/DelSpacesAtStartEnd",                           //31
        "Format/ByInput/DelSpacesBetween",                              //32
        "Completion/Enable",                                            //33
        "Completion/MinWordLen",                                        //34
        "Completion/MaxListLen",                                        //35
        "Completion/CollectWords",                                      //36
        "Completion/EndlessList",                                       //37
        "Completion/AppendBlank",                                       //38
        "Completion/ShowAsTip",                                         //39
        "Completion/AcceptKey",                                         //40
        "Completion/KeepList",                                          //41
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",          //42
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",          //43
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",    //44
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",   //45
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch"      //46
    };
    const int nCount = 47;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

void SvxSwAutoCorrCfg::Load(bool bInit)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    if(bInit)
        EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        SvxSwAutoFormatFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case   0: rParent.bFileRel = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Text/FileLinks",
                    case   1: rParent.bNetRel = *o3tl::doAccess<bool>(pValues[nProp]);  break; // "Text/InternetLinks",
                    case   2: rParent.bAutoTextPreview = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Text/ShowPreview",
                    case   3: rParent.bAutoTextTip = *o3tl::doAccess<bool>(pValues[nProp]);  break; // "Text/ShowToolTip",
                    case   4: rParent.bSearchInAllCategories = *o3tl::doAccess<bool>(pValues[nProp]);  break; //"Text/SearchInAllCategories"
                    case   5: rSwFlags.bAutoCorrect = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/UseReplacementTable",
                    case   6: rSwFlags.bCapitalStartSentence = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/TwoCapitalsAtStart",
                    case   7: rSwFlags.bCapitalStartWord = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/CapitalAtStartSentence",
                    case   8: rSwFlags.bChgWeightUnderl = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/ChangeUnderlineWeight",
                    case   9: rSwFlags.bSetINetAttr = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/SetInetAttribute",
                    case  10: rSwFlags.bChgOrdinalNumber = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/ChangeOrdinalNumber",
                    case  11: rSwFlags.bAddNonBrkSpace = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/AddNonBreakingSpace",
// it doesn't exist here - the common flags are used for that -> LM
//                  case  12: rSwFlags.bChgToEnEmDash = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/ChangeDash",
                    case  13: rSwFlags.bDelEmptyNode = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/DelEmptyParagraphs",
                    case  14: rSwFlags.bChgUserColl = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/ReplaceUserStyle",
                    case  15: rSwFlags.bChgEnumNum = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/ChangeToBullets/Enable",
                    case  16:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.cBullet =
                            sal::static_int_cast< sal_Unicode >(nVal);
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/Char",
                    case  17:
                    {
                        OUString sTemp; pValues[nProp] >>= sTemp;
                        rSwFlags.aBulletFont.SetFamilyName(sTemp);
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/Font",
                    case  18:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetFamily(FontFamily(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",
                    case  19:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetCharSet(rtl_TextEncoding(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",
                    case  20:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetPitch(FontPitch(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",
                    case  21: rSwFlags.bRightMargin = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/CombineParagraphs",
                    case  22:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nRightMargin =
                            sal::static_int_cast< sal_uInt8 >(nVal);
                    }
                    break; // "Format/Option/CombineValue",
                    case  23: rSwFlags.bAFormatDelSpacesAtSttEnd =  *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/DelSpacesAtStartEnd",
                    case  24: rSwFlags.bAFormatDelSpacesBetweenLines = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/Option/DelSpacesBetween",
                    case  25: rParent.bAutoFmtByInput = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/Enable",
                    case  26: rSwFlags.bChgToEnEmDash = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/ChangeDash",
                    case  27: rSwFlags.bSetNumRule = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/ApplyNumbering/Enable",
                    case  28: rSwFlags.bSetBorder = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/ChangeToBorders",
                    case  29: rSwFlags.bCreateTable = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/ChangeToTable",
                    case  30: rSwFlags.bReplaceStyles =  *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/ReplaceStyle",
                    case  31: rSwFlags.bAFormatByInpDelSpacesAtSttEnd =  *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/DelSpacesAtStartEnd",
                    case  32: rSwFlags.bAFormatByInpDelSpacesBetweenLines = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Format/ByInput/DelSpacesBetween",
                    case  33: rSwFlags.bAutoCompleteWords = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Completion/Enable",
                    case  34:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltWordLen =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; // "Completion/MinWordLen",
                    case  35:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltListLen =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; // "Completion/MaxListLen",
                    case  36: rSwFlags.bAutoCmpltCollectWords = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Completion/CollectWords",
                    case  37: rSwFlags.bAutoCmpltEndless = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Completion/EndlessList",
                    case  38: rSwFlags.bAutoCmpltAppendBlanc = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Completion/AppendBlank",
                    case  39: rSwFlags.bAutoCmpltShowAsTip = *o3tl::doAccess<bool>(pValues[nProp]); break; // "Completion/ShowAsTip",
                    case  40:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltExpandKey =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; // "Completion/AcceptKey"
                    case 41 :rSwFlags.bAutoCmpltKeepList = *o3tl::doAccess<bool>(pValues[nProp]); break;//"Completion/KeepList"
                    case 42 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.cByInputBullet =
                            sal::static_int_cast< sal_Unicode >(nVal);
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",
                    case 43 :
                    {
                        OUString sTemp; pValues[nProp] >>= sTemp;
                        rSwFlags.aByInputBulletFont.SetFamilyName(sTemp);
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",
                    case 44 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetFamily(FontFamily(nVal));
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",
                    case 45 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetCharSet(rtl_TextEncoding(nVal));
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",
                    case 46 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetPitch(FontPitch(nVal));
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch",
                }
            }
        }
    }
}

SvxSwAutoCorrCfg::SvxSwAutoCorrCfg(SvxAutoCorrCfg& rPar) :
    utl::ConfigItem(OUString("Office.Writer/AutoFunction")),
    rParent(rPar)
{
}

SvxSwAutoCorrCfg::~SvxSwAutoCorrCfg()
{
}

void SvxSwAutoCorrCfg::ImplCommit()
{
    SvxSwAutoFormatFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
    PutProperties(
        GetPropertyNames(),
        {css::uno::Any(rParent.bFileRel), // "Text/FileLinks"
         css::uno::Any(rParent.bNetRel), // "Text/InternetLinks"
         css::uno::Any(rParent.bAutoTextPreview), // "Text/ShowPreview"
         css::uno::Any(rParent.bAutoTextTip), // "Text/ShowToolTip"
         css::uno::Any(rParent.bSearchInAllCategories),
            // "Text/SearchInAllCategories"
         css::uno::Any(rSwFlags.bAutoCorrect),
            // "Format/Option/UseReplacementTable"
         css::uno::Any(rSwFlags.bCapitalStartSentence),
            // "Format/Option/TwoCapitalsAtStart"
         css::uno::Any(rSwFlags.bCapitalStartWord),
            // "Format/Option/CapitalAtStartSentence"
         css::uno::Any(rSwFlags.bChgWeightUnderl),
            // "Format/Option/ChangeUnderlineWeight"
         css::uno::Any(rSwFlags.bSetINetAttr),
            // "Format/Option/SetInetAttribute"
         css::uno::Any(rSwFlags.bChgOrdinalNumber),
            // "Format/Option/ChangeOrdinalNumber"
         css::uno::Any(rSwFlags.bAddNonBrkSpace),
            // "Format/Option/AddNonBreakingSpace"
         css::uno::Any(true),
            // "Format/Option/ChangeDash"; it doesn't exist here - the common
            // flags are used for that -> LM
         css::uno::Any(rSwFlags.bDelEmptyNode),
            // "Format/Option/DelEmptyParagraphs"
         css::uno::Any(rSwFlags.bChgUserColl),
            // "Format/Option/ReplaceUserStyle"
         css::uno::Any(rSwFlags.bChgEnumNum),
            // "Format/Option/ChangeToBullets/Enable"
         css::uno::Any(sal_Int32(rSwFlags.cBullet)),
            // "Format/Option/ChangeToBullets/SpecialCharacter/Char"
         css::uno::Any(OUString(rSwFlags.aBulletFont.GetFamilyName())),
            // "Format/Option/ChangeToBullets/SpecialCharacter/Font"
         css::uno::Any(sal_Int32(rSwFlags.aBulletFont.GetFamilyType())),
            // "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily"
         css::uno::Any(sal_Int32(rSwFlags.aBulletFont.GetCharSet())),
            // "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset"
         css::uno::Any(sal_Int32(rSwFlags.aBulletFont.GetPitch())),
            // "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch"
         css::uno::Any(rSwFlags.bRightMargin),
            // "Format/Option/CombineParagraphs"
         css::uno::Any(sal_Int32(rSwFlags.nRightMargin)),
            // "Format/Option/CombineValue"
         css::uno::Any(rSwFlags.bAFormatDelSpacesAtSttEnd),
            // "Format/Option/DelSpacesAtStartEnd"
         css::uno::Any(rSwFlags.bAFormatDelSpacesBetweenLines),
            // "Format/Option/DelSpacesBetween"
         css::uno::Any(rParent.bAutoFmtByInput), // "Format/ByInput/Enable"
         css::uno::Any(rSwFlags.bChgToEnEmDash), // "Format/ByInput/ChangeDash"
         css::uno::Any(rSwFlags.bSetNumRule),
            // "Format/ByInput/ApplyNumbering/Enable"
         css::uno::Any(rSwFlags.bSetBorder), // "Format/ByInput/ChangeToBorders"
         css::uno::Any(rSwFlags.bCreateTable), // "Format/ByInput/ChangeToTable"
         css::uno::Any(rSwFlags.bReplaceStyles),
            // "Format/ByInput/ReplaceStyle"
         css::uno::Any(rSwFlags.bAFormatByInpDelSpacesAtSttEnd),
            // "Format/ByInput/DelSpacesAtStartEnd"
         css::uno::Any(rSwFlags.bAFormatByInpDelSpacesBetweenLines),
            // "Format/ByInput/DelSpacesBetween"
         css::uno::Any(rSwFlags.bAutoCompleteWords), // "Completion/Enable"
         css::uno::Any(sal_Int32(rSwFlags.nAutoCmpltWordLen)),
            // "Completion/MinWordLen"
         css::uno::Any(sal_Int32(rSwFlags.nAutoCmpltListLen)),
            // "Completion/MaxListLen"
         css::uno::Any(rSwFlags.bAutoCmpltCollectWords),
            // "Completion/CollectWords"
         css::uno::Any(rSwFlags.bAutoCmpltEndless), // "Completion/EndlessList"
         css::uno::Any(rSwFlags.bAutoCmpltAppendBlanc),
            // "Completion/AppendBlank"
         css::uno::Any(rSwFlags.bAutoCmpltShowAsTip), // "Completion/ShowAsTip"
         css::uno::Any(sal_Int32(rSwFlags.nAutoCmpltExpandKey)),
            // "Completion/AcceptKey"
         css::uno::Any(rSwFlags.bAutoCmpltKeepList), // "Completion/KeepList"
         css::uno::Any(sal_Int32(rSwFlags.cByInputBullet)),
            // "Format/ByInput/ApplyNumbering/SpecialCharacter/Char"
         css::uno::Any(OUString(rSwFlags.aByInputBulletFont.GetFamilyName())),
            // "Format/ByInput/ApplyNumbering/SpecialCharacter/Font"
         css::uno::Any(sal_Int32(rSwFlags.aByInputBulletFont.GetFamilyType())),
            // "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily"
         css::uno::Any(sal_Int32(rSwFlags.aByInputBulletFont.GetCharSet())),
            // "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset"
         css::uno::Any(sal_Int32(rSwFlags.aByInputBulletFont.GetPitch()))});
            // "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch"
}

void SvxSwAutoCorrCfg::Notify( const Sequence<OUString>& /* aPropertyNames */ )
{
    Load(false);
}

namespace
{
    class theSvxAutoCorrCfg : public rtl::Static<SvxAutoCorrCfg, theSvxAutoCorrCfg>{};
}

SvxAutoCorrCfg& SvxAutoCorrCfg::Get()
{
    return theSvxAutoCorrCfg::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
