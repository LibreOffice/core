/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "cfgitem.hxx"

#include "starmath.hrc"
#include "format.hxx"
namespace binfilter {

using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define A2OU(x)        ::rtl::OUString::createFromAscii( x )

static const char* aRootName = "Office.Math";

#define SYMBOL_LIST         "SymbolList"
#define FONT_FORMAT_LIST    "FontFormatList"

/*N*/ SV_IMPL_OBJARR( SmFntFmtListEntryArr, SmFntFmtListEntry );

/////////////////////////////////////////////////////////////////


/*N*/ static Sequence< OUString > lcl_GetFontPropertyNames()
/*N*/ {
/*N*/ 	static const char * aPropNames[] =
/*N*/ 	{
/*N*/         "Name",
/*N*/         "CharSet",
/*N*/         "Family",
/*N*/         "Pitch",
/*N*/         "Weight",
/*N*/         "Italic",
/*N*/ 		0
/*N*/ 	};
/*N*/ 
/*N*/ 	const char** ppPropName = aPropNames;
/*N*/ 
/*N*/     Sequence< OUString > aNames( 6 );
/*N*/ 	OUString *pNames = aNames.getArray();
/*N*/ 	for( INT32 i = 0; *ppPropName;  ++i, ++ppPropName )
/*N*/ 	{
/*N*/ 		pNames[i] = A2OU( *ppPropName );
/*N*/ 	}
/*N*/ 	//aNames.realloc( i );
/*N*/ 	return aNames;
/*N*/ }

/////////////////////////////////////////////////////////////////


/*N*/ static Sequence< OUString > lcl_GetSymbolPropertyNames()
/*N*/ {
/*N*/ 	static const char * aPropNames[] =
/*N*/ 	{
/*N*/         "Char",
/*N*/         "Set",
/*N*/         "Predefined",
/*N*/         "FontFormatId",
/*N*/ 		0
/*N*/ 	};
/*N*/ 
/*N*/ 	const char** ppPropName = aPropNames;
/*N*/ 
/*N*/     Sequence< OUString > aNames( 4 );
/*N*/ 	OUString *pNames = aNames.getArray();
/*N*/ 	for( INT32 i = 0; *ppPropName;  ++i, ++ppPropName )
/*N*/ 	{
/*N*/ 		pNames[i] = A2OU( *ppPropName );
/*N*/ 	}
/*N*/ 	//aNames.realloc( i );
/*N*/ 	return aNames;
/*N*/ }

/////////////////////////////////////////////////////////////////

/*N*/ static const char * aMathPropNames[] =
/*N*/ {
/*N*/     "Print/Title",
/*N*/     "Print/FormulaText",
/*N*/     "Print/Frame",
/*N*/     "Print/Size",
/*N*/     "Print/ZoomFactor",
/*N*/     //"Misc/NoSymbolsWarning",
/*N*/     "Misc/IgnoreSpacesRight",
/*N*/     "View/ToolboxVisible",
/*N*/     "View/AutoRedraw",
/*N*/     "View/FormulaCursor"
/*N*/ };


//! Beware of order according to *_BEGIN *_END defines in format.hxx !
//! see respective load/save routines here
/*N*/ static const char * aFormatPropNames[] =
/*N*/ {
/*N*/     "StandardFormat/Textmode",
/*N*/     "StandardFormat/ScaleNormalBracket",
/*N*/     "StandardFormat/HorizontalAlignment",
/*N*/     "StandardFormat/BaseSize",
/*N*/     "StandardFormat/TextSize",
/*N*/     "StandardFormat/IndexSize",
/*N*/     "StandardFormat/FunctionSize",
/*N*/     "StandardFormat/OperatorSize",
/*N*/     "StandardFormat/LimitsSize",
/*N*/     "StandardFormat/Distance/Horizontal",
/*N*/     "StandardFormat/Distance/Vertical",
/*N*/     "StandardFormat/Distance/Root",
/*N*/     "StandardFormat/Distance/SuperScript",
/*N*/     "StandardFormat/Distance/SubScript",
/*N*/     "StandardFormat/Distance/Numerator",
/*N*/     "StandardFormat/Distance/Denominator",
/*N*/     "StandardFormat/Distance/Fraction",
/*N*/     "StandardFormat/Distance/StrokeWidth",
/*N*/     "StandardFormat/Distance/UpperLimit",
/*N*/     "StandardFormat/Distance/LowerLimit",
/*N*/     "StandardFormat/Distance/BracketSize",
/*N*/     "StandardFormat/Distance/BracketSpace",
/*N*/     "StandardFormat/Distance/MatrixRow",
/*N*/     "StandardFormat/Distance/MatrixColumn",
/*N*/     "StandardFormat/Distance/OrnamentSize",
/*N*/     "StandardFormat/Distance/OrnamentSpace",
/*N*/     "StandardFormat/Distance/OperatorSize",
/*N*/     "StandardFormat/Distance/OperatorSpace",
/*N*/     "StandardFormat/Distance/LeftSpace",
/*N*/     "StandardFormat/Distance/RightSpace",
/*N*/     "StandardFormat/Distance/TopSpace",
/*N*/     "StandardFormat/Distance/BottomSpace",
/*N*/     "StandardFormat/Distance/NormalBracketSize",
/*N*/     "StandardFormat/VariableFont",
/*N*/     "StandardFormat/FunctionFont",
/*N*/     "StandardFormat/NumberFont",
/*N*/     "StandardFormat/TextFont",
/*N*/     "StandardFormat/SerifFont",
/*N*/     "StandardFormat/SansFont",
/*N*/     "StandardFormat/FixedFont"
/*N*/ };


/*N*/ static Sequence< OUString > lcl_GetPropertyNames( 
/*N*/         const char * aPropNames[], USHORT nCount )
/*N*/ {
/*N*/ 
/*N*/ 	const char** ppPropName = aPropNames;
/*N*/ 
/*N*/     Sequence< OUString > aNames( nCount );
/*N*/ 	OUString *pNames = aNames.getArray();
/*N*/     for (INT32 i = 0;  i < nCount;  ++i, ++ppPropName)
/*N*/ 	{
/*N*/ 		pNames[i] = A2OU( *ppPropName );
/*N*/ 	}
/*N*/ 	//aNames.realloc( i );
/*N*/ 	return aNames;
/*N*/ }

/////////////////////////////////////////////////////////////////
    
/*N*/ Sequence< OUString > SmMathConfigItem::GetFormatPropertyNames()
/*N*/ {
/*N*/     USHORT nCnt = sizeof(aFormatPropNames) / sizeof(aFormatPropNames[0]);
/*N*/     return lcl_GetPropertyNames( aFormatPropNames, nCnt );
/*N*/ }


/*N*/ Sequence< OUString > SmMathConfigItem::GetOtherPropertyNames()
/*N*/ {
/*N*/     USHORT nCnt = sizeof(aMathPropNames) / sizeof(aMathPropNames[0]);
/*N*/     return lcl_GetPropertyNames( aMathPropNames, nCnt );
/*N*/ }

/////////////////////////////////////////////////////////////////

/*N*/ struct SmCfgOther
/*N*/ {
/*N*/     SmPrintSize     ePrintSize;
/*N*/     USHORT          nPrintZoomFactor;
/*N*/     BOOL            bPrintTitle;
/*N*/     BOOL            bPrintFormulaText;
/*N*/     BOOL            bPrintFrame;
/*N*/     BOOL            bIgnoreSpacesRight;
/*N*/     BOOL            bToolboxVisible;
/*N*/     BOOL            bAutoRedraw;
/*N*/     BOOL            bFormulaCursor;
/*N*/     //BOOL            bNoSymbolsWarning;
/*N*/ 
/*N*/     SmCfgOther();
/*N*/ };


/*N*/ SmCfgOther::SmCfgOther()
/*N*/ {
/*N*/     ePrintSize          = PRINT_SIZE_NORMAL;
/*N*/     nPrintZoomFactor    = 100;
/*N*/     bPrintTitle         = bPrintFormulaText   =
/*N*/     bPrintFrame         = bIgnoreSpacesRight  =
/*N*/     bToolboxVisible     = bAutoRedraw         =
/*N*/     bFormulaCursor      = /*bNoSymbolsWarning   =*/ TRUE;
/*N*/ }

/////////////////////////////////////////////////////////////////


/*N*/ SmFontFormat::SmFontFormat()
/*N*/ {
/*N*/     aName.AssignAscii( FONTNAME_MATH );
/*N*/     nCharSet    = RTL_TEXTENCODING_UNICODE;
/*N*/     nFamily     = FAMILY_DONTKNOW;
/*N*/     nPitch      = PITCH_DONTKNOW;
/*N*/     nWeight     = WEIGHT_DONTKNOW;
/*N*/     nItalic     = ITALIC_NONE;
/*N*/ }
    

// /*N*/ SmFontFormat::SmFontFormat( const Font &rFont )
// /*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
// /*N*/ }


/*N*/ const Font SmFontFormat::GetFont() const
/*N*/ {
/*N*/     Font aRes;
/*N*/     aRes.SetName( aName );
/*N*/     aRes.SetCharSet( (rtl_TextEncoding) nCharSet );
/*N*/     aRes.SetFamily( (FontFamily) nFamily );
/*N*/     aRes.SetPitch( (FontPitch) nPitch );
/*N*/     aRes.SetWeight( (FontWeight) nWeight );
/*N*/     aRes.SetItalic( (FontItalic) nItalic );
/*N*/     return aRes;
/*N*/ }

    


/////////////////////////////////////////////////////////////////

/*N*/ SmFntFmtListEntry::SmFntFmtListEntry( const String &rId, const SmFontFormat &rFntFmt ) :
/*N*/     aId     (rId),
/*N*/     aFntFmt (rFntFmt)
/*N*/ {
/*N*/ }


/*N*/ SmFontFormatList::SmFontFormatList()
/*N*/ {
/*N*/     bModified = FALSE;
/*N*/ }




/*N*/ void SmFontFormatList::AddFontFormat( const String &rFntFmtId, 
/*N*/         const SmFontFormat &rFntFmt )
/*N*/ {
/*N*/     const SmFontFormat *pFntFmt = GetFontFormat( rFntFmtId );
/*N*/     DBG_ASSERT( !pFntFmt, "FontFormatId already exists" );
/*N*/     if (!pFntFmt)
/*N*/     {
/*N*/         SmFntFmtListEntry aEntry( rFntFmtId, rFntFmt );
/*N*/         aEntries.Insert( aEntry, aEntries.Count() );
/*N*/         SetModified( TRUE );
/*N*/     }
/*N*/ }
    
    

    
/*N*/ const SmFontFormat * SmFontFormatList::GetFontFormat( const String &rFntFmtId ) const
/*N*/ {
/*N*/     SmFontFormat *pRes = 0;
/*N*/ 
/*N*/     USHORT nCnt = aEntries.Count();
/*N*/     USHORT i;
/*N*/     for (i = 0;  i < nCnt  &&  !pRes;  ++i)
/*N*/     {
/*N*/         if (aEntries[i].aId == rFntFmtId)
/*N*/             pRes = &aEntries[i].aFntFmt;
/*N*/     }
/*N*/     
/*N*/     return pRes;
/*N*/ }



// /*N*/ const SmFontFormat * SmFontFormatList::GetFontFormat( USHORT nPos ) const
// /*N*/ {
// /*N*/     SmFontFormat *pRes = 0;
// /*N*/     if (nPos < aEntries.Count())
// /*N*/         pRes = &aEntries[ nPos ].aFntFmt;
// /*N*/     return pRes;
// /*N*/ }









/////////////////////////////////////////////////////////////////

/*N*/ SmMathConfig::SmMathConfig()
/*N*/ {
/*N*/     pFormat         = 0;
/*N*/     pOther          = 0;
/*N*/     pFontFormatList = 0;
/*N*/ 	pSymbols		= 0;
/*N*/ 	nSymbolCount	= 0;
/*N*/ 
/*N*/     bIsOtherModified = bIsFormatModified = FALSE;
/*N*/ 	
/*N*/     aSaveTimer.SetTimeout( 3000 );
/*N*/ 	aSaveTimer.SetTimeoutHdl( LINK( this, SmMathConfig, TimeOut ) );
/*N*/ }
    

/*N*/ SmMathConfig::~SmMathConfig()
/*N*/ {
/*N*/     Save();
/*N*/     delete pFormat;
/*N*/     delete pOther;
/*N*/     delete pFontFormatList;
/*N*/     delete [] pSymbols;
/*N*/ }

          
/*N*/ void SmMathConfig::SetOtherModified( BOOL bVal )
/*N*/ {
/*N*/     if ((bIsOtherModified = bVal))
/*?*/ 		aSaveTimer.Start();
/*N*/ }


/*N*/ void SmMathConfig::SetFormatModified( BOOL bVal )
/*N*/ {
/*N*/     if ((bIsFormatModified = bVal))
/*?*/ 		aSaveTimer.Start();
/*N*/ }


/*N*/ SmSym SmMathConfig::ReadSymbol( SmMathConfigItem &rCfg,
/*N*/ 						const ::rtl::OUString &rSymbolName, 
/*N*/ 						const ::rtl::OUString &rBaseNode ) const
/*N*/ {
/*N*/ 	SmSym aRes;
/*N*/ 
/*N*/ 	Sequence< OUString > aNames = lcl_GetSymbolPropertyNames();
/*N*/ 	INT32 nProps = aNames.getLength();
/*N*/ 
/*N*/ 	OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
/*N*/ 	OUString *pName = aNames.getArray();
/*N*/ 	for (INT32 i = 0;  i < nProps;  ++i)
/*N*/ 	{
/*N*/ 		OUString &rName = pName[i];
/*N*/ 		OUString aTmp( rName );
/*N*/ 		rName = rBaseNode;
/*N*/ 		rName += aDelim;
/*N*/ 		rName += rSymbolName;
/*N*/ 		rName += aDelim;
/*N*/ 		rName += aTmp;
/*N*/ 	}
/*N*/ 
/*N*/ 	const Sequence< Any > aValues = rCfg.GetProperties( aNames );
/*N*/ 
/*N*/ 	if (nProps  &&  aValues.getLength() == nProps)
/*N*/ 	{
/*N*/ 		const Any * pValue = aValues.getConstArray();
/*N*/         Font        aFont;
/*N*/         sal_Unicode cChar;
/*N*/         String      aSet;
/*N*/         BOOL        bPredefined;
/*N*/ 
/*N*/         OUString    aTmpStr;
/*N*/         INT32       nTmp32 = 0;
/*N*/         INT16       nTmp16 = 0;
/*N*/         BOOL        bTmp = FALSE;
/*N*/ 
/*N*/         BOOL bOK = TRUE;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= nTmp32))
/*N*/             cChar = (sal_Unicode) nTmp32;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/         ++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
/*N*/             aSet = aTmpStr;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= bTmp))
/*N*/             bPredefined = bTmp;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
/*N*/         {
/*N*/             const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
/*N*/             DBG_ASSERT( pFntFmt, "unknown FontFormat" );
/*N*/             if (pFntFmt)
/*N*/                 aFont = pFntFmt->GetFont();
/*N*/         }
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/         ++pValue;
/*N*/ 
/*N*/         if (bOK)
/*N*/         {
/*N*/             String aUiName( rSymbolName );
/*N*/             String aUiSetName( aSet );
/*N*/             if (bPredefined)
/*N*/             {
/*N*/                 String aTmp;
/*N*/                 aTmp = GetUiSymbolName( rSymbolName );
/*N*/                 DBG_ASSERT( aTmp.Len(), "localized symbol-name not found" );
/*N*/                 if (aTmp.Len())
/*N*/                     aUiName = aTmp;
/*N*/                 aTmp = GetUiSymbolSetName( aSet );
/*N*/                 DBG_ASSERT( aTmp.Len(), "localized symbolset-name not found" );
/*N*/                 if (aTmp.Len())
/*N*/                     aUiSetName = aTmp;
/*N*/             }
/*N*/ 
/*N*/             aRes = SmSym( aUiName, aFont, cChar, aUiSetName, bPredefined );
/*N*/             if (aUiName != String(rSymbolName))
/*?*/                 aRes.SetExportName( rSymbolName );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             DBG_ERROR( "symbol read error" );
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRes;
/*N*/ }


