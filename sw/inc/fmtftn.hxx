/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _FMTFTN_HXX
#define _FMTFTN_HXX

#include <tools/string.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"

class SwDoc;
class SwTxtFtn;

// ATT_FTN **********************************************************

class SW_DLLPUBLIC SwFmtFtn: public SfxPoolItem
{
    friend class SwTxtFtn;
    SwTxtFtn* pTxtAttr;     //mein TextAttribut
    String  aNumber;        //Benutzerdefinierte 'Nummer'
    sal_uInt16  nNumber;        //Automatische Nummerierung
    bool    m_bEndNote;     // is it an End note?

    // geschuetzter CopyCtor
    SwFmtFtn& operator=(const SwFmtFtn& rFtn);
    SwFmtFtn( const SwFmtFtn& );

public:
    SwFmtFtn( bool bEndNote = false );
    virtual ~SwFmtFtn();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    const String &GetNumStr() const { return aNumber; }
    const sal_uInt16 &GetNumber() const { return nNumber; }
          bool    IsEndNote() const { return m_bEndNote;}

    void SetNumStr( const String& rStr )    { aNumber = rStr; }
    void SetNumber( sal_uInt16 nNo )            { nNumber = nNo; }
    void SetEndNote( bool b );

    void SetNumber( const SwFmtFtn& rFtn )
        { nNumber = rFtn.nNumber; aNumber = rFtn.aNumber; }

    const SwTxtFtn *GetTxtFtn() const   { return pTxtAttr; }
          SwTxtFtn *GetTxtFtn()         { return pTxtAttr; }

    void GetFtnText( String& rStr ) const;

    // returnt den anzuzeigenden String der Fuss-/Endnote
    String GetViewNumStr( const SwDoc& rDoc, sal_Bool bInclStrs = sal_False ) const;
};


#endif

