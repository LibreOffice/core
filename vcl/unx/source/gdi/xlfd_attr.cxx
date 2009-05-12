/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xlfd_attr.cxx,v $
 * $Revision: 1.24 $
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
#include "precompiled_vcl.hxx"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sal/alloca.h>
#include "xlfd_attr.hxx"
#include <rtl/tencinfo.h>
#include <vcl/vclenum.hxx>

// ---------------------------------------------------------------------------
//
//
// Attribute is a container for simple name value pairs
// eg. ( "times", FAMILY_ROMAN ) or ( "demi bold", WEIGHT_SEMIBOLD )
// enriched with an annotation which is a pretty-printed version of the
// string, i.e. "itc avant garde" would get an annotation of "Itc Avant Garde"
//
//
// ---------------------------------------------------------------------------

// release the stored string
void
Attribute::Release()
{
    if ( mpAnnotation != NULL )
        delete mpAnnotation;
    if ( mpKeyName != NULL )
        delete mpKeyName;
    if ( mpName != NULL )
        free( (void*)mpName );
}

// get a private copy of the given argument
void
Attribute::SetName( const char *p, int nLen )
{
    mpName   = (char*)malloc( nLen + 1 );
    mnLength = nLen;
    memcpy( (void*)mpName, p, mnLength );
    ((char*)mpName)[ mnLength ] = '\0';
}

// Compare whether two strings a equal for the first nLen bytes
// i.e. arial == arialnarrow
int
Attribute::Compare( const char *p, int nLen )
{
    return strncmp( mpName, p, nLen );
}

// Get a all lowercase name with all blanks removed
const rtl::OString&
Attribute::GetKey ()
{
    static rtl::OString aEmptyStr;

    if (mpKeyName != NULL)
        return *mpKeyName;
    if (mnLength == 0)
        return aEmptyStr;

    sal_Char* pBuffer = (sal_Char*)alloca (mnLength);

    sal_Int32 i, j;
    for (i = 0, j = 0; i < mnLength; i++)
    {
        if ( mpName[i] != ' ' )
            pBuffer[j++] = mpName[i];
    }
    mpKeyName = new rtl::OString(pBuffer, j);

    return *mpKeyName;
}

void
Attribute::InitKey ()
{
    mpKeyName = NULL;
}

// Compare two strings, they have to be equal for nLen bytes, after nLen
// bytes both strings have to be terminated either by '\0' or by '-'
// this is for comparing a string being a substring in a Xlfd with a
// zeroterminated string
Bool
Attribute::ExactMatch( const char *p, int nLen )
{
    Bool bMatch;
    if ( nLen > 0 )
        bMatch = Compare( p, nLen ) == 0;
    else
        bMatch = True;
    if ( bMatch )
    {
        char c1 = p[ nLen ];
        char c2 = mpName[ nLen ];
        bMatch = (c1 == '-' || c1 == '\0') && (c2 == '-' || c2 == '\0');
    }

    return bMatch;
}

