/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokenuno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:18:37 $
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
#include "precompiled_sc.hxx"


#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <svtools/itemprop.hxx>

#include "tokenuno.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "unonames.hxx"
#include "unoguard.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"
#include "docsh.hxx"
#include "rangeseq.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetFormulaParserMap()
{
    static SfxItemPropertyMap aFormulaParserMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_REFERENCEPOS),    0,  &getCppuType((table::CellAddress*)0),    0, 0 },
        {MAP_CHAR_LEN(SC_UNO_COMPILEENGLISH),  0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_IGNORELEADING),   0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_COMPATIBILITY3D), 0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_R1C1NOTATION),    0,  &getBooleanCppuType(),                   0, 0 },
        {MAP_CHAR_LEN(SC_UNO_OPCODEMAP),       0,  &getCppuType((uno::Sequence< sheet::FormulaOpCodeMapEntry >*)0), 0, 0 },
        {0,0,0,0,0,0}
    };
    return aFormulaParserMap_Impl;
}

SC_SIMPLE_SERVICE_INFO( ScFormulaParserObj, "ScFormulaParserObj", SC_SERVICENAME_FORMULAPARS )

//------------------------------------------------------------------------

ScFormulaParserObj::ScFormulaParserObj(ScDocShell* pDocSh) :
    mpDocShell( pDocSh ),
    mbEnglish( false ),
    mbR1C1( false ),
    mbComp3D( false ),
    mbIgnoreSpaces( true )
{
    mpDocShell->GetDocument()->AddUnoObject(*this);
}

ScFormulaParserObj::~ScFormulaParserObj()
{
    if (mpDocShell)
        mpDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScFormulaParserObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        mpDocShell = NULL;
}

// XFormulaParser

void ScFormulaParserObj::SetCompilerFlags( ScCompiler& rCompiler ) const
{
    // If mxOpCodeMap is not empty it overrides mbEnglish, and vice versa. We
    // don't need to initialize things twice.
    if (mxOpCodeMap.get())
        rCompiler.SetFormulaLanguage( mxOpCodeMap );
    else
    {
        sal_Int32 nFormulaLanguage = mbEnglish ?
            sheet::FormulaLanguage::ENGLISH :
            sheet::FormulaLanguage::NATIVE;
        ScCompiler::OpCodeMapPtr xMap = ScCompiler::GetOpCodeMap( nFormulaLanguage);
        rCompiler.SetFormulaLanguage( xMap);
    }

    ScAddress::Convention eConv = ScAddress::CONV_UNSPECIFIED;
    if ( mbR1C1 )
    {
        // R1C1 is available only with '!' sheet separator
        eConv = ScAddress::CONV_XL_R1C1;
    }
    else
    {
        if ( mbComp3D )
            eConv = ScAddress::CONV_XL_A1;      // '!' and A1
        else
            eConv = ScAddress::CONV_OOO;        // '.' and A1
    }
    rCompiler.SetRefConvention( eConv );
}

uno::Sequence<sheet::FormulaToken> SAL_CALL ScFormulaParserObj::parseFormula( const rtl::OUString& aFormula )
                                throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aRet;

    if (mpDocShell)
    {
        ScCompiler aCompiler( mpDocShell->GetDocument(), maRefPos );
        SetCompilerFlags( aCompiler );

        ScTokenArray* pCode = aCompiler.CompileString( aFormula );
        (void)ScTokenConversion::ConvertToTokenSequence( aRet, *pCode );
        delete pCode;
    }

    return aRet;
}

rtl::OUString SAL_CALL ScFormulaParserObj::printFormula( const uno::Sequence<sheet::FormulaToken>& aTokens )
                                throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;

    if (mpDocShell)
    {
        ScTokenArray aCode;
        (void)ScTokenConversion::ConvertToTokenArray( aCode, aTokens );
        ScCompiler aCompiler( mpDocShell->GetDocument(), maRefPos, aCode );
        SetCompilerFlags( aCompiler );

        rtl::OUStringBuffer aBuffer;
        aCompiler.CreateStringFromTokenArray( aBuffer );
        aRet = aBuffer.makeStringAndClear();
    }

    return aRet;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScFormulaParserObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference< beans::XPropertySetInfo > aRef(new SfxItemPropertySetInfo( lcl_GetFormulaParserMap() ));
    return aRef;
}

