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


#include <basic/sbx.hxx>
#include <image.hxx>
#include <codegen.hxx>
#include <parser.hxx>
#include <cstddef>
#include <limits>
#include <algorithm>
#include <string_view>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/script/ModuleType.hpp>

// nInc is the increment size of the buffers

SbiCodeGen::SbiCodeGen( SbModule& r, SbiParser* p, short nInc )
         : rMod( r ), aCode( p, nInc )
{
    pParser = p;
    bStmnt = false;
    nLine = 0;
    nCol = 0;
    nForLevel = 0;
}

sal_uInt32 SbiCodeGen::GetPC()
{
    return aCode.GetSize();
}

// memorize the statement

void SbiCodeGen::Statement()
{
    if( pParser->IsCodeCompleting() )
        return;

    bStmnt = true;

    nLine = pParser->GetLine();
    nCol  = pParser->GetCol1();

    // #29955 Store the information of the for-loop-layer
    // in the upper Byte of the column
    nCol = (nCol & 0xff) + 0x100 * nForLevel;
}

// Mark the beginning of a statement

void SbiCodeGen::GenStmnt()
{
    if( pParser->IsCodeCompleting() )
        return;

    if( bStmnt )
    {
        bStmnt = false;
        Gen( SbiOpcode::STMNT_, nLine, nCol );
    }
}

// The Gen-Routines return the offset of the 1. operand,
// so that jumps can sink their backchain there.

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode )
{
    if( pParser->IsCodeCompleting() )
        return 0;

#ifdef DBG_UTIL
    if( eOpcode < SbiOpcode::SbOP0_START || eOpcode > SbiOpcode::SbOP0_END )
        pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, "OPCODE1" );
#endif
    GenStmnt();
    aCode += static_cast<sal_uInt8>(eOpcode);
    return GetPC();
}

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode, sal_uInt32 nOpnd )
{
    if( pParser->IsCodeCompleting() )
        return 0;

#ifdef DBG_UTIL
    if( eOpcode < SbiOpcode::SbOP1_START || eOpcode > SbiOpcode::SbOP1_END )
        pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, "OPCODE2" );
#endif
    GenStmnt();
    aCode += static_cast<sal_uInt8>(eOpcode);
    sal_uInt32 n = GetPC();
    aCode += nOpnd;
    return n;
}

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode, sal_uInt32 nOpnd1, sal_uInt32 nOpnd2 )
{
    if( pParser->IsCodeCompleting() )
        return 0;

#ifdef DBG_UTIL
    if( eOpcode < SbiOpcode::SbOP2_START || eOpcode > SbiOpcode::SbOP2_END )
        pParser->Error( ERRCODE_BASIC_INTERNAL_ERROR, "OPCODE3" );
#endif
    GenStmnt();
    aCode += static_cast<sal_uInt8>(eOpcode);
    sal_uInt32 n = GetPC();
    aCode += nOpnd1;
    aCode += nOpnd2;
    return n;
}

// Storing of the created image in the module