void
Attribute::TagFeature( unsigned short nFeature )
{
    if (   (nFeature & XLFD_FEATURE_NARROW)
        && (strstr(mpName, "narrow") != NULL) )
    {
        mnFeature |= XLFD_FEATURE_NARROW;
    }

    if (   (nFeature & XLFD_FEATURE_OL_CURSOR)
        && (strcmp(mpName, "open look cursor") == 0) )
    {
        mnFeature |= XLFD_FEATURE_OL_CURSOR;
    }

    if (   (nFeature & XLFD_FEATURE_OL_GLYPH)
        && (strcmp(mpName, "open look glyph") == 0) )
    {
        mnFeature |= XLFD_FEATURE_OL_GLYPH;
    }

    if (   (nFeature & XLFD_FEATURE_APPLICATION_FONT)
        && (   (strcmp(mpName, "interface user")   == 0)
            || (strcmp(mpName, "interface system") == 0)))
    {
        mnFeature |= XLFD_FEATURE_APPLICATION_FONT;
    }

    if (nFeature & XLFD_FEATURE_INTERFACE_FONT)
    {
        // european
        if (strcmp(mpName, "arial") == 0)
            mnFeature |= (XLFD_FEATURE_INTERFACE_FONT | XLFD_FEATURE_HQ | XLFD_FEATURE_MQ);
        else
        if (strcmp(mpName, "helvetica") == 0)
            mnFeature |= (XLFD_FEATURE_INTERFACE_FONT | XLFD_FEATURE_HQ);
        else
        if (   (strcmp(mpName, "lucidux sans") == 0)
            || (strcmp(mpName, "luxi sans")    == 0))
            mnFeature |= (XLFD_FEATURE_INTERFACE_FONT | XLFD_FEATURE_MQ | XLFD_FEATURE_LQ);
        else
        if (strcmp(mpName, "charter") == 0)
            mnFeature |= (XLFD_FEATURE_INTERFACE_FONT | XLFD_FEATURE_MQ);
        else
        // japanese
        if (   (strcmp(mpName, "hg mincho l")  == 0)    /* Solaris: jisx0208 jisx0201 */
            || (strcmp(mpName, "heiseimin")    == 0)    /* Solaris: jisx0212 */
            || (strcmp(mpName, "minchol")      == 0)    /* TurboLinux */
            || (strcmp(mpName, "mincho")       == 0))   /* Redhat 6.2 JP */
        {
            mnFeature |= XLFD_FEATURE_INTERFACE_FONT;
        }
        else
        // chinese
        if (   (strcmp(mpName, "kai")                 == 0)  /* Solaris */
            || (strcmp(mpName, "ar pl mingti2l big5") == 0)) /* TurboLinux */
        {
            mnFeature |= XLFD_FEATURE_INTERFACE_FONT;
        }
        else
        // korean
        if (   (strcmp(mpName, "myeongjo")    == 0))    /* Solaris */
        {
            mnFeature |= XLFD_FEATURE_INTERFACE_FONT;
        }
    }

    if ( nFeature & XLFD_FEATURE_REDUNDANTSTYLE )
    {
        switch ( mpName[0] )
        {
            case '\0':
                    mnFeature |= XLFD_FEATURE_REDUNDANTSTYLE;
                break;

            case 'b':
                if (   (strcmp(mpName, "bold")        == 0)
                    || (strcmp(mpName, "bold italic") == 0)
                    || (strcmp(mpName, "bold sans")   == 0) )
                    mnFeature |= XLFD_FEATURE_REDUNDANTSTYLE;
                break;

            case 'd':
                if (   (strcmp(mpName, "demi")        == 0)
                    || (strcmp(mpName, "demi italic") == 0) )
                    mnFeature |= XLFD_FEATURE_REDUNDANTSTYLE;
                break;

            case 'i':
                if ( strcmp(mpName, "italic") == 0 )
                    mnFeature |= XLFD_FEATURE_REDUNDANTSTYLE;
                break;

            case 's':
                if (   (strcmp(mpName, "sans")  == 0)
                    || (strcmp(mpName, "serif") == 0) )
                    mnFeature |= XLFD_FEATURE_REDUNDANTSTYLE;
                break;
        }
    }
}

// given Attribute classifications, strings have to be in alphabetical
// order, since they are treated by binary search algorithm

#define InitializeAttributeWith( p, a ) p, sizeof(p) - 1, a, 0, NULL, NULL
#define MembersOf( p ) (sizeof(p) / sizeof(p[0]) )