void SAL_CALL ScFormulaParserObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString(aPropertyName);
    if ( aString.EqualsAscii( SC_UNO_REFERENCEPOS ) )
    {
        table::CellAddress aAddress;
        aValue >>= aAddress;
        ScUnoConversion::FillScAddress( maRefPos, aAddress );
    }
    else if ( aString.EqualsAscii( SC_UNO_COMPILEENGLISH ) )
    {
        bool bOldEnglish = mbEnglish;
        if (aValue >>= mbEnglish)
        {
            // Need to recreate the symbol map to change English property
            // because the map is const. So for performance reasons set
            // CompileEnglish _before_ OpCodeMap!
            if (mxOpCodeMap.get() && mbEnglish != bOldEnglish)
                mxOpCodeMap = ScCompiler::CreateOpCodeMap( maOpCodeMapping, mbEnglish);
        }
        else
            throw lang::IllegalArgumentException();
    }
    else if ( aString.EqualsAscii( SC_UNO_R1C1NOTATION ) )
    {
        aValue >>= mbR1C1;
    }
    else if ( aString.EqualsAscii( SC_UNO_COMPATIBILITY3D ) )
    {
        aValue >>= mbComp3D;
    }
    else if ( aString.EqualsAscii( SC_UNO_IGNORELEADING ) )
    {
        aValue >>= mbIgnoreSpaces;
    }
    else if ( aString.EqualsAscii( SC_UNO_OPCODEMAP ) )
    {
        if (aValue >>= maOpCodeMapping)
            mxOpCodeMap = ScCompiler::CreateOpCodeMap( maOpCodeMapping, mbEnglish);
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScFormulaParserObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;
    String aString(aPropertyName);
    if ( aString.EqualsAscii( SC_UNO_REFERENCEPOS ) )
    {
        table::CellAddress aAddress;
        ScUnoConversion::FillApiAddress( aAddress, maRefPos );
        aRet <<= aAddress;
    }
    else if ( aString.EqualsAscii( SC_UNO_COMPILEENGLISH ) )
    {
        aRet <<= mbEnglish;
    }
    else if ( aString.EqualsAscii( SC_UNO_R1C1NOTATION ) )
    {
        aRet <<= mbR1C1;
    }
    else if ( aString.EqualsAscii( SC_UNO_COMPATIBILITY3D ) )
    {
        aRet <<= mbComp3D;
    }
    else if ( aString.EqualsAscii( SC_UNO_IGNORELEADING ) )
    {
        aRet <<= mbIgnoreSpaces;
    }
    else if ( aString.EqualsAscii( SC_UNO_OPCODEMAP ) )
    {
        aRet <<= maOpCodeMapping;
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScFormulaParserObj )

//------------------------------------------------------------------------

void lcl_SingleRefToCalc( SingleRefData& rRef, const sheet::SingleReference& rAPI )
{
    rRef.InitFlags();

    rRef.nCol    = static_cast<SCsCOL>(rAPI.Column);
    rRef.nRow    = static_cast<SCsROW>(rAPI.Row);
    rRef.nTab    = static_cast<SCsTAB>(rAPI.Sheet);
    rRef.nRelCol = static_cast<SCsCOL>(rAPI.RelativeColumn);
    rRef.nRelRow = static_cast<SCsROW>(rAPI.RelativeRow);
    rRef.nRelTab = static_cast<SCsTAB>(rAPI.RelativeSheet);

    rRef.SetColRel(     ( rAPI.Flags & sheet::ReferenceFlags::COLUMN_RELATIVE ) != 0 );
    rRef.SetRowRel(     ( rAPI.Flags & sheet::ReferenceFlags::ROW_RELATIVE    ) != 0 );
    rRef.SetTabRel(     ( rAPI.Flags & sheet::ReferenceFlags::SHEET_RELATIVE  ) != 0 );
    rRef.SetColDeleted( ( rAPI.Flags & sheet::ReferenceFlags::COLUMN_DELETED  ) != 0 );
    rRef.SetRowDeleted( ( rAPI.Flags & sheet::ReferenceFlags::ROW_DELETED     ) != 0 );
    rRef.SetTabDeleted( ( rAPI.Flags & sheet::ReferenceFlags::SHEET_DELETED   ) != 0 );
    rRef.SetFlag3D(     ( rAPI.Flags & sheet::ReferenceFlags::SHEET_3D        ) != 0 );
    rRef.SetRelName(    ( rAPI.Flags & sheet::ReferenceFlags::RELATIVE_NAME   ) != 0 );
}

void lcl_SingleRefToApi( sheet::SingleReference& rAPI, const SingleRefData& rRef )
{
    rAPI.Column         = rRef.nCol;
    rAPI.Row            = rRef.nRow;
    rAPI.Sheet          = rRef.nTab;
    rAPI.RelativeColumn = rRef.nRelCol;
    rAPI.RelativeRow    = rRef.nRelRow;
    rAPI.RelativeSheet  = rRef.nRelTab;

    sal_Int32 nFlags = 0;
    if ( rRef.IsColRel() )     nFlags |= sheet::ReferenceFlags::COLUMN_RELATIVE;
    if ( rRef.IsRowRel() )     nFlags |= sheet::ReferenceFlags::ROW_RELATIVE;
    if ( rRef.IsTabRel() )     nFlags |= sheet::ReferenceFlags::SHEET_RELATIVE;
    if ( rRef.IsColDeleted() ) nFlags |= sheet::ReferenceFlags::COLUMN_DELETED;
    if ( rRef.IsRowDeleted() ) nFlags |= sheet::ReferenceFlags::ROW_DELETED;
    if ( rRef.IsTabDeleted() ) nFlags |= sheet::ReferenceFlags::SHEET_DELETED;
    if ( rRef.IsFlag3D() )     nFlags |= sheet::ReferenceFlags::SHEET_3D;
    if ( rRef.IsRelName() )    nFlags |= sheet::ReferenceFlags::RELATIVE_NAME;
    rAPI.Flags = nFlags;
}

// static
bool ScTokenConversion::ConvertToTokenArray( ScTokenArray& rTokenArray,
                        const uno::Sequence<sheet::FormulaToken>& rSequence )
{
    bool bError = false;
    sal_Int32 nCount = rSequence.getLength();
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        const sheet::FormulaToken& rAPI = rSequence[nPos];
        OpCode eOpCode = static_cast<OpCode>(rAPI.OpCode);      //! assuming equal values for the moment

        uno::TypeClass eClass = rAPI.Data.getValueTypeClass();
        switch ( eClass )
        {
            case uno::TypeClass_VOID:
                // empty data -> use AddOpCode (does some special cases)
                rTokenArray.AddOpCode( eOpCode );
                break;
            case uno::TypeClass_DOUBLE:
                // double is only used for "push"
                if ( eOpCode == ocPush )
                    rTokenArray.AddDouble( rAPI.Data.get<double>() );
                else
                    bError = true;
                break;
            case uno::TypeClass_LONG:
                {
                    // long is svIndex, used for name / database area, or "byte" for spaces
                    sal_Int32 nValue = rAPI.Data.get<sal_Int32>();
                    if ( eOpCode == ocName || eOpCode == ocDBArea )
                        rTokenArray.AddToken( ScIndexToken( eOpCode, static_cast<USHORT>(nValue) ) );
                    else if ( eOpCode == ocSpaces )
                        rTokenArray.AddToken( ScByteToken( ocSpaces, static_cast<BYTE>(nValue) ) );
                    else
                        bError = true;
                }
                break;
            case uno::TypeClass_STRING:
                {
                    String aStrVal( rAPI.Data.get<rtl::OUString>() );
                    if ( eOpCode == ocPush )
                        rTokenArray.AddString( aStrVal );
                    else if ( eOpCode == ocBad )
                        rTokenArray.AddBad( aStrVal );
                    else if ( eOpCode == ocExternal || eOpCode == ocMacro )
                        rTokenArray.AddToken( ScExternalToken( eOpCode, aStrVal ) );
                    else
                        bError = true;      // unexpected string: don't know what to do with it
                }
                break;
            case uno::TypeClass_STRUCT:
                {
                    uno::Type aType = rAPI.Data.getValueType();
                    if ( aType.equals( cppu::UnoType<sheet::SingleReference>::get() ) )
                    {
                        SingleRefData aSingleRef;
                        sheet::SingleReference aApiRef;
                        rAPI.Data >>= aApiRef;
                        lcl_SingleRefToCalc( aSingleRef, aApiRef );

                        if ( eOpCode == ocPush )
                            rTokenArray.AddSingleReference( aSingleRef );
                        else if ( eOpCode == ocColRowName )
                            rTokenArray.AddColRowName( aSingleRef );
                        else
                            bError = true;
                    }
                    else if ( aType.equals( cppu::UnoType<sheet::ComplexReference>::get() ) )
                    {
                        ComplRefData aComplRef;
                        sheet::ComplexReference aApiRef;
                        rAPI.Data >>= aApiRef;
                        lcl_SingleRefToCalc( aComplRef.Ref1, aApiRef.Reference1 );
                        lcl_SingleRefToCalc( aComplRef.Ref2, aApiRef.Reference2 );

                        if ( eOpCode == ocPush )
                            rTokenArray.AddDoubleReference( aComplRef );
                        else
                            bError = true;
                    }
                    else
                        bError = true;      // unknown struct
                }
                break;
            case uno::TypeClass_SEQUENCE:
                {
                    if ( eOpCode != ocPush )
                        bError = true;      // not an inline array
                    else if (!rAPI.Data.getValueType().equals( getCppuType(
                                    (uno::Sequence< uno::Sequence< uno::Any > > *)0)))
                        bError = true;      // unexpected sequence type
                    else
                    {
                        ScMatrixRef xMat = ScSequenceToMatrix::CreateMixedMatrix( rAPI.Data);
                        if (xMat)
                            rTokenArray.AddMatrix( xMat);
                        else
                            bError = true;
                    }
                }
                break;
            default:
                bError = true;
        }
    }

    return !bError;
}