/*N*/ void SmMathConfig::LoadSymbols()
/*N*/ {
/*N*/ 	SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));
/*N*/ 
/*N*/ 	Sequence< OUString > aNodes( aCfg.GetNodeNames( A2OU( SYMBOL_LIST ) ) );
/*N*/ 	const OUString *pNode = aNodes.getConstArray();
/*N*/ 	INT32 nNodes = aNodes.getLength();
/*N*/ 
/*N*/ 	if (pSymbols)
/*?*/         delete [] pSymbols;
/*N*/ 	pSymbols = nNodes ? new SmSym[ nNodes ] : 0;
/*N*/     nSymbolCount = (USHORT) nNodes;
/*N*/ 
/*N*/ 	for (INT32 i = 0;  i < nNodes;  ++i)
/*N*/ 	{
/*N*/ 		pSymbols[i] = ReadSymbol( aCfg, pNode[i], A2OU( SYMBOL_LIST ) );
/*N*/ 	}
/*N*/ }


/*N*/ void SmMathConfig::Save()
/*N*/ {
/*N*/     SaveOther();
/*N*/     SaveFormat();
/*N*/     SaveFontFormatList();
/*N*/ }


/*N*/ USHORT SmMathConfig::GetSymbolCount() const
/*N*/ {
/*N*/ 	if (!pSymbols)
/*N*/ 		((SmMathConfig *) this)->LoadSymbols();
/*N*/ 	return nSymbolCount;
/*N*/ }


