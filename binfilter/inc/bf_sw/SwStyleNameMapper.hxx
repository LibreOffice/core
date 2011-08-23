/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SWSTYLENAMEMAPPER_HXX
#define _SWSTYLENAMEMAPPER_HXX

#include <sal/types.h>
#include <SwGetPoolIdFromName.hxx>
#include <tools/string.hxx>
#include <hash_map>

class String; 

namespace binfilter {
class SvStringsDtor; 


/* This class holds all data about the names of styles used in the user
 * interface (UI names...these are localised into different languages).
 * These UI names are loaded from the resource files on demand.
 *
 * It also holds all information about the 'Programmatic' names of styles
 * which remain static (and are hardcoded in the corresponding cxx file) 
 * for all languages. 
 *
 * This class also provides static functions which can be used for the
 * following conversions:
 *
 * 1. Programmatic Name -> UI Name
 * 2. Programmatic Name -> Pool ID
 * 3. UI Name -> Programmatic Name
 * 4. UI Name -> Pool ID
 * 5. Pool ID -> UI Name
 * 6. Pool ID -> Programmatic Name
 *
 * The relationship of these tables to the style families is as follows:
 *
 * 1. Paragraph contains the Text, Lists, Extra, Register, Doc and HTML 
 *    name arrays.
 * 2. Character contains the ChrFmt and HTMLChrFmt name arrays.
 * 3. Page contains the PageDesc name array.
 * 4. Frame contains the FrmFmt name array.
 * 5. Numbering Rule contains the NumRule name array.
 */

/*
 * There is a further complication that came to light later. If someone enters
 * a user-defined style name which is the same as a programmatic name, this
 * name clash must be handled.
 *
 * Therefore, when there is a danger of a nameclash, the boolean bDisambiguate
 * must be set to true in the SwStyleNameMapper call (it defaults to false).
 * This will cause the following to happen:
 *
 * If the UI style name either equals a programmatic name or already ends
 * with " (user)", then it must append " (user)" to the end.
 *
 * When a programmatic name is being converted to a UI name, if it ends in
 * " (user)", we simply remove it.
 */

struct SwTableEntry;
struct StringEq
{
    sal_Bool operator() ( const String *r1,
                            const String *r2) const
    {
        return r1->Equals(*r2);
    }
};

struct StringHash
{
    size_t operator() ( const String *rString) const
    {
        sal_Int32 h, nLen; 
        h = nLen = rString->Len();
        const sal_Unicode *pStr = rString->GetBuffer();

        if ( nLen < 16 )
            while ( nLen-- > 0 )
                h = (h*37) + *(pStr++);
        else
        {
            sal_Int32               nSkip;
            const sal_Unicode* pEndStr = pStr+nLen-5;

            /* only sample some characters */
            /* the first 3, some characters between, and the last 5 */
            h = (h*39) + *(pStr++);
            h = (h*39) + *(pStr++);
            h = (h*39) + *(pStr++);

            nSkip = nLen / nLen < 32 ? 4 : 8;
            nLen -= 8;
            while ( nLen > 0 )
            {
                h = (h*39) + ( *pStr );
                pStr += nSkip;
                nLen -= nSkip;
            }

            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
            h = (h*39) + *(pEndStr++);
        }
        return h;
    }
};


typedef ::std::hash_map < const String*, sal_uInt16, StringHash, StringEq > NameToIdHash;

class SwStyleNameMapper
{
    friend void _InitCore();
    friend void _FinitCore();

protected:
    // UI Name tables
    static SvStringsDtor	*pTextUINameArray,		
                            *pListsUINameArray,		
                            *pExtraUINameArray,
                            *pRegisterUINameArray,
                            *pDocUINameArray,
                            *pHTMLUINameArray,
                            *pFrmFmtUINameArray,
                            *pChrFmtUINameArray,
                            *pHTMLChrFmtUINameArray,
                            *pPageDescUINameArray,
                            *pNumRuleUINameArray,
    // Programmatic Name tables
                            *pTextProgNameArray,			
                            *pListsProgNameArray,		
                            *pExtraProgNameArray,
                            *pRegisterProgNameArray,
                            *pDocProgNameArray,
                            *pHTMLProgNameArray,
                            *pFrmFmtProgNameArray,
                            *pChrFmtProgNameArray,
                            *pHTMLChrFmtProgNameArray,
                            *pPageDescProgNameArray,
                            *pNumRuleProgNameArray;

