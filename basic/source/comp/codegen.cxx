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
#include "sbcomp.hxx"
#include "image.hxx"
#include <limits>
#include <algorithm>
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
    if( aMiscOptions.IsExperimentalMode() && pParser->IsCodeCompleting() )
        return;

    bStmnt = true;

    nLine = pParser->GetLine();
    nCol  = pParser->GetCol1();

    // #29955 Store the information of the for-loop-layer
    // in the uppper Byte of the column
    nCol = (nCol & 0xff) + 0x100 * nForLevel;
}

// Mark the beginning of a statement

void SbiCodeGen::GenStmnt()
{
    if( aMiscOptions.IsExperimentalMode() && pParser->IsCodeCompleting() )
        return;

    if( bStmnt )
    {
        bStmnt = false;
        Gen( _STMNT, nLine, nCol );
    }
}

// The Gen-Routines return the offset of the 1. operand,
// so that jumps can sink their backchain there.

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode )
{
    if( aMiscOptions.IsExperimentalMode() && pParser->IsCodeCompleting() )
        return 0;

#ifdef DBG_UTIL
    if( eOpcode < SbOP0_START || eOpcode > SbOP0_END )
        pParser->Error( SbERR_INTERNAL_ERROR, "OPCODE1" );
#endif
    GenStmnt();
    aCode += (sal_uInt8) eOpcode;
    return GetPC();
}

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode, sal_uInt32 nOpnd )
{
    if( aMiscOptions.IsExperimentalMode() && pParser->IsCodeCompleting() )
        return 0;

#ifdef DBG_UTIL
    if( eOpcode < SbOP1_START || eOpcode > SbOP1_END )
        pParser->Error( SbERR_INTERNAL_ERROR, "OPCODE2" );
#endif
    GenStmnt();
    aCode += (sal_uInt8) eOpcode;
    sal_uInt32 n = GetPC();
    aCode += nOpnd;
    return n;
}

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode, sal_uInt32 nOpnd1, sal_uInt32 nOpnd2 )
{
    if( aMiscOptions.IsExperimentalMode() && pParser->IsCodeCompleting() )
        return 0;

#ifdef DBG_UTIL
    if( eOpcode < SbOP2_START || eOpcode > SbOP2_END )
        pParser->Error( SbERR_INTERNAL_ERROR, "OPCODE3" );
#endif
    GenStmnt();
    aCode += (sal_uInt8) eOpcode;
    sal_uInt32 n = GetPC();
    aCode += nOpnd1;
    aCode += nOpnd2;
    return n;
}

// Storing of the created image in the module