/*N*/ const SmSym * SmMathConfig::GetSymbol( USHORT nIndex ) const
/*N*/ {
/*N*/ 	SmSym *pRes = 0;
/*N*/ 	if (!pSymbols)
/*?*/ 		((SmMathConfig *) this)->LoadSymbols();
/*N*/ 	if (nIndex < nSymbolCount)
/*N*/ 		pRes = &pSymbols[ nIndex ];
/*N*/ 	return pRes;
/*N*/ }




/*N*/ SmFontFormatList & SmMathConfig::GetFontFormatList()
/*N*/ {
/*N*/     if (!pFontFormatList)
/*N*/     {
/*N*/         LoadFontFormatList();
/*N*/     }
/*N*/     return *pFontFormatList;
/*N*/ }


/*N*/ void SmMathConfig::LoadFontFormatList()
/*N*/ {
/*N*/     if (!pFontFormatList)
/*N*/         pFontFormatList = new SmFontFormatList;
/*N*/     else
/*?*/     {DBG_BF_ASSERT(0, "STRIP");} //STRIP001     pFontFormatList->Clear();
/*N*/ 
/*N*/     SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ) );
/*N*/ 
/*N*/     Sequence< OUString > aNodes( aCfg.GetNodeNames( A2OU( FONT_FORMAT_LIST ) ) );
/*N*/ 	const OUString *pNode = aNodes.getConstArray();
/*N*/ 	INT32 nNodes = aNodes.getLength();
/*N*/ 
/*N*/ 	for (INT32 i = 0;  i < nNodes;  ++i)
/*N*/ 	{
/*N*/         SmFontFormat aFntFmt( ReadFontFormat( aCfg, pNode[i], A2OU( FONT_FORMAT_LIST ) ) );
/*N*/         if (!pFontFormatList->GetFontFormat( pNode[i] ))
/*N*/         {
/*N*/             DBG_ASSERT( 0 == pFontFormatList->GetFontFormat( pNode[i] ), 
/*N*/                     "FontFormat ID already exists" );
/*N*/             pFontFormatList->AddFontFormat( pNode[i], aFntFmt );
/*N*/         }
/*N*/ 	}
/*N*/     pFontFormatList->SetModified( FALSE );
/*N*/ }
    

