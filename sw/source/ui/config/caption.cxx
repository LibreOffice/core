/*************************************************************************
 *
 *  $RCSfile: caption.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
    nPos(1),
    nLevel(0),
    cSeparator('.'),
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
    sCaption = rOpt.sCaption;
    nPos = rOpt.nPos;
    nLevel = rOpt.nLevel;
    cSeparator = rOpt.cSeparator;
    bIgnoreSeqOpts = rOpt.bIgnoreSeqOpts;
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

SvStream& operator>>( SvStream& rIStream, InsCaptionOpt& rCapOpt )
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
    rCapOpt.cSeparator = UniString( ByteString(cVal) , eEncoding).GetChar(0);

    return rIStream;
}

/*************************************************************************
|*
|*    InsCaptionOpt::operator<<()
|*
|*    Beschreibung      Stream-Schreiboperator
|*
*************************************************************************/

SvStream& operator<<( SvStream& rOStream, const InsCaptionOpt& rCapOpt )
{
    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    rOStream    << (BYTE)CAPTION_VERSION
                << (BYTE)rCapOpt.UseCaption()
                << (UINT16)rCapOpt.eObjType
                << rCapOpt.aOleId;

    rOStream.WriteByteString( rCapOpt.sCategory, eEncoding );

    rOStream    << (UINT16)rCapOpt.nNumType;

    rOStream.WriteByteString( rCapOpt.sCaption, eEncoding );

    BYTE cSep = ByteString(UniString(rCapOpt.cSeparator), eEncoding).GetChar(0);
    rOStream    << (UINT16)rCapOpt.nPos
                << (UINT16)rCapOpt.nLevel
                << cSep;

    return rOStream;
}

/*--------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.5  2000/09/18 16:05:15  willem.vandorp
    OpenOffice header added.

    Revision 1.4  2000/04/11 08:02:23  os
    UNICODE

    Revision 1.3  1999/06/09 08:49:44  OS
    #66732# Insert caption: copy border and shadow optionally


      Rev 1.2   09 Jun 1999 10:49:44   OS
   #66732# Insert caption: copy border and shadow optionally

      Rev 1.1   17 Nov 1998 10:50:12   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.0   06 Nov 1998 17:01:32   OM
   Initial revision.

 --------------------------------------------------------------------*/
