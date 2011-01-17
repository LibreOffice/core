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
#include "precompiled_sw.hxx"


#include <tools/debug.hxx>

#include "numrule.hxx"
#include "caption.hxx"

#define VERSION_01      1
#define CAPTION_VERSION VERSION_01

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

InsCaptionOpt::InsCaptionOpt(const SwCapObjType eType, const SvGlobalName* pOleId) :
    bUseCaption(sal_False),
    eObjType(eType),
    nNumType(SVX_NUM_ARABIC),
    sNumberSeparator( ::rtl::OUString::createFromAscii(". ") ),
    nPos(1),
    nLevel(0),
    sSeparator( String::CreateFromAscii( ": " ) ),
    bIgnoreSeqOpts(sal_False),
    bCopyAttributes(sal_False)
{
    if (pOleId)
        aOleId = *pOleId;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

InsCaptionOpt::InsCaptionOpt(const InsCaptionOpt& rOpt)
{
    *this = rOpt;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

InsCaptionOpt::~InsCaptionOpt()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

InsCaptionOpt& InsCaptionOpt::operator=( const InsCaptionOpt& rOpt )
{
    bUseCaption = rOpt.bUseCaption;
    eObjType = rOpt.eObjType;
    aOleId = rOpt.aOleId;
    sCategory = rOpt.sCategory;
    nNumType = rOpt.nNumType;
    sNumberSeparator = rOpt.sNumberSeparator;
    sCaption = rOpt.sCaption;
    nPos = rOpt.nPos;
    nLevel = rOpt.nLevel;
    sSeparator = rOpt.sSeparator;
    bIgnoreSeqOpts = rOpt.bIgnoreSeqOpts;
    sCharacterStyle = rOpt.sCharacterStyle;
    bCopyAttributes = rOpt.bCopyAttributes;

    return *this;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool InsCaptionOpt::operator==( const InsCaptionOpt& rOpt ) const
{
    return (eObjType == rOpt.eObjType &&
            aOleId == rOpt.aOleId); // Damit gleiche Ole-IDs nicht mehrfach eingefuegt
                                    // werden koennen, auf nichts weiteres vergleichen


/*           &&
            sCategory == rOpt.sCategory &&
            nNumType == rOpt.nNumType &&
            sCaption == rOpt.sCaption &&
            nPos == rOpt.nPos &&
            nLevel == rOpt.nLevel &&
            cSeparator == rOpt.cSeparator);*/
}

/*************************************************************************
|*
|*    InsCaptionOpt::operator>>()
|*
|*    Beschreibung      Stream-Leseoperator
|*
*************************************************************************/

/*SvStream& operator>>( SvStream& rIStream, InsCaptionOpt& rCapOpt )
{
    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    sal_uInt16 nVal;
    sal_uInt8   cVal;
    sal_uInt8   nVersion;

    rIStream >> nVersion;
    rIStream >> cVal;               rCapOpt.UseCaption() = cVal != 0;
    rIStream >> nVal;               rCapOpt.eObjType = (SwCapObjType)nVal;
    rIStream >> rCapOpt.aOleId;

    rIStream.ReadByteString( rCapOpt.sCategory, eEncoding );
    rIStream >> nVal;               rCapOpt.nNumType = nVal;
    rIStream.ReadByteString( rCapOpt.sCaption, eEncoding );
    rIStream >> nVal;               rCapOpt.nPos = nVal;
    rIStream >> nVal;               rCapOpt.nLevel = nVal;

    rIStream >> cVal;
    rCapOpt.sSeparator = UniString(
        ByteString(static_cast< char >(cVal)) , eEncoding).GetChar(0);

    return rIStream;
}
*/
/*************************************************************************
|*
|*    InsCaptionOpt::operator<<()
|*
|*    Beschreibung      Stream-Schreiboperator
|*
*************************************************************************/

/*SvStream& operator<<( SvStream& rOStream, const InsCaptionOpt& rCapOpt )
{
    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    rOStream    << (sal_uInt8)CAPTION_VERSION
                << (sal_uInt8)rCapOpt.UseCaption()
                << (sal_uInt16)rCapOpt.eObjType
                << rCapOpt.aOleId;

    rOStream.WriteByteString( rCapOpt.sCategory, eEncoding );

    rOStream    << (sal_uInt16)rCapOpt.nNumType;

    rOStream.WriteByteString( rCapOpt.sCaption, eEncoding );

    sal_uInt8 cSep = ByteString(rCapOpt.sSeparator, eEncoding).GetChar(0);
    rOStream    << (sal_uInt16)rCapOpt.nPos
                << (sal_uInt16)rCapOpt.nLevel
                << cSep;

    return rOStream;
}
*/

