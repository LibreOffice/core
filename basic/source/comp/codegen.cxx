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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include "image.hxx"
#include <limits>
#include <com/sun/star/script/ModuleType.hpp>

// nInc ist die Inkrementgroesse der Puffer

SbiCodeGen::SbiCodeGen( SbModule& r, SbiParser* p, short nInc )
         : rMod( r ), aCode( p, nInc )
{
    pParser = p;
    bStmnt = sal_False;
    nLine = 0;
    nCol = 0;
    nForLevel = 0;
}

sal_uInt32 SbiCodeGen::GetPC()
{
    return aCode.GetSize();
}

// Statement merken

void SbiCodeGen::Statement()
{
    bStmnt = sal_True;

    nLine = pParser->GetLine();
    nCol  = pParser->GetCol1();

    // #29955 Information der for-Schleifen-Ebene
    // in oberen Byte der Spalte speichern
    nCol = (nCol & 0xff) + 0x100 * nForLevel;
}

// Anfang eines Statements markieren

void SbiCodeGen::GenStmnt()
{
    if( bStmnt )
    {
        bStmnt = sal_False;
        Gen( _STMNT, nLine, nCol );
    }
}

// Die Gen-Routinen returnen den Offset des 1. Operanden,
// damit Jumps dort ihr Backchain versenken koennen

