/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

    
    
    ::ucbhelper::Content aContent;
    Reference < ucb::XCommandEnvironment > xEnv;
    ::utl::UCBContentHelper::ensureFolder(comphelper::getProcessComponentContext(), xEnv, sUserPath, aContent);

    OUString* pS = &sSharePath;
    for( sal_uInt16 n = 0; n < 2; ++n, pS = &sUserPath )
    {
        INetURLObject aPath( *pS );
        aPath.insertName(OUString("acor"));
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

void SvxAutoCorrCfg::SetAutoCorrect( SvxAutoCorrect* pNew )
{
    if( pNew && pNew != pAutoCorrect )
    {
        if( pAutoCorrect->GetFlags() != pNew->GetFlags() )
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
        "Exceptions/TwoCapitalsAtStart",        
        "Exceptions/CapitalAtStartSentence",    
        "UseReplacementTable",                  
        "TwoCapitalsAtStart",                   
        "CapitalAtStartSentence",               
        "ChangeUnderlineWeight",                
        "SetInetAttribute",                     
        "ChangeOrdinalNumber",                  
        "AddNonBreakingSpace",                  
        "ChangeDash",                           
        "RemoveDoubleSpaces",                   
        "ReplaceSingleQuote",                   
        "SingleQuoteAtStart",                   
        "SingleQuoteAtEnd",                     
        "ReplaceDoubleQuote",                   
        "DoubleQuoteAtStart",                   
        "DoubleQuoteAtEnd",                     
        "CorrectAccidentalCapsLock"             
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
        long nFlags = 0;        
        sal_Int32 nTemp = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= SaveWordCplSttLst;
                    break;
                    case  1:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= SaveWordWrdSttLst;
                    break;
                    case  2:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= Autocorrect;
                    break;
                    case  3:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= CptlSttWrd;
                    break;
                    case  4:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= CptlSttSntnc;
                    break;
                    case  5:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgWeightUnderl;
                    break;
                    case  6:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= SetINetAttr;
                    break;
                    case  7:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgOrdinalNumber;
                    break;
                    case 8:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                             nFlags |= AddNonBrkSpace;
                    break;
                    case  9:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgToEnEmDash;
                    break;
                    case 10:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= IgnoreDoubleSpace;
                    break;
                    case 11:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgSglQuotes;
                    break;
                    case 12:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetStartSingleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;
                    case 13:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetEndSingleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;
                    case 14:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgQuotes;
                    break;
                    case 15:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetStartDoubleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;
                    case 16:
                        pValues[nProp] >>= nTemp;
                        rParent.pAutoCorrect->SetEndDoubleQuote(
                            sal::static_int_cast< sal_Unicode >( nTemp ) );
                    break;
                    case 17:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= CorrectCapsLock;
                    break;
                }
            }
        }
        if( nFlags )
            rParent.pAutoCorrect->SetAutoCorrFlag( nFlags, sal_True );
        rParent.pAutoCorrect->SetAutoCorrFlag( ( 0xffff & ~nFlags ), sal_False );

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