const Attribute pFamilyAttribute[] = {
    { InitializeAttributeWith( "arial",                 FAMILY_SWISS )  },
    { InitializeAttributeWith( "arioso",                FAMILY_SCRIPT ) },
    { InitializeAttributeWith( "avant garde",           FAMILY_SWISS )  },
    { InitializeAttributeWith( "avantgarde",            FAMILY_SWISS )  },
    { InitializeAttributeWith( "bembo",                 FAMILY_ROMAN )  },
    { InitializeAttributeWith( "bookman",               FAMILY_ROMAN )  },
    { InitializeAttributeWith( "conga",                 FAMILY_ROMAN )  },
    { InitializeAttributeWith( "courier",               FAMILY_MODERN ) },
    { InitializeAttributeWith( "curl",                  FAMILY_SCRIPT ) },
    { InitializeAttributeWith( "fixed",                 FAMILY_MODERN ) },
    { InitializeAttributeWith( "gill",                  FAMILY_SWISS )  },
    { InitializeAttributeWith( "helmet",                FAMILY_MODERN ) },
    { InitializeAttributeWith( "helvetica",             FAMILY_SWISS )  },
    { InitializeAttributeWith( "international",         FAMILY_MODERN ) },
    { InitializeAttributeWith( "lucida",                FAMILY_SWISS )  },
    { InitializeAttributeWith( "new century schoolbook", FAMILY_ROMAN ) },
    { InitializeAttributeWith( "palatino",              FAMILY_ROMAN )  },
    { InitializeAttributeWith( "roman",                 FAMILY_ROMAN )  },
    { InitializeAttributeWith( "sans serif",            FAMILY_SWISS )  },
    { InitializeAttributeWith( "sansserif",             FAMILY_SWISS )  },
    { InitializeAttributeWith( "serf",                  FAMILY_ROMAN )  },
    { InitializeAttributeWith( "serif",                 FAMILY_ROMAN )  },
    { InitializeAttributeWith( "times",                 FAMILY_ROMAN )  },
    { InitializeAttributeWith( "utopia",                FAMILY_ROMAN )  },
    { InitializeAttributeWith( "zapf chancery",         FAMILY_SCRIPT ) },
    { InitializeAttributeWith( "zapfchancery",          FAMILY_SCRIPT ) }
};

const Attribute pWeightAttribute[] = {
    { InitializeAttributeWith( "black",                 WEIGHT_BLACK )      },
    { InitializeAttributeWith( "bold",                  WEIGHT_BOLD )       },
    { InitializeAttributeWith( "book",                  WEIGHT_LIGHT )      },
    { InitializeAttributeWith( "demi",                  WEIGHT_SEMIBOLD )   },
    { InitializeAttributeWith( "demi bold",             WEIGHT_SEMIBOLD )   },
    { InitializeAttributeWith( "demibold",              WEIGHT_SEMIBOLD )   },
    { InitializeAttributeWith( "light",                 WEIGHT_LIGHT )      },
    { InitializeAttributeWith( "medium",                WEIGHT_MEDIUM )     },
    { InitializeAttributeWith( "normal",                WEIGHT_NORMAL )     },
    { InitializeAttributeWith( "regular",               WEIGHT_NORMAL )     },
    { InitializeAttributeWith( "roman",                 WEIGHT_NORMAL )     },
    { InitializeAttributeWith( "semicondensed",         WEIGHT_LIGHT )      },
    { InitializeAttributeWith( "ultrabold",             WEIGHT_ULTRABOLD )  }
};

const Attribute pSlantAttribute[] = {
    { InitializeAttributeWith( "i",                     ITALIC_NORMAL )     },
    { InitializeAttributeWith( "o",                     ITALIC_OBLIQUE )    },
    { InitializeAttributeWith( "r",                     ITALIC_NONE )       }
};

const Attribute pSetwidthAttribute[] = {
    { InitializeAttributeWith( "bold",              WIDTH_SEMI_EXPANDED )   },
    { InitializeAttributeWith( "condensed",         WIDTH_CONDENSED )       },
    { InitializeAttributeWith( "double wide",       WIDTH_ULTRA_EXPANDED )  },
    { InitializeAttributeWith( "expanded",          WIDTH_EXPANDED )        },
    { InitializeAttributeWith( "extracondensed",    WIDTH_EXTRA_CONDENSED ) },
    { InitializeAttributeWith( "extraexpanded",     WIDTH_EXTRA_EXPANDED )  },
    { InitializeAttributeWith( "medium",            WIDTH_NORMAL )          },
    { InitializeAttributeWith( "narrow",            WIDTH_CONDENSED )       },
    { InitializeAttributeWith( "normal",            WIDTH_NORMAL )          },
    { InitializeAttributeWith( "semicondensed",     WIDTH_SEMI_CONDENSED )  },
    { InitializeAttributeWith( "semiexpanded",      WIDTH_SEMI_EXPANDED )   },
    { InitializeAttributeWith( "ultracondensed",    WIDTH_ULTRA_CONDENSED ) },
    { InitializeAttributeWith( "ultraexpanded",     WIDTH_ULTRA_EXPANDED )  },
    { InitializeAttributeWith( "wide",              WIDTH_EXPANDED )        }
};