void SbiCodeGen::Save()
{
    if( pParser->IsCodeCompleting() )
        return;

    std::unique_ptr<SbiImage> p( new SbiImage );
    rMod.StartDefinitions();
    // OPTION BASE-Value:
    p->nDimBase = pParser->nBase;
    // OPTION take over the EXPLICIT-Flag
    if( pParser->bExplicit )
        p->SetFlag( SbiImageFlags::EXPLICIT );

    int nIfaceCount = 0;
    if( rMod.mnType == css::script::ModuleType::CLASS )
    {
        rMod.bIsProxyModule = true;
        p->SetFlag( SbiImageFlags::CLASSMODULE );
        GetSbData()->pClassFac->AddClassModule( &rMod );

        nIfaceCount = pParser->aIfaceVector.size();
        if( !rMod.pClassData )
            rMod.pClassData.reset( new SbClassData );
        if( nIfaceCount )
        {
            for( int i = 0 ; i < nIfaceCount ; i++ )
            {
                const OUString& rIfaceName = pParser->aIfaceVector[i];
                SbxVariable* pIfaceVar = new SbxVariable( SbxVARIANT );
                pIfaceVar->SetName( rIfaceName );
                SbxArray* pIfaces = rMod.pClassData->mxIfaces.get();
                pIfaces->Insert( pIfaceVar, pIfaces->Count() );
            }
        }

        rMod.pClassData->maRequiredTypes = pParser->aRequiredTypes;
    }
    else
    {
        GetSbData()->pClassFac->RemoveClassModule( &rMod );
        // Only a ClassModule can revert to Normal
        if ( rMod.mnType == css::script::ModuleType::CLASS )
        {
            rMod.mnType = css::script::ModuleType::NORMAL;
        }
        rMod.bIsProxyModule = false;
    }

    // GlobalCode-Flag
    if( pParser->HasGlobalCode() )
    {
        p->SetFlag( SbiImageFlags::INITCODE );
    }
    // The entry points:
    for( SbiSymDef* pDef = pParser->aPublics.First(); pDef;
         pDef = pParser->aPublics.Next() )
    {
        SbiProcDef* pProc = pDef->GetProcDef();
        if( pProc && pProc->IsDefined() )
        {
            OUString aProcName = pProc->GetName();
            OUStringBuffer aIfaceProcName;
            OUString aIfaceName;
            sal_uInt16 nPassCount = 1;
            if( nIfaceCount )
            {
                int nPropPrefixFound = aProcName.indexOf("Property ");
                OUString aPureProcName = aProcName;
                OUString aPropPrefix;
                if( nPropPrefixFound == 0 )
                {
                    aPropPrefix = aProcName.copy( 0, 13 );      // 13 == Len( "Property ?et " )
                    aPureProcName = aProcName.copy( 13 );
                }
                for( int i = 0 ; i < nIfaceCount ; i++ )
                {
                    const OUString& rIfaceName = pParser->aIfaceVector[i];
                    int nFound = aPureProcName.indexOf( rIfaceName );
                    if( nFound == 0 && aPureProcName[rIfaceName.getLength()] == '_' )
                    {
                        if( nPropPrefixFound == 0 )
                        {
                            aIfaceProcName.append(aPropPrefix);
                        }
                        aIfaceProcName.append(std::u16string_view(aPureProcName).substr(rIfaceName.getLength() + 1) );
                        aIfaceName = rIfaceName;
                        nPassCount = 2;
                        break;
                    }
                }
            }
            SbMethod* pMeth = nullptr;
            for( sal_uInt16 nPass = 0 ; nPass < nPassCount ; nPass++ )
            {
                if( nPass == 1 )
                {
                    aProcName = aIfaceProcName.toString();
                }
                PropertyMode ePropMode = pProc->getPropertyMode();
                if( ePropMode != PropertyMode::NONE )
                {
                    SbxDataType ePropType = SbxEMPTY;
                    switch( ePropMode )
                    {
                    case PropertyMode::Get:
                        ePropType = pProc->GetType();
                        break;
                    case PropertyMode::Let:
                    {
                        // type == type of first parameter
                        ePropType = SbxVARIANT;     // Default
                        SbiSymPool* pPool = &pProc->GetParams();
                        if( pPool->GetSize() > 1 )
                        {
                            SbiSymDef* pPar = pPool->Get( 1 );
                            if( pPar )
                            {
                                ePropType = pPar->GetType();
                            }
                        }
                        break;
                    }
                    case PropertyMode::Set:
                        ePropType = SbxOBJECT;
                        break;
                    default:
                        OSL_FAIL("Illegal PropertyMode");
                        break;
                    }
                    OUString aPropName = pProc->GetPropName();
                    if( nPass == 1 )
                    {
                        aPropName = aPropName.copy( aIfaceName.getLength() + 1 );
                    }
                    rMod.GetProcedureProperty( aPropName, ePropType );
                }
                if( nPass == 1 )
                {
                    rMod.GetIfaceMapperMethod( aProcName, pMeth );
                }
                else
                {
                    pMeth = rMod.GetMethod( aProcName, pProc->GetType() );

                    if( !pProc->IsPublic() )
                    {
                        pMeth->SetFlag( SbxFlagBits::Private );
                    }
                    // Declare? -> Hidden
                    if( !pProc->GetLib().isEmpty())
                    {
                        pMeth->SetFlag( SbxFlagBits::Hidden );
                    }
                    pMeth->nStart = pProc->GetAddr();
                    pMeth->nLine1 = pProc->GetLine1();
                    pMeth->nLine2 = pProc->GetLine2();
                    // The parameter:
                    SbxInfo* pInfo = pMeth->GetInfo();
                    OUString aHelpFile, aComment;
                    sal_uInt32 nHelpId = 0;
                    if( pInfo )
                    {
                        // Rescue the additional data
                        aHelpFile = pInfo->GetHelpFile();
                        aComment  = pInfo->GetComment();
                        nHelpId   = pInfo->GetHelpId();
                    }
                    // And reestablish the parameter list
                    pInfo = new SbxInfo( aHelpFile, nHelpId );
                    pInfo->SetComment( aComment );
                    SbiSymPool* pPool = &pProc->GetParams();
                    // The first element is always the value of the function!
                    for( sal_uInt16 i = 1; i < pPool->GetSize(); i++ )
                    {
                        SbiSymDef* pPar = pPool->Get( i );
                        SbxDataType t = pPar->GetType();
                        if( !pPar->IsByVal() )
                        {
                            t = static_cast<SbxDataType>( t | SbxBYREF );
                        }
                        if( pPar->GetDims() )
                        {
                            t = static_cast<SbxDataType>( t | SbxARRAY );
                        }
                        // #33677 hand-over an Optional-Info
                        SbxFlagBits nFlags = SbxFlagBits::Read;
                        if( pPar->IsOptional() )
                        {
                            nFlags |= SbxFlagBits::Optional;
                        }
                        pInfo->AddParam( pPar->GetName(), t, nFlags );

                        sal_uInt32 nUserData = 0;
                        sal_uInt16 nDefaultId = pPar->GetDefaultId();
                        if( nDefaultId )
                        {
                            nUserData |= nDefaultId;
                        }
                        if( pPar->IsParamArray() )
                        {
                            nUserData |= PARAM_INFO_PARAMARRAY;
                        }
                        if( pPar->IsWithBrackets() )
                        {
                            nUserData |= PARAM_INFO_WITHBRACKETS;
                        }
                        SbxParamInfo* pParam = nullptr;
                        if( nUserData )
                        {
                            pParam = const_cast<SbxParamInfo*>(pInfo->GetParam( i ));
                        }
                        if( pParam )
                        {
                            pParam->nUserData = nUserData;
                        }
                    }
                    pMeth->SetInfo( pInfo );
                }
            }   // for( iPass...
        }
    }
    // The code
    p->AddCode( std::unique_ptr<char[]>(aCode.GetBuffer()), aCode.GetSize() );

    // The global StringPool. 0 is not occupied.
    SbiStringPool* pPool = &pParser->aGblStrings;
    sal_uInt16 nSize = pPool->GetSize();
    p->MakeStrings( nSize );
    sal_uInt16 i;
    for( i = 1; i <= nSize; i++ )
    {
        p->AddString( pPool->Find( i ) );
    }
    // Insert types
    sal_uInt16 nCount = pParser->rTypeArray->Count();
    for (i = 0; i < nCount; i++)
    {
         p->AddType(static_cast<SbxObject *>(pParser->rTypeArray->Get(i)));
    }
    // Insert enum objects
    nCount = pParser->rEnumArray->Count();
    for (i = 0; i < nCount; i++)
    {
         p->AddEnum(static_cast<SbxObject *>(pParser->rEnumArray->Get(i)));
    }
    if( !p->IsError() )
    {
        rMod.pImage = std::move(p);
    }
    rMod.EndDefinitions();
}