    static NameToIdHash     *pParaUIMap,
                            *pCharUIMap,
                            *pPageUIMap,
                            *pFrameUIMap,
                            *pNumRuleUIMap,

                            *pParaProgMap,
                            *pCharProgMap,
                            *pPageProgMap,
                            *pFrameProgMap,
                            *pNumRuleProgMap;

    static SvStringsDtor* NewUINameArray( SvStringsDtor*&, 
                                      sal_uInt16 nStt, 
                                      sal_uInt16 nEnd );

    static SvStringsDtor* NewProgNameArray( SvStringsDtor*&, 
                                          const SwTableEntry *pTable, 
                                          sal_uInt8 nCount);

    static void fillNameFromId ( sal_uInt16 nId, String &rName, sal_Bool bProgName );
    static const String& getNameFromId ( sal_uInt16 nId, const String &rName, sal_Bool bProgName );
    static const NameToIdHash& getHashTable ( SwGetPoolIdFromName, sal_Bool bProgName );
    static sal_Bool SuffixIsUser ( const String & rString );
    static void CheckSuffixAndDelete ( String & rString );

public:
    // This gets the UI Name from the programmatic name
    static const String& GetUIName ( const String& rName, SwGetPoolIdFromName );
    static 		   void FillUIName ( const String& rName, String& rFillName, SwGetPoolIdFromName, sal_Bool bDisambiguate = sal_False );
    // Get the programmatic Name from the UI name
    static const String& GetProgName ( const String& rName, SwGetPoolIdFromName );
    static 		   void FillProgName ( const String& rName, String& rFillName, SwGetPoolIdFromName, sal_Bool bDisambiguate = sal_False );

    // This gets the UI Name from the Pool ID
    static          void FillUIName ( sal_uInt16 nId, String& rFillName );
    static const String& GetUIName  ( sal_uInt16 nId, const String& rName );

    // This gets the programmatic Name from the Pool ID
    static 		    void FillProgName( sal_uInt16 nId, String& rFillName );
    static const String& GetProgName ( sal_uInt16 nId, const String& rName );

    // This gets the PoolId from the UI Name
    static sal_uInt16 GetPoolIdFromUIName( const String& rName, SwGetPoolIdFromName );
    // Get the Pool ID from the programmatic name
    static sal_uInt16 GetPoolIdFromProgName( const String& rName, SwGetPoolIdFromName );

    // used to convert the 4 special ExtraProg/UINames for
    // RES_POOLCOLL_LABEL_DRAWING,  RES_POOLCOLL_LABEL_ABB,
    // RES_POOLCOLL_LABEL_TABLE, RES_POOLCOLL_LABEL_FRAME
    // forth and back.
    // Non-matching names remain unchanged.
    static const String GetSpecialExtraProgName( const String& rExtraUIName );
    static const String GetSpecialExtraUIName( const String& rExtraProgName );

    static const SvStringsDtor& GetTextUINameArray();
    static const SvStringsDtor& GetListsUINameArray();
    static const SvStringsDtor& GetExtraUINameArray();
    static const SvStringsDtor& GetRegisterUINameArray();
    static const SvStringsDtor& GetDocUINameArray();
    static const SvStringsDtor& GetHTMLUINameArray();
    static const SvStringsDtor& GetFrmFmtUINameArray();
    static const SvStringsDtor& GetChrFmtUINameArray();
    static const SvStringsDtor& GetHTMLChrFmtUINameArray();
    static const SvStringsDtor& GetPageDescUINameArray();
    static const SvStringsDtor& GetNumRuleUINameArray();

    static const SvStringsDtor& GetTextProgNameArray();
    static const SvStringsDtor& GetListsProgNameArray();
    static const SvStringsDtor& GetExtraProgNameArray();
    static const SvStringsDtor& GetRegisterProgNameArray();
    static const SvStringsDtor& GetDocProgNameArray();
    static const SvStringsDtor& GetHTMLProgNameArray();
    static const SvStringsDtor& GetFrmFmtProgNameArray();
    static const SvStringsDtor& GetChrFmtProgNameArray();
    static const SvStringsDtor& GetHTMLChrFmtProgNameArray();
    static const SvStringsDtor& GetPageDescProgNameArray();
    static const SvStringsDtor& GetNumRuleProgNameArray();
};
} //namespace binfilter
#endif // _NAME_MAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