sal_uInt32 SbiCodeGen::Gen( SbiOpcode eOpcode )
{
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

// Abspeichern des erzeugten Images im Modul

void SbiCodeGen::Save()
{
    SbiImage* p = new SbiImage;
    rMod.StartDefinitions();
    // OPTION BASE-Wert:
    p->nDimBase = pParser->nBase;
    // OPTION EXPLICIT-Flag uebernehmen
    if( pParser->bExplicit )
        p->SetFlag( SBIMG_EXPLICIT );

    int nIfaceCount = 0;
    if( rMod.mnType == com::sun::star::script::ModuleType::CLASS )
    {
                OSL_TRACE("COdeGen::save() classmodule processing");
        rMod.bIsProxyModule = true;
        p->SetFlag( SBIMG_CLASSMODULE );
        pCLASSFAC->AddClassModule( &rMod );

        nIfaceCount = pParser->aIfaceVector.size();
        if( !rMod.pClassData )
            rMod.pClassData = new SbClassData;
        if( nIfaceCount )
        {
            for( int i = 0 ; i < nIfaceCount ; i++ )
            {
                const String& rIfaceName = pParser->aIfaceVector[i];
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
        pCLASSFAC->RemoveClassModule( &rMod );
        // Only a ClassModule can revert to Normal
                if ( rMod.mnType == com::sun::star::script::ModuleType::CLASS )
            rMod.mnType = com::sun::star::script::ModuleType::NORMAL;
        rMod.bIsProxyModule = false;
    }

    if( pParser->bText )
        p->SetFlag( SBIMG_COMPARETEXT );
    // GlobalCode-Flag
    if( pParser->HasGlobalCode() )
        p->SetFlag( SBIMG_INITCODE );
    // Die Entrypoints:
    for( SbiSymDef* pDef = pParser->aPublics.First(); pDef;
                   pDef = pParser->aPublics.Next() )
    {
        SbiProcDef* pProc = pDef->GetProcDef();
        if( pProc && pProc->IsDefined() )
        {
            String aProcName = pProc->GetName();
            String aIfaceProcName;
            String aIfaceName;
            sal_uInt16 nPassCount = 1;
            if( nIfaceCount )
            {
                int nPropPrefixFound =
                    aProcName.Search( String( RTL_CONSTASCII_USTRINGPARAM("Property ") ) );
                String aPureProcName = aProcName;
                String aPropPrefix;
                if( nPropPrefixFound == 0 )
                {
                    aPropPrefix = aProcName.Copy( 0, 13 );      // 13 == Len( "Property ?et " )
                    aPureProcName = aProcName.Copy( 13 );
                }
                for( int i = 0 ; i < nIfaceCount ; i++ )
                {
                    const String& rIfaceName = pParser->aIfaceVector[i];
                    int nFound = aPureProcName.Search( rIfaceName );
                    if( nFound == 0 && '_' == aPureProcName.GetChar( rIfaceName.Len() ) )
                    {
                        if( nPropPrefixFound == 0 )
                            aIfaceProcName += aPropPrefix;
                        aIfaceProcName += aPureProcName.Copy( rIfaceName.Len() + 1 );
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
                    aProcName = aIfaceProcName;

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
                                    ePropType = pPar->GetType();
                            }
                            break;
                        }
                        case PROPERTY_MODE_SET:
                            ePropType = SbxOBJECT;
                            break;
                        case PROPERTY_MODE_NONE:
                            DBG_ERROR( "Illegal PropertyMode PROPERTY_MODE_NONE" );
                            break;
                    }
                    String aPropName = pProc->GetPropName();
                    if( nPass == 1 )
                        aPropName = aPropName.Copy( aIfaceName.Len() + 1 );
                    SbProcedureProperty* pProcedureProperty = NULL;
                    pProcedureProperty = rMod.GetProcedureProperty( aPropName, ePropType );
                }
                if( nPass == 1 )
                {
                    SbIfaceMapperMethod* pMapperMeth = NULL;
                    pMapperMeth = rMod.GetIfaceMapperMethod( aProcName, pMeth );
                }
                else
                {
                    pMeth = rMod.GetMethod( aProcName, pProc->GetType() );

                    // #110004
                    if( !pProc->IsPublic() )
                        pMeth->SetFlag( SBX_PRIVATE );

                    // Declare? -> Hidden
                    if( pProc->GetLib().Len() > 0 )
                        pMeth->SetFlag( SBX_HIDDEN );

                    pMeth->nStart = pProc->GetAddr();
                    pMeth->nLine1 = pProc->GetLine1();
                    pMeth->nLine2 = pProc->GetLine2();
                    // Die Parameter:
                    SbxInfo* pInfo = pMeth->GetInfo();
                    String aHelpFile, aComment;
                    sal_uIntPtr nHelpId = 0;
                    if( pInfo )
                    {
                        // Die Zusatzdaten retten
                        aHelpFile = pInfo->GetHelpFile();
                        aComment  = pInfo->GetComment();
                        nHelpId   = pInfo->GetHelpId();
                    }
                    // Und die Parameterliste neu aufbauen
                    pInfo = new SbxInfo( aHelpFile, nHelpId );
                    pInfo->SetComment( aComment );
                    SbiSymPool* pPool = &pProc->GetParams();
                    // Das erste Element ist immer der Funktionswert!
                    for( sal_uInt16 i = 1; i < pPool->GetSize(); i++ )
                    {
                        SbiSymDef* pPar = pPool->Get( i );
                        SbxDataType t = pPar->GetType();
                        if( !pPar->IsByVal() )
                            t = (SbxDataType) ( t | SbxBYREF );
                        if( pPar->GetDims() )
                            t = (SbxDataType) ( t | SbxARRAY );
                        // #33677 Optional-Info durchreichen
                        sal_uInt16 nFlags = SBX_READ;
                        if( pPar->IsOptional() )
                            nFlags |= SBX_OPTIONAL;

                        pInfo->AddParam( pPar->GetName(), t, nFlags );

                        sal_uInt32 nUserData = 0;
                        sal_uInt16 nDefaultId = pPar->GetDefaultId();
                        if( nDefaultId )
                            nUserData |= nDefaultId;
                        if( pPar->IsParamArray() )
                            nUserData |= PARAM_INFO_PARAMARRAY;
                        if( pPar->IsWithBrackets() )
                            nUserData |= PARAM_INFO_WITHBRACKETS;
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
    // Der Code
    p->AddCode( aCode.GetBuffer(), aCode.GetSize() );

    // Der globale StringPool. 0 ist nicht belegt.
    SbiStringPool* pPool = &pParser->aGblStrings;
    sal_uInt16 nSize = pPool->GetSize();
    p->MakeStrings( nSize );
    sal_uInt16 i;
    for( i = 1; i <= nSize; i++ )
        p->AddString( pPool->Find( i ) );

    // Typen einfuegen
    sal_uInt16 nCount = pParser->rTypeArray->Count();
    for (i = 0; i < nCount; i++)
         p->AddType((SbxObject *)pParser->rTypeArray->Get(i));

    // Insert enum objects
    nCount = pParser->rEnumArray->Count();
    for (i = 0; i < nCount; i++)
         p->AddEnum((SbxObject *)pParser->rEnumArray->Get(i));

    if( !p->IsError() )
        rMod.pImage = p;
    else
        delete p;

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
        if ( result > max )
            return max;

        return static_cast<S>(result);
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
