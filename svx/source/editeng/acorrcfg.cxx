/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acorrcfg.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 15:37:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "acorrcfg.hxx"

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/urihelper.hxx>

#include <svx/svxacorr.hxx>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

static SvxAutoCorrCfg* pAutoCorrCfg = 0;

/*--------------------------------------------------------------------
    Beschreibung:   Ctor Dtor
 --------------------------------------------------------------------*/

SvxAutoCorrCfg::SvxAutoCorrCfg() :
    aBaseConfig(*this),
    aSwConfig(*this),
    bFileRel(TRUE),
    bNetRel(TRUE),
    bAutoTextTip(TRUE),
    bAutoTextPreview(FALSE),
    bAutoFmtByInput(TRUE),
    bSearchInAllCategories(FALSE)
{
    SvtPathOptions aPathOpt;
    String sSharePath, sUserPath, sAutoPath( aPathOpt.GetAutoCorrectPath() );

    String* pS = &sSharePath;
    for( USHORT n = 0; n < 2; ++n, pS = &sUserPath )
    {
        *pS = sAutoPath.GetToken( n, ';' );
        INetURLObject aPath( *pS );
        aPath.insertName( String::CreateFromAscii("acor") );
        *pS = aPath.GetMainURL(INetURLObject::DECODE_TO_IURI);
    }
    pAutoCorrect = new SvxAutoCorrect( sSharePath, sUserPath );

    aBaseConfig.Load(sal_True);
    aSwConfig.Load(sal_True);
}

SvxAutoCorrCfg::~SvxAutoCorrCfg()
{
    delete pAutoCorrect;
}

