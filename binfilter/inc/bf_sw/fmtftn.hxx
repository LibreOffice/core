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
#ifndef _FMTFTN_HXX
#define _FMTFTN_HXX


#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
namespace binfilter {

class SwDoc;
class SwTxtFtn;

// ATT_FTN **********************************************************

class SwFmtFtn: public SfxPoolItem
{
    friend class SwTxtFtn;
    SwTxtFtn* pTxtAttr;		//mein TextAttribut
    String  aNumber;		//Benutzerdefinierte 'Nummer'
    USHORT  nNumber;		//Automatische Nummerierung
    BOOL	bEndNote;		//Ist es eine Endnote?

    // geschuetzter CopyCtor
    SwFmtFtn& operator=(const SwFmtFtn& rFtn);
    SwFmtFtn( const SwFmtFtn& );

public:
    SwFmtFtn( BOOL bEndNote = FALSE );
    ~SwFmtFtn();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;

    const String &GetNumStr() const { return aNumber; }
    const USHORT &GetNumber() const { return nNumber; }
          BOOL	  IsEndNote() const	{ return bEndNote;}

    void SetNumStr( const String& rStr )	{ aNumber = rStr; }
    void SetNumber( USHORT nNo ) 			{ nNumber = nNo; }
    void SetEndNote( BOOL b );

    void SetNumber( const SwFmtFtn& rFtn )
        { nNumber = rFtn.nNumber; aNumber = rFtn.aNumber; }

    const SwTxtFtn *GetTxtFtn() const   { return pTxtAttr; }
          SwTxtFtn *GetTxtFtn()	  		{ return pTxtAttr; }


    // returnt den anzuzeigenden String der Fuss-/Endnote
    String GetViewNumStr( const SwDoc& rDoc, BOOL bInclStrs = FALSE ) const;
};


} //namespace binfilter
#endif