// static
bool ScTokenConversion::ConvertToTokenSequence( uno::Sequence<sheet::FormulaToken>& rSequence,
                        const ScTokenArray& rTokenArray )
{
    bool bError = false;

    sal_Int32 nLen = static_cast<sal_Int32>(rTokenArray.GetLen());
    ScToken** pTokens = rTokenArray.GetArray();
    if ( pTokens )
    {
        rSequence.realloc(nLen);
        for (sal_Int32 nPos=0; nPos<nLen; nPos++)
        {
            const ScToken& rToken = *pTokens[nPos];
            sheet::FormulaToken& rAPI = rSequence[nPos];

            OpCode eOpCode = rToken.GetOpCode();
            rAPI.OpCode = static_cast<sal_Int32>(eOpCode);      //! assuming equal values for the moment
            switch ( rToken.GetType() )
            {
                case svByte:
                    // Only the count of spaces is stored as "long". Parameter count is ignored.
                    if ( eOpCode == ocSpaces )
                        rAPI.Data <<= (sal_Int32) rToken.GetByte();
                    else
                        rAPI.Data.clear();      // no data
                    break;
                case svDouble:
                    rAPI.Data <<= rToken.GetDouble();
                    break;
                case svString:
                    rAPI.Data <<= rtl::OUString( rToken.GetString() );
                    break;
                case svExternal:
                    // Function name is stored as string.
                    // Byte (parameter count) is ignored.
                    rAPI.Data <<= rtl::OUString( rToken.GetExternal() );
                    break;
                case svSingleRef:
                    {
                        sheet::SingleReference aSingleRef;
                        lcl_SingleRefToApi( aSingleRef, rToken.GetSingleRef() );
                        rAPI.Data <<= aSingleRef;
                    }
                    break;
                case svDoubleRef:
                    {
                        sheet::ComplexReference aCompRef;
                        lcl_SingleRefToApi( aCompRef.Reference1, rToken.GetSingleRef() );
                        lcl_SingleRefToApi( aCompRef.Reference2, rToken.GetSingleRef2() );
                        rAPI.Data <<= aCompRef;
                    }
                    break;
                case svIndex:
                    rAPI.Data <<= static_cast<sal_Int32>( rToken.GetIndex() );
                    break;
                case svMatrix:
                    if (!ScRangeToSequence::FillMixedArray( rAPI.Data, rToken.GetMatrix(), true))
                        rAPI.Data.clear();
                    break;
                default:
                    rAPI.Data.clear();      // no data
            }
        }
    }
    else
        rSequence.realloc(0);

    return !bError;
}