void SbiCodeGen::Save()
{
    if( aMiscOptions.IsExperimentalMode() && pParser->IsCodeCompleting() )
        return;

    SbiImage* p = new SbiImage;
    rMod.StartDefinitions();
    // OPTION BASE-Value:
    p->nDimBase = pParser->nBase;
    // OPTION take over the EXPLICIT-Flag
    if( pParser->bExplicit )
        p->SetFlag( SBIMG_EXPLICIT );

    int nIfaceCount = 0;
    if( rMod.mnType == com::sun::star::script::ModuleType::CLASS )
    {
                OSL_TRACE("COdeGen::save() classmodule processing");
        rMod.bIsProxyModule = true;
        p->SetFlag( SBIMG_CLASSMODULE );
        GetSbData()->pClassFac->AddClassModule( &rMod );

        nIfaceCount = pParser->aIfaceVector.size();
        if( !rMod.pClassData )
            rMod.pClassData = new SbClassData;
        if( nIfaceCount )
        {
            for( int i = 0 ; i < nIfaceCount ; i++ )
            {
                const OUString& rIfaceName = pParser->aIfaceVector[i];
                SbxVariable* pIfaceVar = new SbxVariable( SbxVARIANT );
                pIfaceVar->SetName( rIfaceName );
                SbxArray* pIfaces = rMod.pClassData->mxIfaces;
                pIfaces->Insert( pIfaceVar, pIfaces->Count() );
            }
        }

        rMod.pClassData->maRequiredTypes = pParser->aRequiredTypes;
    }
    else
    {
        GetSbData()->pClassFac->RemoveClassModule( &rMod );
        // Only a ClassModule can revert to Normal
        if ( rMod.mnType == com::sun::star::script::ModuleType::CLASS )
        {
            rMod.mnType = com::sun::star::script::ModuleType::NORMAL;
        }
        rMod.bIsProxyModule = false;
    }

    // GlobalCode-Flag
    if( pParser->HasGlobalCode() )
    {
        p->SetFlag( SBIMG_INITCODE );
    }
    // Die Entrypoints:
    for( SbiSymDef* pDef = pParser->aPublics.First(); pDef;
         pDef = pParser->aPublics.Next() )
    {
        SbiProcDef* pProc = pDef->GetProcDef();
        if( pProc && pProc->IsDefined() )
        {
            OUString aProcName = pProc->GetName();
            OUString aIfaceProcName;
            OUString aIfaceName;
            sal_uInt16 nPassCount = 1;
            if( nIfaceCount )
            {
                int nPropPrefixFound = aProcName.indexOf(OUString("Property "));
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
                    if( nFound == 0 && '_' == aPureProcName[rIfaceName.getLength()] )
                    {
                        if( nPropPrefixFound == 0 )
                        {
                            aIfaceProcName += aPropPrefix;
                        }
                        aIfaceProcName += aPureProcName.copy( rIfaceName.getLength() + 1 );
                        aIfaceName = rIfaceName;
                        nPassCount = 2;
                        break;
                    }
                }
            }
            SbMethod* pMeth = NULL;
            for( sal_uInt16 nPass = 0 ; nPass < nPassCount ; nPass++ )
            {
                if( nPass == 1 )
                {
                    aProcName = aIfaceProcName;
                }
                PropertyMode ePropMode = pProc->getPropertyMode();
                if( ePropMode != PROPERTY_MODE_NONE )
                {
                    SbxDataType ePropType = SbxEMPTY;
                    switch( ePropMode )
                    {
                    case PROPERTY_MODE_GET:
                        ePropType = pProc->GetType();
                        break;
                    case PROPERTY_MODE_LET:
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
                    case PROPERTY_MODE_SET:
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
                    OSL_TRACE("*** getProcedureProperty for thing %s",
                              OUStringToOString( aPropName,RTL_TEXTENCODING_UTF8 ).getStr() );
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
                        pMeth->SetFlag( SBX_PRIVATE );
                    }
                    // Declare? -> Hidden
                    if( !pProc->GetLib().isEmpty())
                    {
                        pMeth->SetFlag( SBX_HIDDEN );
                    }
                    pMeth->nStart = pProc->GetAddr();
                    pMeth->nLine1 = pProc->GetLine1();
                    pMeth->nLine2 = pProc->GetLine2();
                    // The parameter:
                    SbxInfo* pInfo = pMeth->GetInfo();
                    OUString aHelpFile, aComment;
                    sal_uIntPtr nHelpId = 0;
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
                            t = (SbxDataType) ( t | SbxBYREF );
                        }
                        if( pPar->GetDims() )
                        {
                            t = (SbxDataType) ( t | SbxARRAY );
                        }
                        // #33677 hand-over an Optional-Info
                        sal_uInt16 nFlags = SBX_READ;
                        if( pPar->IsOptional() )
                        {
                            nFlags |= SBX_OPTIONAL;
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
                        if( nUserData )
                        {
                            SbxParamInfo* pParam = (SbxParamInfo*)pInfo->GetParam( i );
                            pParam->nUserData = nUserData;
                        }
                    }
                    pMeth->SetInfo( pInfo );
                }
            }   // for( iPass...
        }
    }
    // The code
    p->AddCode( aCode.GetBuffer(), aCode.GetSize() );

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
         p->AddType((SbxObject *)pParser->rTypeArray->Get(i));
    }
    // Insert enum objects
    nCount = pParser->rEnumArray->Count();
    for (i = 0; i < nCount; i++)
    {
         p->AddEnum((SbxObject *)pParser->rEnumArray->Get(i));
    }
    if( !p->IsError() )
    {
        rMod.pImage = p;
    }
    else
    {
        delete p;
    }
    rMod.EndDefinitions();
}

template < class T >
class PCodeVisitor
{
public:
    virtual ~PCodeVisitor();

    virtual void start( sal_uInt8* pStart ) = 0;
    virtual void processOpCode0( SbiOpcode eOp ) = 0;
    virtual void processOpCode1( SbiOpcode eOp, T nOp1 ) = 0;
    virtual void processOpCode2( SbiOpcode eOp, T nOp1, T nOp2 ) = 0;
    virtual bool processParams() = 0;
    virtual void end() = 0;
};

template <class T> PCodeVisitor< T >::~PCodeVisitor()
{}