template < class T >
class PCodeVisitor
{
public:
    virtual ~PCodeVisitor();

    virtual void start( const sal_uInt8* pStart ) = 0;
    virtual void processOpCode0( SbiOpcode eOp ) = 0;
    virtual void processOpCode1( SbiOpcode eOp, T nOp1 ) = 0;
    virtual void processOpCode2( SbiOpcode eOp, T nOp1, T nOp2 ) = 0;
    virtual bool processParams() = 0;
};

template <class T> PCodeVisitor< T >::~PCodeVisitor()
{}

template <class T>
class PCodeBufferWalker
{
private:
    T  m_nBytes;
    const sal_uInt8* m_pCode;
    static T readParam( sal_uInt8 const *& pCode )
    {
        T nOp1=0;
        for ( std::size_t i=0; i<sizeof( T ); ++i )
            nOp1 |= *pCode++ << ( i * 8);
        return nOp1;
    }
public:
    PCodeBufferWalker( const sal_uInt8* pCode, T nBytes ): m_nBytes( nBytes ), m_pCode( pCode )
    {
    }
    void visitBuffer( PCodeVisitor< T >& visitor )
    {
        const sal_uInt8* pCode = m_pCode;
        if ( !pCode )
            return;
        const sal_uInt8* pEnd = pCode + m_nBytes;
        visitor.start( m_pCode );
        T nOp1 = 0, nOp2 = 0;
        for( ; pCode < pEnd; )
        {
            SbiOpcode eOp = static_cast<SbiOpcode>(*pCode++);

            if ( eOp <= SbiOpcode::SbOP0_END )
                visitor.processOpCode0( eOp );
            else if( eOp >= SbiOpcode::SbOP1_START && eOp <= SbiOpcode::SbOP1_END )
            {
                if ( visitor.processParams() )
                    nOp1 = readParam( pCode );
                else
                    pCode += sizeof( T );
                visitor.processOpCode1( eOp, nOp1 );
            }
            else if( eOp >= SbiOpcode::SbOP2_START && eOp <= SbiOpcode::SbOP2_END )
            {
                if ( visitor.processParams() )
                {
                    nOp1 = readParam( pCode );
                    nOp2 = readParam( pCode );
                }
                else
                    pCode += ( sizeof( T ) * 2 );
                visitor.processOpCode2( eOp, nOp1, nOp2 );
            }
        }
    }
};

