/*************************************************************************
 *
 *  $RCSfile: biffdump.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
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



#ifndef _BIFFDUMP_HXX
#define _BIFFDUMP_HXX


#ifdef DEBUG
#define DEBUGGING__________
#endif

#ifdef DEBUGGING


#include <tools/string.hxx>
#include <tools/list.hxx>
#include <so3/svstor.hxx>

#include "excform.hxx"
#include "root.hxx"


#define MODE_DUMP               0x0000
#define MODE_SKIP               0x0001

#define MODE_HEX                0x0002
#define MODE_BODY               0x0000
#define MODE_NAMEONLY           0x0008

#define MODE_PLAIN              0x0000
#define MODE_DETAIL             0x0004


#define CT_EOL                  0x01
#define CT_NUM                  0x02
#define CT_ALPHA                0x04
#define CT_ALPHANUM             0x08
#define CT_HEX                  0x10
#define CT_LOWERALPHA           0x20
#define CT_UPPERALPHA           0x40
#define CT_BLANK                0x80


#define DELANDNULL(p)           {delete p;p=NULL;}

inline void CopyStrpOnStrp( ByteString*& rpToString, const ByteString* pFromString );


struct DUMP_ERR
{
    DUMP_ERR*           pNext;
    UINT32              nLine;
    ByteString          aText;
    ByteString*         pHint;

    inline              DUMP_ERR( const UINT32 nLine, const ByteString& rText, const ByteString& rHint );
    inline              DUMP_ERR( const UINT32 nLine, const ByteString& rText );
                        ~DUMP_ERR();
};




enum _KEYWORD           { KW_Unknown, Skipdump, Contload, Parsep, Maxbodylines, Include, Exclude,
                            Hex, Body, Comment, Output, Title, NameOnly, ClearFile, SkipOffset };




struct IdRange
{
    UINT16                      nFirst;
    UINT16                      nLast;
    IdRange( const UINT16 nF, const UINT16 nL ) : nFirst( nF ), nLast( nL ) {}
};




class IdRangeList : protected List
{
private:
protected:
public:
    virtual                 ~IdRangeList();

    List::Count;

    inline const IdRange*   First( void );
    inline const IdRange*   Next( void );
    inline const IdRange*   Get( const UINT32 n ) const;

    inline void             Append( const UINT16 nFirst, const UINT16 nNext );

    void                    Clear( void );
    };




class Biff8RecDumper : public ExcRoot
{
private:
protected:
    static const sal_Char*      pLevelPreString;
    static const sal_Char*      pLevelPreStringNT;
    const sal_Char*             pLevelPre;
    UINT16                      nLevelCnt;

    ByteString*                 pTitle;
    ByteString*                 pOutName;

    SvFileStream*               pDumpStream;
    SvStream*                   pIn;
    SvStorage*                  pPivotCache;

    UINT32                      nMaxBodyLines;
    BOOL                        bEndLoading;
    BOOL                        bSkip;
    BOOL                        bSkipOffset;
    BOOL                        bClearFile;

    static const UINT16         nRecCnt;
    UINT16*                     pDumpModes;
    ByteString**                ppRecNames;

    static UINT32               nInstances;

    static sal_Char*            pBlankLine;
    static const UINT16         nLenBlankLine;

    static UINT8*               pCharType;
    static UINT8*               pCharVal;

    DUMP_ERR*                   pFirst;
    DUMP_ERR*                   pLast;
    DUMP_ERR*                   pAct;

    sal_Char                    cParSep;
    sal_Char                    cComm1;
    sal_Char                    cComm2;

    void                        Print( const ByteString& rStr );
    void                        Print( const sal_Char* pStr );
    void                        DumpPivotCache( const UINT16 nStrId );
    UINT16                      DumpXF( SvStream& rIn, const sal_Char* pPre );
    void                        DumpValidPassword( SvStream& rIn, const sal_Char* pPre );
    void                        RecDump( const UINT16 nR, const UINT16 nL );
    void                        EscherDump( const UINT16 nL );
    void                        ObjDump( const UINT16 nL );
    void                        ContDump( const UINT16 nL );
    void                        FormulaDump( const UINT16 nL, const FORMULA_TYPE eFT );
    static const sal_Char*      GetBlanks( const UINT16 nNumOfBlanks );
    static BOOL                 IsLineEnd( const sal_Char c, sal_Char& rNext, SvStream& rIn, INT32& rLeft );
    void                        Init( void );
    static _KEYWORD             GetKeyType( const ByteString& rString );
    BOOL                        ExecCommand( const UINT32 nLine, const ByteString& rCommand,
                                                const ByteString* pVal = NULL );
    BOOL                        ExecSetVal( const UINT32 nLine, const ByteString& rId,
                                            const ByteString* pName, const ByteString* pInExClude,
                                            const ByteString* pHexBody );

    inline void                 SetMode( const UINT16 nRecNum, const UINT16 nMode );
    inline UINT16               GetMode( const UINT16 nRecNum ) const;
    inline UINT16*              GetModeRef( const UINT16 nRecNum ) const;
    inline BOOL                 HasMode( const UINT16 nRecNum, const UINT16 nRefMode ) const;
    inline BOOL                 HasModeDump( const UINT16 nRecNum ) const;
    inline BOOL                 HasModeSkip( const UINT16 nRecNum ) const;
    inline BOOL                 HasModeHex( const UINT16 nRecNum ) const;
    inline BOOL                 HasModeBody( const UINT16 nRecNum ) const;
    inline BOOL                 HasModePlain( const UINT16 nRecNum ) const;
    inline BOOL                 HasModeDetail( const UINT16 nRecNum ) const;
    inline BOOL                 HasModeNameOnly( const UINT16 nRecNum ) const;
    void                        SetFlag( const UINT16 nFirst, const UINT16 nLast, const UINT16 nFlags );
    void                        ClrFlag( const UINT16 nFirst, const UINT16 nLast, const UINT16 nFlags );

    inline void                 SetName( const UINT16 nRecNum, const sal_Char* pName );
    inline void                 SetName( const UINT16 nRecNum, const ByteString& rName );
    void                        SetName( const UINT16 nRecNum, ByteString* pName );
    inline const ByteString*    GetName( const UINT16 nRecNum );

    inline static BOOL          IsNum( const sal_Char c );
    inline static BOOL          IsAlpha( const sal_Char c );
    inline static BOOL          IsAlphaNum( const sal_Char c );
    inline static BOOL          IsHex( const sal_Char c );
    inline static BOOL          IsEndOfLine( const sal_Char c );
    inline static BOOL          IsBlank( const sal_Char c );
    static UINT32               GetVal( const ByteString& rString );
    BOOL                        FillIdRangeList( const UINT32 nLine, IdRangeList& rRangeList,
                                                const ByteString& rVals );
    inline static UINT8         GetVal( const sal_Char c );
    BOOL                        CreateOutStream( const UINT32 nL = 0xFFFFFFFF );
    static SvFileStream*        CreateInStream( const sal_Char* pName );
    static SvFileStream*        CreateInStream( const sal_Char* pPath, const sal_Char* pName );

    void                        AddError( const UINT32 nLine, const ByteString& rText, const ByteString& rHint );
    inline void                 AddError( const UINT32 nLine, const sal_Char* pText, const ByteString& rHint );
    inline void                 AddError( const UINT32 nLine, const ByteString& rText );
    inline void                 AddError( const UINT32 nLine, const sal_Char* pText );
    inline const DUMP_ERR*      FirstErr( void );
    inline const DUMP_ERR*      NextErr( void );
public:
                                Biff8RecDumper( RootData& rRootData );
                                ~Biff8RecDumper();
    BOOL                        Dump( SvStream& rIn );
                                // = TRUE -> nicht weiter laden

    inline static BOOL          IsPrintable( const UINT8 nC );
    };




inline void CopyStrpOnStrp( ByteString*& rp, const ByteString* p )
{
    if( p )
    {
        if( rp )
            *rp = *p;
        else
            rp = new ByteString( *p );
    }
    else if( rp )
    {
        delete rp;
        rp = NULL;
    }
}




inline DUMP_ERR::DUMP_ERR( const UINT32 n, const ByteString& rT, const ByteString& rH ) :
    nLine( n ), aText( rT ), pNext( NULL )
{
    pHint = new ByteString( rH );
}


inline DUMP_ERR::DUMP_ERR( const UINT32 n, const ByteString& rT ) :
    nLine( n ), aText( rT ), pHint( NULL ), pNext( NULL )
{
}




inline const IdRange* IdRangeList::First( void )
{
    return ( const IdRange* ) List::First();
}


inline const IdRange* IdRangeList::Next( void )
{
    return ( const IdRange* ) List::Next();
}


inline const IdRange* IdRangeList::Get( const UINT32 n ) const
{
    return ( const IdRange* ) List::GetObject( n );
}


inline void IdRangeList::Append( const UINT16 n1, const UINT16 n2 )
{
    List::Insert( new IdRange( n1, n2 ), LIST_APPEND );
}




inline void Biff8RecDumper::SetMode( const UINT16 n, const UINT16 nM )
{
    if( n < nRecCnt )
        pDumpModes[ n ] = nM;
}


inline UINT16 Biff8RecDumper::GetMode( const UINT16 n ) const
{
    if( n < nRecCnt )
        return pDumpModes[ n ];
    else
        return 0xFFFF;
}


inline UINT16* Biff8RecDumper::GetModeRef( const UINT16 n ) const
{
    if( n < nRecCnt )
        return pDumpModes + n;
    else
        return NULL;
}


inline BOOL Biff8RecDumper::HasMode( const UINT16 n, const UINT16 nM ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & nM ) == nM;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModeDump( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_SKIP ) == 0;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModeSkip( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_SKIP ) != 0;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModeHex( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_HEX ) != 0;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModeBody( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_HEX ) == 0;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModePlain( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_DETAIL ) == 0;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModeDetail( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_DETAIL ) != 0;
    else
        return FALSE;
}


inline BOOL Biff8RecDumper::HasModeNameOnly( const UINT16 n ) const
{
    if( n < nRecCnt )
        return ( pDumpModes[ n ] & MODE_NAMEONLY ) != 0;
    else
        return FALSE;
}


inline void Biff8RecDumper::SetName( const UINT16 n, const sal_Char* p )
{
    if( n < nRecCnt )
    {
        if( ppRecNames[ n ] )
            *ppRecNames[ n ] = p;
        else
            ppRecNames[ n ] = new ByteString( p );
    }
}


inline void Biff8RecDumper::SetName( const UINT16 n, const ByteString& r )
{
    if( n < nRecCnt )
    {
        if( ppRecNames[ n ] )
            *ppRecNames[ n ] = r;
        else
            ppRecNames[ n ] = new ByteString( r );
    }
}


inline const ByteString* Biff8RecDumper::GetName( const UINT16 n )
{
    if( n < nRecCnt )
        return ppRecNames[ n ];
    else
        return NULL;
}


inline BOOL Biff8RecDumper::IsNum( const sal_Char c )
{
    return ( pCharType[ ( UINT8 ) c ] & CT_NUM ) != 0;
}


inline BOOL Biff8RecDumper::IsAlpha( const sal_Char c )
{
    return ( pCharType[ ( UINT8 ) c ] & CT_ALPHA ) != 0;
}


inline BOOL Biff8RecDumper::IsAlphaNum( const sal_Char c )
{
    return ( pCharType[ ( UINT8 ) c ] & CT_ALPHANUM ) != 0;
}


inline BOOL Biff8RecDumper::IsHex( const sal_Char c )
{
    return ( pCharType[ ( UINT8 ) c ] & CT_HEX ) != 0;
}


inline BOOL Biff8RecDumper::IsEndOfLine( const sal_Char c )
{
    return ( pCharType[ ( UINT8 ) c ] & CT_EOL ) != 0;
}


inline BOOL Biff8RecDumper::IsBlank( const sal_Char c )
{
    return ( pCharType[ ( UINT8 ) c ] & CT_BLANK ) != 0;
}


inline UINT8 Biff8RecDumper::GetVal( const sal_Char c )
{
    return pCharVal[ ( UINT8 ) c ];
}


inline void Biff8RecDumper::AddError( const UINT32 n, const sal_Char* p, const ByteString& r )
{
    AddError( n, ByteString( p ), r );
}


inline void Biff8RecDumper::AddError( const UINT32 nLine, const ByteString& rText )
{
}


inline void Biff8RecDumper::AddError( const UINT32 nLine, const sal_Char* pText )
{
}


inline const DUMP_ERR* Biff8RecDumper::FirstErr( void )
{
    pAct = pFirst;
    return pAct;
}


inline const DUMP_ERR* Biff8RecDumper::NextErr( void )
{
    if( pAct )
        pAct = pAct->pNext;

    return pAct;
}


inline BOOL Biff8RecDumper::IsPrintable( const UINT8 n )
{
    return n >= ' ';
}


#endif

#endif




