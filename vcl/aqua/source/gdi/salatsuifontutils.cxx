/*************************************************************************
 *
 *  $RCSfile: salatsuifontutils.cxx,v $
 *
 *  $Revision: 1.1 $
 *  last change: $Author: bmahbod $ $Date: 2001-03-12 23:15:32 $
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

#define _SV_SALATSUIFONTUTILS_CXX

#ifndef _SV_SALATSUIFONTUTILS_HXX
    #include <salatsuifontutils.hxx>
#endif

// =======================================================================

// =======================================================================

static SInt16 AppendMenuName( MenuHandle  hMenu,
                              long        nMenuLength,
                              char        pMenuName[],
                              ScriptCode  nMenuScriptCode
                            );

static MacOSBoolean ATSUIBetterEncoding( const FontNameEncodingPtr  pFontNewNameEncoding,
                                         const FontNameEncodingPtr  pFontOldNameEncoding,
                                         const FontNameEncodingPtr  pFontFamilyNameEncodingPrefs
                                       );

static FontNamesPtr ATSUIBuildFontNamesRecord( ATSUFontID          nFontID,
                                               ATSUIInstanceIndex  nFontInstanceIndex,
                                               FontNameCode        nFontNameCode
                                             );

static long ATSUICompareFontNames( FontNamesPtr pFontName1,
                                   FontNamesPtr pFontName2
                                 );

static void ATSUIInsertFontInSortedList( FontNamesPtr  pFontNames,
                                         FontNamesPtr  pSortedFontList[],
                                         ItemCount    *pFontListLength
                                       );

static ATSUIInstanceIndex ATSUIInstanceMatchingDefaultVariations( ATSUFontID nFontID );

static ATSUIInstanceIndex ATSUIInstanceMatchingVariations( ATSUFontID                    nFontID,
                                                           ItemCount                     nFontVariationsCount,
                                                           const ATSUFontVariationValue  pFontVariationValuesToMatch[]
                                                         );

static ItemCount ATSUILookForFontInstances( ATSUFontID  *rFontIDList,
                                            ItemCount    nFontItemsCount
                                          );

static void FontListToHierarchialMenu( MenuHandle               hFontMenu,
                                       short                    nHierarchiaFontMenuID,
                                       const FontNamesMatrix    hFontList,
                                       ItemCount                nFontItemsCount,
                                       short                   *rFontSubmenusCount,
                                       void                   **hFontMenuLookupCookie
                                     );

static MenuHandle NewMenuName( short  nMenuID,
                               long   nMenuItemLength,
                               char   nMenuTitle[]
                             );

// =======================================================================

// =======================================================================

OSStatus ATSUIGetOneAttribute( ATSUStyle              aFontStyle,
                               ATSUAttributeTag       nFontAttributeTag,
                               ByteCount              nFontExpValueSize,
                               ATSUAttributeValuePtr  rFontAttributeValue
                             )
{
    // Filters out kATSUNotSetErr

    OSStatus   nGetAttributeStatus = noErr;
    OSStatus   nAttributeStatus    = noErr;
    ByteCount *pFontActValueSize   = NULL;

    nGetAttributeStatus = ATSUGetAttribute( aFontStyle,
                                            nFontAttributeTag,
                                            nFontExpValueSize,
                                            rFontAttributeValue,
                                            pFontActValueSize
                                          );

    if ( nGetAttributeStatus != kATSUNotSetErr )
    {
        nAttributeStatus = nGetAttributeStatus;
    }

    return nAttributeStatus;
} // ATSUIGetOneAttribute

// -----------------------------------------------------------------------

OSStatus ATSUISetIntAttribute( ATSUStyle         aFontStyle,
                               SInt32            nFontAttributeValue,
                               ATSUAttributeTag  nFontAttributeTag
                             )
{
    ByteCount               nFontExpValueSize    = sizeof(nFontAttributeValue);
    ATSUAttributeValuePtr   pFontAttributeValue  = &nFontAttributeValue;
    ItemCount               nFontAttributeCount  = 1;
    OSStatus                nSetAttributesStatus = noErr;

    if ( IsPointerValid( (MacOSPtr)pFontAttributeValue ) )
    {
        nSetAttributesStatus = ATSUSetAttributes(  aFontStyle,
                                                   nFontAttributeCount,
                                                  &nFontAttributeTag,
                                                  &nFontExpValueSize,
                                                  &pFontAttributeValue
                                                );
    } // if

    return nSetAttributesStatus;
} // ATSUISetIntAttribute

// -----------------------------------------------------------------------
//
// Given a font ID number, a font instance index, and name code for the
// style name, we shall attempt to find a usable matching strings for the
// font family and style name, and build up a font names structure. The
// font names structure packs family and all style strings together.
//
// -----------------------------------------------------------------------

static FontNamesPtr ATSUIBuildFontNamesRecord( ATSUFontID          nFontID,
                                               ATSUIInstanceIndex  nFontInstanceIndex,
                                               FontNameCode        nFontNameCode
                                             )
{
    ByteCount         nFontFamilyMaxNameLength = 0;
    ByteCount         nFontFamilyNameLength    = 0;
    FontNamesPtr      pFontFamilyNames         = NULL;
    FontNameCode      nFontFamilyNameCode      = kFontFamilyName;
    FontLanguageCode  nFontFamilyLanguageCode  = kFontNoLanguage;
    FontPlatformCode  nFontFamilyPlatformCode  = kFontNoPlatform;
    FontScriptCode    nFontFamilyScriptCode    = kFontNoScript;
    ItemCount         nFontFamilyIndex         = 0;
    OSStatus          nStatus                  = noErr;
    MacOSPtr          pFontFamilyName          = NULL;

    // If one needs font names that match the font's (QD Text) script, we
    // may get the corresponding QD family ID, and the script for that
    // family, and then pass this parameter to the ATSUIFindBestFontName()
    // API, that can be found in this file. This will in turn force ATSUI
    // to look for a name in a desired script first.

    nStatus = ATSUIFindBestFontName(  nFontID,
                                      nFontFamilyNameCode,
                                     &nFontFamilyPlatformCode,
                                     &nFontFamilyScriptCode,
                                     &nFontFamilyLanguageCode,
                                      nFontFamilyMaxNameLength,
                                      pFontFamilyName,
                                     &nFontFamilyNameLength,
                                     &nFontFamilyIndex
                                   );

    // If there was no Mac name there is nothing to be done here.
    // Instead, we shall attempt to convert the font name first.

    if (    ( nStatus == noErr )
         && ( nFontFamilyPlatformCode == kFontMacintoshPlatform )
       )
    {
        ByteCount         nFontStyleNameLength   = 0;
        FontLanguageCode  nFontStyleLanguageCode = nFontFamilyLanguageCode;
        FontPlatformCode  nFontStylePlatformCode = nFontFamilyPlatformCode;
        FontScriptCode    nFontStyleScriptCode   = nFontFamilyScriptCode;
        ItemCount         nFontStyleIndex        = 0;

        // Find the font's style name whilst attempting to match
        // the family name's encoding and language.

        nStatus = ATSUIFindBestFontName(  nFontID,
                                          nFontNameCode,
                                         &nFontStylePlatformCode,
                                         &nFontStyleScriptCode,
                                         &nFontStyleLanguageCode,
                                          nFontFamilyMaxNameLength,
                                          pFontFamilyName,
                                         &nFontStyleNameLength,
                                         &nFontStyleIndex
                                       );

        // As before, if there was no Mac name there is no need to continue.
        // Again, we shall attempt to convert the font name first.

        if (    ( nStatus == noErr )
             && ( nFontStylePlatformCode == kFontMacintoshPlatform )
           )
        {
            long  nFontNamesOffset = offsetof(FontNamesRecord, mpFontName);
            long  nFontNameSize    =   nFontNamesOffset
                                     + nFontFamilyNameLength
                                     + nFontStyleNameLength;

            pFontFamilyNames = (FontNamesPtr)NewPtrClear( nFontNameSize );

            nStatus = MemError();

            if (    ( nStatus == noErr)
                 && ( IsPointerValid( (MacOSPtr)pFontFamilyNames ) )
               )
            {
                FontNameCode      *pFontNameCode         = NULL;
                FontPlatformCode  *pFontNamePlatformCode = NULL;
                FontScriptCode    *pFontNameScriptCode   = NULL;
                FontLanguageCode  *pFontNameLanguageCode = NULL;

                pFontFamilyNames->mnFontID                        = nFontID;
                pFontFamilyNames->maFontFamilyNameEncoding.mnFontScriptCode   = nFontFamilyScriptCode;
                pFontFamilyNames->maFontFamilyNameEncoding.mnFontPlatformCode = nFontFamilyPlatformCode;
                pFontFamilyNames->maFontFamilyNameEncoding.mnFontLanguageCode = nFontFamilyLanguageCode;

                nStatus = ATSUGetIndFontName(   nFontID,
                                                nFontFamilyIndex,
                                                nFontFamilyNameLength,
                                               &pFontFamilyNames->mpFontName[ 0 ],
                                               &pFontFamilyNames->maFontFamilyNameEncoding.mnFontItemCount,
                                                pFontNameCode,
                                                pFontNamePlatformCode,
                                                pFontNameScriptCode,
                                                pFontNameLanguageCode
                                            );

                if ( nStatus == noErr)
                {
                    pFontFamilyNames->mnFontStyleByteOffset          = nFontFamilyNameLength;
                    pFontFamilyNames->maFontStyleNameEncoding.mnFontScriptCode   = nFontStyleScriptCode;
                    pFontFamilyNames->maFontStyleNameEncoding.mnFontPlatformCode = nFontStylePlatformCode;
                    pFontFamilyNames->maFontStyleNameEncoding.mnFontLanguageCode = nFontStyleLanguageCode;

                    nStatus = ATSUGetIndFontName(   nFontID,
                                                    nFontStyleIndex,
                                                    nFontStyleNameLength,
                                                   &pFontFamilyNames->mpFontName[ nFontFamilyNameLength ],
                                                   &pFontFamilyNames->maFontStyleNameEncoding.mnFontItemCount,
                                                    pFontNameCode,
                                                    pFontNamePlatformCode,
                                                    pFontNameScriptCode,
                                                    pFontNameLanguageCode
                                                );

                    if ( nStatus == noErr )
                    {
                        pFontFamilyNames->mnFontInstanceIndex = nFontInstanceIndex;

                        // Filter out invisible fonts.

                        if (    ( pFontFamilyNames->mpFontName[ 0 ] == kInvalidFontNamePrefixPeriodSign  )
                             || ( pFontFamilyNames->mpFontName[ 0 ] == kInvalidFontNamePrefixPercentSign )
                           )
                        {
                            nStatus = kATSUInvalidFontErr;
                        } // if

                        if (    ( nStatus          != noErr )
                             && ( pFontFamilyNames != NULL  )
                           )
                        {
                            DisposePtr( (MacOSPtr) pFontFamilyNames );

                            pFontFamilyNames = NULL;
                        } // if
                    } // if
                    else
                    {
                        DisposePtr( (MacOSPtr) pFontFamilyNames );

                        pFontFamilyNames = NULL;
                    } // else
                } // if
                else
                {
                    DisposePtr( (MacOSPtr) pFontFamilyNames );

                    pFontFamilyNames = NULL;
                } // else
            } // if
        } // if
    } // if

    return pFontFamilyNames;
} // ATSUIBuildFontNamesRecord

// -----------------------------------------------------------------------
//
// Sort the font by family name and then by style.
//
// -----------------------------------------------------------------------

static long ATSUICompareFontNames( FontNamesPtr pFontName1,
                                   FontNamesPtr pFontName2
                                 )
{
    long      nTextOrder       = 0;
    LangCode  pFontLangCode[2] = { currentDefLang, currentDefLang };

    nTextOrder = TextOrder( &pFontName1->mpFontName[ 0 ],
                            &pFontName2->mpFontName[ 0 ],
                             pFontName1->maFontFamilyNameEncoding.mnFontItemCount,
                             pFontName2->maFontFamilyNameEncoding.mnFontItemCount,
                             pFontName1->maFontFamilyNameEncoding.mnFontScriptCode,
                             pFontName2->maFontFamilyNameEncoding.mnFontScriptCode,
                             pFontLangCode[0],
                             pFontLangCode[1]
                           );

    if ( nTextOrder == 0 )
    {
        nTextOrder = TextOrder( &pFontName1->mpFontName[ pFontName1->mnFontStyleByteOffset ],
                                &pFontName2->mpFontName[ pFontName2->mnFontStyleByteOffset ],
                                 pFontName1->maFontStyleNameEncoding.mnFontItemCount,
                                 pFontName2->maFontStyleNameEncoding.mnFontItemCount,
                                 pFontName1->maFontStyleNameEncoding.mnFontScriptCode,
                                 pFontName2->maFontStyleNameEncoding.mnFontScriptCode,
                                 pFontLangCode[0],
                                 pFontLangCode[1]
                              );
    } // if

    return nTextOrder;
} // ATSUICompareFontNames

// -----------------------------------------------------------------------

static void ATSUIInsertFontInSortedList( FontNamesPtr   pFontNames,
                                         FontNamesPtr   pSortedFontList[],
                                         ItemCount     *pFontListLength
                                       )
{
    if ( IsPointerValid( (MacOSPtr)pFontNames ) )
    {
        long  nHighIndex   = *pFontListLength;
        long  nLowIndex    =  0;
        long  nInsertIndex =  0;
        long  nTextOrder   =  0;

        // Find the insertion position and then start Binary search procedure.

        nInsertIndex = nLowIndex + (nHighIndex - nLowIndex) / 2;

        while ( nLowIndex < nHighIndex )
        {
            nTextOrder = ATSUICompareFontNames( pFontNames,
                                                pSortedFontList[ nInsertIndex ]
                                              );

            if ( nTextOrder < 0 )
            {
                // New font goes before the current high index

                nHighIndex = nInsertIndex;
            } // if
            else if ( nTextOrder > 0 )
            {
                // New font goes after the current low index

                nLowIndex = nInsertIndex + 1;
            } // else if
            else
            {
                // same as the current index, we infer duplicates,
                // and hence avoid inserting duplicates in the list

                nInsertIndex = -1;

                break;
            } // else

            nInsertIndex = nLowIndex + (nHighIndex - nLowIndex) / 2;
        } // while

        if ( nInsertIndex >= 0 )
        {
            MacOSSize  aByteCount = (*pFontListLength - nInsertIndex) * sizeof(FontNamesPtr);

            // Add a font to our current list.

            BlockMoveData( &pSortedFontList[ nInsertIndex ],
                           &pSortedFontList[ nInsertIndex + 1 ],
                           aByteCount
                         );

            pSortedFontList[ nInsertIndex ] = pFontNames;

            *pFontListLength += 1;
        } // if
    } // while
} // ATSUIInsertFontInSortedList

// -----------------------------------------------------------------------
//
// Create a new menu with the given ID and title
//
// -----------------------------------------------------------------------

static MenuHandle NewMenuName( short  nMenuID,
                               long   nMenuItemLength,
                               char   hMenuTitle[]
                             )
{
    MenuHandle  hMenu = NULL;

    if ( IsPointerValid( (MacOSPtr)hMenuTitle ) )
    {
        Str255  pMenuItemNameBuffer;

        if ( nMenuItemLength < 255 )
        {
            pMenuItemNameBuffer[ 0 ] = nMenuItemLength;
        } // if
        else
        {
            pMenuItemNameBuffer[ 0 ] = 255;
        } // else

        BlockMove(  hMenuTitle,
                   &pMenuItemNameBuffer[ 1 ],
                    pMenuItemNameBuffer[ 0 ]
                 );

        hMenu = NewMenu( nMenuID, pMenuItemNameBuffer );
    } // if

    return hMenu;
} // NewMenuName

// -----------------------------------------------------------------------
//
// Append the given string to the menu
//
// -----------------------------------------------------------------------

static SInt16 AppendMenuName( MenuHandle  hMenu,
                              long        nMenuLength,
                              char        pMenuName[],
                              ScriptCode  nMenuScriptCode
                            )
{
    SInt16  nItemNum = 0;

    if (    ( IsHandleValid( (Handle)hMenu ) )
         && ( IsPointerValid( (MacOSPtr)pMenuName ) )
       )
    {
        OSErr   nStatus  = noErr;
        Str255  pMenuBufferName;

        if ( nMenuLength <= kMenuBufferMaxLength )
        {
            pMenuBufferName[ 0 ] = nMenuLength;
        } // if
        else
        {
            short  nCharByteType = 0;

            pMenuBufferName[ 0 ] = kMenuBufferMaxLength;

            nCharByteType = CharacterByteType( pMenuName,
                                               kMenuBufferMaxLength,
                                               nMenuScriptCode
                                             );

            if (  nCharByteType > smSingleByte )
            {
                // truncate properly for multi-byte.

                --pMenuBufferName[ 0 ];
            } // if
        } // else

        BlockMoveData(  pMenuName,
                       &pMenuBufferName[ 1 ],
                        pMenuBufferName[ 0 ]
                     );

        // avoid metacharacter processing

        AppendMenu( hMenu, kMenuItemGenericName );

        nItemNum = CountMenuItems( hMenu );

        SetMenuItemText( hMenu,
                         nItemNum,
                         pMenuBufferName
                       );

        nStatus = SetMenuItemTextEncoding( hMenu,
                                           nItemNum,
                                           nMenuScriptCode
                                         );
    } // if

    return nItemNum;
} // AppendMenuName

// -----------------------------------------------------------------------
//
// Construct the font hierarchial menu using our font ID list.
//
// -----------------------------------------------------------------------

static void FontListToHierarchialMenu( MenuHandle               hFontMenu,
                                       short                    nHierarchiaFontMenuID,
                                       const FontNamesMatrix    hFontList,
                                       ItemCount                nFontItemsCount,
                                       short                   *rFontSubmenusCount,
                                       void                   **hFontMenuLookupCookie
                                     )
{
     hFontMenuLookupCookie = NULL;
    *rFontSubmenusCount    = 0;

    if ( nFontItemsCount > 0 )
    {
        long                 nFontMenuHandleSize = nFontItemsCount * sizeof(FontMenuItemRecord);
        OSStatus             nStatus             = noErr;
        FontMenuItemsHandle  hFontMenuItems      = NULL;

        hFontMenuItems = (FontMenuItemsHandle)NewHandleClear( nFontMenuHandleSize );

        nStatus = MemError();

        if (    ( nStatus == noErr )
             && ( IsHandleValid( (Handle)hFontMenuItems )  )
           )
        {
            SInt8  nFlags = noErr;

            nFlags = HGetState( (Handle)hFontMenuItems );

            if ( nFlags == noErr )
            {
                long        i                    = 0;
                long        j                    = 0;
                long        k                    = 0;
                long        nFontMenuItem        = 0;
                long        nRecordNum           = 0;
                long        nTextOrder           = 0;
                long        nFontSubItemNum      = 0;
                short       nFontSubMenuID       = 0;
                short       nFontSubmenusCreated = 0;
                LangCode    aFontLangCode[2]     = { currentDefLang, currentDefLang };
                MenuHandle  hFontSubMenu         = NULL;
                OSStatus    nStatus              = noErr;

                HLock( (Handle)hFontMenuItems );

                    while ( i < nFontItemsCount )
                    {
                        // Collect up the fonts with the same family name

                        for ( j = i + 1; j < nFontItemsCount; j++ )
                        {
                            nTextOrder = TextOrder( &hFontList[ i ]->mpFontName[ 0 ],
                                                    &hFontList[ j ]->mpFontName[ 0 ],
                                                     hFontList[ i ]->maFontFamilyNameEncoding.mnFontItemCount,
                                                     hFontList[ j ]->maFontFamilyNameEncoding.mnFontItemCount,
                                                     hFontList[ i ]->maFontFamilyNameEncoding.mnFontScriptCode,
                                                     hFontList[ j ]->maFontFamilyNameEncoding.mnFontScriptCode,
                                                     aFontLangCode[0],
                                                     aFontLangCode[1]
                                                  );

                            if ( nTextOrder != 0 )
                            {
                                break;
                            } // if
                        } // for

                        // Should the style name be added to the top level if there's only one style?
                        // How about using full name instead?

                        nFontMenuItem = AppendMenuName(  hFontMenu,
                                                         hFontList[ i ]->maFontFamilyNameEncoding.mnFontItemCount,
                                                        &hFontList[ i ]->mpFontName[ 0 ],
                                                         hFontList[ i ]->maFontFamilyNameEncoding.mnFontScriptCode
                                                      );

                        // Add a submenu for the styles

                        if ( j - i > 1 )
                        {
                            nFontSubMenuID = nHierarchiaFontMenuID + nFontSubmenusCreated++;

                            hFontSubMenu   = NewMenuName(  nFontSubMenuID,
                                                           hFontList[ i ]->maFontFamilyNameEncoding.mnFontItemCount,
                                                          &hFontList[ i ]->mpFontName[ 0 ]
                                                        );

                            if ( hFontSubMenu != NULL )
                            {
                                for ( k = i; k < j; k++ )
                                {
                                    nFontSubItemNum
                                        = AppendMenuName(  hFontSubMenu,
                                                               hFontList[ k ]->maFontStyleNameEncoding.mnFontItemCount,
                                                              &hFontList[ k ]->mpFontName[ hFontList[ k ]->mnFontStyleByteOffset ],
                                                               hFontList[ k ]->maFontStyleNameEncoding.mnFontScriptCode
                                                            );

                                    if (    ( kATSUIFontInstanceMenuItemStyle     != 0                     )
                                         && ( hFontList[ k ]->mnFontInstanceIndex != kATSUIDefaultInstance )
                                       )
                                    {
                                        SetItemStyle( hFontSubMenu,
                                                      nFontSubItemNum,
                                                      kATSUIFontInstanceMenuItemStyle
                                                    );
                                    } //if

                                    // leave a trail so we can recover the font ID

                                    (*hFontMenuItems)[ nRecordNum ].mnFontID            = hFontList[ k ]->mnFontID;
                                    (*hFontMenuItems)[ nRecordNum ].mnFontInstanceIndex = hFontList[ k ]->mnFontInstanceIndex;
                                    (*hFontMenuItems)[ nRecordNum ].mnFontMenuID        = nFontSubMenuID;
                                    (*hFontMenuItems)[ nRecordNum ].mnFontItemNum       = nFontSubItemNum;

                                    nRecordNum += 1;
                                } // for

                                InsertMenu( hFontSubMenu, hierMenu );

                                nStatus = SetMenuItemHierarchicalID( hFontMenu,
                                                                     nFontMenuItem,
                                                                     nFontSubMenuID
                                                                   );
                            } //if
                        } // if
                        else
                        {
                            // leave a trail so we can recover the font ID

                            (*hFontMenuItems)[ nRecordNum ].mnFontID            = hFontList[ i ]->mnFontID;
                            (*hFontMenuItems)[ nRecordNum ].mnFontInstanceIndex = hFontList[ i ]->mnFontInstanceIndex;
                            (*hFontMenuItems)[ nRecordNum ].mnFontMenuID        = GetMenuID(hFontMenu);
                            (*hFontMenuItems)[ nRecordNum ].mnFontItemNum       = nFontMenuItem;

                            nRecordNum += 1;

                        } // else

                        i = j;  // next family name
                    } // while

                    if ( rFontSubmenusCount != NULL )
                    {
                        *rFontSubmenusCount = nFontSubmenusCreated;
                    } // if

                    *hFontMenuLookupCookie = hFontMenuItems;

                HSetState( (Handle)hFontMenuItems, nFlags );
            } // if
            else
            {
                // If we can't lock the font menu items handle, dispose of it
                // and exit

                DisposeHandle( (Handle)hFontMenuItems );
            } // else
        } // if
    } // if
} // FontListToHierarchialMenu

// -----------------------------------------------------------------------
//
// Loop throught our font ID list, count the font instances, and if a
// "bad" font is encountered, remove it from the font ID list.
//
// -----------------------------------------------------------------------

static ItemCount ATSUILookForFontInstances( ATSUFontID  *rFontIDList,
                                            ItemCount    nFontItemsCount
                                          )
{
    ItemCount  nFontInstancesCount = 0;

    if ( IsPointerValid( (MacOSPtr)rFontIDList ) )
    {
        OSStatus    nStatus         = noErr;
        ItemCount   nFontsInstances = 0;
        long        nFontInstanceIndex;

        for ( nFontInstanceIndex = 0;
              nFontInstanceIndex < nFontItemsCount;
              ++nFontInstanceIndex
            )
        {
            nStatus = ATSUCountFontInstances(  rFontIDList[ nFontInstanceIndex ],
                                              &nFontsInstances
                                            );

            if ( nStatus == noErr )
            {
                nFontInstancesCount += nFontsInstances;
            }
            else
            {
                rFontIDList[ nFontInstanceIndex ] = kATSUInvalidFontID;
            }
        } // for
    } // if

    return nFontInstancesCount;
} // ATSUILookForFontInstances

// -----------------------------------------------------------------------
//
// Loop through a font's instances, if any, and check whether any of them
// match the given variations.  The input array must be complete and
// in canonical order. If unsuccessful and no match were to be found, then
// we shall return the constant kATSUINotAnInstance.
//
// -----------------------------------------------------------------------

static ATSUIInstanceIndex ATSUIInstanceMatchingVariations( ATSUFontID                    nFontID,
                                                           ItemCount                     nFontVariationsCount,
                                                           const ATSUFontVariationValue  pFontVariationsToMatch[]
                                                         )
{
    ATSUIInstanceIndex  nFontInstanceIndex  = kATSUINotAnInstance;
    ItemCount           nFontInstancesCount = 0;
    OSStatus            nStatus             = noErr;

    nStatus = ATSUCountFontInstances( nFontID, &nFontInstancesCount );

    if ( nStatus == noErr )
    {
        ATSUFontVariationValue   pFontVariationInstanceValue[ kATSUIMaxVariations ];
        ATSUFontVariationAxis   *pFontVariationAxis        = NULL;
        ItemCount                nFontIndex                = 0;
        ItemCount                nFontMatchIndex           = 0;
        ItemCount                nFontActualVariationCount = 0;

        // Values in an instance are always in canonical order,
        // and hence we shall utilize these values and compare
        // them directly.

        for ( nFontIndex = 0;
              nFontIndex < nFontInstancesCount;
              ++nFontIndex
            )
        {
            nStatus = ATSUGetFontInstance(   nFontID,
                                             nFontIndex,
                                             nFontVariationsCount,
                                             pFontVariationAxis,
                                             pFontVariationInstanceValue,
                                            &nFontActualVariationCount
                                         );

            if (    ( nStatus == noErr )
                 && ( nFontActualVariationCount == nFontVariationsCount )
               )
            {
                nFontInstanceIndex = nFontIndex;

                for ( nFontMatchIndex = 0;
                         nFontMatchIndex < nFontVariationsCount
                      && nFontIndex      < kATSUIMaxVariations;
                      ++nFontMatchIndex
                    )
                {
                    if (    pFontVariationsToMatch[ nFontMatchIndex ]
                         != pFontVariationInstanceValue[ nFontMatchIndex ]
                       )
                    {
                        nFontInstanceIndex = kATSUINotAnInstance;

                        break;
                    } // if
                } // for

                if ( nFontInstanceIndex != kATSUINotAnInstance )
                {
                    break;
                } // if
            } // if
        } // for
    } // if

    return nFontInstanceIndex;
} // ATSUIInstanceMatchingVariations

// -----------------------------------------------------------------------
//
// Find the font instance index, that matches the default variation
// values, and if we were unable to find such a match, then we shall
// kATSUIDefaultInstance.
//
// -----------------------------------------------------------------------

static ATSUIInstanceIndex ATSUIInstanceMatchingDefaultVariations( ATSUFontID nFontID )
{
    ItemCount  nFontInstancesCount = 0;
    OSStatus   nStatus             = noErr;

    nStatus = ATSUCountFontInstances( nFontID, &nFontInstancesCount );

    if ( nStatus == noErr )
    {
        if ( nFontInstancesCount == 0 )
        {
            return kATSUIDefaultInstance;
        } // if
        else
        {
            ItemCount  nFontVariationsCount = 0;

            nStatus = ATSUCountFontVariations( nFontID, &nFontVariationsCount );

            if ( nStatus == noErr )
            {
                if ( nFontVariationsCount > 0 )
                {
                    long                     nFontVariationIndex        = 0;
                    ATSUIInstanceIndex       nFoundInstanceIndex        = 0;
                    ATSUFontVariationAxis   *pATSUFontVariationAxis     = NULL;
                    ATSUFontVariationValue  *pFontMinimumVariationValue = NULL;
                    ATSUFontVariationValue  *pFontMaximumVariationValue = NULL;
                    ATSUFontVariationValue   pFontDefaultVariationValues[ kATSUIMaxVariations ];

                    // Get the variations in canonical order, as we only require
                    // the font variation values.

                    for ( nFontVariationIndex = 0;
                             nFontVariationIndex < nFontVariationsCount
                          && nFontVariationIndex < kATSUIMaxVariations;
                          ++nFontVariationIndex
                        )
                    {
                        nStatus = ATSUGetIndFontVariation(  nFontID,
                                                            nFontVariationIndex,
                                                            pATSUFontVariationAxis,
                                                            pFontMinimumVariationValue,
                                                            pFontMaximumVariationValue,
                                                           &pFontDefaultVariationValues[ nFontVariationIndex ]
                                                         );

                        if ( nStatus != noErr )
                        {
                            return kATSUIDefaultInstance;
                        } // if
                    } // for

                    nFoundInstanceIndex = ATSUIInstanceMatchingVariations( nFontID,
                                                                           nFontVariationsCount,
                                                                           pFontDefaultVariationValues
                                                                         );

                    if ( nFoundInstanceIndex != kATSUINotAnInstance )
                    {
                        return nFoundInstanceIndex;
                    } // if
                } // if
            } // if
            else
            {
                return kATSUIDefaultInstance;
            } // else
        } // else
    } // if

    return kATSUIDefaultInstance;
} // ATSUIInstanceMatchingDefaultVariations

// -----------------------------------------------------------------------
//
// Here we shall construct a hierarchial font menu. The top level of our
// font menu consists of font family name, and if there's more than one
// style for a given family, a submenu is created for those styles. We
// will  treat font instances as styles.
//
// Note that:
//
// * If there's only one representative of a family, we shall insert the
//   full font name into the menu instead of only the family name. We can
//   not determine this immediately until all the fonts are sorted.
//
// * If there are multiple fonts with the same name, all but the first
//   occurance of this particular font is kept.
//
// -----------------------------------------------------------------------

OSStatus ATSUIAppendFontMenu( MenuHandle    hFontMenu,
                              short         nHierarchiaFontMenuID,
                              short        *rSubmenuCount,
                              void        **hFontMenuLookupCookie
                            )
{
    OSStatus   nStatus        = noErr;
    ItemCount  nFontItemsSize = 0;

    nStatus = ATSUFontCount( &nFontItemsSize );

    if (    ( nStatus == noErr )
         && ( nFontItemsSize > 0 )
       )
    {
        ATSUFontID   *pFontIDList     = NULL;
        long          nFontIDListSize = nFontItemsSize * sizeof(ATSUFontID);

        pFontIDList = (ATSUFontID *)NewPtrClear( nFontIDListSize );

        nStatus = MemError();

        if (    ( nStatus == noErr )
             && ( IsPointerValid( (MacOSPtr)pFontIDList ) )
           )
        {
            ItemCount  *pFontItemsCount = NULL;

            nStatus = ATSUGetFontIDs( pFontIDList,
                                      nFontItemsSize,
                                      pFontItemsCount
                                    );

            if ( nStatus == noErr )
            {
                long             nSortedFontListSize   = 0;
                ItemCount        nTotalFontInstances   = 0;
                ItemCount        nSortedFontListLength = 0;
                FontNamesMatrix  hSortedFontList       = NULL;

                nTotalFontInstances = ATSUILookForFontInstances( pFontIDList, nFontItemsSize );

                // The allocated size for the sorted font list might be larger than
                // neccessary because of "bad" fonts, but at least we have had more
                // than enough memory storage.

                nSortedFontListSize =   (nFontItemsSize + nTotalFontInstances)
                                      * sizeof(FontNamesPtr);

                hSortedFontList = (FontNamesMatrix)NewPtrClear( nSortedFontListSize );

                nStatus = MemError();

                if (    ( nStatus == noErr )
                     && ( IsPointerValid( (MacOSPtr)hSortedFontList ) )
                   )
                {
                    ATSUIInstanceIndex  nFontInstanceIndex    = 0;
                    FontNamesPtr        pFontNames            = NULL;
                    FontNameCode        nFontInstanceNameCode = 0;
                    ItemCount           nFontInstance         = 0;
                    ItemCount           nFontInstancesCount   = 0;
                    ItemCount           nFontIndex;

                    for ( nFontIndex = 0;
                          nFontIndex < nFontItemsSize;
                          nFontIndex++
                        )
                    {
                        if ( pFontIDList[ nFontIndex ] != kATSUInvalidFontID )
                         {
                            // If there exists a name list then add the font's
                            // instances to this particular list.

                            nStatus = ATSUCountFontInstances(  pFontIDList[ nFontIndex ],
                                                              &nFontInstancesCount
                                                            );

                            for ( nFontInstance = 0;
                                  nFontInstance < nFontInstancesCount;
                                  ++nFontInstance
                                )
                            {
                                nStatus = ATSUGetFontInstanceNameCode(  pFontIDList[ nFontIndex ],
                                                                        nFontInstance,
                                                                       &nFontInstanceNameCode
                                                                     );

                                if ( nStatus == noErr )
                                {
                                    pFontNames = ATSUIBuildFontNamesRecord( pFontIDList[ nFontIndex ],
                                                                            nFontInstance,
                                                                            nFontInstanceNameCode
                                                                          );

                                    if ( pFontNames != NULL )
                                    {
                                        ATSUIInsertFontInSortedList(  pFontNames,
                                                                      hSortedFontList,
                                                                     &nSortedFontListLength
                                                                   );
                                    } // if
                                } // if
                            } // if

                            // Now add the font. However, and if this font has the same name as
                            // an instance index, skip this font, and defer any actions until
                            // name collisions issue is resolved.

                            nFontInstanceIndex
                                = ATSUIInstanceMatchingDefaultVariations( pFontIDList[ nFontIndex ] );

                            if ( nFontInstanceIndex == kATSUIDefaultInstance )
                            {
                                pFontNames = ATSUIBuildFontNamesRecord( pFontIDList[ nFontIndex ],
                                                                        kATSUIDefaultInstance,
                                                                        kFontStyleName
                                                                      );

                                if ( pFontNames != NULL )
                                {
                                    ATSUIInsertFontInSortedList(   pFontNames,
                                                                   hSortedFontList,
                                                                  &nSortedFontListLength
                                                               );
                                } // if
                            } // if
                        } // if
                    } // for

                    FontListToHierarchialMenu( hFontMenu,
                                               nHierarchiaFontMenuID,
                                               hSortedFontList,
                                               nSortedFontListLength,
                                               rSubmenuCount,
                                               hFontMenuLookupCookie
                                             );

                    for ( nFontIndex = 0;
                          nFontIndex < nSortedFontListLength;
                          nFontIndex++
                        )
                    {
                        if ( hSortedFontList[ nFontIndex ] != NULL )
                        {
                            DisposePtr( (MacOSPtr)hSortedFontList[ nFontIndex ] );
                        } // if
                    } // for

                    DisposePtr( (MacOSPtr)hSortedFontList );
                } // if
            } // if

            DisposePtr( (MacOSPtr)pFontIDList );
        } // if
    } // if

    return nStatus;
} // ATSUIAppendFontMenu

// -----------------------------------------------------------------------

ATSUFontID ATSUIGetFontForFontMenuItem( short                nMenuID,
                                        short                nMenuItemNum,
                                        void                *pMenuLookupCookie,
                                        ATSUIInstanceIndex  *rFontInstance
                                      )
{
    if (    ( IsPointerValid( (MacOSPtr)pMenuLookupCookie ) )
         && ( IsPointerValid( (MacOSPtr)rFontInstance     ) )
       )
    {
        // Have to search for this font in the data structure we're
        // about to build.

        FontMenuItemsHandle  hFontMenuItems = (FontMenuItemsHandle)pMenuLookupCookie;

        if ( IsHandleValid( (Handle)hFontMenuItems ) )
        {
            SInt8  nFlags = noErr;

            nFlags = HGetState( (Handle)hFontMenuItems );

            if ( nFlags == noErr )
            {
                short  nFontMenuItemHandleSize = GetHandleSize( (Handle)hFontMenuItems );
                short  nFontMenuItemRecSize    = sizeof(FontMenuItemRecord);
                short  nMenuEntriesCount       = nFontMenuItemHandleSize / nFontMenuItemRecSize;
                short  nMenuEntriesIndex;

                HLock( (Handle)hFontMenuItems );

                    // To find the font, perform a linear search.

                    for ( nMenuEntriesIndex = 0;
                          nMenuEntriesIndex < nMenuEntriesCount;
                          ++nMenuEntriesIndex
                        )
                    {
                        if (    ( nMenuID      == (*hFontMenuItems)[nMenuEntriesIndex ].mnFontMenuID   )
                             && ( nMenuItemNum == (*hFontMenuItems)[ nMenuEntriesIndex ].mnFontItemNum )
                           )
                        {
                            if ( rFontInstance != NULL )
                            {
                                *rFontInstance = (*hFontMenuItems)[ nMenuEntriesIndex ].mnFontInstanceIndex;
                            } // if

                            // Set font menu items handle state before leaving

                            HSetState( (Handle)hFontMenuItems, nFlags );

                            return (*hFontMenuItems)[ nMenuEntriesIndex ].mnFontID;
                        } // if
                    } // for

                // If we get here then our linear search yielded no results

                HSetState( (Handle)hFontMenuItems, nFlags );
            }// if
        } // if
    } // if

    // If we get here, either we didn't find the font, an invalid
    // pointer was passed in, or there was a problem with the handle
    // we constructed.

    return kATSUInvalidFontID;
} // ATSUIGetFontForFontMenuItem

// -----------------------------------------------------------------------

OSStatus ATSUIDisposeFontMenuLookupCookie( void *pMenuLookupCookie )
{
    OSStatus  nStatus = noErr;

    if ( IsHandleValid( (Handle)pMenuLookupCookie ) )
    {
        DisposeHandle( (Handle) pMenuLookupCookie );

        nStatus = MemError();
    } // if
    else
    {
        nStatus = nilHandleErr;
    } // else

    return nStatus;
} // ATSUIDisposeFontMenuLookupCookie

// -----------------------------------------------------------------------

OSStatus ATSUISetStyleFontInstance( ATSUStyle           aFontStyle,
                                    ATSUFontID          nFontID,
                                    ATSUIInstanceIndex  nFontInstanceIndex
                                  )
{
    ATSUFontVariationValue   pFontVariationValues[ kATSUIMaxVariations ];
    ATSUFontVariationAxis    pFontVariationAxis[ kATSUIMaxVariations ];
    ATSUFontVariationAxis   *pClearFontVariations = NULL;
    ItemCount                aClearFontAxisCount  = kATSUClearAll;
    ItemCount                nFontVariationsCount = 0;
    OSStatus                 nStatus              = noErr;

    if ( nFontInstanceIndex == kATSUIDefaultInstance )
    {
        nStatus = ATSUClearFontVariations( aFontStyle,
                                           aClearFontAxisCount,
                                           pClearFontVariations
                                         );

        if ( nStatus == noErr )
        {
            // If the font has variations, then place the default font
            // variation values into the style.  Doing so, allows one
            // to deal with font variations in a uniform manner and aid
            // the glyph cache.

            nStatus = ATSUCountFontVariations( nFontID, &nFontVariationsCount );

            if ( nStatus == noErr )
            {
                if (    ( nStatus == noErr )
                     && ( 0 < nFontVariationsCount )
                     && ( nFontVariationsCount < kATSUIMaxVariations )
                   )
                {
                    ATSUFontVariationValue  *pFontVariationMinimumValue = NULL;
                    ATSUFontVariationValue  *pFontVariationMaximumValue = NULL;
                    long                     nFontVariationIndex;

                    for ( nFontVariationIndex = 0;
                          nFontVariationIndex < nFontVariationsCount;
                          ++nFontVariationIndex
                        )
                    {
                        nStatus = ATSUGetIndFontVariation(  nFontID,
                                                            nFontVariationIndex,
                                                           &pFontVariationAxis[ nFontVariationIndex ],
                                                            pFontVariationMinimumValue,
                                                            pFontVariationMaximumValue,
                                                           &pFontVariationValues[ nFontVariationIndex ]
                                                         );

                        if ( nStatus != noErr )
                        {
                            return nStatus;
                        } // if
                    } // for

                    nStatus = ATSUSetVariations( aFontStyle,
                                                 nFontVariationsCount,
                                                 pFontVariationAxis,
                                                 pFontVariationValues
                                               );
                } // if
            } // if
        } // if
    } // if
    else if ( nFontInstanceIndex >= 0 )
    {
        ItemCount  nFontInstanceIndex = kATSUIMaxVariations;

        nStatus = ATSUGetFontInstance(  nFontID,
                                        nFontInstanceIndex,
                                        nFontInstanceIndex,
                                        pFontVariationAxis,
                                        pFontVariationValues,
                                       &nFontVariationsCount
                                     );

        if (    ( nStatus == noErr )
             && ( nFontVariationsCount < kATSUIMaxVariations )
           )
        {
            nStatus = ATSUClearFontVariations( aFontStyle,
                                               aClearFontAxisCount,
                                               pClearFontVariations
                                             );

            nStatus = ATSUSetVariations( aFontStyle,
                                         nFontVariationsCount,
                                         pFontVariationAxis,
                                         pFontVariationValues
                                       );
        } // if
    } // else if

    if ( nStatus == noErr )
    {
        nStatus = ATSUISetFont( aFontStyle, nFontID );
    } // if

    return nStatus;
} // ATSUISetStyleFontInstance

// -----------------------------------------------------------------------

OSStatus ATSUIGetStyleFontInstance( ATSUStyle            aFontStyle,
                                    ATSUFontID          *rFontID,
                                    ATSUIInstanceIndex  *rFontInstanceIndex
                                  )
{
    ATSUFontID          nFontID              = kATSUInvalidFontID;
    ATSUIInstanceIndex  nFontInstanceIndex   = 0;
    ItemCount           nFontInstancesCount  = 0;
    ItemCount           nFontVariationsCount = 0;
    OSStatus            nStatus              = noErr;

    // Determine font attributes including variations and instances

    nStatus = ATSUIGetFont( aFontStyle, &nFontID );

    if ( nStatus == noErr )
    {
        if ( nFontID != kATSUInvalidFontID )
        {
            nStatus = ATSUCountFontVariations( nFontID, &nFontVariationsCount );

            nStatus = ATSUCountFontInstances( nFontID, &nFontInstancesCount );
        } // if

        if ( nFontVariationsCount > 0 )
        {
            long                     nFontVariationIndex;
            ATSUFontVariationAxis    pFontVariationAxis[ kATSUIMaxVariations ];
            ATSUFontVariationValue   pFontStyleVaraitionValues[ kATSUIMaxVariations ];
            ATSUFontVariationValue  *pFontVariationMinimumValue = NULL;
            ATSUFontVariationValue  *pFontVariationMaximumValue = NULL;
            ItemCount                nFontStyleVariationCount   = 0;
            MacOSBoolean             bFontsDefaultValues        = true;

            // First we shall determine the font's variation axis and default
            // variation values.  This will further give us the canonical text
            // order for the variations.

            for ( nFontVariationIndex = 0;
                     nFontVariationIndex < nFontVariationsCount
                  && nFontVariationIndex < kATSUIMaxVariations;
                  ++nFontVariationIndex
                )
            {
                nStatus = ATSUGetIndFontVariation(  nFontID,
                                                    nFontVariationIndex,
                                                   &pFontVariationAxis[nFontVariationIndex],
                                                    pFontVariationMinimumValue,
                                                    pFontVariationMaximumValue,
                                                   &pFontStyleVaraitionValues[nFontVariationIndex]
                                                 );

                if ( nStatus != noErr )
                {
                    break;
                } // if
            } // if

            if ( nStatus == noErr )
            {
                // Here we shall determine all of particular font's style variations.
                // Also, we shall note whether or not these values are default font
                // settings.  Furthermore, we shall only proceed if the there are
                // no variations in the style.

                nStatus = ATSUICountStyleFontVariations( aFontStyle, &nFontStyleVariationCount );

                if (    ( nStatus == noErr )
                     && ( nFontStyleVariationCount > 0 )
                   )
                {
                    ATSUFontVariationValue  aFontVariationValue = 0;

                    for ( nFontVariationIndex = 0;
                          nFontVariationIndex < nFontVariationsCount;
                          ++nFontVariationIndex
                        )
                    {
                        nStatus = ATSUGetFontVariationValue(  aFontStyle,
                                                              pFontVariationAxis[nFontVariationIndex],
                                                             &aFontVariationValue
                                                           );

                        if (    ( nStatus == noErr )
                             && ( aFontVariationValue != pFontStyleVaraitionValues[nFontVariationIndex] )
                           )
                        {
                            // If the value's not been set, the we shall treat this as an error.

                            pFontStyleVaraitionValues[nFontVariationIndex] = aFontVariationValue;

                            bFontsDefaultValues = false;
                        }
                    }
                } // if

                // Do the search.

                nFontInstanceIndex = ATSUIInstanceMatchingVariations( nFontID,
                                                                      nFontVariationsCount,
                                                                      pFontStyleVaraitionValues
                                                                    );

                // The current settings may match the defaults, for which there
                // may not have been a named instance index.

                if (    ( nFontInstanceIndex == kATSUINotAnInstance )
                     && ( bFontsDefaultValues )
                   )
                {
                    nFontInstanceIndex = kATSUIDefaultInstance;
                } // if
            } // if
        } // if
        else
        {
            nFontInstanceIndex = kATSUIDefaultInstance;
        } // else

        if ( rFontID != NULL )
        {
            *rFontID = nFontID;
        } // if

        if ( rFontInstanceIndex != NULL )
        {
            *rFontInstanceIndex = nFontInstanceIndex;
        } // if
    } // if

    return nStatus;
} // ATSUIGetStyleFontInstance

// -----------------------------------------------------------------------
//
// Determines if the first font encoding is a better choice than the
// second font encoding.
//
// -----------------------------------------------------------------------

static MacOSBoolean ATSUIBetterEncoding( const FontNameEncodingPtr  pFontNewNameEncoding,
                                         const FontNameEncodingPtr  pFontOldNameEncoding,
                                         const FontNameEncodingPtr  pFontFamilyNameEncodingPrefs
                                       )
{
    MacOSBoolean  bCheckPlatform = false;
    MacOSBoolean  bCheckScript   = false;
    MacOSBoolean  bCheckLanguage = false;

    if (    EncodingMatchesPrefs( *pFontNewNameEncoding, *pFontFamilyNameEncodingPrefs )
         && EncodingMatchesPrefs( *pFontOldNameEncoding, *pFontFamilyNameEncodingPrefs )
       )
    {
        // If both font names match the font family name encoding preferences,
        // then determine which elements need to be compared.

        bCheckPlatform = pFontFamilyNameEncodingPrefs->mnFontPlatformCode == kFontNoPlatform;
        bCheckScript   = pFontFamilyNameEncodingPrefs->mnFontScriptCode   == kFontNoScript;
        bCheckLanguage = pFontFamilyNameEncodingPrefs->mnFontLanguageCode == kFontNoLanguage;
    } // if
    else if ( EncodingMatchesPrefs( *pFontOldNameEncoding, *pFontFamilyNameEncodingPrefs ) )
    {
        // If only one name matches the font family name encoding preferences,
        // then we like that one better.

        return false;
    } // else if
    else if ( EncodingMatchesPrefs( *pFontNewNameEncoding, *pFontFamilyNameEncodingPrefs ) )
    {
        return true;
    } // else if
    else
    {
        // If neither matches the font family name encoding preferences,
        // then we have no choice but to compare everything.

        bCheckPlatform = true;
        bCheckScript   = true;
        bCheckLanguage = true;
    } // else

    // We have to compare everything.
    //
    // Should language have priority over platform or script?

    if (    ( bCheckPlatform )
         && ( pFontNewNameEncoding->mnFontPlatformCode != pFontOldNameEncoding->mnFontPlatformCode )
       )
    {
        if ( pFontOldNameEncoding->mnFontPlatformCode == kFontMacintoshPlatform )
        {
            // We like Macintosh font names best

            return false;
        } // if
        else if ( pFontNewNameEncoding->mnFontPlatformCode == kFontMacintoshPlatform )
        {
            return true;
        } // else if
        else if ( pFontOldNameEncoding->mnFontPlatformCode == kFontUnicodePlatform )
        {
            // Unicode font names are the next best thing

            return false;
        } // else if
        else if ( pFontNewNameEncoding->mnFontPlatformCode == kFontUnicodePlatform )
        {
            return true;
        } // else

        // If it's neither Unicode font nor Macintosh font,
        // then it's not important to us the kind of font
        // we have been processing.
    } // if

    // Font script is meaningful if and only if it is associated with
    // a particular platform.  Whence, the fonts can be compared if
    // the platforms are the same.

    if (    ( bCheckScript  )
         && ( pFontNewNameEncoding->mnFontPlatformCode == pFontOldNameEncoding->mnFontPlatformCode )
         && ( pFontNewNameEncoding->mnFontScriptCode   != pFontOldNameEncoding->mnFontScriptCode   )
       )
    {
        if ( pFontNewNameEncoding->mnFontPlatformCode == kFontMacintoshPlatform )
        {
            short         nFontScriptOrder = 0;
            MacOSBoolean  bFontScriptOrder = false;

            // For Macintosh scripts, we use the system script text order.
            // Since we know that the scripts are different, we can exit
            // this API at this juncture.

            nFontScriptOrder = ScriptOrder( pFontNewNameEncoding->mnFontScriptCode,
                                            pFontOldNameEncoding->mnFontScriptCode
                                          );

            bFontScriptOrder = nFontScriptOrder < 0;

            return bFontScriptOrder;
        }
        // No preference otherwise.
    } // if

    // Font language codes are (so far) identical to the standard Mac
    // language codes, hence we can just use language order to determine
    // a preference regardless of the encoding.

    if (    ( bCheckLanguage )
         && (    pFontNewNameEncoding->mnFontLanguageCode
              != pFontOldNameEncoding->mnFontLanguageCode
            )
       )
    {
        short         nFontLanguageOrder = 0;
        MacOSBoolean  bFontLanguageOrder = false;

        // For Macintosh languages, we use the system language order.
        // Since we know that the languages are different, we can exit
        // this API at this juncture.

        nFontLanguageOrder = LanguageOrder( pFontNewNameEncoding->mnFontLanguageCode,
                                            pFontOldNameEncoding->mnFontLanguageCode
                                          );

        bFontLanguageOrder = nFontLanguageOrder < 0;

        return bFontLanguageOrder;
    } // if

    // If no better matches were discovered, return the earlier one.

    return false;
} // ATSUIBetterEncoding

// -----------------------------------------------------------------------
//
// Given a font and name code, a preference for platform, script and
// language,  and font variation values, determine the best match
// for the current system.
//
// -----------------------------------------------------------------------

OSStatus ATSUIFindBestFontName( ATSUFontID         nFontID,
                                FontNameCode       nFontNameCode,
                                FontPlatformCode  *rFontPlatformCode,
                                FontScriptCode    *rFontScriptCode,
                                FontLanguageCode  *rFontLanguageCode,
                                ByteCount          nFontMaxNameLength,
                                MacOSPtr                rFontName,
                                ByteCount         *rFontNameLength,
                                ItemCount         *rFontNameIndex
                              )
{
    OSStatus                nStatus                = kATSUFontsNotMatched;
    MacOSBoolean            bScriptWithoutPlatform = false;
    FontNameEncodingRecord  aFontFamilyNameEncodingPrefs;

    aFontFamilyNameEncodingPrefs.mnFontPlatformCode = *rFontPlatformCode;
    aFontFamilyNameEncodingPrefs.mnFontScriptCode   = *rFontScriptCode;
    aFontFamilyNameEncodingPrefs.mnFontLanguageCode = *rFontLanguageCode;

    // Script without a platform, is meaningless, so make sure
    // one is provided if required.

    bScriptWithoutPlatform =    ( *rFontScriptCode   != kFontNoScript   )
                             && ( *rFontPlatformCode == kFontNoPlatform );

    if ( !bScriptWithoutPlatform )
    {
        // If a specific name was requested, t.hen try to find a
        // match with that specific name.

        if (    ( *rFontPlatformCode != kFontNoPlatform )
             && ( *rFontScriptCode   != kFontNoScript   )
             && ( *rFontLanguageCode != kFontNoLanguage )
           )
        {
            nStatus = ATSUFindFontName(   nFontID,
                                          nFontNameCode,
                                         *rFontPlatformCode,
                                         *rFontScriptCode,
                                         *rFontLanguageCode,
                                          nFontMaxNameLength,
                                          rFontName,
                                          rFontNameLength,
                                          rFontNameIndex
                                      );
        }

        // If at first we were unsuccessful, then search the name table.

        if ( nStatus != noErr )
        {
            FontNameEncodingRecord   aBestFontNameEncoding;
            FontNameCode            *pFontNameCode           = NULL;
            FontPlatformCode         nFontNamePlatformCode   = kFontNoPlatform;
            FontScriptCode           nFontNameScriptCode     = kFontNoScript;
            FontLanguageCode         nFontNameLanguageCode   = kFontNoLanguage;
            ItemCount                nBestFontIndex          = ULONG_MAX;
            ItemCount                nNextFontIndex          = 0;
            ItemCount                nFontNamesCount         = 0;
            ByteCount                nFontExpectedNameLength = 0;
            ByteCount               *pFontActualNameLength   = NULL;
            MacOSPtr                 pFontName               = NULL;

            // Firstly, check to see if there are any names at all with
            // the given code.  Store the attributes associated with that
            // name.  This will be used be as our initial guess.

            nStatus = ATSUFindFontName(  nFontID,
                                         nFontNameCode,
                                         nFontNamePlatformCode,
                                         nFontNameScriptCode,
                                         nFontNameLanguageCode,
                                         nFontExpectedNameLength,
                                         pFontName,
                                         pFontActualNameLength,
                                        &nBestFontIndex
                                      );

            if ( nStatus == noErr )
             {
                nNextFontIndex = nBestFontIndex + 1;

                nStatus = ATSUGetIndFontName(  nFontID,
                                               nBestFontIndex,
                                               nFontExpectedNameLength,
                                               pFontName,
                                               pFontActualNameLength,
                                               pFontNameCode,
                                              &aBestFontNameEncoding.mnFontPlatformCode,
                                              &aBestFontNameEncoding.mnFontScriptCode,
                                              &aBestFontNameEncoding.mnFontLanguageCode
                                            );
            }

            // Now go through the rest of the names by comparing them with
            // our best guess so far.  The proceeding search will yield
            // the initial best guess, and as a result we can proceed with
            // our search starting at the next index.

            nStatus = ATSUCountFontNames( nFontID, &nFontNamesCount );

            while (    ( nStatus == noErr )
                    && ( nNextFontIndex < nFontNamesCount )
                  )
            {
                FontNameEncodingRecord  aNextFontNameEncoding;
                FontNameCode            nNextFontNameCode      = 0;
                MacOSBoolean            bGotBetterFontEncoding = false;

                nStatus = ATSUGetIndFontName(  nFontID,
                                               nNextFontIndex,
                                               nFontExpectedNameLength,
                                               pFontName,
                                               pFontActualNameLength,
                                              &nNextFontNameCode,
                                              &aNextFontNameEncoding.mnFontPlatformCode,
                                              &aNextFontNameEncoding.mnFontScriptCode,
                                              &aNextFontNameEncoding.mnFontLanguageCode
                                            );

                bGotBetterFontEncoding = ATSUIBetterEncoding( &aNextFontNameEncoding,
                                                              &aBestFontNameEncoding,
                                                              &aFontFamilyNameEncodingPrefs
                                                            );

                if (    ( nStatus == noErr )
                     && ( nNextFontNameCode == nFontNameCode )
                     && ( bGotBetterFontEncoding )
                   )
                {
                    nBestFontIndex        = nNextFontIndex;
                    aBestFontNameEncoding = aNextFontNameEncoding;
                }

                ++nNextFontIndex;
            }

            if ( nStatus == noErr )
            {
                // If we have gotten this far, then we do have a good candidate

                *rFontNameIndex    = nBestFontIndex;
                *rFontPlatformCode = aBestFontNameEncoding.mnFontPlatformCode;
                *rFontScriptCode   = aBestFontNameEncoding.mnFontScriptCode;
                *rFontLanguageCode = aBestFontNameEncoding.mnFontLanguageCode;

                if ( rFontName || rFontNameLength )
                {
                    FontPlatformCode  *pFontNamePlatformCode = NULL;
                    FontScriptCode    *pFontNameScriptCode   = NULL;
                    FontLanguageCode  *pFontNameLanguageCode = NULL;

                    nStatus = ATSUGetIndFontName( nFontID,
                                                  nBestFontIndex,
                                                  nFontMaxNameLength,
                                                  rFontName,
                                                  rFontNameLength,
                                                  pFontNameCode,
                                                  pFontNamePlatformCode,
                                                  pFontNameScriptCode,
                                                  pFontNameLanguageCode
                                                );
                } // if
            } // if
        } // if
    } //if

    return nStatus;
} // ATSUIFindBestFontName

// -----------------------------------------------------------------------

OSStatus ATSUIFONDtoFontID( short            nFONDNumber,
                            StyleParameter   nFONDStyle,
                            ATSUFontID      *rFontID,
                            StyleParameter  *rIntrinsicStyleParameter
                          )
{
    OSStatus  nStatus = noErr;

    nStatus = FMGetFontFromFontFamilyInstance( nFONDNumber,
                                               nFONDStyle,
                                               rFontID,
                                               rIntrinsicStyleParameter
                                             );

    return nStatus;
} // ATSUIFONDtoFontID

// -----------------------------------------------------------------------

void ATSUIPrintFontList( const ItemCount  nFontItemsCount,
                         FontNamesMatrix  hFontList
                       )
{
    if ( IsPointerValid( (MacOSPtr)hFontList ) )
    {
        const void  *pFontNames[2]            = { NULL, NULL };
        long         i                        = 0;
        long         j                        = 0;
        long         k                        = 0;
        long         nTextOrder               = 0;
        short        pFontFamilyLength[2]     = { 0, 0 };
        LangCode     pFontFamilyLangCode[2]   = { currentDefLang, currentDefLang };
        ScriptCode   pFontFamilyScriptCode[2] = { smRoman, smRoman };
        char         pFontFamilyStyle[kFontStyleNameLength];
        char         pFontFamilyName[kFontFamilyNameLength];

        while ( i < nFontItemsCount )
        {
            for ( j = i + 1;
                  j < nFontItemsCount;
                  j++
                )
            {
                pFontNames[0]            = &hFontList[ i ]->mpFontName[ 0 ];
                pFontNames[1]            = &hFontList[ j ]->mpFontName[ 0 ];
                pFontFamilyLength[0]     =  hFontList[ i ]->maFontFamilyNameEncoding.mnFontItemCount;
                pFontFamilyLength[1]     =  hFontList[ j ]->maFontFamilyNameEncoding.mnFontItemCount;
                pFontFamilyScriptCode[0] =  hFontList[ i ]->maFontFamilyNameEncoding.mnFontScriptCode;
                pFontFamilyScriptCode[1] =  hFontList[ j ]->maFontFamilyNameEncoding.mnFontScriptCode;

                nTextOrder = TextOrder( pFontNames[0],
                                        pFontNames[1],
                                        pFontFamilyLength[0],
                                        pFontFamilyLength[1],
                                        pFontFamilyScriptCode[0],
                                        pFontFamilyScriptCode[1],
                                        pFontFamilyLangCode[0],
                                        pFontFamilyLangCode[1]
                                      );

                if ( nTextOrder != 0 )
                {
                    break;
                } // if
            } // for

            memset( pFontFamilyName, 0, kFontFamilyNameMemSize );

            strncpy(   pFontFamilyName,
                      &hFontList[ i ]->mpFontName[ 0 ],
                       hFontList[ i ]->mnFontStyleByteOffset
                   );

            printf( "%s ", pFontFamilyName );

            if ( j - i > 1 )
            {
                for ( k = i; k < j; k++ )
                {
                    strcpy(  pFontFamilyStyle,
                            &hFontList[ k ]->mpFontName[ hFontList[ k ]->mnFontStyleByteOffset ]
                          );

                    printf( "%s ", pFontFamilyStyle );
                } // for
            } // if

            printf( "\n " );

            i = j;  // next family name
        } // while
    } // if
} // ATSUIPrintFontList

// -----------------------------------------------------------------------

void ATSUIFPrintFontList( const char       *pFileName,
                          const char       *pFilePermission,
                          const ItemCount   nFontItemsCount,
                          FontNamesMatrix   hFontList
                        )
{
    FILE  *pFileStream = NULL;

    if ( pFileName != NULL )
    {
        if ( pFilePermission == NULL )
        {
            pFileStream = fopen( pFileName, "w" );
        } // if
        else
        {
            pFileStream = fopen( pFileName, pFilePermission );
        } // else

        if (    ( pFileStream != NULL )
             && ( IsPointerValid( (MacOSPtr)hFontList   ) )
           )
        {
            const void  *pFontNames[2]            = { NULL, NULL };
            long         i                        = 0;
            long         j                        = 0;
            long         k                        = 0;
            long         nTextOrder               = 0;
            short        pFontFamilyLength[2]     = { 0, 0 };
            LangCode     pFontFamilyLangCode[2]   = { currentDefLang, currentDefLang };
            ScriptCode   pFontFamilyScriptCode[2] = { smRoman, smRoman };
            char         pFontFamilyStyle[kFontStyleNameLength];
            char         pFontFamilyName[kFontFamilyNameLength];

            while ( i < nFontItemsCount )
            {
                for ( j = i + 1;
                      j < nFontItemsCount;
                      j++
                    )
                {
                    pFontNames[0]            = &hFontList[ i ]->mpFontName[ 0 ];
                    pFontNames[1]            = &hFontList[ j ]->mpFontName[ 0 ];
                    pFontFamilyLength[0]     =  hFontList[ i ]->maFontFamilyNameEncoding.mnFontItemCount;
                    pFontFamilyLength[1]     =  hFontList[ j ]->maFontFamilyNameEncoding.mnFontItemCount;
                    pFontFamilyScriptCode[0] =  hFontList[ i ]->maFontFamilyNameEncoding.mnFontScriptCode;
                    pFontFamilyScriptCode[1] =  hFontList[ j ]->maFontFamilyNameEncoding.mnFontScriptCode;

                    nTextOrder = TextOrder( pFontNames[0],
                                            pFontNames[1],
                                            pFontFamilyLength[0],
                                            pFontFamilyLength[1],
                                            pFontFamilyScriptCode[0],
                                            pFontFamilyScriptCode[1],
                                            pFontFamilyLangCode[0],
                                            pFontFamilyLangCode[1]
                                          );

                    if ( nTextOrder != 0 )
                    {
                        break;
                    } // if
                } // for

                memset( pFontFamilyName, 0, kFontFamilyNameMemSize );

                strncpy(   pFontFamilyName,
                          &hFontList[ i ]->mpFontName[ 0 ],
                           hFontList[ i ]->mnFontStyleByteOffset
                       );

                fprintf( pFileStream, "%s ", pFontFamilyName );

                if ( j - i > 1 )
                {
                    for ( k = i; k < j; k++ )
                    {
                        strcpy(  pFontFamilyStyle,
                                &hFontList[ k ]->mpFontName[ hFontList[ k ]->mnFontStyleByteOffset ]
                              );

                        fprintf( pFileStream, "%s ", pFontFamilyStyle );
                    } // for
                } // if

                fprintf( pFileStream, "\n" );

                i = j;  // next family name
            } // while

            fclose( pFileStream );
        } // if
    } // if
} // ATSUIFPrintFontList

// -----------------------------------------------------------------------
//
// Here we shall construct a font list.
//
// Note that:
//
// * If there's only one representative of a family, we shall insert the
//   full font name into the list instead of only the family name. We can
//   not determine this immediately until all the fonts are sorted.
//
// * If there are multiple fonts with the same name, all but the first
//   occurance of this particular font is kept.
//
// -----------------------------------------------------------------------

FontNamesMatrix  ATSUIGetFontNames( ItemCount *rSortedFontListLength )
{
    FontNamesMatrix  hSortedFontList = NULL;
    ItemCount        nFontItemsSize  = 0;
    OSStatus         nStatus         = noErr;

    nStatus = ATSUFontCount( &nFontItemsSize );

    if (    ( nStatus        == noErr )
         && ( nFontItemsSize >  0     )
       )
    {
        ATSUFontID   *pFontIDList     = NULL;
        long          nFontIDListSize = nFontItemsSize * sizeof(ATSUFontID);

        pFontIDList = (ATSUFontID *)NewPtrClear( nFontIDListSize );

        nStatus = MemError();

        if (    ( nStatus == noErr )
             && ( IsPointerValid( (MacOSPtr)pFontIDList )  )
           )
        {
            ItemCount *pFontItemsCount = NULL;

            nStatus = ATSUGetFontIDs( pFontIDList,
                                      nFontItemsSize,
                                      pFontItemsCount
                                    );

            if ( nStatus == noErr )
            {
                long       nSortedFontListSize = 0;
                ItemCount  nTotalFontInstances = 0;

                nTotalFontInstances
                    = ATSUILookForFontInstances( pFontIDList,
                                                 nFontItemsSize
                                               );

                // This might be too much because of "bad" fonts, but
                // it's guaranteed to be big enough.

                nSortedFontListSize =   (nFontItemsSize + nTotalFontInstances)
                                      * sizeof(FontNamesPtr);

                hSortedFontList = (FontNamesMatrix)NewPtrClear( nSortedFontListSize );

                nStatus = MemError();

                if (    ( nStatus == noErr )
                     && ( IsPointerValid( (MacOSPtr)hSortedFontList ) )
                   )
                {
                    ATSUIInstanceIndex   nFontInstanceIndex    = 0;
                    FontNamesPtr         pFontNames            = NULL;
                    FontNameCode         nFontInstanceNameCode = 0;
                    ItemCount            nFontInstance         = 0;
                    ItemCount            nFontInstancesCount   = 0;
                    ItemCount            nFontIndex;

                    for ( nFontIndex = 0;
                          nFontIndex < nFontItemsSize;
                          nFontIndex++
                        )
                    {
                        if ( pFontIDList[ nFontIndex ] != kATSUInvalidFontID )
                         {
                            // Add the font's instances to the name list, if any.

                            nStatus = ATSUCountFontInstances(  pFontIDList[ nFontIndex ],
                                                              &nFontInstancesCount
                                                            );

                            for ( nFontInstance = 0;
                                  nFontInstance < nFontInstancesCount;
                                  ++nFontInstance
                                )
                            {
                                nStatus = ATSUGetFontInstanceNameCode(  pFontIDList[ nFontIndex ],
                                                                        nFontInstance,
                                                                       &nFontInstanceNameCode
                                                                     );

                                if ( nStatus == noErr )
                                {
                                    pFontNames = ATSUIBuildFontNamesRecord( pFontIDList[ nFontIndex ],
                                                                            nFontInstance,
                                                                            nFontInstanceNameCode
                                                                          );

                                    if ( pFontNames != NULL )
                                    {
                                        ATSUIInsertFontInSortedList( pFontNames,
                                                                     hSortedFontList,
                                                                     rSortedFontListLength
                                                                   );
                                    } // if
                                } // if
                            } // if

                            // Add the font itself. If it happens to have the same name as an nFontInstanceIndex,
                            // it'll be skipped until we figure out what to do about name clashes.

                            nFontInstanceIndex
                                = ATSUIInstanceMatchingDefaultVariations( pFontIDList[ nFontIndex ] );

                            if ( nFontInstanceIndex == kATSUIDefaultInstance )
                            {
                                pFontNames = ATSUIBuildFontNamesRecord( pFontIDList[ nFontIndex ],
                                                                        kATSUIDefaultInstance,
                                                                        kFontStyleName
                                                                      );

                                if ( pFontNames != NULL )
                                {
                                    ATSUIInsertFontInSortedList(  pFontNames,
                                                                  hSortedFontList,
                                                                  rSortedFontListLength
                                                               );
                                } // if
                            } // if
                        } // if
                    } // for
                } // if
            } // if

            DisposePtr( (MacOSPtr)pFontIDList );
        } // if
    } // if

    return hSortedFontList;
} // ATSUIPrintFonts

// -----------------------------------------------------------------------

OSStatus ATSUIDisposeFontNames( const ItemCount  nFontListLength,
                                FontNamesMatrix  hFontNames
                              )
{
    OSStatus  nStatus = noErr;

    if (    ( nFontListLength > 0 )
         && ( IsPointerValid( (MacOSPtr)hFontNames ) )
       )
    {
        ItemCount  nFontIndex;

        for ( nFontIndex = 0;
              nFontIndex < nFontListLength;
              nFontIndex++
            )
        {
            if ( IsPointerValid( (MacOSPtr)hFontNames[ nFontIndex ] ) )
            {
                DisposePtr( (MacOSPtr)hFontNames[ nFontIndex ] );
            } // if
        } // for

        DisposePtr( (MacOSPtr)hFontNames );

        nStatus = MemError();

        hFontNames = NULL;
    } // if

    return nStatus;
} // ATSUIDisposeFontNames

// =======================================================================

// =======================================================================

