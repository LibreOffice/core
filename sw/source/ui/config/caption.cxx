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