/*N*/ SmFontFormat SmMathConfig::ReadFontFormat( SmMathConfigItem &rCfg,
/*N*/         const OUString &rSymbolName, const OUString &rBaseNode ) const
/*N*/ {
/*N*/     SmFontFormat aRes;
/*N*/ 
/*N*/     Sequence< OUString > aNames = lcl_GetFontPropertyNames();
/*N*/ 	INT32 nProps = aNames.getLength();
/*N*/ 
/*N*/ 	OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
/*N*/ 	OUString *pName = aNames.getArray();
/*N*/ 	for (INT32 i = 0;  i < nProps;  ++i)
/*N*/ 	{
/*N*/ 		OUString &rName = pName[i];
/*N*/ 		OUString aTmp( rName );
/*N*/ 		rName = rBaseNode;
/*N*/ 		rName += aDelim;
/*N*/ 		rName += rSymbolName;
/*N*/ 		rName += aDelim;
/*N*/ 		rName += aTmp;
/*N*/ 	}
/*N*/ 
/*N*/ 	const Sequence< Any > aValues = rCfg.GetProperties( aNames );
/*N*/ 
/*N*/ 	if (nProps  &&  aValues.getLength() == nProps)
/*N*/ 	{
/*N*/ 		const Any * pValue = aValues.getConstArray();
/*N*/ 
/*N*/         OUString    aTmpStr;
/*N*/         INT16       nTmp16 = 0;
/*N*/ 
/*N*/         BOOL bOK = TRUE;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= aTmpStr))
/*N*/             aRes.aName = aTmpStr;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/         ++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
/*N*/             aRes.nCharSet = nTmp16; // 6.0 file-format GetSOLoadTextEncoding not needed
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
/*N*/             aRes.nFamily = nTmp16;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
/*N*/             aRes.nPitch = nTmp16;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
/*N*/             aRes.nWeight = nTmp16;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/         if (pValue->hasValue()  &&  (*pValue >>= nTmp16))
/*N*/             aRes.nItalic = nTmp16;
/*N*/         else
/*N*/             bOK = FALSE;
/*N*/ 		++pValue;
/*N*/ 
/*N*/         DBG_ASSERT( bOK, "read FontFormat failed" );
/*N*/ 	}
/*N*/     
/*N*/     return aRes;
/*N*/ }