const Attribute pEnhancedCharsetAttribute[] = {
    { InitializeAttributeWith( "iso8859-1",     RTL_TEXTENCODING_MS_1252 ) },
    { InitializeAttributeWith( "iso8859_1",     RTL_TEXTENCODING_MS_1252 ) }
};

// -------------------------------------------------------------------------
//
//  String handling utility functions
//
// -------------------------------------------------------------------------


void
AppendAttribute( Attribute *pAttribute, ByteString &rString )
{
    if ( pAttribute == NULL )
        return ;

    int   nLength = pAttribute->GetLength();
    char *pBuffer = (char*)alloca( nLength + 1);

    pBuffer[ 0 ] = '-';
    memcpy( pBuffer + 1, pAttribute->GetName(), nLength );
    rString.Append( pBuffer, nLength + 1);
}

//
// Prettify the font name: make each leading character of a fontname
// uppercase. For example
//      times new roman -> Times New Roman
//

static void
ToUpper( char *pCharacter )
{
    // replace [a,z] with [A,Z]
    if ( (*pCharacter >= 97) && (*pCharacter <= 122) )
        *pCharacter -= 32;
}

static String*
Capitalize( const char *pStr, int nLength )
{
    char *pCopy = (char*)alloca( nLength + 1 );
    char *pPtr = pCopy;
    memcpy( pPtr, pStr, nLength + 1 );

    // loop over string data and uppercase first char and all chars
    // following a space (other white space would be unexpected here)
    char  nPreviousChar = ' ';
    while ( *pPtr )
    {
        if ( nPreviousChar == ' ' )
            ToUpper( pPtr );
        nPreviousChar = *pPtr++;
    }

    return new String( pCopy, RTL_TEXTENCODING_ISO_8859_1 );
}

String*
AnnotateString( const Attribute& rAttribute )
{
    return Capitalize(rAttribute.GetName(), rAttribute.GetLength());
}

String*
AnnotateSlant( const Attribute& rAttribute )
{
    const char* pStr = rAttribute.GetName();
    int         nLen = rAttribute.GetLength();

    static const struct {
            const char *pFrom; const char *pTo;
    } pTranslation[] = {
             { "r",  "Roman" },
            { "o",  "Oblique" },
            { "i",  "Italic" },
            { "ri", "Reverse Italic" },
            { "ro", "Reverse Oblique" },
            { "ot", "Other" }
    };

    for ( unsigned int i = 0; i < MembersOf(pTranslation); i++ )
        if ( strcmp(pStr, pTranslation[i].pFrom) == 0 )
        {
            return new String( pTranslation[i].pTo,
                               RTL_TEXTENCODING_ISO_8859_1 );
        }

    return Capitalize(pStr, nLen);
}

String*
AnnotateNone( const Attribute& )
{
    return new String();
}

// ---------------------------------------------------------------------------
//
//
// manage global lists of Attributes
// since XListFonts does never list more than 64K fonts this storage does
// handle array size and indices with unsigned short values for low
// memory consumption
//
//
// ---------------------------------------------------------------------------

AttributeStorage::AttributeStorage( unsigned short nDefaultValue ) :
        mpList( NULL ),
        mnSize( 0 ),
        mnCount( 0 ),
        mnLastmatch( 0 ),
        mnDefaultValue( nDefaultValue )
{
}

AttributeStorage::~AttributeStorage()
{
    if ( mpList != NULL )
    {
        for ( int i = 0; i < mnCount; i++ )
            mpList[i].Release();
        free( mpList );
    }
}

#if OSL_DEBUG_LEVEL > 1
void
AttributeStorage::Dump()
{
    fprintf(stderr, "AttributeStorage: size=%i, used=%i\n", mnSize, mnCount);
    for ( int i = 0; i < mnCount; i++ )
    {
        ByteString aAnnotation = ByteString(
                mpList[i].GetAnnotation(),
                RTL_TEXTENCODING_ISO_8859_1 );
        fprintf(stderr, "\t%4i: <%s><len=%i><val=%i><%s>\n", i, mpList[i].GetName(),
                mpList[i].GetLength(), mpList[i].GetValue(),
                aAnnotation.GetBuffer() );
    }
    fprintf(stderr, "\n");
}
#endif