void SvxBaseAutoCorrCfg::Commit()
{
    Sequence<OUString> aNames( GetPropertyNames() );

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    sal_Bool bVal;
    const long nFlags = rParent.pAutoCorrect->GetFlags();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0:
                bVal = 0 != (nFlags & SaveWordCplSttLst);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  1:
                bVal = 0 != (nFlags & SaveWordWrdSttLst);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  2:
                bVal = 0 != (nFlags & Autocorrect);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  3:
                bVal = 0 != (nFlags & CptlSttWrd);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  4:
                bVal = 0 != (nFlags & CptlSttSntnc);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  5:
                bVal = 0 != (nFlags & ChgWeightUnderl);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  6:
                bVal = 0 != (nFlags & SetINetAttr);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  7:
                bVal = 0 != (nFlags & ChgOrdinalNumber);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case 8:
                bVal = 0 != (nFlags & AddNonBrkSpace);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case  9:
                bVal = 0 != (nFlags & ChgToEnEmDash);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case 10:
                bVal = 0 != (nFlags & IgnoreDoubleSpace);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case 11:
                bVal = 0 != (nFlags & ChgSglQuotes);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case 12:
                pValues[nProp] <<= (sal_Int32)rParent.pAutoCorrect->GetStartSingleQuote();
            break;
            case 13:
                pValues[nProp] <<= (sal_Int32) rParent.pAutoCorrect->GetEndSingleQuote();
            break;
            case 14:
                bVal = 0 != (nFlags & ChgQuotes);
                pValues[nProp].setValue(&bVal, rType);
            break;
            case 15:
                pValues[nProp] <<= (sal_Int32) rParent.pAutoCorrect->GetStartDoubleQuote();
            break;
            case 16:
                pValues[nProp] <<= (sal_Int32) rParent.pAutoCorrect->GetEndDoubleQuote();
            break;
            case 17:
                bVal = 0 != (nFlags & CorrectCapsLock);
                pValues[nProp].setValue(&bVal, rType);
            break;
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
        "Text/FileLinks",                             
        "Text/InternetLinks",                         
        "Text/ShowPreview",                           
        "Text/ShowToolTip",                           
        "Text/SearchInAllCategories",                                   
        "Format/Option/UseReplacementTable",                            
        "Format/Option/TwoCapitalsAtStart",                             
        "Format/Option/CapitalAtStartSentence",                         
        "Format/Option/ChangeUnderlineWeight",                          
        "Format/Option/SetInetAttribute",                               
        "Format/Option/ChangeOrdinalNumber",                            
        "Format/Option/AddNonBreakingSpace",                            
        "Format/Option/ChangeDash",                                     
        "Format/Option/DelEmptyParagraphs",                             
        "Format/Option/ReplaceUserStyle",                               
        "Format/Option/ChangeToBullets/Enable",                         
        "Format/Option/ChangeToBullets/SpecialCharacter/Char",          
        "Format/Option/ChangeToBullets/SpecialCharacter/Font",          
        "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",    
        "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",   
        "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",     
        "Format/Option/CombineParagraphs",                              
        "Format/Option/CombineValue",                                   
        "Format/Option/DelSpacesAtStartEnd",                            
        "Format/Option/DelSpacesBetween",                               
        "Format/ByInput/Enable",                                        
        "Format/ByInput/ChangeDash",                                    
        "Format/ByInput/ApplyNumbering/Enable",                         
        "Format/ByInput/ChangeToBorders",                               
        "Format/ByInput/ChangeToTable",                                 
        "Format/ByInput/ReplaceStyle",                                  
        "Format/ByInput/DelSpacesAtStartEnd",                           
        "Format/ByInput/DelSpacesBetween",                              
        "Completion/Enable",                                            
        "Completion/MinWordLen",                                        
        "Completion/MaxListLen",                                        
        "Completion/CollectWords",                                      
        "Completion/EndlessList",                                       
        "Completion/AppendBlank",                                       
        "Completion/ShowAsTip",                                         
        "Completion/AcceptKey",                                         
        "Completion/KeepList",                                          
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",          
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",          
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",    
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",   
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch"      
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
        SvxSwAutoFmtFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case   0: rParent.bFileRel = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case   1: rParent.bNetRel = *(sal_Bool*)pValues[nProp].getValue();  break; 
                    case   2: rParent.bAutoTextPreview = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case   3: rParent.bAutoTextTip = *(sal_Bool*)pValues[nProp].getValue();  break; 
                    case   4: rParent.bSearchInAllCategories = *(sal_Bool*)pValues[nProp].getValue();  break; 
                    case   5: rSwFlags.bAutoCorrect = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case   6: rSwFlags.bCptlSttSntnc = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case   7: rSwFlags.bCptlSttWrd = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case   8: rSwFlags.bChgWeightUnderl = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case   9: rSwFlags.bSetINetAttr = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  10: rSwFlags.bChgOrdinalNumber = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  11: rSwFlags.bAddNonBrkSpace = *(sal_Bool*)pValues[nProp].getValue( ); break; 


                    case  13: rSwFlags.bDelEmptyNode = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  14: rSwFlags.bChgUserColl = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  15: rSwFlags.bChgEnumNum = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  16:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.cBullet =
                            sal::static_int_cast< sal_Unicode >(nVal);
                    }
                    break; 
                    case  17:
                    {
                        OUString sTemp; pValues[nProp] >>= sTemp;
                        rSwFlags.aBulletFont.SetName(sTemp);
                    }
                    break; 
                    case  18:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetFamily(FontFamily(nVal));
                    }
                    break; 
                    case  19:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetCharSet(rtl_TextEncoding(nVal));
                    }
                    break; 
                    case  20:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetPitch(FontPitch(nVal));
                    }
                    break; 
                    case  21: rSwFlags.bRightMargin = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  22:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nRightMargin =
                            sal::static_int_cast< sal_uInt8 >(nVal);
                    }
                    break; 
                    case  23: rSwFlags.bAFmtDelSpacesAtSttEnd =  *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  24: rSwFlags.bAFmtDelSpacesBetweenLines = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  25: rParent.bAutoFmtByInput = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  26: rSwFlags.bChgToEnEmDash = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  27: rSwFlags.bSetNumRule = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  28: rSwFlags.bSetBorder = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  29: rSwFlags.bCreateTable = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  30: rSwFlags.bReplaceStyles =  *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  31: rSwFlags.bAFmtByInpDelSpacesAtSttEnd =  *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  32: rSwFlags.bAFmtByInpDelSpacesBetweenLines = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  33: rSwFlags.bAutoCompleteWords = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  34:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltWordLen =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; 
                    case  35:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltListLen =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; 
                    case  36: rSwFlags.bAutoCmpltCollectWords = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  37: rSwFlags.bAutoCmpltEndless = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  38: rSwFlags.bAutoCmpltAppendBlanc = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  39: rSwFlags.bAutoCmpltShowAsTip = *(sal_Bool*)pValues[nProp].getValue(); break; 
                    case  40:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltExpandKey =
                            sal::static_int_cast< sal_uInt16 >(nVal);
                    }
                    break; 
                    case 41 :rSwFlags.bAutoCmpltKeepList = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 42 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.cByInputBullet =
                            sal::static_int_cast< sal_Unicode >(nVal);
                    }
                    break;
                    case 43 :
                    {
                        OUString sTemp; pValues[nProp] >>= sTemp;
                        rSwFlags.aByInputBulletFont.SetName(sTemp);
                    }
                    break;
                    case 44 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetFamily(FontFamily(nVal));
                    }
                    break;
                    case 45 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetCharSet(rtl_TextEncoding(nVal));
                    }
                    break;
                    case 46 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetPitch(FontPitch(nVal));
                    }
                    break;
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

