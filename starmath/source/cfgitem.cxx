/*************************************************************************
 *
 *  $RCSfile: cfgitem.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: tl $ $Date: 2001-07-17 08:28:20 $
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

#pragma hdrstop

#include "cfgitem.hxx"

#include "starmath.hrc"
#include "smdll.hxx"
#include "format.hxx"

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define A2OU(x)        rtl::OUString::createFromAscii( x )

static const char* aRootName = "Office.Math";

#define SYMBOL_LIST         "SymbolList"
#define FONT_FORMAT_LIST    "FontFormatList"

SV_IMPL_OBJARR( SmFntFmtListEntryArr, SmFntFmtListEntry );

/////////////////////////////////////////////////////////////////


static Sequence< OUString > lcl_GetFontPropertyNames()
{
    static const char * aPropNames[] =
    {
        "Name",
        "CharSet",
        "Family",
        "Pitch",
        "Weight",
        "Italic",
        0
    };

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( 6 );
    OUString *pNames = aNames.getArray();
    for( INT32 i = 0; *ppPropName;  ++i, ++ppPropName )
    {
        pNames[i] = A2OU( *ppPropName );
    }
    //aNames.realloc( i );
    return aNames;
}

/////////////////////////////////////////////////////////////////


static Sequence< OUString > lcl_GetSymbolPropertyNames()
{
    static const char * aPropNames[] =
    {
        "Char",
        "Set",
        "Predefined",
        "FontFormatId",
        0
    };

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( 4 );
    OUString *pNames = aNames.getArray();
    for( INT32 i = 0; *ppPropName;  ++i, ++ppPropName )
    {
        pNames[i] = A2OU( *ppPropName );
    }
    //aNames.realloc( i );
    return aNames;
}

/////////////////////////////////////////////////////////////////

static const char * aMathPropNames[] =
{
    "Print/Title",
    "Print/FormulaText",
    "Print/Frame",
    "Print/Size",
    "Print/ZoomFactor",
    "Misc/NoSymbolsWarning",
    "Misc/IgnoreSpacesRight",
    "View/ToolboxVisible",
    "View/AutoRedraw",
    "View/FormulaCursor"
};


//! Beware of order according to *_BEGIN *_END defines in format.hxx !
//! see respective load/save routines here
static const char * aFormatPropNames[] =
{
    "StandardFormat/Textmode",
    "StandardFormat/ScaleNormalBracket",
    "StandardFormat/HorizontalAlignment",
    "StandardFormat/BaseSize",
    "StandardFormat/TextSize",
    "StandardFormat/IndexSize",
    "StandardFormat/FunctionSize",
    "StandardFormat/OperatorSize",
    "StandardFormat/LimitsSize",
    "StandardFormat/Distance/Horizontal",
    "StandardFormat/Distance/Vertical",
    "StandardFormat/Distance/Root",
    "StandardFormat/Distance/SuperScript",
    "StandardFormat/Distance/SubScript",
    "StandardFormat/Distance/Numerator",
    "StandardFormat/Distance/Denominator",
    "StandardFormat/Distance/Fraction",
    "StandardFormat/Distance/StrokeWidth",
    "StandardFormat/Distance/UpperLimit",
    "StandardFormat/Distance/LowerLimit",
    "StandardFormat/Distance/BracketSize",
    "StandardFormat/Distance/BracketSpace",
    "StandardFormat/Distance/MatrixRow",
    "StandardFormat/Distance/MatrixColumn",
    "StandardFormat/Distance/OrnamentSize",
    "StandardFormat/Distance/OrnamentSpace",
    "StandardFormat/Distance/OperatorSize",
    "StandardFormat/Distance/OperatorSpace",
    "StandardFormat/Distance/LeftSpace",
    "StandardFormat/Distance/RightSpace",
    "StandardFormat/Distance/TopSpace",
    "StandardFormat/Distance/BottomSpace",
    "StandardFormat/Distance/NormalBracketSize",
    "StandardFormat/VariableFont",
    "StandardFormat/FunctionFont",
    "StandardFormat/NumberFont",
    "StandardFormat/TextFont",
    "StandardFormat/SerifFont",
    "StandardFormat/SansFont",
    "StandardFormat/FixedFont"
};


static Sequence< OUString > lcl_GetPropertyNames(
        const char * aPropNames[], USHORT nCount )
{

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( nCount );
    OUString *pNames = aNames.getArray();
    for (INT32 i = 0;  i < nCount;  ++i, ++ppPropName)
    {
        pNames[i] = A2OU( *ppPropName );
    }
    //aNames.realloc( i );
    return aNames;
}

/////////////////////////////////////////////////////////////////

Sequence< OUString > SmMathConfigItem::GetFormatPropertyNames()
{
    USHORT nCnt = sizeof(aFormatPropNames) / sizeof(aFormatPropNames[0]);
    return lcl_GetPropertyNames( aFormatPropNames, nCnt );
}


Sequence< OUString > SmMathConfigItem::GetOtherPropertyNames()
{
    USHORT nCnt = sizeof(aMathPropNames) / sizeof(aMathPropNames[0]);
    return lcl_GetPropertyNames( aMathPropNames, nCnt );
}

/////////////////////////////////////////////////////////////////

struct SmCfgOther
{
    SmPrintSize     ePrintSize;
    USHORT          nPrintZoomFactor;
    BOOL            bPrintTitle;
    BOOL            bPrintFormulaText;
    BOOL            bPrintFrame;
    BOOL            bIgnoreSpacesRight;
    BOOL            bToolboxVisible;
    BOOL            bAutoRedraw;
    BOOL            bFormulaCursor;
    BOOL            bNoSymbolsWarning;

    SmCfgOther();
};


SmCfgOther::SmCfgOther()
{
    ePrintSize          = PRINT_SIZE_NORMAL;
    nPrintZoomFactor    = 100;
    bPrintTitle         = bPrintFormulaText   =
    bPrintFrame         = bIgnoreSpacesRight  =
    bToolboxVisible     = bAutoRedraw         =
    bFormulaCursor      = bNoSymbolsWarning   = TRUE;
}

/////////////////////////////////////////////////////////////////


SmFontFormat::SmFontFormat()
{
    aName.AssignAscii( FONTNAME_MATH );
    nCharSet    = RTL_TEXTENCODING_UNICODE;
    nFamily     = FAMILY_DONTKNOW;
    nPitch      = PITCH_DONTKNOW;
    nWeight     = WEIGHT_DONTKNOW;
    nItalic     = ITALIC_NONE;
}


SmFontFormat::SmFontFormat( const Font &rFont )
{
    aName       = rFont.GetName();
    nCharSet    = (INT16) rFont.GetCharSet();
    nFamily     = (INT16) rFont.GetFamily();
    nPitch      = (INT16) rFont.GetPitch();
    nWeight     = (INT16) rFont.GetWeight();
    nItalic     = (INT16) rFont.GetItalic();
}


const Font SmFontFormat::GetFont() const
{
    Font aRes;
    aRes.SetName( aName );
    aRes.SetCharSet( (rtl_TextEncoding) nCharSet );
    aRes.SetFamily( (FontFamily) nFamily );
    aRes.SetPitch( (FontPitch) nPitch );
    aRes.SetWeight( (FontWeight) nWeight );
    aRes.SetItalic( (FontItalic) nItalic );
    return aRes;
}


BOOL SmFontFormat::operator == ( const SmFontFormat &rFntFmt ) const
{
    return  aName    == rFntFmt.aName       &&
            nCharSet == rFntFmt.nCharSet    &&
            nFamily  == rFntFmt.nFamily     &&
            nPitch   == rFntFmt.nPitch      &&
            nWeight  == rFntFmt.nWeight     &&
            nItalic  == rFntFmt.nItalic;
}


/////////////////////////////////////////////////////////////////

SmFntFmtListEntry::SmFntFmtListEntry( const String &rId, const SmFontFormat &rFntFmt ) :
    aId     (rId),
    aFntFmt (rFntFmt)
{
}


SmFontFormatList::SmFontFormatList()
{
    bModified = FALSE;
}


void SmFontFormatList::Clear()
{
    USHORT nCnt = aEntries.Count();
    if (nCnt)
    {
        aEntries.Remove( 0, nCnt );
        SetModified( TRUE );
    }
}


void SmFontFormatList::AddFontFormat( const String &rFntFmtId,
        const SmFontFormat &rFntFmt )
{
    const SmFontFormat *pFntFmt = GetFontFormat( rFntFmtId );
    DBG_ASSERT( !pFntFmt, "FontFormatId already exists" );
    if (!pFntFmt)
    {
        SmFntFmtListEntry aEntry( rFntFmtId, rFntFmt );
        aEntries.Insert( aEntry, aEntries.Count() );
        SetModified( TRUE );
    }
}


void SmFontFormatList::RemoveFontFormat( const String &rFntFmtId )
{
    USHORT nPos = 0xFFFF;

    // search for entry
    USHORT nCnt = aEntries.Count();
    for (USHORT i = 0;  i < nCnt  &&  nPos == 0xFFFF;  ++i)
    {
        if (aEntries[i].aId == rFntFmtId)
            nPos = i;
    }

    // remove entry if found
    if (nPos != 0xFFFF)
    {
        aEntries.Remove( nPos );
        SetModified( TRUE );
    }
}


const SmFontFormat * SmFontFormatList::GetFontFormat( const String &rFntFmtId ) const
{
    SmFontFormat *pRes = 0;

    USHORT nCnt = aEntries.Count();
    USHORT i;
    for (i = 0;  i < nCnt  &&  !pRes;  ++i)
    {
        if (aEntries[i].aId == rFntFmtId)
            pRes = &aEntries[i].aFntFmt;
    }

    return pRes;
}



const SmFontFormat * SmFontFormatList::GetFontFormat( USHORT nPos ) const
{
    SmFontFormat *pRes = 0;
    if (nPos < aEntries.Count())
        pRes = &aEntries[ nPos ].aFntFmt;
    return pRes;
}


const String SmFontFormatList::GetFontFormatId( const SmFontFormat &rFntFmt ) const
{
    String aRes;

    USHORT nCnt = aEntries.Count();
    USHORT i;
    for (i = 0;  i < nCnt  &&  0 == aRes.Len();  ++i)
    {
        if (aEntries[i].aFntFmt == rFntFmt)
            aRes = aEntries[i].aId;
    }

    return aRes;
}


const String SmFontFormatList::GetFontFormatId( const SmFontFormat &rFntFmt, BOOL bAdd )
{
    String aRes( GetFontFormatId( rFntFmt) );
    if (0 == aRes.Len()  &&  bAdd)
    {
        aRes = GetNewFontFormatId();
        AddFontFormat( aRes, rFntFmt );
    }
    return aRes;
}


const String SmFontFormatList::GetFontFormatId( USHORT nPos ) const
{
    String aRes;
    if (nPos < aEntries.Count())
        aRes = aEntries[nPos].aId;
    return aRes;
}


const String SmFontFormatList::GetNewFontFormatId() const
{
    // returns first unused FormatId

    String aRes;

    String aPrefix( RTL_CONSTASCII_STRINGPARAM( "Id" ) );
    INT32 nCnt = GetCount();
    for (INT32 i = 1;  i <= nCnt + 1  &&  0 == aRes.Len();  ++i)
    {
        String aTmpId( aPrefix );
        aTmpId += String::CreateFromInt32( i );
        if (!GetFontFormat( aTmpId ))
            aRes = aTmpId;
    }
    DBG_ASSERT( 0 != aRes.Len(), "failed to create new FontFormatId" );

    return aRes;
}

/////////////////////////////////////////////////////////////////

SmMathConfig::SmMathConfig()
{
    pFormat         = 0;
    pOther          = 0;
    pFontFormatList = 0;
    pSymbols        = 0;
    nSymbolCount    = 0;

    bIsOtherModified = bIsFormatModified = FALSE;

    aSaveTimer.SetTimeout( 3000 );
    aSaveTimer.SetTimeoutHdl( LINK( this, SmMathConfig, TimeOut ) );
}


SmMathConfig::~SmMathConfig()
{
    Save();
    delete pFormat;
    delete pOther;
    delete pFontFormatList;
    delete [] pSymbols;
}


void SmMathConfig::SetOtherModified( BOOL bVal )
{
    if ((bIsOtherModified = bVal))
        aSaveTimer.Start();
}


void SmMathConfig::SetFormatModified( BOOL bVal )
{
    if ((bIsFormatModified = bVal))
        aSaveTimer.Start();
}


SmSym SmMathConfig::ReadSymbol( SmMathConfigItem &rCfg,
                        const rtl::OUString &rSymbolName,
                        const rtl::OUString &rBaseNode ) const
{
    SmSym aRes;

    Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
    INT32 nProps = aNames.getLength();

    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    OUString *pName = aNames.getArray();
    for (INT32 i = 0;  i < nProps;  ++i)
    {
        OUString &rName = pName[i];
        OUString aTmp( rName );
        rName = rBaseNode;
        rName += aDelim;
        rName += rSymbolName;
        rName += aDelim;
        rName += aTmp;
    }

    const Sequence< Any > aValues = rCfg.GetProperties( aNames );

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any * pValue = aValues.getConstArray();
        Font        aFont;
        sal_Unicode cChar;
        String      aSet;
        BOOL        bPredefined;

        OUString    aTmpStr;
        INT32       nTmp32 = 0;
        INT16       nTmp16 = 0;
        BOOL        bTmp = FALSE;

        BOOL bOK = TRUE;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp32))
            cChar = (sal_Unicode) nTmp32;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
            aSet = aTmpStr;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= bTmp))
            bPredefined = bTmp;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
        {
            const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
            DBG_ASSERT( pFntFmt, "unknown FontFormat" );
            if (pFntFmt)
                aFont = pFntFmt->GetFont();
        }
        else
            bOK = FALSE;
        ++pValue;

        if (bOK)
        {
            String aUiName( rSymbolName );
            String aUiSetName( aSet );
            if (bPredefined)
            {
                String aTmp;
                aTmp = GetUiSymbolName( rSymbolName );
                DBG_ASSERT( aTmp.Len(), "localized symbol-name not found" );
                if (aTmp.Len())
                    aUiName = aTmp;
                aTmp = GetUiSymbolSetName( aSet );
                DBG_ASSERT( aTmp.Len(), "localized symbolset-name not found" );
                if (aTmp.Len())
                    aUiSetName = aTmp;
            }

            aRes = SmSym( aUiName, aFont, cChar, aUiSetName, bPredefined );
            if (aUiName != String(rSymbolName))
                aRes.SetExportName( rSymbolName );
        }
        else
        {
            DBG_ERROR( "symbol read error" );
        }
    }

    return aRes;
}


void SmMathConfig::LoadSymbols()
{
    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));

    Sequence< OUString > aNodes( aCfg.GetNodeNames( A2OU( SYMBOL_LIST ) ) );
    const OUString *pNode = aNodes.getConstArray();
    INT32 nNodes = aNodes.getLength();

    if (pSymbols)
        delete [] pSymbols;
    pSymbols = nNodes ? new SmSym[ nNodes ] : 0;
    nSymbolCount = (USHORT) nNodes;

    for (INT32 i = 0;  i < nNodes;  ++i)
    {
        pSymbols[i] = ReadSymbol( aCfg, pNode[i], A2OU( SYMBOL_LIST ) );
    }
}


void SmMathConfig::Save()
{
    SaveOther();
    SaveFormat();
    SaveFontFormatList();
}


USHORT SmMathConfig::GetSymbolCount() const
{
    if (!pSymbols)
        ((SmMathConfig *) this)->LoadSymbols();
    return nSymbolCount;
}


const SmSym * SmMathConfig::GetSymbol( USHORT nIndex ) const
{
    SmSym *pRes = 0;
    if (!pSymbols)
        ((SmMathConfig *) this)->LoadSymbols();
    if (nIndex < nSymbolCount)
        pRes = &pSymbols[ nIndex ];
    return pRes;
}


void SmMathConfig::ReplaceSymbols( const SmSym *pNewSymbols[], USHORT nCount )
{
    // clear old symbols and have the new ones loaded on demand
    if (pSymbols)
    {
        delete [] pSymbols;
        pSymbols = 0;
    }

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ),
                           CONFIG_MODE_DELAYED_UPDATE );

    Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
    const OUString *pNames = aNames.getConstArray();
    INT32 nSymbolProps = aNames.getLength();

    Sequence< PropertyValue > aValues( nCount * nSymbolProps );
    PropertyValue *pValues = aValues.getArray();

    PropertyValue *pVal = pValues;
    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    for (USHORT i = 0;  i < nCount;  ++i)
    {
        const SmSym &rSymbol = *pNewSymbols[i];
        const Font  &rFont = rSymbol.GetFace();
        OUString  aNodeNameDelim( A2OU( SYMBOL_LIST ) );
        aNodeNameDelim += aDelim;
        aNodeNameDelim += rSymbol.GetExportName();
        aNodeNameDelim += aDelim;

        const OUString *pName = pNames;

        // Char
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (INT32) rSymbol.GetCharacter();
        pVal++;
        // Set
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        OUString aTmp( rSymbol.GetSetName() );
        if (rSymbol.IsPredefined())
            aTmp = GetExportSymbolSetName( aTmp );
        pVal->Value <<= aTmp;
        pVal++;
        // Predefined
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (BOOL) rSymbol.IsPredefined();
        pVal++;
        // FontFormatId
        SmFontFormat aFntFmt( rSymbol.GetFace() );
        String aFntFmtId( GetFontFormatList().GetFontFormatId( aFntFmt, TRUE ) );
        DBG_ASSERT( aFntFmtId.Len(), "FontFormatId not found" );
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= OUString( aFntFmtId );
        pVal++;
    }
    DBG_ASSERT( pVal - pValues == nCount * nSymbolProps, "properties missing" );
    aCfg.ReplaceSetProperties( A2OU( SYMBOL_LIST ) , aValues );

    StripFontFormatList( pNewSymbols, nCount );
    SaveFontFormatList();
}


SmFontFormatList & SmMathConfig::GetFontFormatList()
{
    if (!pFontFormatList)
    {
        LoadFontFormatList();
    }
    return *pFontFormatList;
}


void SmMathConfig::LoadFontFormatList()
{
    if (!pFontFormatList)
        pFontFormatList = new SmFontFormatList;
    else
        pFontFormatList->Clear();

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ) );

    Sequence< OUString > aNodes( aCfg.GetNodeNames( A2OU( FONT_FORMAT_LIST ) ) );
    const OUString *pNode = aNodes.getConstArray();
    INT32 nNodes = aNodes.getLength();

    for (INT32 i = 0;  i < nNodes;  ++i)
    {
        SmFontFormat aFntFmt( ReadFontFormat( aCfg, pNode[i], A2OU( FONT_FORMAT_LIST ) ) );
        if (!pFontFormatList->GetFontFormat( pNode[i] ))
        {
            DBG_ASSERT( 0 == pFontFormatList->GetFontFormat( pNode[i] ),
                    "FontFormat ID already exists" );
            pFontFormatList->AddFontFormat( pNode[i], aFntFmt );
        }
    }
    pFontFormatList->SetModified( FALSE );
}


SmFontFormat SmMathConfig::ReadFontFormat( SmMathConfigItem &rCfg,
        const OUString &rSymbolName, const OUString &rBaseNode ) const
{
    SmFontFormat aRes;

    Sequence< OUString > aNames = lcl_GetFontPropertyNames();
    INT32 nProps = aNames.getLength();

    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    OUString *pName = aNames.getArray();
    for (INT32 i = 0;  i < nProps;  ++i)
    {
        OUString &rName = pName[i];
        OUString aTmp( rName );
        rName = rBaseNode;
        rName += aDelim;
        rName += rSymbolName;
        rName += aDelim;
        rName += aTmp;
    }

    const Sequence< Any > aValues = rCfg.GetProperties( aNames );

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any * pValue = aValues.getConstArray();

        OUString    aTmpStr;
        INT16       nTmp16 = 0;

        BOOL bOK = TRUE;
        if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
            aRes.aName = aTmpStr;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aRes.nCharSet = nTmp16;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aRes.nFamily = nTmp16;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aRes.nPitch = nTmp16;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aRes.nWeight = nTmp16;
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aRes.nItalic = nTmp16;
        else
            bOK = FALSE;
        ++pValue;

        DBG_ASSERT( bOK, "read FontFormat failed" );
    }

    return aRes;
}


void SmMathConfig::SaveFontFormatList()
{
    SmFontFormatList &rFntFmtList = GetFontFormatList();

    if (!rFntFmtList.IsModified())
        return;

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ) );

    Sequence< OUString > aNames = lcl_GetFontPropertyNames();
    const OUString *pNames = aNames.getConstArray();
    INT32 nSymbolProps = aNames.getLength();

    USHORT nCount = rFntFmtList.GetCount();

    Sequence< PropertyValue > aValues( nCount * nSymbolProps );
    PropertyValue *pValues = aValues.getArray();

    PropertyValue *pVal = pValues;
    OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
    for (USHORT i = 0;  i < nCount;  ++i)
    {
        String aFntFmtId( rFntFmtList.GetFontFormatId( i ) );
        const SmFontFormat aFntFmt( *rFntFmtList.GetFontFormat( aFntFmtId ) );

        OUString  aNodeNameDelim( A2OU( FONT_FORMAT_LIST ) );
        aNodeNameDelim += aDelim;
        aNodeNameDelim += aFntFmtId;
        aNodeNameDelim += aDelim;

        const OUString *pName = pNames;

        // Name
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= OUString( aFntFmt.aName );
        pVal++;
        // CharSet
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (INT16) aFntFmt.nCharSet;
        pVal++;
        // Family
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (INT16) aFntFmt.nFamily;
        pVal++;
        // Pitch
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (INT16) aFntFmt.nPitch;
        pVal++;
        // Weight
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (INT16) aFntFmt.nWeight;
        pVal++;
        // Italic
        pVal->Name  = aNodeNameDelim;
        pVal->Name += *pName++;
        pVal->Value <<= (INT16) aFntFmt.nItalic;
        pVal++;
    }
    DBG_ASSERT( pVal - pValues == nCount * nSymbolProps, "properties missing" );
    aCfg.ReplaceSetProperties( A2OU( FONT_FORMAT_LIST ) , aValues );

    rFntFmtList.SetModified( FALSE );
}


void SmMathConfig::StripFontFormatList( const SmSym *pUsedSymbols[], USHORT nCount )
{
    USHORT i;

    // build list of used font-formats
    //!! font-format IDs may be different !!
    SmFontFormatList aUsedList;
    for (i = 0;  i < nCount;  ++i)
    {
        DBG_ASSERT( pUsedSymbols[i], "null pointer for symbol" );
        aUsedList.GetFontFormatId( SmFontFormat( pUsedSymbols[i]->GetFace() ) , TRUE );
    }
    const SmFormat & rStdFmt = GetStandardFormat();
    for (i = FNT_BEGIN;  i <= FNT_END;  ++i)
    {
        aUsedList.GetFontFormatId( SmFontFormat( rStdFmt.GetFont( i ) ) , TRUE );
    }

    // remove unused font-formats from list
    SmFontFormatList &rFntFmtList = GetFontFormatList();
    USHORT nCnt = rFntFmtList.GetCount();
    SmFontFormat *pFormat = new SmFontFormat[ nCnt ];
    String       *pId     = new String      [ nCnt ];
    INT32 k;
    for (k = 0;  k < nCnt;  ++k)
    {
        pFormat[k] = *rFntFmtList.GetFontFormat( (USHORT) k );
        pId[k]     = rFntFmtList.GetFontFormatId( (USHORT) k );
    }
    for (k = 0;  k < nCnt;  ++k)
    {
        if (0 == aUsedList.GetFontFormatId( pFormat[k] ))
        {
            rFntFmtList.RemoveFontFormat( pId[k] );
        }
    }
    delete [] pId;
    delete [] pFormat;
}


void SmMathConfig::LoadOther()
{
    if (!pOther)
        pOther = new SmCfgOther;

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));

    Sequence< OUString > aNames( aCfg.GetOtherPropertyNames() );
    INT32 nProps = aNames.getLength();

    Sequence< Any > aValues( aCfg.GetProperties( aNames ) );
    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any *pValues = aValues.getConstArray();
        const Any *pVal = pValues;

        INT16   nTmp16;
        BOOL    bTmp;

        // Print/Title
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bPrintTitle = bTmp;
        ++pVal;
        // Print/FormulaText
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bPrintFormulaText = bTmp;
        ++pVal;
        // Print/Frame
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bPrintFrame = bTmp;
        ++pVal;
        // Print/Size
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pOther->ePrintSize = (SmPrintSize) nTmp16;
        ++pVal;
        // Print/ZoomFactor
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pOther->nPrintZoomFactor = nTmp16;
        ++pVal;
        // Misc/NoSymbolsWarning
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bNoSymbolsWarning = bTmp;
        ++pVal;
        // Misc/IgnoreSpacesRight
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bIgnoreSpacesRight = bTmp;
        ++pVal;
        // View/ToolboxVisible
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bToolboxVisible = bTmp;
        ++pVal;
        // View/AutoRedraw
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bAutoRedraw = bTmp;
        ++pVal;
        // View/FormulaCursor
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bFormulaCursor = bTmp;
        ++pVal;

        DBG_ASSERT( pVal - pValues == nProps, "property mismatch" );
        SetOtherModified( FALSE );
    }
}


void SmMathConfig::SaveOther()
{
    if (!pOther || !IsOtherModified())
        return;

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));

    Sequence< OUString > aNames( aCfg.GetOtherPropertyNames() );
    const OUString *pNames = aNames.getConstArray();
    const OUString *pName = pNames;
    INT32 nProps = aNames.getLength();

    Sequence< PropertyValue > aValues( nProps );
    PropertyValue *pValues = aValues.getArray();
    PropertyValue *pVal = pValues;

    // Print/Title
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bPrintTitle;
    pVal++;
    // Print/FormulaText
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bPrintFormulaText;
    pVal++;
    // Print/Frame
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bPrintFrame;
    pVal++;
    // Print/Size
    pVal->Name  = *pNames++;
    pVal->Value <<= (INT16) pOther->ePrintSize;
    pVal++;
    // Print/ZoomFactor
    pVal->Name  = *pNames++;
    pVal->Value <<= (INT16) pOther->nPrintZoomFactor;
    pVal++;
    // Misc/NoSymbolsWarning
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bNoSymbolsWarning;
    pVal++;
    // Misc/IgnoreSpacesRight
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bIgnoreSpacesRight;
    pVal++;
    // View/ToolboxVisible
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bToolboxVisible;
    pVal++;
    // View/AutoRedraw
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bAutoRedraw;
    pVal++;
    // View/FormulaCursor
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pOther->bFormulaCursor;
    pVal++;
    DBG_ASSERT( pVal - pValues == nProps, "property mismatch" );

    SetOtherModified( FALSE );
}

void SmMathConfig::LoadFormat()
{
    if (!pFormat)
        pFormat = new SmFormat;

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));

    Sequence< OUString > aNames( aCfg.GetFormatPropertyNames() );
    INT32 nProps = aNames.getLength();

    Sequence< Any > aValues( aCfg.GetProperties( aNames ) );
    if (nProps  &&  aValues.getLength() == nProps)
    {
        const Any *pValues = aValues.getConstArray();
        const Any *pVal = pValues;

        OUString    aTmpStr;
        INT16       nTmp16;
        BOOL        bTmp;

        // StandardFormat/Textmode
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pFormat->SetTextmode( bTmp );
        ++pVal;
        // StandardFormat/ScaleNormalBracket
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pFormat->SetScaleNormalBrackets( bTmp );
        ++pVal;
        // StandardFormat/HorizontalAlignment
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetHorAlign( (SmHorAlign) nTmp16 );
        ++pVal;
        // StandardFormat/BaseSize
        if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
            pFormat->SetBaseSize( Size(0, SmPtsTo100th_mm( nTmp16 )) );
        ++pVal;

        USHORT i;
        for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
        {
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                pFormat->SetRelSize( i, nTmp16 );
            ++pVal;
        }

        for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
        {
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                pFormat->SetDistance( i, nTmp16 );
            ++pVal;
        }

        for (i = FNT_BEGIN;  i < FNT_END;  ++i)
        {
            Font aFnt;

            if (pVal->hasValue()  &&  (*pVal >>= aTmpStr))
            {
                const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
                DBG_ASSERT( pFntFmt, "unknown FontFormat" );
                if (pFntFmt)
                    aFnt = pFntFmt->GetFont();
            }
            ++pVal;

            aFnt.SetSize( pFormat->GetBaseSize() );
            pFormat->SetFont( i, aFnt );
        }

        DBG_ASSERT( pVal - pValues == nProps, "property mismatch" );
        SetFormatModified( FALSE );
    }
}


void SmMathConfig::SaveFormat()
{
    if (!pFormat || !IsFormatModified())
        return;

    SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));

    Sequence< OUString > aNames( aCfg.GetFormatPropertyNames() );
    const OUString *pNames = aNames.getConstArray();
    const OUString *pName = pNames;
    INT32 nProps = aNames.getLength();

    Sequence< PropertyValue > aValues( nProps );
    PropertyValue *pValues = aValues.getArray();
    PropertyValue *pVal = pValues;

    // StandardFormat/Textmode
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pFormat->IsTextmode();
    pVal++;
    // StandardFormat/ScaleNormalBracket
    pVal->Name  = *pNames++;
    pVal->Value <<= (BOOL) pFormat->IsScaleNormalBrackets();
    pVal++;
    // StandardFormat/HorizontalAlignment
    pVal->Name  = *pNames++;
    pVal->Value <<= (INT16) pFormat->GetHorAlign();
    pVal++;
    // StandardFormat/BaseSize
    pVal->Name  = *pNames++;
    pVal->Value <<= (INT16) SmRoundFraction( Sm100th_mmToPts(
                                    pFormat->GetBaseSize().Height() ) );
    pVal++;

    USHORT i;
    for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
    {
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) pFormat->GetRelSize( i );
        ++pVal;
    }

    for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
    {
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) pFormat->GetDistance( i );
        ++pVal;
    }

    for (i = FNT_BEGIN;  i < FNT_END;  ++i)
    {
        SmFontFormat aFntFmt( pFormat->GetFont( i ) );
        String aFntFmtId( GetFontFormatList().GetFontFormatId( aFntFmt ) );
        DBG_ASSERT( aFntFmtId.Len(), "FontFormatId not found" );

        pVal->Name  = *pNames++;
        pVal->Value <<= OUString( aFntFmtId );
        ++pVal;
    }
    DBG_ASSERT( pVal - pValues == nProps, "property mismatch" );

    SetFormatModified( FALSE );
}


const SmFormat & SmMathConfig::GetStandardFormat() const
{
    if (!pFormat)
        ((SmMathConfig *) this)->LoadFormat();
    return *pFormat;
}


void SmMathConfig::SetStandardFormat( const SmFormat &rFormat )
{
    if (!pFormat)
        LoadFormat();
    if (rFormat != *pFormat)
    {
        *pFormat = rFormat;
        SetFormatModified( TRUE );
    }
}


SmPrintSize SmMathConfig::GetPrintSize() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->ePrintSize;
}


void SmMathConfig::SetPrintSize( SmPrintSize eSize )
{
    if (!pOther)
        LoadOther();
    if (eSize != pOther->ePrintSize)
    {
        pOther->ePrintSize = eSize;
        SetOtherModified( TRUE );
    }
}


USHORT SmMathConfig::GetPrintZoomFactor() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->nPrintZoomFactor;
}


void SmMathConfig::SetPrintZoomFactor( USHORT nVal )
{
    if (!pOther)
        LoadOther();
    if (nVal != pOther->nPrintZoomFactor)
    {
        pOther->nPrintZoomFactor = nVal;
        SetOtherModified( TRUE );
    }
}


void SmMathConfig::SetOtherIfNotEqual( BOOL &rbItem, BOOL bNewVal )
{
    if (bNewVal != rbItem)
    {
        rbItem = bNewVal;
        SetOtherModified( TRUE );
    }
}


BOOL SmMathConfig::IsPrintTitle() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bPrintTitle;
}


void SmMathConfig::SetPrintTitle( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintTitle, bVal );
}


BOOL SmMathConfig::IsPrintFormulaText() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bPrintFormulaText;
}


void SmMathConfig::SetPrintFormulaText( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintFormulaText, bVal );
}


BOOL SmMathConfig::IsPrintFrame() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bPrintFrame;
}


void SmMathConfig::SetPrintFrame( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bPrintFrame, bVal );
}


BOOL SmMathConfig::IsIgnoreSpacesRight() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bIgnoreSpacesRight;
}


void SmMathConfig::SetIgnoreSpacesRight( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bIgnoreSpacesRight, bVal );
}


BOOL SmMathConfig::IsToolboxVisible() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bToolboxVisible;
}


void SmMathConfig::SetToolboxVisible( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bToolboxVisible, bVal );
}


BOOL SmMathConfig::IsAutoRedraw() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bAutoRedraw;
}


void SmMathConfig::SetAutoRedraw( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bAutoRedraw, bVal );
}


BOOL SmMathConfig::IsShowFormulaCursor() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bFormulaCursor;
}


void SmMathConfig::SetShowFormulaCursor( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bFormulaCursor, bVal );
}


BOOL SmMathConfig::IsNoSymbolsWarning() const
{
    if (!pOther)
        ((SmMathConfig *) this)->LoadOther();
    return pOther->bNoSymbolsWarning;
}


void SmMathConfig::SetNoSymbolsWarning( BOOL bVal )
{
    if (!pOther)
        LoadOther();
    SetOtherIfNotEqual( pOther->bNoSymbolsWarning, bVal );
}


IMPL_LINK( SmMathConfig, TimeOut, Timer *, p )
{
    Save();
    return 0;
}

/////////////////////////////////////////////////////////////////