/*N*/ void SmMathConfig::SaveFontFormatList()
/*N*/ {
/*N*/     SmFontFormatList &rFntFmtList = GetFontFormatList();
/*N*/     
/*N*/     if (!rFntFmtList.IsModified())
/*N*/         return;
/*N*/ 
/*?*/     SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ) );
/*?*/     
/*?*/     Sequence< OUString > aNames = lcl_GetFontPropertyNames();
/*?*/     INT32 nSymbolProps = aNames.getLength();
/*?*/     
/*?*/     USHORT nCount = rFntFmtList.GetCount();
/*?*/ 
/*?*/     Sequence< PropertyValue > aValues( nCount * nSymbolProps );
/*?*/     PropertyValue *pValues = aValues.getArray();
/*?*/ 
/*?*/     PropertyValue *pVal = pValues;
/*?*/     OUString aDelim( OUString::valueOf( (sal_Unicode) '/' ) );
/*?*/     for (USHORT i = 0;  i < nCount;  ++i)
/*?*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 String aFntFmtId( rFntFmtList.GetFontFormatId( i ) );
/*?*/     }
/*?*/     DBG_ASSERT( pVal - pValues == nCount * nSymbolProps, "properties missing" );
/*?*/     aCfg.ReplaceSetProperties( A2OU( FONT_FORMAT_LIST ) , aValues );
/*?*/     
/*?*/     rFntFmtList.SetModified( FALSE );
/*N*/ }



       
/*N*/ void SmMathConfig::LoadOther()
/*N*/ {
/*N*/     if (!pOther)
/*N*/         pOther = new SmCfgOther;
/*N*/ 
/*N*/ 	SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));
/*N*/     
/*N*/     Sequence< OUString > aNames( aCfg.GetOtherPropertyNames() );
/*N*/     INT32 nProps = aNames.getLength();
/*N*/ 
/*N*/     Sequence< Any > aValues( aCfg.GetProperties( aNames ) );
/*N*/     if (nProps  &&  aValues.getLength() == nProps)
/*N*/     {
/*N*/         const Any *pValues = aValues.getConstArray();
/*N*/         const Any *pVal = pValues;
/*N*/ 
/*N*/         INT16   nTmp16;
/*N*/         BOOL    bTmp;
/*N*/ 
/*N*/         // Print/Title
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bPrintTitle = bTmp;
/*N*/         ++pVal;
/*N*/         // Print/FormulaText
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bPrintFormulaText = bTmp;
/*N*/         ++pVal;
/*N*/         // Print/Frame
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bPrintFrame = bTmp;
/*N*/         ++pVal;
/*N*/         // Print/Size
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
/*N*/             pOther->ePrintSize = (SmPrintSize) nTmp16;
/*N*/         ++pVal;
/*N*/         // Print/ZoomFactor
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
/*N*/             pOther->nPrintZoomFactor = nTmp16;
/*        ++pVal;
        // Misc/NoSymbolsWarning
        if (pVal->hasValue()  &&  (*pVal >>= bTmp))
            pOther->bNoSymbolsWarning = bTmp;
*/
/*N*/         ++pVal;
/*N*/         // Misc/IgnoreSpacesRight
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bIgnoreSpacesRight = bTmp;
/*N*/         ++pVal;
/*N*/         // View/ToolboxVisible
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bToolboxVisible = bTmp;
/*N*/         ++pVal;
/*N*/         // View/AutoRedraw
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bAutoRedraw = bTmp;
/*N*/         ++pVal;
/*N*/         // View/FormulaCursor
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pOther->bFormulaCursor = bTmp;
/*N*/         ++pVal;
/*N*/ 
/*N*/         DBG_ASSERT( pVal - pValues == nProps, "property mismatch" );
/*N*/         SetOtherModified( FALSE );
/*N*/     }
/*N*/ }