Attribute*
AttributeStorage::Retrieve( unsigned short nIndex ) const
{
    return nIndex < mnCount ? &mpList[ nIndex ] : (Attribute*)NULL ;
}

// pClassification contains a list of name-value pairs. If names in
// the AttributeStorage match those in the pClassification then
// the according value is copied. Matching means match for the length
// of the string in pClassification (i.e. arial matches arialnarrow)
// the strings in pClassification must be in alphabetical order, all
// strings Lowercase
void
AttributeStorage::AddClassification( Attribute *pClassification,
        unsigned short nNum )
{
    for ( int i = 0; i < mnCount; i++ )
    {
        unsigned int nLower = 0;
        unsigned int nUpper = nNum;
        unsigned int nCurrent;
          int nComparison = 1;
        Attribute *pHaystack = 0, *pNeedle;

        pNeedle = &mpList[ i ];

        // binary search
          while ( nLower < nUpper )
        {
              nCurrent = (nLower + nUpper) / 2;
            pHaystack = &pClassification[ nCurrent ];
              nComparison = pNeedle->Compare( pHaystack->GetName(),
                                            pHaystack->GetLength() );
              if (nComparison < 0)
                nUpper = nCurrent;
              else
            if (nComparison > 0)
                nLower = nCurrent + 1;
              else
                break;
        }

        // if there's a match store the according classification in the
        // Attribute storage, otherwise do nothing since defaults are
        // already provided in AttributeStorage::Insert()
        if ( nComparison == 0 )
            pNeedle->SetValue( pHaystack->GetValue() );
    }
}

void
AttributeStorage::AddClassification( AttributeClassifierT Classify )
{
    for ( int i = 0; i < mnCount; i++ )
    {
        Attribute& rCurrent = mpList[i] ;
        int nValue = Classify( rCurrent.GetName() );
        rCurrent.SetValue( nValue );
    }
}

void
AttributeStorage::AddAnnotation( AttributeAnnotatorT Annotate )
{
    for ( int i = 0; i < mnCount; i++ )
    {
        String* pAnnotation = Annotate( mpList[i] );
        mpList[i].SetAnnotation( pAnnotation );
    }
}

void
AttributeStorage::TagFeature( unsigned short nFeature )
{
    for ( int i = 0; i < mnCount; i++ )
        mpList[i].TagFeature( nFeature );
}

// Enlarge the list of Attributes
void
AttributeStorage::Enlarge()
{
    if ( mnSize == 0 )
    {
        mnSize  = 8;
        mpList  = (Attribute*) malloc( mnSize * sizeof(Attribute) );
    }
    else
    {
        mnSize = mnSize < 32768 ? (mnSize * 2) : 65535;
        mpList  = (Attribute*) realloc( mpList, mnSize * sizeof(Attribute) );
    }
}

// nLength is the length as it would be reported by strlen(3)
// for an null-terminated string. if a string is part of a Xlfd
// the field separator '-' is taken as '\0'
// the AttributeStorage itself is NOT sorted to make sure that the
// leased keys are still valid
unsigned short
AttributeStorage::Insert( const char *pString, int nLength )
{
    // check whether the last match is still equal to the current
    // string since XListFonts lists fonts in sets of similar fontnames
    if ( mnLastmatch < mnCount )
    {
        if ( mpList[mnLastmatch].ExactMatch(pString, nLength) )
            return mnLastmatch;
    }

    // otherwise search in list
    for ( int i = 0; i < mnCount; i++ )
    {
        if ( mpList[i].ExactMatch(pString, nLength) )
            return mnLastmatch = i;
    }

    // if still not found we have to Insert the new string
    if ( mnSize == mnCount )
        Enlarge();
    mpList[mnCount].SetName( pString, nLength );
    mpList[mnCount].SetValue( mnDefaultValue );
    mpList[mnCount].SetAnnotation( NULL );
    mpList[mnCount].SetFeature( XLFD_FEATURE_NONE );
    mpList[mnCount].InitKey( );
    mnLastmatch = mnCount;
    mnCount = mnCount < 65535 ? mnCount + 1 : mnCount;

    return mnLastmatch;
}


// ---------------------------------------------------------------------------
//
//
// Attribute provider is a frame for a set of AttributeStorages.
//
//
// ---------------------------------------------------------------------------