template < class T, class S >
class OffSetAccumulator : public PCodeVisitor< T >
{
    T m_nNumOp0;
    T m_nNumSingleParams;
    T m_nNumDoubleParams;
public:

    OffSetAccumulator() : m_nNumOp0(0), m_nNumSingleParams(0), m_nNumDoubleParams(0){}
    virtual void start( const sal_uInt8* /*pStart*/ ) override {}
    virtual void processOpCode0( SbiOpcode /*eOp*/ ) override { ++m_nNumOp0; }
    virtual void processOpCode1( SbiOpcode /*eOp*/, T /*nOp1*/ ) override {  ++m_nNumSingleParams; }
    virtual void processOpCode2( SbiOpcode /*eOp*/, T /*nOp1*/, T /*nOp2*/ ) override { ++m_nNumDoubleParams; }
    S offset()
    {
        typedef decltype(T(1) + S(1)) larger_t; // type capable to hold both value ranges of T and S
        T result = 0 ;
        static const S max = std::numeric_limits< S >::max();
        result = m_nNumOp0 + ( ( sizeof(S) + 1 ) * m_nNumSingleParams ) + ( (( sizeof(S) * 2 )+ 1 )  * m_nNumDoubleParams );
        return std::min<larger_t>(max, result);
    }
    virtual bool processParams() override { return false; }
};