/*String SvxAutoCorrCfg::GetName() const
{
    return String( OffResId( RID_STR_NAME_ACORRCFG ));
}
*/
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
/*-- 12.10.00 11:44:17---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        "ChangeFraction",                       //  8
        "ChangeDash",                           //  9
        "RemoveDoubleSpaces",                   // 10
        "ReplaceSingleQuote",                   // 11
        "SingleQuoteAtStart",                   // 12
        "SingleQuoteAtEnd",                     // 13
        "ReplaceDoubleQuote",                   // 14
        "DoubleQuoteAtStart",                   // 15
        "DoubleQuoteAtEnd"                      // 16
    };
    const int nCount = 17;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}
/*-- 12.10.00 11:44:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxBaseAutoCorrCfg::Load(sal_Bool bInit)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    if(bInit)
        EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        long nFlags = 0;        // default alles aus
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
                    break;//"Exceptions/TwoCapitalsAtStart",
                    case  1:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= SaveWordWrdSttLst;
                    break;//"Exceptions/CapitalAtStartSentence",
                    case  2:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= Autocorrect;
                    break;//"UseReplacementTable",
                    case  3:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= CptlSttWrd;
                    break;//"TwoCapitalsAtStart",
                    case  4:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= CptlSttSntnc;
                    break;//"CapitalAtStartSentence",
                    case  5:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgWeightUnderl;
                    break;//"ChangeUnderlineWeight",
                    case  6:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= SetINetAttr;
                    break;//"SetInetAttribute",
                    case  7:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgOrdinalNumber;
                    break;//"ChangeOrdinalNumber",
                    case  8:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgFractionSymbol;
                    break;//"ChangeFraction",
                    case  9:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= ChgToEnEmDash;
                    break;//"ChangeDash",
                    case 10:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            nFlags |= IngnoreDoubleSpace;
                    break;//"RemoveDoubleSpaces",
                    case 11:
                        if(*(sal_Bool*)pValues[nProp].getValue())
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
                        if(*(sal_Bool*)pValues[nProp].getValue())
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
                }
            }
        }
        if( nFlags )
            rParent.pAutoCorrect->SetAutoCorrFlag( nFlags, TRUE );
        rParent.pAutoCorrect->SetAutoCorrFlag( ( 0xffff & ~nFlags ), FALSE );

    }
}
/*-- 12.10.00 11:44:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxBaseAutoCorrCfg::SvxBaseAutoCorrCfg(SvxAutoCorrCfg& rPar) :
    utl::ConfigItem(C2U("Office.Common/AutoCorrect")),
    rParent(rPar)
{
}
/*-- 12.10.00 11:44:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxBaseAutoCorrCfg::~SvxBaseAutoCorrCfg()
{
}
/*-- 12.10.00 11:44:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxBaseAutoCorrCfg::Commit()
{
    Sequence<OUString> aNames( GetPropertyNames() );

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    BOOL bVal;
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
                bVal = 0 != (nFlags & CptlSttWrd);
                pValues[nProp].setValue(&bVal, rType);
            break;//"TwoCapitalsAtStart",
            case  4:
                bVal = 0 != (nFlags & CptlSttSntnc);
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
            case  8:
                bVal = 0 != (nFlags & ChgFractionSymbol);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ChangeFraction",
            case  9:
                bVal = 0 != (nFlags & ChgToEnEmDash);
                pValues[nProp].setValue(&bVal, rType);
            break;//"ChangeDash",
            case 10:
                bVal = 0 != (nFlags & IngnoreDoubleSpace);
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
        }
    }
    PutProperties(aNames, aValues);
}
/*-- 12.10.00 11:44:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxBaseAutoCorrCfg::Notify( const Sequence<OUString>& /* aPropertyNames */)
{
    Load(sal_False);
}
/*-- 12.10.00 11:51:48---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        "Format/Option/ChangeFraction",                                 //11
        "Format/Option/ChangeDash",                                     //12
        "Format/Option/DelEmptyParagraphs",                             //13
        "Format/Option/ReplaceUserStyle",                               //14
        "Format/Option/ChangeToBullets/Enable",                         //15
        "Format/Option/ChangeToBullets/SpecialCharacter/Char",          //16
        "Format/Option/ChangeToBullets/SpecialCharacter/Font",          //17
        "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",    //18
        "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",   //19
        "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",     //20
        "Format/Option/ReplaceQuote",                                   //21
        "Format/Option/CombineParagraphs",                              //22
        "Format/Option/CombineValue",                                   //23
        "Format/Option/DelSpacesAtStartEnd",                            //24
        "Format/Option/DelSpacesBetween",                               //25
        "Format/ByInput/Enable",                                        //26
        "Format/ByInput/ChangeDash",                                    //27
        "Format/ByInput/ApplyNumbering/Enable",                         //28
        "Format/ByInput/ChangeToBorders",                               //29
        "Format/ByInput/ChangeToTable",                                 //30
        "Format/ByInput/ReplaceStyle",                                  //31
        "Format/ByInput/DelSpacesAtStartEnd",                           //32
        "Format/ByInput/DelSpacesBetween",                              //33
        "Completion/Enable",                                            //34
        "Completion/MinWordLen",                                        //35
        "Completion/MaxListLen",                                        //36
        "Completion/CollectWords",                                      //37
        "Completion/EndlessList",                                       //38
        "Completion/AppendBlank",                                       //39
        "Completion/ShowAsTip",                                         //40
        "Completion/AcceptKey",                                         //41
        "Completion/KeepList",                                          //42
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",          //43
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",          //44
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",    //45
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",   //46
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch",     //47
    };
    const int nCount = 48;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}
/*-- 12.10.00 11:51:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxSwAutoCorrCfg::Load(sal_Bool bInit)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    if(bInit)
        EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        SvxSwAutoFmtFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case   0: rParent.bFileRel = *(sal_Bool*)pValues[nProp].getValue(); break; // "Text/FileLinks",
                    case   1: rParent.bNetRel = *(sal_Bool*)pValues[nProp].getValue();  break; // "Text/InternetLinks",
                    case   2: rParent.bAutoTextPreview = *(sal_Bool*)pValues[nProp].getValue(); break; // "Text/ShowPreview",
                    case   3: rParent.bAutoTextTip = *(sal_Bool*)pValues[nProp].getValue();  break; // "Text/ShowToolTip",
                    case   4: rParent.bSearchInAllCategories = *(sal_Bool*)pValues[nProp].getValue();  break; //"Text/SearchInAllCategories"
                    case   5: rSwFlags.bAutoCorrect = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/UseReplacementTable",
                    case   6: rSwFlags.bCptlSttSntnc = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/TwoCapitalsAtStart",
                    case   7: rSwFlags.bCptlSttWrd = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/CapitalAtStartSentence",
                    case   8: rSwFlags.bChgWeightUnderl = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeUnderlineWeight",
                    case   9: rSwFlags.bSetINetAttr = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/SetInetAttribute",
                    case  10: rSwFlags.bChgOrdinalNumber = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeOrdinalNumber",
                    case  11: rSwFlags.bChgFracionSymbol = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeFraction",
// it doesn't exist here - the common flags are used for that -> LM
//                  case  12: rSwFlags.bChgToEnEmDash = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeDash",
                    case  13: rSwFlags.bDelEmptyNode = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/DelEmptyParagraphs",
                    case  14: rSwFlags.bChgUserColl = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ReplaceUserStyle",
                    case  15: rSwFlags.bChgEnumNum = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeToBullets/Enable",
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
                        rSwFlags.aBulletFont.SetCharSet(CharSet(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",
                    case  20:
                    {
                                            sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetPitch(FontPitch(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",
                    case  21: rSwFlags.bReplaceQuote = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ReplaceQuote",
                    case  22: rSwFlags.bRightMargin = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/CombineParagraphs",
                    case  23:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nRightMargin =
                            sal::static_int_cast< BYTE >(nVal);
                    }
                    break; // "Format/Option/CombineValue",
                    case  24: rSwFlags.bAFmtDelSpacesAtSttEnd =  *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/DelSpacesAtStartEnd",
                    case  25: rSwFlags.bAFmtDelSpacesBetweenLines = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/DelSpacesBetween",
                    case  26: rParent.bAutoFmtByInput = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/Enable",
                    case  27: rSwFlags.bChgToEnEmDash = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ChangeDash",
                    case  28: rSwFlags.bSetNumRule = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ApplyNumbering/Enable",
                    case  29: rSwFlags.bSetBorder = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ChangeToBorders",
                    case  30: rSwFlags.bCreateTable = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ChangeToTable",
                    case  31: rSwFlags.bReplaceStyles =  *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ReplaceStyle",
                    case  32: rSwFlags.bAFmtByInpDelSpacesAtSttEnd =  *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/DelSpacesAtStartEnd",
                    case  33: rSwFlags.bAFmtByInpDelSpacesBetweenLines = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/DelSpacesBetween",
                    case  34: rSwFlags.bAutoCompleteWords = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/Enable",
                    case  35:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltWordLen =
                            sal::static_int_cast< USHORT >(nVal);
                    }
                    break; // "Completion/MinWordLen",
                    case  36:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltListLen =
                            sal::static_int_cast< USHORT >(nVal);
                    }
                    break; // "Completion/MaxListLen",
                    case  37: rSwFlags.bAutoCmpltCollectWords = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/CollectWords",
                    case  38: rSwFlags.bAutoCmpltEndless = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/EndlessList",
                    case  39: rSwFlags.bAutoCmpltAppendBlanc = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/AppendBlank",
                    case  40: rSwFlags.bAutoCmpltShowAsTip = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/ShowAsTip",
                    case  41:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltExpandKey =
                            sal::static_int_cast< USHORT >(nVal);
                    }
                    break; // "Completion/AcceptKey"
                    case 42 :rSwFlags.bAutoCmpltKeepList = *(sal_Bool*)pValues[nProp].getValue(); break;//"Completion/KeepList"
                    case 43 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.cByInputBullet =
                            sal::static_int_cast< sal_Unicode >(nVal);
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",
                    case 44 :
                    {
                        OUString sTemp; pValues[nProp] >>= sTemp;
                        rSwFlags.aByInputBulletFont.SetName(sTemp);
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",
                    case 45 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetFamily(FontFamily(nVal));
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",
                    case 46 :
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aByInputBulletFont.SetCharSet(CharSet(nVal));
                    }
                    break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",
                    case 47 :
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
/*-- 12.10.00 11:51:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxSwAutoCorrCfg::SvxSwAutoCorrCfg(SvxAutoCorrCfg& rPar) :
    utl::ConfigItem(C2U("Office.Writer/AutoFunction")),
    rParent(rPar)
{
}
/*-- 12.10.00 11:51:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxSwAutoCorrCfg::~SvxSwAutoCorrCfg()
{
}
/*-- 12.10.00 11:51:48---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxSwAutoCorrCfg::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    BOOL bVal;
    SvxSwAutoFmtFlags& rSwFlags = rParent.pAutoCorrect->GetSwFlags();
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
            case   6: bVal = rSwFlags.bCptlSttSntnc; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/TwoCapitalsAtStart",
            case   7: bVal = rSwFlags.bCptlSttWrd; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/CapitalAtStartSentence",
            case   8: bVal = rSwFlags.bChgWeightUnderl; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeUnderlineWeight",
            case   9: bVal = rSwFlags.bSetINetAttr; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/SetInetAttribute",
            case  10: bVal = rSwFlags.bChgOrdinalNumber; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeOrdinalNumber",
            case  11: bVal = rSwFlags.bChgFracionSymbol; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeFraction",
// it doesn't exist here - the common flags are used for that -> LM
            case  12:
                bVal = sal_True;  pValues[nProp].setValue(&bVal, rType);
            break; // "Format/Option/ChangeDash",
            case  13: bVal = rSwFlags.bDelEmptyNode; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelEmptyParagraphs",
            case  14: bVal = rSwFlags.bChgUserColl; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ReplaceUserStyle",
            case  15: bVal = rSwFlags.bChgEnumNum; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeToBullets/Enable",
            case  16:
                pValues[nProp] <<= (sal_Int32)rSwFlags.cBullet;
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/Char",
            case  17:
                pValues[nProp] <<= OUString(rSwFlags.aBulletFont.GetName());
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
            case  21: bVal = rSwFlags.bReplaceQuote; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ReplaceQuote",
            case  22: bVal = rSwFlags.bRightMargin; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/CombineParagraphs",
            case  23:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nRightMargin;
            break; // "Format/Option/CombineValue",
            case  24: bVal = rSwFlags.bAFmtDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelSpacesAtStartEnd",
            case  25: bVal = rSwFlags.bAFmtDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelSpacesBetween",
            case  26: bVal = rParent.bAutoFmtByInput; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/Enable",
            case  27: bVal = rSwFlags.bChgToEnEmDash; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeDash",
            case  28: bVal = rSwFlags.bSetNumRule; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ApplyNumbering/Enable",
            case  29: bVal = rSwFlags.bSetBorder; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeToBorders",
            case  30: bVal = rSwFlags.bCreateTable; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeToTable",
            case  31: bVal = rSwFlags.bReplaceStyles; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ReplaceStyle",
            case  32: bVal = rSwFlags.bAFmtByInpDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/DelSpacesAtStartEnd",
            case  33: bVal = rSwFlags.bAFmtByInpDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/DelSpacesBetween",
            case  34: bVal = rSwFlags.bAutoCompleteWords; pValues[nProp].setValue(&bVal, rType); break; // "Completion/Enable",
            case  35:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltWordLen;
            break; // "Completion/MinWordLen",
            case  36:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltListLen;
            break; // "Completion/MaxListLen",
            case  37: bVal = rSwFlags.bAutoCmpltCollectWords; pValues[nProp].setValue(&bVal, rType); break; // "Completion/CollectWords",
            case  38: bVal = rSwFlags.bAutoCmpltEndless; pValues[nProp].setValue(&bVal, rType); break; // "Completion/EndlessList",
            case  39: bVal = rSwFlags.bAutoCmpltAppendBlanc; pValues[nProp].setValue(&bVal, rType); break; // "Completion/AppendBlank",
            case  40: bVal = rSwFlags.bAutoCmpltShowAsTip; pValues[nProp].setValue(&bVal, rType); break; // "Completion/ShowAsTip",
            case  41:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nAutoCmpltExpandKey;
            break; // "Completion/AcceptKey"
            case 42 :bVal = rSwFlags.bAutoCmpltKeepList; pValues[nProp].setValue(&bVal, rType); break;// "Completion/KeepList"
            case 43 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.cByInputBullet;
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",
            case 44 :
                pValues[nProp] <<= OUString(rSwFlags.aByInputBulletFont.GetName());
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",
            case 45 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetFamily();
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",
            case 46 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetCharSet();
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset",
            case 47 :
                pValues[nProp] <<= (sal_Int32)rSwFlags.aByInputBulletFont.GetPitch();
            break;// "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch",
        }
    }
    PutProperties(aNames, aValues);
}
/*-- 12.10.00 11:51:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvxSwAutoCorrCfg::Notify( const Sequence<OUString>& /* aPropertyNames */ )
{
    Load(sal_False);
}

SvxAutoCorrCfg* SvxAutoCorrCfg::Get()
{
    if( !pAutoCorrCfg )
        pAutoCorrCfg = new SvxAutoCorrCfg;
    return pAutoCorrCfg;
}
