/*************************************************************************
 *
 *  $RCSfile: tokstack.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: er $ $Date: 2001-03-08 15:42:32 $
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

#ifndef _TOKSTACK_HXX
#define _TOKSTACK_HXX

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif


typedef OpCode DefTokenId;
// in PRODUCT version: ambiguity between OpCode (being USHORT) and UINT16
// Unfortunately a typedef is just a dumb alias and not a real type ...
//typedef UINT16 TokenId;
struct TokenId
{
        UINT16          nId;

                        TokenId() {}
                        TokenId( UINT16 n ) : nId( n ) {}
                        TokenId( const TokenId& r ) : nId( r.nId ) {}
    inline  TokenId&    operator =( const TokenId& r ) { nId = r.nId; return *this; }
    inline  TokenId&    operator =( UINT16 n ) { nId = n; return *this; }
    inline              operator UINT16&() { return nId; }
    inline              operator UINT16() const { return nId; }
    inline  BOOL        operator <( UINT16 n ) const { return nId < n; }
    inline  BOOL        operator >( UINT16 n ) const { return nId > n; }
    inline  BOOL        operator <=( UINT16 n ) const { return nId <= n; }
    inline  BOOL        operator >=( UINT16 n ) const { return nId >= n; }
    inline  BOOL        operator ==( UINT16 n ) const { return nId == n; }
    inline  BOOL        operator !=( UINT16 n ) const { return nId != n; }
};


//------------------------------------------------------------------------
struct ComplRefData;
class TokenStack;
class ScToken;


enum E_TYPE
{
    T_Id,   // Id-Folge
    T_Str,  // String
    T_D,    // Double
    T_RefC, // Cell Reference
    T_RefA, // Area Reference
    T_RN,   // Range Name
    T_Ext,  // irgendwas Unbekanntes mit Funktionsnamen
    T_Nlf,  // token for natural language formula
    T_Error // fuer Abfrage im Fehlerfall
};




class TokenPool
{
    // !ACHTUNG!: externe Id-Basis ist 1, interne 0!
    // Ausgabe Id = 0 -> Fehlerfall
    private:
        String**                    ppP_Str;    // Pool fuer Strings
        UINT16                      nP_Str;     // ...mit Groesse
        UINT16                      nP_StrAkt;  // ...und Schreibmarke

        double*                     pP_Dbl;     // Pool fuer Doubles
        UINT16                      nP_Dbl;
        UINT16                      nP_DblAkt;

        SingleRefData**             ppP_RefTr;  // Pool fuer Referenzen
        UINT16                      nP_RefTr;
        UINT16                      nP_RefTrAkt;

        UINT16*                     pP_Id;      // Pool fuer Id-Folgen
        UINT16                      nP_Id;
        UINT16                      nP_IdAkt;
        UINT16                      nP_IdLast;  // letzter Folgen-Beginn

        struct  EXTCONT
        {
            DefTokenId              eId;
            String                  aText;
                                    EXTCONT( const DefTokenId e, const String& r ) :
                                        eId( e ), aText( r ){}
        };
        EXTCONT**                   ppP_Ext;
        UINT16                      nP_Ext;
        UINT16                      nP_ExtAkt;

        struct  NLFCONT
        {
            SingleRefData           aRef;
                                    NLFCONT( const SingleRefData& r ) : aRef( r )   {}
        };
        NLFCONT**                   ppP_Nlf;
        UINT16                      nP_Nlf;
        UINT16                      nP_NlfAkt;

        UINT16*                     pElement;   // Array mit Indizes fuer Elemente
        E_TYPE*                     pType;      // ...mit Typ-Info
        UINT16*                     pSize;      // ...mit Laengenangabe (Anz. UINT16)
        UINT16                      nElement;
        UINT16                      nElementAkt;

        static const UINT16         nScTokenOff;// Offset fuer SC-Token
#ifdef DBG_UTIL
        UINT16                      nRek;       // Rekursionszaehler
#endif
        ScTokenArray*               pScToken;   // Tokenbastler

        void                        GrowString( void );
        void                        GrowDouble( void );
        void                        GrowTripel( void );
        void                        GrowId( void );
        void                        GrowElement( void );
        void                        GrowExt( void );
        void                        GrowNlf( void );
        void                        GetElement( const UINT16 nId );
        void                        GetElementRek( const UINT16 nId );
    public:
                                    TokenPool( void );
                                    ~TokenPool();
        inline TokenPool&           operator <<( const TokenId nId );
        inline TokenPool&           operator <<( const DefTokenId eId );
        inline TokenPool&           operator <<( TokenStack& rStack );
        void                        operator >>( TokenId& rId );
        inline void                 operator >>( TokenStack& rStack );
        inline TokenId              Store( void );
        TokenId                     Store( const double& rDouble );

                                    // nur fuer Range-Names
        TokenId                     Store( const UINT16 nIndex );
        inline TokenId              Store( const INT16 nWert );
        TokenId                     Store( const String& rString );
        TokenId                     Store( const SingleRefData& rTr );
        TokenId                     Store( const ComplRefData& rTr );

        TokenId                     Store( const DefTokenId eId, const String& rName );
                                        // 4 externals (e.g. AddIns, Makros...)
        TokenId                     StoreNlf( const SingleRefData& rTr );
        inline TokenId              LastId( void ) const;
        inline const ScTokenArray*  operator []( const TokenId nId );
        void                        Reset( void );
        inline E_TYPE               GetType( TokenId nId ) const;
        inline const SingleRefData* GetSRD( TokenId nId ) const;
        BOOL                        IsSingleOp( TokenId nId, const DefTokenId eId ) const;
        BOOL                        IsExternal( TokenId nId ) const;

        const String*               GetString( TokenId nId ) const;
};




class TokenStack
    // Stack fuer Token-Ids: Id 0 sollte reserviert bleiben als
    //  fehlerhafte Id, da z.B. Get() im Fehlerfall 0 liefert
{
    private:
        TokenId*                    pStack;     // Stack als Array
        UINT16                      nPos;       // Schreibmarke
        UINT16                      nSize;      // Erster Index ausserhalb des Stacks
    public:
                                    TokenStack( UINT16 nNewSize = 1024 );
                                    ~TokenStack();
        inline TokenStack&          operator <<( const TokenId nNewId );
        inline void                 operator >>( TokenId &rId );

        inline void                 Reset( void );

        inline TokenId              Get( void );
};




inline TokenId TokenStack::Get( void )
{
    DBG_ASSERT( nPos > 0,
        "*TokenStack::Get(): Leer ist leer, ist leer, ist leer, ist..." );

    register TokenId nRet;

    if( nPos == 0 )
        nRet = 0;
    else
    {
        nPos--;
        nRet = pStack[ nPos ];
    }

    return nRet;
}


inline TokenStack &TokenStack::operator <<( const TokenId nNewId )
{// Element auf Stack
    DBG_ASSERT( nPos < nSize, "*TokenStack::<<(): Stackueberlauf" );
    if( nPos < nSize )
    {
        pStack[ nPos ] = nNewId;
        nPos++;
    }

    return *this;
}


inline void TokenStack::operator >>( TokenId& rId )
{// Element von Stack
    DBG_ASSERT( nPos > 0,
        "*TokenStack::>>(): Leer ist leer, ist leer, ist leer, ..." );
    if( nPos > 0 )
    {
        nPos--;
        rId = pStack[ nPos ];
    }
}


inline void TokenStack::Reset( void )
{
    nPos = 0;
}




inline TokenPool& TokenPool::operator <<( const TokenId nId )
{
    // POST: nId's werden hintereinander im Pool unter einer neuen Id
    //       abgelegt. Vorgang wird mit >> oder Store() abgeschlossen
    // nId -> ( UINT16 ) nId - 1;
    DBG_ASSERT( ( UINT16 ) nId < nScTokenOff,
        "-TokenPool::operator <<: TokenId im DefToken-Bereich!" );

    if( nP_IdAkt >= nP_Id )
        GrowId();

    pP_Id[ nP_IdAkt ] = ( ( UINT16 ) nId ) - 1;
    nP_IdAkt++;

    return *this;
}


inline TokenPool& TokenPool::operator <<( const DefTokenId eId )
{
    DBG_ASSERT( ( UINT32 ) eId + nScTokenOff < 0xFFFF,
        "-TokenPool::operator<<: enmum zu gross!" );

    if( nP_IdAkt >= nP_Id )
        GrowId();

    pP_Id[ nP_IdAkt ] = ( ( UINT16 ) eId ) + nScTokenOff;
    nP_IdAkt++;

    return *this;
}


inline TokenPool& TokenPool::operator <<( TokenStack& rStack )
{
    if( nP_IdAkt >= nP_Id )
        GrowId();

    pP_Id[ nP_IdAkt ] = ( ( UINT16 ) rStack.Get() ) - 1;
    nP_IdAkt++;

    return *this;
}


inline void TokenPool::operator >>( TokenStack& rStack )
{
    register TokenId nId;
    *this >> nId;
    rStack << nId;
}


inline TokenId TokenPool::Store( void )
{
    register TokenId nId;
    *this >> nId;
    return nId;
}


inline TokenId TokenPool::Store( const INT16 nWert )
{
    return Store( ( double ) nWert );
}


inline TokenId TokenPool::LastId( void ) const
{
    return ( TokenId ) nElementAkt; // stimmt, da Ausgabe mit Offset 1!
}


const inline ScTokenArray* TokenPool::operator []( const TokenId nId )
{
    pScToken->Clear();

    if( nId )
    {//...nur wenn nId > 0!
#ifdef DBG_UTIL
        nRek = 0;
#endif
        GetElement( ( UINT16 ) nId - 1 );
    }

    return pScToken;
}


inline E_TYPE TokenPool::GetType( TokenId nId ) const
{
    register E_TYPE nRet;

    nId--;

    if( nId < nElementAkt )
        nRet = pType[ ( UINT16 ) nId ] ;
    else
        nRet = T_Error;

    return nRet;
}


inline const SingleRefData* TokenPool::GetSRD( TokenId nId ) const
{
    register SingleRefData* pRet;

    nId--;

    if( nId < nElementAkt && pType[ ( UINT16 ) nId ] == T_RefC )
        pRet = ppP_RefTr[ pElement[ ( UINT16 ) nId ] ];
    else
        pRet = NULL;

    return pRet;
}



#endif