/*N*/ void SmMathConfig::SaveOther()
/*N*/ {
/*N*/     if (!pOther || !IsOtherModified())
/*N*/         return;
/*N*/ 
/*?*/ 	SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));
/*?*/     
/*?*/     const Sequence< OUString > aNames( aCfg.GetOtherPropertyNames() );
/*?*/     INT32 nProps = aNames.getLength();
/*?*/ 
/*?*/     Sequence< Any > aValues( nProps );
/*?*/     Any *pValues = aValues.getArray();
/*?*/     Any *pValue  = pValues;
/*?*/ 
/*?*/     // Print/Title
/*?*/     *pValue++ <<= (BOOL) pOther->bPrintTitle;
/*?*/     // Print/FormulaText
/*?*/     *pValue++ <<= (BOOL) pOther->bPrintFormulaText;
/*?*/     // Print/Frame
/*?*/     *pValue++ <<= (BOOL) pOther->bPrintFrame;
/*?*/     // Print/Size
/*?*/     *pValue++ <<= (INT16) pOther->ePrintSize;
/*?*/     // Print/ZoomFactor
/*?*/     *pValue++ <<= (INT16) pOther->nPrintZoomFactor;
/*    // Misc/NoSymbolsWarning
    *pValue++ <<= (BOOL) pOther->bNoSymbolsWarning;
*/
/*?*/     // Misc/IgnoreSpacesRight
/*?*/     *pValue++ <<= (BOOL) pOther->bIgnoreSpacesRight;
/*?*/     // View/ToolboxVisible
/*?*/     *pValue++ <<= (BOOL) pOther->bToolboxVisible;
/*?*/     // View/AutoRedraw
/*?*/     *pValue++ <<= (BOOL) pOther->bAutoRedraw;
/*?*/     // View/FormulaCursor
/*?*/     *pValue++ <<= (BOOL) pOther->bFormulaCursor;
/*?*/     
/*?*/     DBG_ASSERT( pValue - pValues == nProps, "property mismatch" );
/*?*/     aCfg.PutProperties( aNames , aValues );
/*?*/ 
/*?*/     SetOtherModified( FALSE );
/*N*/ }

