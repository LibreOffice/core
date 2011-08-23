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

#include "sbx.hxx"
/*?*/ // #include "sbcomp.hxx"
#include "image.hxx"
#include "buffer.hxx"
#include "opcodes.hxx"
#include "codegen.hxx"
#include <limits>

namespace binfilter {
    
template < class T >
class PCodeVisitor
{
public:
    virtual ~PCodeVisitor();

    virtual void start( BYTE* pStart ) = 0;
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
    BYTE* m_pCode;
    T readParam( BYTE*& pCode )
    {
        short nBytes = sizeof( T );
        T nOp1=0;
        for ( int i=0; i<nBytes; ++i )
            nOp1 |= *pCode++ << ( i * 8);
        return nOp1;
    }
public:
    PCodeBufferWalker( BYTE* pCode, T nBytes ): m_nBytes( nBytes ), m_pCode( pCode )
    {
    }
    void visitBuffer( PCodeVisitor< T >& visitor )
    {
        BYTE* pCode = m_pCode;
        if ( !pCode )
            return;
        BYTE* pEnd = pCode + m_nBytes;
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
    virtual void start( BYTE* /*pStart*/ ){}
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
    BYTE* m_pStart;
    SbiBuffer m_ConvertedBuf;
public:
    BufferTransformer():m_pStart(NULL), m_ConvertedBuf( NULL, 1024 ) {}
    virtual void start( BYTE* pStart ){ m_pStart = pStart; }
    virtual void processOpCode0( SbiOpcode eOp ) 
    {
        m_ConvertedBuf += (UINT8)eOp;
    }
    virtual void processOpCode1( SbiOpcode eOp, T nOp1 )
    {
        m_ConvertedBuf += (UINT8)eOp;
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
        m_ConvertedBuf += (UINT8)eOp;
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
    static S convertBufferOffSet( BYTE* pStart, T nOp1 )
    {
        PCodeBufferWalker< T > aBuff( pStart, nOp1);
        OffSetAccumulator< T, S > aVisitor;
        aBuff.visitBuffer( aVisitor ); 
        return aVisitor.offset();
    }
};

UINT32 
SbiCodeGen::calcNewOffSet( BYTE* pCode, UINT16 nOffset )
{
    return BufferTransformer< UINT16, UINT32 >::convertBufferOffSet( pCode, nOffset );
}

UINT16 
SbiCodeGen::calcLegacyOffSet( BYTE* pCode, UINT32 nOffset )
{
    return BufferTransformer< UINT32, UINT16 >::convertBufferOffSet( pCode, nOffset );
}

template <class T, class S>
void
PCodeBuffConvertor<T,S>::convert()
{
    PCodeBufferWalker< T > aBuf( m_pStart, m_nSize );
    BufferTransformer< T, S > aTrnsfrmer;
    aBuf.visitBuffer( aTrnsfrmer );	
    m_pCnvtdBuf = (BYTE*)aTrnsfrmer.buffer().GetBuffer();
    m_nCnvtdSize = static_cast<S>( aTrnsfrmer.buffer().GetSize() );
}

// instantiate for types needed in SbiImage::Load and SbiImage::Save
template class PCodeBuffConvertor<UINT16, UINT32 >;
template class PCodeBuffConvertor<UINT32, UINT16>;

}
