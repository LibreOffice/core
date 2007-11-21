/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: caption.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 18:23:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CAPTION_HXX
#define _CAPTION_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
//#ifndef _STREAM_HXX //autogen
//#include <tools/stream.hxx>
//#endif
#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
#include <SwCapObjType.hxx>

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC InsCaptionOpt
{
private:
    BOOL            bUseCaption;
    SwCapObjType    eObjType;
    SvGlobalName    aOleId;
    String          sCategory;
    USHORT          nNumType;
    ::rtl::OUString sNumberSeparator;
    String          sCaption;
    USHORT          nPos;
    USHORT          nLevel;
//  sal_Unicode     cSeparator;
    String          sSeparator;
    String          sCharacterStyle;

    BOOL         bIgnoreSeqOpts;    // wird nicht gespeichert
    BOOL         bCopyAttributes;   //          -""-

public:
    InsCaptionOpt(const SwCapObjType eType = FRAME_CAP, const SvGlobalName* pOleId = 0);
    InsCaptionOpt(const InsCaptionOpt&);
    ~InsCaptionOpt();

    inline BOOL&            UseCaption()                    { return bUseCaption; }
    inline BOOL             UseCaption() const              { return bUseCaption; }

    inline SwCapObjType     GetObjType() const              { return eObjType; }
    inline void             SetObjType(const SwCapObjType eType) { eObjType = eType; }

    inline const SvGlobalName&  GetOleId() const                { return aOleId; }
    inline void             SetOleId(const SvGlobalName &rId)   { aOleId = rId; }

    inline const String&    GetCategory() const             { return sCategory; }
    inline void             SetCategory(const String& rCat) { sCategory = rCat; }

    inline USHORT           GetNumType() const              { return nNumType; }
    inline void             SetNumType(const USHORT nNT)    { nNumType = nNT; }

    const ::rtl::OUString&  GetNumSeparator() const { return sNumberSeparator; }
    void                    SetNumSeparator(const ::rtl::OUString& rSet) {sNumberSeparator = rSet;}

    inline const String&    GetCaption() const              { return sCaption; }
    inline void             SetCaption(const String& rCap)  { sCaption = rCap; }

    inline USHORT           GetPos() const                  { return nPos; }
    inline void             SetPos(const USHORT nP)         { nPos = nP; }

    inline USHORT           GetLevel() const                { return nLevel; }
    inline void             SetLevel(const USHORT nLvl)     { nLevel = nLvl; }

//  inline sal_Unicode      GetSeparator() const                { return cSeparator; }
//  inline void             SetSeparator(const sal_Unicode cSep){ cSeparator = cSep; }
    inline const String&    GetSeparator() const                { return sSeparator; }
    inline void             SetSeparator(const String& rSep)    { sSeparator = rSep; }

    const String&           GetCharacterStyle() const { return sCharacterStyle; }
    void                    SetCharacterStyle(const String& rStyle)
                                    { sCharacterStyle = rStyle; }

    inline BOOL&            IgnoreSeqOpts()                 { return bIgnoreSeqOpts; }
    inline BOOL             IgnoreSeqOpts() const           { return bIgnoreSeqOpts; }

    inline BOOL&            CopyAttributes()                { return bCopyAttributes; }
    inline BOOL             CopyAttributes() const          { return bCopyAttributes; }

    BOOL                    operator==( const InsCaptionOpt& rOpt ) const;
    InsCaptionOpt&          operator= ( const InsCaptionOpt& rOpt );
    inline BOOL             operator< ( const InsCaptionOpt & rObj ) const
                                                { return aOleId < rObj.aOleId; }
//  friend SvStream&        operator>>( SvStream& rIStream, InsCaptionOpt& rCapOpt );
//  friend SvStream&        operator<<( SvStream& rOStream, const InsCaptionOpt& rCapOpt );
};

#endif


