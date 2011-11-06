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
#include "precompiled_svtools.hxx"

// include ---------------------------------------------------------------

#include <tools/shl.hxx>
#include <tools/debug.hxx>

//#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/DirectionProperty.hpp>

#include <i18npool/lang.h>
#include <i18npool/mslangid.hxx>

#include <svtools/svtools.hrc>
#include <svtools/svtdata.hxx>
#include <svtools/langtab.hxx>
#include <unotools/syslocale.hxx>


using namespace ::com::sun::star;

//------------------------------------------------------------------------

SVT_DLLPUBLIC const String ApplyLreOrRleEmbedding( const String &rText )
{
    const sal_uInt16 nLen = rText.Len();
    if (nLen == 0)
        return String();

    const sal_Unicode cLRE_Embedding  = 0x202A;      // the start char of an LRE embedding
    const sal_Unicode cRLE_Embedding  = 0x202B;      // the start char of an RLE embedding
    const sal_Unicode cPopDirectionalFormat = 0x202C;   // the unicode PDF (POP_DIRECTIONAL_FORMAT) char that terminates an LRE/RLE embedding

    // check if there are alreay embedding characters at the strings start
    // if so change nothing
    const sal_Unicode cChar = rText.GetBuffer()[0];
    if (cChar == cLRE_Embedding || cChar == cRLE_Embedding)
        return rText;

    // since we only call the function getCharacterDirection
    // it does not matter which locale the CharClass is for.
    // Thus we can readily make use of SvtSysLocale::GetCharClass()
    // which should come at no cost...
    SvtSysLocale aSysLocale;
    const CharClass &rCharClass = aSysLocale.GetCharClass();

    // we should look for the first non-neutral LTR or RTL character
    // and use that to determine the embedding of the whole text...
    // Thus we can avoid to check every character of the text.
    bool bFound     = false;
    bool bIsRtlText = false;
    for (sal_uInt16 i = 0;  i < nLen && !bFound;  ++i)
    {
        sal_Int16 nDirection = rCharClass.getCharacterDirection( rText, i );
        switch (nDirection)
        {
            case i18n::DirectionProperty_LEFT_TO_RIGHT :
            case i18n::DirectionProperty_LEFT_TO_RIGHT_EMBEDDING :
            case i18n::DirectionProperty_LEFT_TO_RIGHT_OVERRIDE :
            case i18n::DirectionProperty_EUROPEAN_NUMBER :
            case i18n::DirectionProperty_ARABIC_NUMBER :        // yes! arabic numbers are written from left to right
            {
                bIsRtlText  = false;
                bFound      = true;
                break;
            }

            case i18n::DirectionProperty_RIGHT_TO_LEFT :
            case i18n::DirectionProperty_RIGHT_TO_LEFT_ARABIC :
            case i18n::DirectionProperty_RIGHT_TO_LEFT_EMBEDDING :
            case i18n::DirectionProperty_RIGHT_TO_LEFT_OVERRIDE :
            {
                bIsRtlText  = true;
                bFound      = true;
                break;
            }

            default:
            {
                // nothing to be done, character is considered to be neutral we need to look further ...
            }
        }
    }

    sal_Unicode cStart  = cLRE_Embedding;   // default is to use LRE embedding characters
    if (bIsRtlText)
        cStart = cRLE_Embedding;            // then use RLE embedding

    // add embedding start and end chars to the text if the direction could be determined
    String aRes( rText );
    if (bFound)
    {
        aRes.Insert( cStart, 0 );
        aRes.Insert( cPopDirectionalFormat );
    }

    return aRes;
}

//------------------------------------------------------------------------

SvtLanguageTable::SvtLanguageTable() :
    ResStringArray( SvtResId( STR_ARR_SVT_LANGUAGE_TABLE ) )
{
}

//------------------------------------------------------------------------

SvtLanguageTable::~SvtLanguageTable()
{
}

//------------------------------------------------------------------------

const String& SvtLanguageTable::GetString( const LanguageType eType ) const
{
    LanguageType eLang = MsLangId::getReplacementForObsoleteLanguage( eType);
    sal_uInt32 nPos = FindIndex( eLang );

    if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
        return ResStringArray::GetString( nPos );
    else
    {
        // If we knew what a simple "en" should alias to (en_US?) we could
        // generally raise an error.
        OSL_ENSURE(
            eLang == LANGUAGE_ENGLISH, "language entry not found in resource" );

        nPos = FindIndex( LANGUAGE_DONTKNOW );

        if ( RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count() )
            return ResStringArray::GetString( nPos );
    }
    static String aEmptyStr;
    return aEmptyStr;
}

String SvtLanguageTable::GetLanguageString( const LanguageType eType )
{
    static const SvtLanguageTable aLangTable;
    return aLangTable.GetString( eType );
}

//------------------------------------------------------------------------

LanguageType SvtLanguageTable::GetType( const String& rStr ) const
{
    LanguageType eType = LANGUAGE_DONTKNOW;
    sal_uInt32 nCount = Count();

    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        if ( rStr == ResStringArray::GetString( i ) )
        {
            eType = LanguageType( GetValue( i ) );
            break;
        }
    }
    return eType;
}

//------------------------------------------------------------------------

sal_uInt32 SvtLanguageTable::GetEntryCount() const
{
    return Count();
}

//------------------------------------------------------------------------

LanguageType SvtLanguageTable::GetTypeAtIndex( sal_uInt32 nIndex ) const
{
    LanguageType nType = LANGUAGE_DONTKNOW;
    if (nIndex < Count())
        nType = LanguageType( GetValue( nIndex ) );
    return nType;
}

//------------------------------------------------------------------------