AttributeProvider::AttributeProvider ()
{
    mpField[eXLFDFoundry     ] = new AttributeStorage(0);
    mpField[eXLFDFamilyName  ] = new AttributeStorage(FAMILY_DONTKNOW);
    mpField[eXLFDWeightName  ] = new AttributeStorage(WEIGHT_NORMAL);
    mpField[eXLFDSlant       ] = new AttributeStorage(ITALIC_NONE);
    mpField[eXLFDSetwidthName] = new AttributeStorage(WIDTH_NORMAL);
    mpField[eXLFDAddstyleName] = new AttributeStorage(RTL_TEXTENCODING_DONTKNOW);
    mpField[eXLFDCharset     ] = new AttributeStorage(RTL_TEXTENCODING_DONTKNOW);
}

AttributeProvider::~AttributeProvider()
{
    for ( int i = 0; i < eXLFDMaxEntry; i++ )
        delete mpField[ i ];
}

#if OSL_DEBUG_LEVEL > 1
void
AttributeProvider::Dump()
{
    for ( int i = 0; i < eXLFDMaxEntry; i++ )
        mpField[ i ]->Dump();
}
#endif

extern "C" rtl_TextEncoding
GetTextEncodingFromAddStylename( const sal_Char *pAddStylename )
{
    int nBufferLength = strlen( pAddStylename ) + 1;
    sal_Char *pBuffer = (sal_Char*)alloca( nBufferLength );
    for ( int i = 0; i < nBufferLength; i++ )
        pBuffer[i] = pAddStylename[i] == '_' ? '-' : pAddStylename[i] ;

    return rtl_getTextEncodingFromUnixCharset( pBuffer );
}


// classification information is needed before sorting because of course the
// classification is the sort criteria
void
AttributeProvider::AddClassification()
{
    /* mpField[ eXLFDFoundry ] doesn't need classification */
    mpField[ eXLFDFamilyName   ]->AddClassification(
                                    (Attribute*)pFamilyAttribute,
                                    MembersOf(pFamilyAttribute) );
    mpField[ eXLFDWeightName   ]->AddClassification(
                                    (Attribute*)pWeightAttribute,
                                    MembersOf(pWeightAttribute) );
    mpField[ eXLFDSlant        ]->AddClassification(
                                    (Attribute*)pSlantAttribute,
                                    MembersOf(pSlantAttribute) );
    mpField[ eXLFDSetwidthName ]->AddClassification(
                                    (Attribute*)pSetwidthAttribute,
                                    MembersOf(pSetwidthAttribute) );
    mpField[ eXLFDAddstyleName ]->AddClassification(
                                    GetTextEncodingFromAddStylename );
    mpField[ eXLFDCharset      ]->AddClassification(
                                    rtl_getTextEncodingFromUnixCharset );
}

// add some pretty print description
void
AttributeProvider::AddAnnotation()
{
    mpField[ eXLFDFoundry      ]->AddAnnotation( AnnotateNone );
    mpField[ eXLFDFamilyName   ]->AddAnnotation( AnnotateString );
    mpField[ eXLFDWeightName   ]->AddAnnotation( AnnotateString );
    mpField[ eXLFDSlant        ]->AddAnnotation( AnnotateSlant );
    mpField[ eXLFDSetwidthName ]->AddAnnotation( AnnotateString );
    mpField[ eXLFDAddstyleName ]->AddAnnotation( AnnotateNone );
    mpField[ eXLFDCharset      ]->AddAnnotation( AnnotateNone );
}

// this is the misc or any section: dirty hacks for dirty xlfd usage
void
AttributeProvider::TagFeature()
{
    mpField[ eXLFDFamilyName   ]->TagFeature(
                                      XLFD_FEATURE_OL_GLYPH
                                    | XLFD_FEATURE_OL_CURSOR
                                    | XLFD_FEATURE_NARROW
                                    | XLFD_FEATURE_INTERFACE_FONT
                                    | XLFD_FEATURE_APPLICATION_FONT);

    mpField[ eXLFDSetwidthName ]->TagFeature(
                                      XLFD_FEATURE_NARROW );

    mpField[ eXLFDAddstyleName ]->TagFeature(
                                      XLFD_FEATURE_REDUNDANTSTYLE );
}