template < class T, class S >
class BufferTransformer : public PCodeVisitor< T >
{
    const sal_uInt8* m_pStart;
    SbiBuffer m_ConvertedBuf;
public:
    BufferTransformer():m_pStart(nullptr), m_ConvertedBuf( nullptr, 1024 ) {}
    virtual void start( const sal_uInt8* pStart ) override { m_pStart = pStart; }
    virtual void processOpCode0( SbiOpcode eOp ) override
    {
        m_ConvertedBuf += static_cast<sal_uInt8>(eOp);
    }
    virtual void processOpCode1( SbiOpcode eOp, T nOp1 ) override
    {
        m_ConvertedBuf += static_cast<sal_uInt8>(eOp);
        switch( eOp )
        {
            case SbiOpcode::JUMP_:
            case SbiOpcode::JUMPT_:
            case SbiOpcode::JUMPF_:
            case SbiOpcode::GOSUB_:
            case SbiOpcode::CASEIS_:
            case SbiOpcode::RETURN_:
            case SbiOpcode::ERRHDL_:
            case SbiOpcode::TESTFOR_:
                nOp1 = static_cast<T>( convertBufferOffSet(m_pStart, nOp1) );
                break;
            case SbiOpcode::RESUME_:
                if ( nOp1 > 1 )
                    nOp1 = static_cast<T>( convertBufferOffSet(m_pStart, nOp1) );
                break;
            default:
                break;

        }
        m_ConvertedBuf += static_cast<S>(nOp1);
    }
    virtual void processOpCode2( SbiOpcode eOp, T nOp1, T nOp2 ) override
    {
        m_ConvertedBuf += static_cast<sal_uInt8>(eOp);
        if ( eOp == SbiOpcode::CASEIS_  && nOp1 )
            nOp1 = static_cast<T>( convertBufferOffSet(m_pStart, nOp1) );
        m_ConvertedBuf += static_cast<S>(nOp1);
        m_ConvertedBuf += static_cast<S>(nOp2);

    }
    virtual bool processParams() override { return true; }
    // yeuch, careful here, you can only call
    // GetBuffer on the returned SbiBuffer once, also
    // you (as the caller) get to own the memory
    SbiBuffer& buffer()
    {
        return m_ConvertedBuf;
    }
    static S convertBufferOffSet( const sal_uInt8* pStart, T nOp1 )
    {
        PCodeBufferWalker< T > aBuff( pStart, nOp1);
        OffSetAccumulator< T, S > aVisitor;
        aBuff.visitBuffer( aVisitor );
        return aVisitor.offset();
    }
};

sal_uInt32
SbiCodeGen::calcNewOffSet( sal_uInt8 const * pCode, sal_uInt16 nOffset )
{
    return BufferTransformer< sal_uInt16, sal_uInt32 >::convertBufferOffSet( pCode, nOffset );
}

sal_uInt16
SbiCodeGen::calcLegacyOffSet( sal_uInt8 const * pCode, sal_uInt32 nOffset )
{
    return BufferTransformer< sal_uInt32, sal_uInt16 >::convertBufferOffSet( pCode, nOffset );
}

template <class T, class S>
void
PCodeBuffConvertor<T,S>::convert()
{
    PCodeBufferWalker< T > aBuf( m_pStart, m_nSize );
    BufferTransformer< T, S > aTrnsfrmer;
    aBuf.visitBuffer( aTrnsfrmer );
    m_pCnvtdBuf = reinterpret_cast<sal_uInt8*>(aTrnsfrmer.buffer().GetBuffer());
    m_nCnvtdSize = static_cast<S>( aTrnsfrmer.buffer().GetSize() );
}

template class PCodeBuffConvertor< sal_uInt16, sal_uInt32 >;
template class PCodeBuffConvertor< sal_uInt32, sal_uInt16 >;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
