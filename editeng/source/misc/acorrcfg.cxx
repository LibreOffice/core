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
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include <svl/urihelper.hxx>

#include <editeng/svxacorr.hxx>
#include <com/sun/star/uno/Any.hxx>
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
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= SaveWordCplSttLst;
                    break;//"Exceptions/TwoCapitalsAtStart",
                    case  1:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= SaveWordWrdSttLst;
                    break;//"Exceptions/CapitalAtStartSentence",
                    case  2:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= Autocorrect;
                    break;//"UseReplacementTable",
                    case  3:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= CapitalStartWord;
                    break;//"TwoCapitalsAtStart",
                    case  4:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= CapitalStartSentence;
                    break;//"CapitalAtStartSentence",
                    case  5:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= ChgWeightUnderl;
                    break;//"ChangeUnderlineWeight",
                    case  6:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= SetINetAttr;
                    break;//"SetInetAttribute",
                    case  7:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= ChgOrdinalNumber;
                    break;//"ChangeOrdinalNumber",
                    case 8:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                             nFlags |= AddNonBrkSpace;
                    break;//"AddNonBreakingSpace"
                    case  9:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= ChgToEnEmDash;
                    break;//"ChangeDash",
                    case 10:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
                            nFlags |= IgnoreDoubleSpace;
                    break;//"RemoveDoubleSpaces",
                    case 11:
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
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
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
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
                        if(*static_cast<sal_Bool const *>(pValues[nProp].getValue()))
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
    Sequence<OUString> aNames( GetPropertyNames() );

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = cppu::UnoType<bool>::get();
    sal_Bool bVal;
    const long nFlags = rParent.pAutoCorrect->GetFlags();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0:
                bVal = 0 != (nFlags & SaveWordCplSttLst);
                pValues[nProp].setValue(&bVal, rType);
            break;//"Exceptions/TwoCapitalsAtStart",
            case  1:
                bVal = 0 != (nFlags & SaveWordWrdSttLst);
                pValues[nProp].setValue(&bVal, rType);
            break;//"Exceptions/CapitalAtStartSentence",
            case  2:
                bVal = 0 != (nFlags & Autocorrect);
                pValues[nProp].setValue(&bVal, rType);
            break;//"UseReplacementTable",
            case  3:
                bVal = 0 != (nFlags & CapitalStartWord);
                pValues[nProp].setValue(&bVal, rType);
            break;//"TwoCapitalsAtStart",
            case  4:
                bVal = 0 != (nFlags & CapitalStartSentence);
                pValues[nProp].setValue(&bVal, rType);
            break;//"CapitalAtStartSentence",
            case  5:
                bVal = 0 != (nFlags & ChgWeightUnderl);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ChangeUnderlineWeight",
            case  6:
                bVal = 0 != (nFlags & SetINetAttr);
                pValues[nProp].setValue(&bVal, rType);
            break;//"SetInetAttribute",
            case  7:
                bVal = 0 != (nFlags & ChgOrdinalNumber);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ChangeOrdinalNumber",
            case 8:
                bVal = 0 != (nFlags & AddNonBrkSpace);
                pValues[nProp].setValue(&bVal, rType);
            break;//"AddNonBreakingSpace"
            case  9:
                bVal = 0 != (nFlags & ChgToEnEmDash);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ChangeDash",
            case 10:
                bVal = 0 != (nFlags & IgnoreDoubleSpace);
                pValues[nProp].setValue(&bVal, rType);
            break;//"RemoveDoubleSpaces",
            case 11:
                bVal = 0 != (nFlags & ChgSglQuotes);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ReplaceSingleQuote",
            case 12:
                pValues[nProp] <<= (sal_Int32)rParent.pAutoCorrect->GetStartSingleQuote();
            break;//"SingleQuoteAtStart",
            case 13:
                pValues[nProp] <<= (sal_Int32) rParent.pAutoCorrect->GetEndSingleQuote();
            break;//"SingleQuoteAtEnd",
            case 14:
                bVal = 0 != (nFlags & ChgQuotes);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ReplaceDoubleQuote",
            case 15:
                pValues[nProp] <<= (sal_Int32) rParent.pAutoCorrect->GetStartDoubleQuote();
            break;//"DoubleQuoteAtStart",
            case 16:
                pValues[nProp] <<= (sal_Int32) rParent.pAutoCorrect->GetEndDoubleQuote();
            break;//"DoubleQuoteAtEnd"
            case 17:
                bVal = 0 != (nFlags & CorrectCapsLock);
                pValues[nProp].setValue(&bVal, rType);
            break;//"CorrectAccidentalCapsLock"
        }
    }
    PutProperties(aNames, aValues);
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
                    case   0: rParent.bFileRel = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Text/FileLinks",
                    case   1: rParent.bNetRel = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break; // "Text/InternetLinks",
                    case   2: rParent.bAutoTextPreview = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Text/ShowPreview",
                    case   3: rParent.bAutoTextTip = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break; // "Text/ShowToolTip",
                    case   4: rParent.bSearchInAllCategories = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break; //"Text/SearchInAllCategories"
                    case   5: rSwFlags.bAutoCorrect = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/UseReplacementTable",
                    case   6: rSwFlags.bCapitalStartSentence = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/TwoCapitalsAtStart",
                    case   7: rSwFlags.bCapitalStartWord = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/CapitalAtStartSentence",
                    case   8: rSwFlags.bChgWeightUnderl = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/ChangeUnderlineWeight",
                    case   9: rSwFlags.bSetINetAttr = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/SetInetAttribute",
                    case  10: rSwFlags.bChgOrdinalNumber = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/ChangeOrdinalNumber",
                    case  11: rSwFlags.bAddNonBrkSpace = *static_cast<sal_Bool const *>(pValues[nProp].getValue( )); break; // "Format/Option/AddNonBreakingSpace",