void SvxSwAutoCorrCfg::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    sal_Bool bVal;
    SvxSwAutoFmtFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case   0: pValues[nProp].setValue(&rParent.bFileRel, rType); break; 
            case   1: pValues[nProp].setValue(&rParent.bNetRel, rType);   break; 
            case   2: pValues[nProp].setValue(&rParent.bAutoTextPreview, rType); break; 
            case   3: pValues[nProp].setValue(&rParent.bAutoTextTip, rType); break; 
            case   4: pValues[nProp].setValue(&rParent.bSearchInAllCategories, rType );break; 
            case   5: bVal = rSwFlags.bAutoCorrect; pValues[nProp].setValue(&bVal, rType); break; 
            case   6: bVal = rSwFlags.bCptlSttSntnc; pValues[nProp].setValue(&bVal, rType); break; 
            case   7: bVal = rSwFlags.bCptlSttWrd; pValues[nProp].setValue(&bVal, rType); break; 
            case   8: bVal = rSwFlags.bChgWeightUnderl; pValues[nProp].setValue(&bVal, rType); break; 
            case   9: bVal = rSwFlags.bSetINetAttr; pValues[nProp].setValue(&bVal, rType); break; 
            case  10: bVal = rSwFlags.bChgOrdinalNumber; pValues[nProp].setValue(&bVal, rType); break; 
            case  11: bVal = rSwFlags.bAddNonBrkSpace;  pValues[nProp].setValue(&bVal, rType); break; 

            case  12:
                bVal = sal_True;  pValues[nProp].setValue(&bVal, rType);
            break; 
            case  13: bVal = rSwFlags.bDelEmptyNode; pValues[nProp].setValue(&bVal, rType); break; 
            case  14: bVal = rSwFlags.bChgUserColl; pValues[nProp].setValue(&bVal, rType); break; 
            case  15: bVal = rSwFlags.bChgEnumNum; pValues[nProp].setValue(&bVal, rType); break; 
            case  16:
                pValues[nProp] <<= (sal_Int32)rSwFlags.cBullet;
            break; 
            case  17:
                pValues[nProp] <<= OUString(rSwFlags.aBulletFont.GetName());
            break; 
            case  18:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetFamily();
            break; 
            case  19:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetCharSet();
            break; 
            case  20:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetPitch();
            break; 
            case  21: bVal = rSwFlags.bRightMargin; pValues[nProp].setValue(&bVal, rType); break; 
            case  22:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nRightMargin;
            break; 
            case  23: bVal = rSwFlags.bAFmtDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; 
            case  24: bVal = rSwFlags.bAFmtDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; 
            case  25: bVal = rParent.bAutoFmtByInput; pValues[nProp].setValue(&bVal, rType); break; 
            case  26: bVal = rSwFlags.bChgToEnEmDash; pValues[nProp].setValue(&bVal, rType); break; 
            case  27: bVal = rSwFlags.bSetNumRule; pValues[nProp].setValue(&bVal, rType); break; 
            case  28: bVal = rSwFlags.bSetBorder; pValues[nProp].setValue(&bVal, rType); break; 
            case  29: bVal = rSwFlags.bCreateTable; pValues[nProp].setValue(&bVal, rType); break; 
            case  30: bVal = rSwFlags.bReplaceStyles; pValues[nProp].setValue(&bVal, rType); break; 
            case  31: bVal = rSwFlags.bAFmtByInpDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; 
            case  32: bVal = rSwFlags.bAFmtByInpDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; 
            case  33: bVal = rSwFlags.bAutoCompleteWords; pValues[nProp].setValue(&bVal, rType); break; 
            case  34:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltWordLen;
            break; 
            case  35:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltListLen;
            break; 
            case  36: bVal = rSwFlags.bAutoCmpltCollectWords; pValues[nProp].setValue(&bVal, rType); break; 
            case  37: bVal = rSwFlags.bAutoCmpltEndless; pValues[nProp].setValue(&bVal, rType); break; 
            case  38: bVal = rSwFlags.bAutoCmpltAppendBlanc; pValues[nProp].setValue(&bVal, rType); break; 
            case  39: bVal = rSwFlags.bAutoCmpltShowAsTip; pValues[nProp].setValue(&bVal, rType); break; 
            case  40:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltExpandKey;
            break; 
            case 41 :bVal = rSwFlags.bAutoCmpltKeepList; pValues[nProp].setValue(&bVal, rType); break;
            case 42 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.cByInputBullet;
            break;
            case 43 :
                pValues[nProp] <<= OUString(rSwFlags.aByInputBulletFont.GetName());
            break;
            case 44 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetFamily();
            break;
            case 45 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetCharSet();
            break;
            case 46 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetPitch();
            break;
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
