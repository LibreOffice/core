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

#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
namespace binfilter {

enum SwCapObjType
{
    FRAME_CAP, GRAPHIC_CAP, TABLE_CAP, OLE_CAP
};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class InsCaptionOpt
{
private:
    BOOL		 bUseCaption;
    SwCapObjType eObjType;
    SvGlobalName aOleId;
    String		 sCategory;
    USHORT		 nNumType;
    String		 sCaption;
    USHORT		 nPos;
    USHORT		 nLevel;
    sal_Unicode	 cSeparator;
    BOOL		 bIgnoreSeqOpts;	// wird nicht gespeichert
    BOOL		 bCopyAttributes;   // 			-""-

public:
    InsCaptionOpt(const SwCapObjType eType = FRAME_CAP, const SvGlobalName* pOleId = 0);
    ~InsCaptionOpt();

    inline BOOL&			UseCaption()					{ return bUseCaption; }
    inline BOOL				UseCaption() const				{ return bUseCaption; }

    inline SwCapObjType		GetObjType() const				{ return eObjType; }
    inline void				SetObjType(const SwCapObjType eType) { eObjType = eType; }

    inline const SvGlobalName&	GetOleId() const				{ return aOleId; }
    inline void				SetOleId(const SvGlobalName &rId)	{ aOleId = rId; }

    inline const String&	GetCategory() const				{ return sCategory; }
    inline void				SetCategory(const String& rCat) { sCategory = rCat; }

    inline USHORT			GetNumType() const				{ return nNumType; }
    inline void				SetNumType(const USHORT nNT)	{ nNumType = nNT; }

    inline const String&	GetCaption() const				{ return sCaption; }
    inline void				SetCaption(const String& rCap)	{ sCaption = rCap; }

    inline USHORT			GetPos() const					{ return nPos; }
    inline void				SetPos(const USHORT nP)			{ nPos = nP; }

    inline USHORT			GetLevel() const				{ return nLevel; }
    inline void				SetLevel(const USHORT nLvl)		{ nLevel = nLvl; }

    inline sal_Unicode		GetSeparator() const				{ return cSeparator; }
    inline void				SetSeparator(const sal_Unicode cSep){ cSeparator = cSep; }

    inline BOOL&			IgnoreSeqOpts()					{ return bIgnoreSeqOpts; }
    inline BOOL				IgnoreSeqOpts() const			{ return bIgnoreSeqOpts; }

    inline BOOL& 			CopyAttributes() 				{ return bCopyAttributes; }
    inline BOOL 			CopyAttributes() const			{ return bCopyAttributes; }

    inline BOOL				operator< ( const InsCaptionOpt & rObj ) const
                                                { return aOleId < rObj.aOleId; }
};

} //namespace binfilter
#endif