// it doesn't exist here - the common flags are used for that -> LM
//                  case  12: rSwFlags.bChgToEnEmDash = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/ChangeDash",
                    case  13: rSwFlags.bDelEmptyNode = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/DelEmptyParagraphs",
                    case  14: rSwFlags.bChgUserColl = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/ReplaceUserStyle",
                    case  15: rSwFlags.bChgEnumNum = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/ChangeToBullets/Enable",
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
                        rSwFlags.aBulletFont.SetName(sTemp);
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
                    case  21: rSwFlags.bRightMargin = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/CombineParagraphs",
                    case  22:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nRightMargin =
                            sal::static_int_cast< sal_uInt8 >(nVal);
                    }
                    break; // "Format/Option/CombineValue",
                    case  23: rSwFlags.bAFormatDelSpacesAtSttEnd =  *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/DelSpacesAtStartEnd",
                    case  24: rSwFlags.bAFormatDelSpacesBetweenLines = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/Option/DelSpacesBetween",
                    case  25: rParent.bAutoFmtByInput = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/Enable",
                    case  26: rSwFlags.bChgToEnEmDash = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/ChangeDash",
                    case  27: rSwFlags.bSetNumRule = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/ApplyNumbering/Enable",
                    case  28: rSwFlags.bSetBorder = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/ChangeToBorders",
                    case  29: rSwFlags.bCreateTable = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/ChangeToTable",
                    case  30: rSwFlags.bReplaceStyles =  *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/ReplaceStyle",
                    case  31: rSwFlags.bAFormatByInpDelSpacesAtSttEnd =  *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/DelSpacesAtStartEnd",
                    case  32: rSwFlags.bAFormatByInpDelSpacesBetweenLines = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Format/ByInput/DelSpacesBetween",
                    case  33: rSwFlags.bAutoCompleteWords = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Completion/Enable",
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
                    case  36: rSwFlags.bAutoCmpltCollectWords = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Completion/CollectWords",
                    case  37: rSwFlags.bAutoCmpltEndless = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Completion/EndlessList",
                    case  38: rSwFlags.bAutoCmpltAppendBlanc = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Completion/AppendBlank",
                    case  39: rSwFlags.bAutoCmpltShowAsTip = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break; // "Completion/ShowAsTip",
                    case  40:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltExpandKey =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; // "Completion/AcceptKey"
                    case 41 :rSwFlags.bAutoCmpltKeepList = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;//"Completion/KeepList"
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
                        rSwFlags.aByInputBulletFont.SetName(sTemp);
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
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = cppu::UnoType<bool>::get();
    sal_Bool bVal;
    SvxSwAutoFormatFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case   0: pValues[nProp].setValue(&rParent.bFileRel, rType); break; // "Text/FileLinks",
            case   1: pValues[nProp].setValue(&rParent.bNetRel, rType);   break; // "Text/InternetLinks",
            case   2: pValues[nProp].setValue(&rParent.bAutoTextPreview, rType); break; // "Text/ShowPreview",
            case   3: pValues[nProp].setValue(&rParent.bAutoTextTip, rType); break; // "Text/ShowToolTip",
            case   4: pValues[nProp].setValue(&rParent.bSearchInAllCategories, rType );break; //"Text/SearchInAllCategories"
            case   5: bVal = rSwFlags.bAutoCorrect; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/UseReplacementTable",
            case   6: bVal = rSwFlags.bCapitalStartSentence; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/TwoCapitalsAtStart",
            case   7: bVal = rSwFlags.bCapitalStartWord; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/CapitalAtStartSentence",
            case   8: bVal = rSwFlags.bChgWeightUnderl; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeUnderlineWeight",
            case   9: bVal = rSwFlags.bSetINetAttr; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/SetInetAttribute",
            case  10: bVal = rSwFlags.bChgOrdinalNumber; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeOrdinalNumber",
            case  11: bVal = rSwFlags.bAddNonBrkSpace;  pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/AddNonBreakingSpace",
