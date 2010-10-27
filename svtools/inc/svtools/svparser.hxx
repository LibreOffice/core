/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _SVPARSER_HXX
#define _SVPARSER_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <tools/ref.hxx>
#include <rtl/textenc.h>
#include <boost/utility.hpp>


struct SvParser_Impl;
class SvStream;
class SvUShorts;

enum SvParserState
{
    SVPAR_ACCEPTED = 0,
    SVPAR_NOTSTARTED,
    SVPAR_WORKING,
    SVPAR_PENDING,
    SVPAR_WAITFORDATA,
    SVPAR_ERROR
};

class SVT_DLLPUBLIC SvParser : public SvRefBase
{
    DECL_STATIC_LINK( SvParser, NewDataRead, void* );

protected:
    SvStream&       rInput;
    String          aToken;             // gescanntes Token
    ULONG           nlLineNr;           // akt. Zeilen Nummer
    ULONG           nlLinePos;          // akt. Spalten Nummer

    SvParser_Impl   *pImplData;         // interne Daten
    long            nTokenValue;        // zusaetzlicher Wert (RTF)
    BOOL            bTokenHasValue;     // indicates whether nTokenValue is valid
    SvParserState   eState;             // Status auch in abgl. Klassen

    rtl_TextEncoding    eSrcEnc;        // Source encoding

    ULONG nNextChPos;
    sal_Unicode nNextCh;                // Akt. Zeichen fuer die "lex"


    BOOL            bDownloadingFile : 1;// TRUE: Es wird gerade ein externes
                                        //       File geladen. d.h. alle
                                        //       DataAvailable Links muessen
                                        //       ignoriert werden.
                                        // Wenn keibes der folgenden
                                        // Flags gesetzt ist, wird der
                                        // Stream als ANSI gelesen,
                                        // aber als CharSet DONTKNOW
                                        // zurueckgegeben.
    BOOL            bUCS2BSrcEnc : 1;   // oder als big-endian UCS2
    BOOL            bSwitchToUCS2 : 1;  // Umschalten des ist erlaubt

    BOOL            bRTF_InTextRead : 1;  // only for RTF-Parser!!!

    struct TokenStackType
    {
        String  sToken;
        long    nTokenValue;
        BOOL    bTokenHasValue;
        int     nTokenId;

        inline TokenStackType() { nTokenId = 0; }
        inline ~TokenStackType() { }
    };

    // Methoden fuer Token-Stack
    int SkipToken( short nCnt = -1 );       // n Tokens zurueck "skippen"
    TokenStackType* GetStackPtr( short nCnt );
    inline BYTE GetStackPos() const;

    // scanne das naechste Token:
    //  Tokenstack abarbeiten und ggfs. _GetNextToken() rufen. Diese
    //  ist fuers erkennen von neuen Token verantwortlich
    int GetNextToken();
    virtual int _GetNextToken() = 0;

    // wird fuer jedes Token gerufen, das in CallParser erkannt wird
    virtual void NextToken( int nToken );

    // zu Zeiten der SvRefBase-Ableitung darf nicht jeder loeschen
    virtual ~SvParser();

    void ClearTxtConvContext();

private:
    TokenStackType* pTokenStack;
    TokenStackType *pTokenStackPos;
    BYTE nTokenStackSize, nTokenStackPos;

public:
    // Konstruktor
    SvParser( SvStream& rIn, BYTE nStackSize = 3 );

    virtual  SvParserState CallParser() = 0;    // Aufruf des Parsers

    inline SvParserState GetStatus() const  { return eState; }  // StatusInfo

    inline ULONG    GetLineNr() const       { return nlLineNr; }
    inline ULONG    GetLinePos() const      { return nlLinePos; }
    inline ULONG    IncLineNr()             { return ++nlLineNr; }
    inline ULONG    IncLinePos()            { return ++nlLinePos; }
    inline ULONG    SetLineNr( ULONG nlNum );           // inline unten
    inline ULONG    SetLinePos( ULONG nlPos );          // inline unten

    sal_Unicode GetNextChar();
    void RereadLookahead();

    inline int  IsParserWorking() const { return SVPAR_WORKING == eState; }

    Link GetAsynchCallLink() const
        { return STATIC_LINK( this, SvParser, NewDataRead ); }

    long CallAsyncCallLink() { return NewDataRead( this, 0 ); }

