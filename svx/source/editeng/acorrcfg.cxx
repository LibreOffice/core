/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acorrcfg.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:08:09 $
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

#include "svxacorr.hxx"

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
    bAutoFmtByInput(TRUE)
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
        "Format/Option/UseReplacementTable",          // 4
        "Format/Option/TwoCapitalsAtStart",           // 5
        "Format/Option/CapitalAtStartSentence",       // 6
        "Format/Option/ChangeUnderlineWeight",        // 7
        "Format/Option/SetInetAttribute",             // 8
        "Format/Option/ChangeOrdinalNumber",          // 9
        "Format/Option/ChangeFraction",               //10
        "Format/Option/ChangeDash",                   //11
        "Format/Option/DelEmptyParagraphs",           //12
        "Format/Option/ReplaceUserStyle",             //13
        "Format/Option/ChangeToBullets/Enable",       //14
        "Format/Option/ChangeToBullets/SpecialCharacter/Char",        //15
        "Format/Option/ChangeToBullets/SpecialCharacter/Font",        //16
        "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",      //17
        "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",     //18
        "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",   //19
        "Format/Option/ReplaceQuote",                 //20
        "Format/Option/CombineParagraphs",            //21
        "Format/Option/CombineValue",                 //22
        "Format/Option/DelSpacesAtStartEnd",          //23
        "Format/Option/DelSpacesBetween",             //24
        "Format/ByInput/Enable",                      //25
        "Format/ByInput/ChangeDash",                  //26
        "Format/ByInput/ApplyNumbering/Enable",       //27
        "Format/ByInput/ChangeToBorders",             //28
        "Format/ByInput/ChangeToTable",               //29
        "Format/ByInput/ReplaceStyle",                //30
        "Format/ByInput/DelSpacesAtStartEnd",         //31
        "Format/ByInput/DelSpacesBetween",            //32
        "Completion/Enable",                          //33
        "Completion/MinWordLen",                      //34
        "Completion/MaxListLen",                      //35
        "Completion/CollectWords",                    //36
        "Completion/EndlessList",                     //37
        "Completion/AppendBlank",                     //38
        "Completion/ShowAsTip",                       //39
        "Completion/AcceptKey",                       //40
        "Completion/KeepList",                        //41
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Char",        //42
        "Format/ByInput/ApplyNumbering/SpecialCharacter/Font",        //43
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontFamily",  //44
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontCharset", //45
        "Format/ByInput/ApplyNumbering/SpecialCharacter/FontPitch",   //46
    };
    const int nCount = 47;
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
                    case   4: rSwFlags.bAutoCorrect = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/UseReplacementTable",
                    case   5: rSwFlags.bCptlSttSntnc = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/TwoCapitalsAtStart",
                    case   6: rSwFlags.bCptlSttWrd = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/CapitalAtStartSentence",
                    case   7: rSwFlags.bChgWeightUnderl = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeUnderlineWeight",
                    case   8: rSwFlags.bSetINetAttr = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/SetInetAttribute",
                    case   9: rSwFlags.bChgOrdinalNumber = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeOrdinalNumber",
                    case  10: rSwFlags.bChgFracionSymbol = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeFraction",
