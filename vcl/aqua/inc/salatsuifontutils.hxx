/*************************************************************************
 *
 *  $RCSfile: salatsuifontutils.hxx,v $
 *
 *  $Revision: 1.3 $
 *  last change: $Author: bmahbod $ $Date: 2001-03-26 21:53:15 $
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// =======================================================================

// =======================================================================

#ifndef _SV_SALATSUIFONTUTILS_HXX
#define _SV_SALATSUIFONTUTILS_HXX

#ifndef _LIMITS_H
    #include <limits.h>
#endif

#ifndef _STDDEF_H
    #include <stddef.h>
#endif

#ifndef _STDIO_H
    #include <stdio.h>
#endif

#ifndef _STDLIB_H
    #include <stdlib.h>
#endif

#ifndef _STRING_H
    #include <string.h>
#endif

#include <premac.h>
    #include <Carbon/Carbon.h>
    #include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#ifdef __cplusplus
extern "C" {
#endif

// =======================================================================

// =======================================================================

static const short kMacOSCharSize = sizeof(char);
static const short kFontFamilyNameLength = 32;static const short kFontStyleNameLength  = 32;
static const long  kFontFamilyNameMemSize  = kFontFamilyNameLength * kMacOSCharSize;
static const long  kFontStyleNameMemSize   = kFontStyleNameLength  * kMacOSCharSize;

// -----------------------------------------------------------------------

// For simplicity, only this number of font axis variations are considered.

static const short kATSUIMaxVariations  = 32;

// -----------------------------------------------------------------------

// To filter out invisible fonts

static const char kInvalidFontNamePrefixPeriodSign  = '.';
static const char kInvalidFontNamePrefixPercentSign = '%';

// =======================================================================

// =======================================================================

enum
{
    kATSUIDefaultInstance = SHRT_MAX,
    kATSUINotAnInstance   = -1
};

// =======================================================================

// =======================================================================

typedef short ATSUIInstanceIndex;// -----------------------------------------------------------------------struct FontNameEncodingRecord{ FontPlatformCode  mnFontPlatformCode;  FontScriptCode    mnFontScriptCode;    FontLanguageCode  mnFontLanguageCode;  ItemCount         mnFontItemCount;};typedef struct FontNameEncodingRecord   FontNameEncodingRecord;typedef FontNameEncodingRecord         *FontNameEncodingPtr;typedef FontNameEncodingPtr            *FontNameEncodingMatrix;// -----------------------------------------------------------------------struct FontNamesRecord { ATSUFontID              mnFontID;  FontNameEncodingRecord  maFontFamilyNameEncoding;  FontNameEncodingRecord  maFontStyleNameEncoding;   ATSUIInstanceIndex      mnFontInstanceIndex;   ByteOffset              mnFontStyleByteOffset; char                    mpFontName[ 1 ];};typedef struct FontNamesRecord   FontNamesRecord;typedef FontNamesRecord         *FontNamesPtr;typedef FontNamesPtr            *FontNamesMatrix;// =======================================================================

// =======================================================================

// Note that the macro that follows evaluates pref twice.

#define ElementMatches( elem, pref )  ((pref) == -1 || (elem) == (pref))

#define EncodingMatchesPrefs( enc, prefs )                                       \
    ( ElementMatches((enc).mnFontPlatformCode, (prefs).mnFontPlatformCode)   \
    && ElementMatches((enc).mnFontScriptCode, (prefs).mnFontScriptCode)      \
    && ElementMatches((enc).mnFontLanguageCode, (prefs).mnFontLanguageCode) )

// -----------------------------------------------------------------------

#define ATSUISetFont( aFontStyle, nFontID )  \
    ATSUISetIntAttribute( aFontStyle, nFontID, kATSUFontTag )

#define ATSUIGetFont( aFontStyle, rFontID )  \
    ATSUIGetOneAttribute( aFontStyle, kATSUFontTag, sizeof(ATSUFontID), rFontID )

// -----------------------------------------------------------------------

#define ATSUICountStyleFontVariations( aFontStyle, oActualVariationCount ) \
    ATSUGetAllFontVariations( aFontStyle, 0, NULL, NULL, oActualVariationCount )

// =======================================================================

// =======================================================================

OSStatus ATSUIDisposeFontNames( const ItemCount  nFontListLength,
                                FontNamesMatrix  hFontNames
                              );

OSStatus ATSUIFindBestFontName( ATSUFontID         nFontID,
                                FontNameCode       nFontNameCode,
                                FontPlatformCode  *rFontPlatformCode,
                                FontScriptCode    *rFontScriptCode,
                                FontLanguageCode  *rFontLanguageCode,
                                ByteCount          nFontMaxNameLength,
                                MacOSPtr           rFontName,
                                ByteCount         *rFontNameLength,
                                ItemCount         *rFontNameIndex
                              );

OSStatus ATSUIFONDtoFontID( short            nFONDNumber,
                            StyleParameter   nFONDStyle,
                            ATSUFontID      *rFontID,
                            StyleParameter  *rIntrinsicStyleParameter
                          );

void ATSUIFPrintFontList( const char       *pFileName,
                          const char       *pFilePermission,
                          const ItemCount   nFontItemsCount,
                          FontNamesMatrix   hFontList
                        );

FontNamesMatrix ATSUIGetFontNames( ItemCount *rSortedFontListLength );

OSStatus ATSUIGetOneAttribute( ATSUStyle              aFontStyle,
                               ATSUAttributeTag       nFontAttributeTag,
                               ByteCount              nFontExpValueSize,
                               ATSUAttributeValuePtr  rFontAttributeValue
                             );

OSStatus ATSUIGetStyleFontInstance( ATSUStyle            aFontStyle,
                                    ATSUFontID          *rFontID,
                                    ATSUIInstanceIndex  *rFontInstanceIndex
                                  );

OSStatus ATSUISetIntAttribute( ATSUStyle         aFontStyle,
                               SInt32            nFontAttributeValue,
                               ATSUAttributeTag  nFontAttributeTag
                             );

OSStatus ATSUISetStyleFontInstance( ATSUStyle           aFontStyle,
                                    ATSUFontID          nFontID,
                                    ATSUIInstanceIndex  nFontInstanceIndex
                                  );

void ATSUIPrintFontList( const ItemCount  nFontItemsCount,
                         FontNamesMatrix  hFontList
                       );

// =======================================================================

// =======================================================================

#ifdef __cplusplus
}
#endif

#endif  // _SV_SALATSUIFONTUTILS_HXX