/*N*/ void SmMathConfig::LoadFormat()
/*N*/ {
/*N*/     if (!pFormat)
/*N*/         pFormat = new SmFormat;
/*N*/ 
/*N*/ 	SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));
/*N*/     
/*N*/     Sequence< OUString > aNames( aCfg.GetFormatPropertyNames() );
/*N*/     INT32 nProps = aNames.getLength();
/*N*/ 
/*N*/     Sequence< Any > aValues( aCfg.GetProperties( aNames ) );
/*N*/     if (nProps  &&  aValues.getLength() == nProps)
/*N*/     {
/*N*/         const Any *pValues = aValues.getConstArray();
/*N*/         const Any *pVal = pValues;
/*N*/ 
/*N*/         OUString    aTmpStr;
/*N*/         INT16       nTmp16;
/*N*/         BOOL        bTmp;
/*N*/ 
/*N*/         // StandardFormat/Textmode
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pFormat->SetTextmode( bTmp );
/*N*/         ++pVal;
/*N*/         // StandardFormat/ScaleNormalBracket
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= bTmp))
/*N*/             pFormat->SetScaleNormalBrackets( bTmp );
/*N*/         ++pVal;
/*N*/         // StandardFormat/HorizontalAlignment
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
/*N*/             pFormat->SetHorAlign( (SmHorAlign) nTmp16 );
/*N*/         ++pVal;
/*N*/         // StandardFormat/BaseSize
/*N*/         if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
/*N*/             pFormat->SetBaseSize( Size(0, SmPtsTo100th_mm( nTmp16 )) );
/*N*/         ++pVal;
/*N*/ 
/*N*/         USHORT i;
/*N*/         for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
/*N*/         {
/*N*/             if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
/*N*/                 pFormat->SetRelSize( i, nTmp16 );
/*N*/             ++pVal;
/*N*/         }
/*N*/ 
/*N*/         for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
/*N*/         {
/*N*/             if (pVal->hasValue()  &&  (*pVal >>= nTmp16))
/*N*/                 pFormat->SetDistance( i, nTmp16 );
/*N*/             ++pVal;
/*N*/         }
/*N*/ 
/*N*/         LanguageType nLang = Application::GetSettings().GetUILanguage(); 
/*N*/         for (i = FNT_BEGIN;  i < FNT_END;  ++i)
/*N*/         {
/*N*/             Font aFnt;
/*N*/             BOOL bUseDefaultFont = TRUE;
/*N*/             if (pVal->hasValue()  &&  (*pVal >>= aTmpStr))
/*N*/             {
/*N*/                 bUseDefaultFont = 0 == aTmpStr.getLength();
/*N*/                 if (bUseDefaultFont)
/*N*/                 {
/*N*/                     aFnt = pFormat->GetFont( i );
/*N*/                     aFnt.SetName( GetDefaultFontName( nLang, i ) );
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*?*/                     const SmFontFormat *pFntFmt = GetFontFormatList().GetFontFormat( aTmpStr );
/*?*/                     DBG_ASSERT( pFntFmt, "unknown FontFormat" );
/*?*/                     if (pFntFmt)
/*?*/                         aFnt = pFntFmt->GetFont();
/*N*/                 }
/*N*/             }
/*N*/             ++pVal;
/*N*/ 
/*N*/             aFnt.SetSize( pFormat->GetBaseSize() );
/*N*/             pFormat->SetFont( i, aFnt, bUseDefaultFont );
/*N*/         }
/*N*/ 
/*N*/         DBG_ASSERT( pVal - pValues == nProps, "property mismatch" );
/*N*/         SetFormatModified( FALSE );
/*N*/     }
/*N*/ }


