/*************************************************************************
 *
 *  $RCSfile: cfgitem.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tl $ $Date: 2001-05-02 16:56:31 $
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

#define SYMBOL_LIST     "SymbolList"

/////////////////////////////////////////////////////////////////


static Sequence< OUString > lcl_GetSymbolPropertyNames()
{
    static const char * aPropNames[] =
    {
        "Char",                     //  0
        "Set",                      //  1
        "Predefined",               //  2
        "Font/Name",                //  3
        "Font/CharSet",             //  4
        "Font/Family",              //  5
        "Font/Pitch",               //  6
        "Font/Weight",              //  7
        "Font/Italic",              //  8
        0
    };

    const char** ppPropName = aPropNames;

    Sequence< OUString > aNames( 9 );
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
    "StandardFormat/Distance/SubScript",
    "StandardFormat/Distance/SuperScript",
    "StandardFormat/Distance/LowerLimit",
    "StandardFormat/Distance/UpperLimit",
    "StandardFormat/Distance/OperatorSize",
    "StandardFormat/Distance/OperatorSpace",
    "StandardFormat/Distance/Numerator",
    "StandardFormat/Distance/Denominator",
    "StandardFormat/Distance/Fraction",
    "StandardFormat/Distance/StrokeWidth",
    "StandardFormat/Distance/BracketSize",
    "StandardFormat/Distance/BracketSpace",
    "StandardFormat/Distance/NormalBracketSize",
    "StandardFormat/Distance/MatrixRow",
    "StandardFormat/Distance/MatrixColumn",
    "StandardFormat/Distance/OrnamentSize",
    "StandardFormat/Distance/OrnamentSpace",
    "StandardFormat/Distance/LeftSpace",
    "StandardFormat/Distance/RightSpace",
    "StandardFormat/Distance/TopSpace",
    "StandardFormat/Distance/BottomSpace",
    "StandardFormat/VariableFont/Name",
    "StandardFormat/VariableFont/CharSet",
    "StandardFormat/VariableFont/Family",
    "StandardFormat/VariableFont/Pitch",
    "StandardFormat/VariableFont/Weight",
    "StandardFormat/VariableFont/Italic",
    "StandardFormat/FunctionFont/Name",
    "StandardFormat/FunctionFont/CharSet",
    "StandardFormat/FunctionFont/Family",
    "StandardFormat/FunctionFont/Pitch",
    "StandardFormat/FunctionFont/Weight",
    "StandardFormat/FunctionFont/Italic",
    "StandardFormat/NumberFont/Name",
    "StandardFormat/NumberFont/CharSet",
    "StandardFormat/NumberFont/Family",
    "StandardFormat/NumberFont/Pitch",
    "StandardFormat/NumberFont/Weight",
    "StandardFormat/NumberFont/Italic",
    "StandardFormat/TextFont/Name",
    "StandardFormat/TextFont/CharSet",
    "StandardFormat/TextFont/Family",
    "StandardFormat/TextFont/Pitch",
    "StandardFormat/TextFont/Weight",
    "StandardFormat/TextFont/Italic",
    "StandardFormat/SansFont/Name",
    "StandardFormat/SansFont/CharSet",
    "StandardFormat/SansFont/Family",
    "StandardFormat/SansFont/Pitch",
    "StandardFormat/SansFont/Weight",
    "StandardFormat/SansFont/Italic",
    "StandardFormat/SerifFont/Name",
    "StandardFormat/SerifFont/CharSet",
    "StandardFormat/SerifFont/Family",
    "StandardFormat/SerifFont/Pitch",
    "StandardFormat/SerifFont/Weight",
    "StandardFormat/SerifFont/Italic",
    "StandardFormat/FixedFont/Name",
    "StandardFormat/FixedFont/CharSet",
    "StandardFormat/FixedFont/Family",
    "StandardFormat/FixedFont/Pitch",
    "StandardFormat/FixedFont/Weight",
    "StandardFormat/FixedFont/Italic"
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

SmMathConfig::SmMathConfig()
{
    pFormat         = 0;
    pOther          = 0;
    pSymbols        = 0;
    nSymbolCount    = 0;

    bIsOtherModified = bIsFormatModified = FALSE;

    aSaveTimer.SetTimeout( 3000 );
    aSaveTimer.SetTimeoutHdl( LINK( this, SmMathConfig, TimeOut ) );

#ifdef DEBUG
    LoadSymbols();
    LoadOther();
    LoadFormat();
    SaveOther();
    SaveFormat();
#endif
}


SmMathConfig::~SmMathConfig()
{
    Save();
    delete pFormat;
    delete pOther;
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
            aFont.SetName( aTmpStr );
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aFont.SetCharSet( (rtl_TextEncoding) nTmp16 );
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aFont.SetFamily( (FontFamily) nTmp16 );
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aFont.SetPitch( (FontPitch) nTmp16 );
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aFont.SetWeight( (FontWeight) nTmp16 );
        else
            bOK = FALSE;
        ++pValue;
        if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
            aFont.SetItalic( (FontItalic) nTmp16 );
        else
            bOK = FALSE;
        ++pValue;

        if (bOK)
        {
            String aUiName;
            if (bPredefined)
            {
                aUiName = GetUiSymbolName( rSymbolName );
                DBG_ASSERT( aUiName.Len(), "localized symbol-name not found" );
            }
            if (!bPredefined  ||  0 == aUiName.Len())
                aUiName = rSymbolName;

            aRes = SmSym( aUiName, aFont, cChar, aSet, bPredefined );
            if (aUiName != String(rSymbolName))
                aRes.SetExportName( rSymbolName );
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

        // Char
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[0];
        pVal->Value <<= (INT32) rSymbol.GetCharacter();
        pVal++;
        // Set
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[1];
        pVal->Value <<= OUString( rSymbol.GetSetName() );
        pVal++;
        // Predefined
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[2];
        pVal->Value <<= (BOOL) rSymbol.IsPredefined();
        pVal++;
        // Font/Name
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[3];
        pVal->Value <<= OUString( rFont.GetName() );
        pVal++;
        // Font/Charset
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[4];
        pVal->Value <<= (INT16) rFont.GetCharSet();
        pVal++;
        // Font/Famlily
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[5];
        pVal->Value <<= (INT16) rFont.GetFamily();
        pVal++;
        // Font/Pitch
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[6];
        pVal->Value <<= (INT16) rFont.GetPitch();
        pVal++;
        // Font/Weight
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[7];
        pVal->Value <<= (INT16) rFont.GetWeight();
        pVal++;
        // Font/Italic
        pVal->Name  = aNodeNameDelim;
        pVal->Name += pNames[8];
        pVal->Value <<= (INT16) rFont.GetItalic();
        pVal++;
    }
    DBG_ASSERT( pVal - pValues == nCount * nSymbolProps, "properties missing" );
    aCfg.ReplaceSetProperties( A2OU( SYMBOL_LIST ) , aValues );
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
            aFnt.SetSize( pFormat->GetBaseSize() );

            if (pVal->hasValue()  &&  (*pVal >>= aTmpStr))
                aFnt.SetName( aTmpStr );
            ++pVal;
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                aFnt.SetCharSet( (rtl_TextEncoding) nTmp16 );
            ++pVal;
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                aFnt.SetFamily( (FontFamily) nTmp16 );
            ++pVal;
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                aFnt.SetPitch( (FontPitch) nTmp16 );
            ++pVal;
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                aFnt.SetWeight( (FontWeight) nTmp16 );
            ++pVal;
            if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
                aFnt.SetItalic( (FontItalic) nTmp16 );
            ++pVal;

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
        Font aFnt( pFormat->GetFont( i ) );

        pVal->Name  = *pNames++;
        pVal->Value <<= OUString( aFnt.GetName() );
        ++pVal;
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) aFnt.GetCharSet();
        ++pVal;
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) aFnt.GetFamily();
        ++pVal;
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) aFnt.GetPitch();
        ++pVal;
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) aFnt.GetWeight();
        ++pVal;
        pVal->Name  = *pNames++;
        pVal->Value <<= (INT16) aFnt.GetItalic();
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


SmMathConfig::SetOtherIfNotEqual( BOOL &rbItem, BOOL bNewVal )
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