//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScFormulaOpCodeMapperObj, "ScFormulaOpCodeMapperObj", SC_SERVICENAME_OPCODEMAPPER )

//------------------------------------------------------------------------

ScFormulaOpCodeMapperObj::ScFormulaOpCodeMapperObj()
{
}

ScFormulaOpCodeMapperObj::~ScFormulaOpCodeMapperObj()
{
}


::sal_Int32 SAL_CALL ScFormulaOpCodeMapperObj::getOpCodeExternal()
    throw (::com::sun::star::uno::RuntimeException)
{
    return ocExternal;
}


::sal_Int32 SAL_CALL ScFormulaOpCodeMapperObj::getOpCodeUnknown()
    throw (::com::sun::star::uno::RuntimeException)
{
    return ScCompiler::OpCodeMap::getOpCodeUnknown();
}


::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >
SAL_CALL ScFormulaOpCodeMapperObj::getMappings(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames,
        sal_Int32 nLanguage )
    throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException)
{
    ScCompiler::OpCodeMapPtr xMap = ScCompiler::GetOpCodeMap( nLanguage);
    if (!xMap)
        throw lang::IllegalArgumentException();
    return xMap->createSequenceOfFormulaTokens( rNames);
}


::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaOpCodeMapEntry >
SAL_CALL ScFormulaOpCodeMapperObj::getAvailableMappings(
        sal_Int32 nLanguage, sal_Int32 nGroups )
    throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException)
{
    ScCompiler::OpCodeMapPtr xMap = ScCompiler::GetOpCodeMap( nLanguage);
    if (!xMap)
        throw lang::IllegalArgumentException();
    return xMap->createSequenceOfAvailableMappings( nGroups);
}