/*N*/ void SmMathConfig::SaveFormat()
/*N*/ {
/*N*/     if (!pFormat || !IsFormatModified())
/*N*/         return;
/*N*/ 
/*?*/ 	SmMathConfigItem aCfg( String::CreateFromAscii( aRootName ));
/*?*/     
/*?*/     const Sequence< OUString > aNames( aCfg.GetFormatPropertyNames() );
/*?*/     INT32 nProps = aNames.getLength();
/*?*/ 
/*?*/     Sequence< Any > aValues( nProps );
/*?*/     Any *pValues = aValues.getArray();
/*?*/     Any *pValue  = pValues;
/*?*/ 
/*?*/     // StandardFormat/Textmode
/*?*/     *pValue++ <<= (BOOL) pFormat->IsTextmode();
/*?*/     // StandardFormat/ScaleNormalBracket
/*?*/     *pValue++ <<= (BOOL) pFormat->IsScaleNormalBrackets();
/*?*/     // StandardFormat/HorizontalAlignment
/*?*/     *pValue++ <<= (INT16) pFormat->GetHorAlign();
/*?*/     // StandardFormat/BaseSize
/*?*/     *pValue++ <<= (INT16) SmRoundFraction( Sm100th_mmToPts( 
/*?*/                                     pFormat->GetBaseSize().Height() ) );
/*?*/ 
/*?*/     USHORT i;
/*?*/     for (i = SIZ_BEGIN;  i <= SIZ_END;  ++i)
/*?*/         *pValue++ <<= (INT16) pFormat->GetRelSize( i );
/*?*/ 
/*?*/     for (i = DIS_BEGIN;  i <= DIS_END;  ++i)
/*?*/         *pValue++ <<= (INT16) pFormat->GetDistance( i );
/*?*/ 
/*?*/     for (i = FNT_BEGIN;  i < FNT_END;  ++i)
/*?*/     {
/*?*/         OUString aFntFmtId;
/*?*/ 
/*?*/         if (!pFormat->IsDefaultFont( i ))
/*?*/         {
/*?*/             DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SmFontFormat aFntFmt( pFormat->GetFont( i ) );
/*?*/         }
/*?*/ 
/*?*/         *pValue++ <<= aFntFmtId;
/*?*/     }
/*?*/     
/*?*/     DBG_ASSERT( pValue - pValues == nProps, "property mismatch" );
/*?*/     aCfg.PutProperties( aNames , aValues );
/*?*/     
/*?*/     SetFormatModified( FALSE );
/*N*/ }


/*N*/ const SmFormat & SmMathConfig::GetStandardFormat() const
/*N*/ {
/*N*/     if (!pFormat)
/*N*/         ((SmMathConfig *) this)->LoadFormat();
/*N*/     return *pFormat;
/*N*/ }


        

/*N*/ SmPrintSize SmMathConfig::GetPrintSize() const
/*N*/ {
/*N*/     if (!pOther)
/*N*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->ePrintSize;
/*N*/ }




/*N*/ USHORT SmMathConfig::GetPrintZoomFactor() const
/*N*/ {
/*N*/     if (!pOther)
/*?*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->nPrintZoomFactor;
/*N*/ }


    



/*N*/ BOOL SmMathConfig::IsPrintTitle() const
/*N*/ {
/*N*/     if (!pOther)
/*?*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->bPrintTitle;
/*N*/ }




/*N*/ BOOL SmMathConfig::IsPrintFormulaText() const
/*N*/ {
/*N*/     if (!pOther)
/*?*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->bPrintFormulaText;
/*N*/ }




/*N*/ BOOL SmMathConfig::IsPrintFrame() const
/*N*/ {
/*N*/     if (!pOther)
/*?*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->bPrintFrame;
/*N*/ }




/*N*/ BOOL SmMathConfig::IsIgnoreSpacesRight() const
/*N*/ {
/*N*/     if (!pOther)
/*?*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->bIgnoreSpacesRight;
/*N*/ }


    void SmMathConfigItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}
    void SmMathConfigItem::Commit() {}






/*N*/ BOOL SmMathConfig::IsAutoRedraw() const
/*N*/ {
/*N*/     if (!pOther)
/*?*/         ((SmMathConfig *) this)->LoadOther();
/*N*/     return pOther->bAutoRedraw;
/*N*/ }

/*N*/ IMPL_LINK( SmMathConfig, TimeOut, Timer *, p )
/*N*/ {
/*N*/   DBG_BF_ASSERT(0, "STRIP"); //STRIP001   Save();
/*N*/ 	return 0;
/*N*/ }

/////////////////////////////////////////////////////////////////

}