    // fuers asynchrone lesen aus dem SvStream
    /*virtual*/ void SaveState( int nToken );
    /*virtual*/ void RestoreState();
    virtual void Continue( int nToken );

    inline void SetDownloadingFile( BOOL bSet ) { bDownloadingFile = bSet; }
    inline BOOL IsDownloadingFile() const { return bDownloadingFile; }

    // Set/get source encoding. The UCS2BEncoding flag is valid if source
    // encoding is UCS2. It specifies a big endian encoding.
    void SetSrcEncoding( rtl_TextEncoding eSrcEnc );
    rtl_TextEncoding GetSrcEncoding() const { return eSrcEnc; }

    void SetSrcUCS2BEncoding( BOOL bSet ) { bUCS2BSrcEnc = bSet; }
    BOOL IsSrcUCS2BEncoding() const { return bUCS2BSrcEnc; }

    // Darf der Zeichensatz auf UCS/2 umgeschaltet werden, wenn
    // in den ersten beiden Zeichen im Stream eine BOM steht?
    void SetSwitchToUCS2( BOOL bSet ) { bSwitchToUCS2 = bSet; }
    BOOL IsSwitchToUCS2() const { return bSwitchToUCS2; }

    // Aus wie vielen Bytes betseht ein Zeichen
    inline USHORT GetCharSize() const;

    int GetSaveToken() const;

    // Aufbau einer Which-Map 'rWhichMap' aus einem Array von
    // 'pWhichIds' von Which-Ids. Es hat die Lange 'nWhichIds'.
    // Die Which-Map wird nicht geloescht.
    static void BuildWhichTbl( SvUShorts &rWhichMap,
                               USHORT *pWhichIds,
                               USHORT nWhichIds );
};


#ifndef GOODIES_DECL_SVPARSER_DEFINED
#define GOODIES_DECL_SVPARSER_DEFINED
SV_DECL_REF(SvParser)
#endif
SV_IMPL_REF(SvParser)



inline ULONG SvParser::SetLineNr( ULONG nlNum )
{   ULONG nlOld = nlLineNr; nlLineNr = nlNum; return nlOld; }

inline ULONG SvParser::SetLinePos( ULONG nlPos )
{   ULONG nlOld = nlLinePos; nlLinePos = nlPos; return nlOld; }

inline BYTE SvParser::GetStackPos() const
{   return nTokenStackPos; }

inline USHORT SvParser::GetCharSize() const
{
    return (RTL_TEXTENCODING_UCS2 == eSrcEnc) ? 2 : 1;
}


/*========================================================================
 *
 * SvKeyValue.
 *
 *======================================================================*/

SV_DECL_REF(SvKeyValueIterator)

class SvKeyValue
{
    /** Representation.
    */
    String m_aKey;
    String m_aValue;

public:
    /** Construction.
    */
    SvKeyValue (void)
    {}

    SvKeyValue (const String &rKey, const String &rValue)
        : m_aKey (rKey), m_aValue (rValue)
    {}

    SvKeyValue (const SvKeyValue &rOther)
        : m_aKey (rOther.m_aKey), m_aValue (rOther.m_aValue)
    {}

    /** Assignment.
    */
    SvKeyValue& operator= (SvKeyValue &rOther)
    {
        m_aKey   = rOther.m_aKey;
        m_aValue = rOther.m_aValue;
        return *this;
    }

    /** Operation.
    */
    const String& GetKey   (void) const { return m_aKey; }
    const String& GetValue (void) const { return m_aValue; }

    void SetKey   (const String &rKey  ) { m_aKey = rKey; }
    void SetValue (const String &rValue) { m_aValue = rValue; }
};

/*========================================================================
 *
 * SvKeyValueIterator.
 *
 *======================================================================*/
class SvKeyValueList_Impl;
class SVT_DLLPUBLIC SvKeyValueIterator : public SvRefBase,
    private boost::noncopyable
{
    /** Representation.
    */
    SvKeyValueList_Impl* m_pList;
    USHORT               m_nPos;

public:
    /** Construction/Destruction.
    */
    SvKeyValueIterator (void);
    virtual ~SvKeyValueIterator (void);

    /** Operation.
    */
    virtual BOOL GetFirst (SvKeyValue &rKeyVal);
    virtual BOOL GetNext  (SvKeyValue &rKeyVal);
    virtual void Append   (const SvKeyValue &rKeyVal);
};

SV_IMPL_REF(SvKeyValueIterator);

#endif //_SVPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
