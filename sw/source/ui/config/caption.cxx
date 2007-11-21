/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: caption.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 18:20:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "numrule.hxx"
#include "caption.hxx"

#define VERSION_01      1
#define CAPTION_VERSION VERSION_01

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

InsCaptionOpt::InsCaptionOpt(const SwCapObjType eType, const SvGlobalName* pOleId) :
    bUseCaption(FALSE),
    eObjType(eType),
    nNumType(SVX_NUM_ARABIC),
    sNumberSeparator( ::rtl::OUString::createFromAscii(". ") ),
    nPos(1),
    nLevel(0),
    sSeparator( String::CreateFromAscii( ": " ) ),
    bIgnoreSeqOpts(FALSE),
    bCopyAttributes(FALSE)
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

BOOL InsCaptionOpt::operator==( const InsCaptionOpt& rOpt ) const
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
    UINT16 nVal;
    BYTE   cVal;
    BYTE   nVersion;

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
    rOStream    << (BYTE)CAPTION_VERSION
                << (BYTE)rCapOpt.UseCaption()
                << (UINT16)rCapOpt.eObjType
                << rCapOpt.aOleId;

    rOStream.WriteByteString( rCapOpt.sCategory, eEncoding );

    rOStream    << (UINT16)rCapOpt.nNumType;

    rOStream.WriteByteString( rCapOpt.sCaption, eEncoding );

    BYTE cSep = ByteString(rCapOpt.sSeparator, eEncoding).GetChar(0);
    rOStream    << (UINT16)rCapOpt.nPos
                << (UINT16)rCapOpt.nLevel
                << cSep;

    return rOStream;
}
*/