// it doesn't exist here - the common flags are used for that -> LM
            case  12:
                bVal = true;  pValues[nProp].setValue(&bVal, rType);
            break; // "Format/Option/ChangeDash",
            case  13: bVal = rSwFlags.bDelEmptyNode; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelEmptyParagraphs",
            case  14: bVal = rSwFlags.bChgUserColl; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ReplaceUserStyle",
            case  15: bVal = rSwFlags.bChgEnumNum; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeToBullets/Enable",
            case  16:
                pValues[nProp] <<= (sal_Int32)rSwFlags.cBullet;
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/Char",
            case  17:
                pValues[nProp] <<= OUString(rSwFlags.aBulletFont.GetFamilyName());
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/Font",
            case  18:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetFamily();
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",
            case  19:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetCharSet();
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",
            case  20:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetPitch();
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",
            case  21: bVal = rSwFlags.bRightMargin; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/CombineParagraphs",
            case  22:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nRightMargin;
            break; // "Format/Option/CombineValue",
            case  23: bVal = rSwFlags.bAFormatDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelSpacesAtStartEnd",
            case  24: bVal = rSwFlags.bAFormatDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelSpacesBetween",
            case  25: bVal = rParent.bAutoFmtByInput; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/Enable",
            case  26: bVal = rSwFlags.bChgToEnEmDash; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeDash",
            case  27: bVal = rSwFlags.bSetNumRule; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ApplyNumbering/Enable",
            case  28: bVal = rSwFlags.bSetBorder; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeToBorders",
            case  29: bVal = rSwFlags.bCreateTable; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeToTable",
            case  30: bVal = rSwFlags.bReplaceStyles; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ReplaceStyle",
            case  31: bVal = rSwFlags.bAFormatByInpDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/DelSpacesAtStartEnd",
            case  32: bVal = rSwFlags.bAFormatByInpDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/DelSpacesBetween",
            case  33: bVal = rSwFlags.bAutoCompleteWords; pValues[nProp].setValue(&bVal, rType); break; // "Completion/Enable",
            case  34:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltWordLen;
            break; // "Completion/MinWordLen",
            case  35:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltListLen;
            break; // "Completion/MaxListLen",
            case  36: bVal = rSwFlags.bAutoCmpltCollectWords; pValues[nProp].setValue(&bVal, rType); break; // "Completion/CollectWords",
            case  37: bVal = rSwFlags.bAutoCmpltEndless; pValues[nProp].setValue(&bVal, rType); break; // "Completion/EndlessList",
            case  38: bVal = rSwFlags.bAutoCmpltAppendBlanc; pValues[nProp].setValue(&bVal, rType); break; // "Completion/AppendBlank",
            case  39: bVal = rSwFlags.bAutoCmpltShowAsTip; pValues[nProp].setValue(&bVal, rType); break; // "Completion/ShowAsTip",
            case  40:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltExpandKey;
            break; // "Completion/AcceptKey"
            case 41 :bVal = rSwFlags.bAutoCmpltKeepList; pValues[nProp].setValue(&bVal, rType); break;// "Completion/KeepList"
            case 42 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.cByInputBullet;
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",
            case 43 :
                pValues[nProp] <<= OUString(rSwFlags.aByInputBulletFont.GetFamilyName());
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",
            case 44 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetFamily();
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",
            case 45 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetCharSet();
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",
            case 46 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetPitch();
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch",
        }
    }
    PutProperties(aNames, aValues);
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