template <class T>
class PCodeBufferWalker
{
private:
    T  m_nBytes;
    sal_uInt8* m_pCode;
    T readParam( sal_uInt8*& pCode )
    {
        short nBytes = sizeof( T );
        T nOp1=0;
        for ( int i=0; i<nBytes; ++i )
            nOp1 |= *pCode++ << ( i * 8);
        return nOp1;
    }
public:
    PCodeBufferWalker( sal_uInt8* pCode, T nBytes ): m_nBytes( nBytes ), m_pCode( pCode )
    {
    }
    void visitBuffer( PCodeVisitor< T >& visitor )
    {
        sal_uInt8* pCode = m_pCode;
        if ( !pCode )
            return;
        sal_uInt8* pEnd = pCode + m_nBytes;
        visitor.start( m_pCode );
        T nOp1 = 0, nOp2 = 0;
        for( ; pCode < pEnd; )
        {
            SbiOpcode eOp = (SbiOpcode)(*pCode++);

            if ( eOp <= SbOP0_END )
                visitor.processOpCode0( eOp );
            else if( eOp >= SbOP1_START && eOp <= SbOP1_END )
            {
                if ( visitor.processParams() )
                    nOp1 = readParam( pCode );
                else
                    pCode += sizeof( T );
                visitor.processOpCode1( eOp, nOp1 );
            }
            else if( eOp >= SbOP2_START && eOp <= SbOP2_END )
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
        visitor.end();
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
    virtual void start( sal_uInt8* /*pStart*/ ){}
    virtual void processOpCode0( SbiOpcode /*eOp*/ ){ ++m_nNumOp0; }
    virtual void processOpCode1( SbiOpcode /*eOp*/, T /*nOp1*/ ){  ++m_nNumSingleParams; }
    virtual void processOpCode2( SbiOpcode /*eOp*/, T /*nOp1*/, T /*nOp2*/ ) { ++m_nNumDoubleParams; }
    virtual void end(){}
    S offset()
    {
        T result = 0 ;
        static const S max = std::numeric_limits< S >::max();
        result = m_nNumOp0 + ( ( sizeof(S) + 1 ) * m_nNumSingleParams ) + ( (( sizeof(S) * 2 )+ 1 )  * m_nNumDoubleParams );
        return std::min(static_cast<T>(max), result);
    }
   virtual bool processParams(){ return false; }
};



template < class T, class S >

class BufferTransformer : public PCodeVisitor< T >
{
    sal_uInt8* m_pStart;
    SbiBuffer m_ConvertedBuf;
public:
    BufferTransformer():m_pStart(NULL), m_ConvertedBuf( NULL, 1024 ) {}
    virtual void start( sal_uInt8* pStart ){ m_pStart = pStart; }
    virtual void processOpCode0( SbiOpcode eOp )
    {
        m_ConvertedBuf += (sal_uInt8)eOp;
    }
    virtual void processOpCode1( SbiOpcode eOp, T nOp1 )
    {
        m_ConvertedBuf += (sal_uInt8)eOp;
        switch( eOp )
        {
            case _JUMP:
            case _JUMPT:
            case _JUMPF:
            case _GOSUB:
            case _CASEIS:
            case _RETURN:
            case _ERRHDL:
            case _TESTFOR:
                nOp1 = static_cast<T>( convertBufferOffSet(m_pStart, nOp1) );
                break;
            case _RESUME:
                if ( nOp1 > 1 )
                    nOp1 = static_cast<T>( convertBufferOffSet(m_pStart, nOp1) );
                break;
            default:
                break; //

        }
        m_ConvertedBuf += (S)nOp1;
    }
    virtual void processOpCode2( SbiOpcode eOp, T nOp1, T nOp2 )
    {
        m_ConvertedBuf += (sal_uInt8)eOp;
        if ( eOp == _CASEIS )
                if ( nOp1 )
                    nOp1 = static_cast<T>( convertBufferOffSet(m_pStart, nOp1) );
        m_ConvertedBuf += (S)nOp1;
        m_ConvertedBuf += (S)nOp2;

    }
    virtual bool processParams(){ return true; }
    virtual void end() {}
    // yeuch, careful here, you can only call
    // GetBuffer on the returned SbiBuffer once, also
    // you (as the caller) get to own the memory
    SbiBuffer& buffer()
    {
        return m_ConvertedBuf;
    }
    static S convertBufferOffSet( sal_uInt8* pStart, T nOp1 )
    {
        PCodeBufferWalker< T > aBuff( pStart, nOp1);
        OffSetAccumulator< T, S > aVisitor;
        aBuff.visitBuffer( aVisitor );
        return aVisitor.offset();
    }
};

sal_uInt32
SbiCodeGen::calcNewOffSet( sal_uInt8* pCode, sal_uInt16 nOffset )
{
    return BufferTransformer< sal_uInt16, sal_uInt32 >::convertBufferOffSet( pCode, nOffset );
}

sal_uInt16
SbiCodeGen::calcLegacyOffSet( sal_uInt8* pCode, sal_uInt32 nOffset )
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
    m_pCnvtdBuf = (sal_uInt8*)aTrnsfrmer.buffer().GetBuffer();
    m_nCnvtdSize = static_cast<S>( aTrnsfrmer.buffer().GetSize() );
}

template class PCodeBuffConvertor< sal_uInt16, sal_uInt32 >;
template class PCodeBuffConvertor< sal_uInt32, sal_uInt16 >;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