// it doesn't exist here - the common flags are used for that -> LM
//                  case  11: rSwFlags.bChgToEnEmDash = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeDash",
                    case  12: rSwFlags.bDelEmptyNode = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/DelEmptyParagraphs",
                    case  13: rSwFlags.bChgUserColl = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ReplaceUserStyle",
                    case  14: rSwFlags.bChgEnumNum = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ChangeToBullets/Enable",
                    case  15:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.cBullet =
                            sal::static_int_cast< sal_Unicode >(nVal);
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/Char",
                    case  16:
                    {
                        OUString sTemp; pValues[nProp] >>= sTemp;
                        rSwFlags.aBulletFont.SetName(sTemp);
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/Font",
                    case  17:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetFamily(FontFamily(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",
                    case  18:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetCharSet(CharSet(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",
                    case  19:
                    {
                                            sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.aBulletFont.SetPitch(FontPitch(nVal));
                    }
                    break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",
                    case  20: rSwFlags.bReplaceQuote = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/ReplaceQuote",
                    case  21: rSwFlags.bRightMargin = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/CombineParagraphs",
                    case  22:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nRightMargin =
                            sal::static_int_cast< BYTE >(nVal);
                    }
                    break; // "Format/Option/CombineValue",
                    case  23: rSwFlags.bAFmtDelSpacesAtSttEnd =  *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/DelSpacesAtStartEnd",
                    case  24: rSwFlags.bAFmtDelSpacesBetweenLines = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/Option/DelSpacesBetween",
                    case  25: rParent.bAutoFmtByInput = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/Enable",
                    case  26: rSwFlags.bChgToEnEmDash = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ChangeDash",
                    case  27: rSwFlags.bSetNumRule = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ApplyNumbering/Enable",
                    case  28: rSwFlags.bSetBorder = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ChangeToBorders",
                    case  29: rSwFlags.bCreateTable = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ChangeToTable",
                    case  30: rSwFlags.bReplaceStyles =  *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/ReplaceStyle",
                    case  31: rSwFlags.bAFmtByInpDelSpacesAtSttEnd =  *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/DelSpacesAtStartEnd",
                    case  32: rSwFlags.bAFmtByInpDelSpacesBetweenLines = *(sal_Bool*)pValues[nProp].getValue(); break; // "Format/ByInput/DelSpacesBetween",
                    case  33: rSwFlags.bAutoCompleteWords = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/Enable",
                    case  34:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltWordLen =
                            sal::static_int_cast< USHORT >(nVal);
                    }
                    break; // "Completion/MinWordLen",
                    case  35:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltListLen =
                            sal::static_int_cast< USHORT >(nVal);
                    }
                    break; // "Completion/MaxListLen",
                    case  36: rSwFlags.bAutoCmpltCollectWords = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/CollectWords",
                    case  37: rSwFlags.bAutoCmpltEndless = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/EndlessList",
                    case  38: rSwFlags.bAutoCmpltAppendBlanc = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/AppendBlank",
                    case  39: rSwFlags.bAutoCmpltShowAsTip = *(sal_Bool*)pValues[nProp].getValue(); break; // "Completion/ShowAsTip",
                    case  40:
                    {
                        sal_Int32 nVal = 0; pValues[nProp] >>= nVal;
                        rSwFlags.nAutoCmpltExpandKey =
                            sal::static_int_cast< USHORT >(nVal);
                    }
                    break; // "Completion/AcceptKey"
                    case 41 :rSwFlags.bAutoCmpltKeepList = *(sal_Bool*)pValues[nProp].getValue(); break;//"Completion/KeepList"
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
                        rSwFlags.aByInputBulletFont.SetCharSet(CharSet(nVal));
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
            case   4: bVal = rSwFlags.bAutoCorrect; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/UseReplacementTable",
            case   5: bVal = rSwFlags.bCptlSttSntnc; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/TwoCapitalsAtStart",
            case   6: bVal = rSwFlags.bCptlSttWrd; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/CapitalAtStartSentence",
            case   7: bVal = rSwFlags.bChgWeightUnderl; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeUnderlineWeight",
            case   8: bVal = rSwFlags.bSetINetAttr; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/SetInetAttribute",
            case   9: bVal = rSwFlags.bChgOrdinalNumber; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeOrdinalNumber",
            case  10: bVal = rSwFlags.bChgFracionSymbol; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeFraction",
// it doesn't exist here - the common flags are used for that -> LM
            case  11:
                bVal = sal_True;  pValues[nProp].setValue(&bVal, rType);
            break; // "Format/Option/ChangeDash",
            case  12: bVal = rSwFlags.bDelEmptyNode; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelEmptyParagraphs",
            case  13: bVal = rSwFlags.bChgUserColl; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ReplaceUserStyle",
            case  14: bVal = rSwFlags.bChgEnumNum; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ChangeToBullets/Enable",
            case  15:
                pValues[nProp] <<= (sal_Int32)rSwFlags.cBullet;
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/Char",
            case  16:
                pValues[nProp] <<= OUString(rSwFlags.aBulletFont.GetName());
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/Font",
            case  17:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetFamily();
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontFamily",
            case  18:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetCharSet();
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontCharset",
            case  19:
                pValues[nProp] <<= (sal_Int32)rSwFlags.aBulletFont.GetPitch();
            break; // "Format/Option/ChangeToBullets/SpecialCharacter/FontPitch",
            case  20: bVal = rSwFlags.bReplaceQuote; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/ReplaceQuote",
            case  21: bVal = rSwFlags.bRightMargin; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/CombineParagraphs",
            case  22:
                pValues[nProp] <<= (sal_Int32)rSwFlags.nRightMargin;
            break; // "Format/Option/CombineValue",
            case  23: bVal = rSwFlags.bAFmtDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelSpacesAtStartEnd",
            case  24: bVal = rSwFlags.bAFmtDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; // "Format/Option/DelSpacesBetween",
            case  25: bVal = rParent.bAutoFmtByInput; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/Enable",
            case  26: bVal = rSwFlags.bChgToEnEmDash; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeDash",
            case  27: bVal = rSwFlags.bSetNumRule; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ApplyNumbering/Enable",
            case  28: bVal = rSwFlags.bSetBorder; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeToBorders",
            case  29: bVal = rSwFlags.bCreateTable; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ChangeToTable",
            case  30: bVal = rSwFlags.bReplaceStyles; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/ReplaceStyle",
            case  31: bVal = rSwFlags.bAFmtByInpDelSpacesAtSttEnd; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/DelSpacesAtStartEnd",
            case  32: bVal = rSwFlags.bAFmtByInpDelSpacesBetweenLines; pValues[nProp].setValue(&bVal, rType); break; // "Format/ByInput/DelSpacesBetween",
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
                pValues[nProp] <<= OUString(rSwFlags.aByInputBulletFont.GetName());
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
