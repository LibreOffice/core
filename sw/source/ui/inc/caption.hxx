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
#ifndef _CAPTION_HXX
#define _CAPTION_HXX

#include <tools/string.hxx>
//#ifndef _STREAM_HXX //autogen
//#include <tools/stream.hxx>
//#endif
#include <tools/globname.hxx>
#include <SwCapObjType.hxx>
#include "swdllapi.h"

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC InsCaptionOpt
{
private:
    sal_Bool            bUseCaption;
    SwCapObjType    eObjType;
    SvGlobalName    aOleId;
    String          sCategory;
    sal_uInt16          nNumType;
    ::rtl::OUString sNumberSeparator;
    String          sCaption;
    sal_uInt16          nPos;
    sal_uInt16          nLevel;
//  sal_Unicode     cSeparator;
    String          sSeparator;
    String          sCharacterStyle;

    sal_Bool         bIgnoreSeqOpts;    // wird nicht gespeichert
    sal_Bool         bCopyAttributes;   //          -""-

public:
    InsCaptionOpt(const SwCapObjType eType = FRAME_CAP, const SvGlobalName* pOleId = 0);
    InsCaptionOpt(const InsCaptionOpt&);
    ~InsCaptionOpt();

    inline sal_Bool&            UseCaption()                    { return bUseCaption; }
    inline sal_Bool             UseCaption() const              { return bUseCaption; }

    inline SwCapObjType     GetObjType() const              { return eObjType; }
    inline void             SetObjType(const SwCapObjType eType) { eObjType = eType; }

    inline const SvGlobalName&  GetOleId() const                { return aOleId; }
    inline void             SetOleId(const SvGlobalName &rId)   { aOleId = rId; }

    inline const String&    GetCategory() const             { return sCategory; }
    inline void             SetCategory(const String& rCat) { sCategory = rCat; }

    inline sal_uInt16           GetNumType() const              { return nNumType; }
    inline void             SetNumType(const sal_uInt16 nNT)    { nNumType = nNT; }

    const ::rtl::OUString&  GetNumSeparator() const { return sNumberSeparator; }
    void                    SetNumSeparator(const ::rtl::OUString& rSet) {sNumberSeparator = rSet;}

    inline const String&    GetCaption() const              { return sCaption; }
    inline void             SetCaption(const String& rCap)  { sCaption = rCap; }

    inline sal_uInt16           GetPos() const                  { return nPos; }
    inline void             SetPos(const sal_uInt16 nP)         { nPos = nP; }

    inline sal_uInt16           GetLevel() const                { return nLevel; }
    inline void             SetLevel(const sal_uInt16 nLvl)     { nLevel = nLvl; }

//  inline sal_Unicode      GetSeparator() const                { return cSeparator; }
//  inline void             SetSeparator(const sal_Unicode cSep){ cSeparator = cSep; }
    inline const String&    GetSeparator() const                { return sSeparator; }
    inline void             SetSeparator(const String& rSep)    { sSeparator = rSep; }

    const String&           GetCharacterStyle() const { return sCharacterStyle; }
    void                    SetCharacterStyle(const String& rStyle)
                                    { sCharacterStyle = rStyle; }

    inline sal_Bool&            IgnoreSeqOpts()                 { return bIgnoreSeqOpts; }
    inline sal_Bool             IgnoreSeqOpts() const           { return bIgnoreSeqOpts; }

    inline sal_Bool&            CopyAttributes()                { return bCopyAttributes; }
    inline sal_Bool             CopyAttributes() const          { return bCopyAttributes; }

    sal_Bool                    operator==( const InsCaptionOpt& rOpt ) const;
    InsCaptionOpt&          operator= ( const InsCaptionOpt& rOpt );
    inline sal_Bool             operator< ( const InsCaptionOpt & rObj ) const
                                                { return aOleId < rObj.aOleId; }
//  friend SvStream&        operator>>( SvStream& rIStream, InsCaptionOpt& rCapOpt );
//  friend SvStream&        operator<<( SvStream& rOStream, const InsCaptionOpt& rCapOpt );
};

#endif


