/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SWSTYLENAMEMAPPER_HXX
#define _SWSTYLENAMEMAPPER_HXX

#include <sal/types.h>
#include <tools/string.hxx>
#include <SwGetPoolIdFromName.hxx>
#include "swdllapi.h"

#include <boost/unordered_map.hpp>
#include <stringhash.hxx>

#include <boost/ptr_container/ptr_vector.hpp>

/** This class holds all data about the names of styles used in the user
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

/**
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

class String;
struct SwTableEntry;

typedef ::boost::unordered_map < const String*, sal_uInt16, StringHash, StringEq > NameToIdHash;

class SwStyleNameMapper
{
    friend void _InitCore();
    friend void _FinitCore();

protected:
    // UI Name tables
    static boost::ptr_vector<String> *pTextUINameArray,
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

    static void fillNameFromId ( sal_uInt16 nId, String &rName, bool bProgName );
    static const String& getNameFromId ( sal_uInt16 nId, const String &rName, bool bProgName );
    static const NameToIdHash& getHashTable ( SwGetPoolIdFromName, bool bProgName );

public:
    // This gets the UI Name from the programmatic name
    static const String& GetUIName ( const String& rName, SwGetPoolIdFromName );
    static         void FillUIName ( const String& rName, String& rFillName, SwGetPoolIdFromName, bool bDisambiguate = false );

    // Get the programmatic Name from the UI name
    static const String& GetProgName ( const String& rName, SwGetPoolIdFromName );
    static         void FillProgName ( const String& rName, String& rFillName, SwGetPoolIdFromName, bool bDisambiguate = false );

    // This gets the UI Name from the Pool ID
    SW_DLLPUBLIC static          void FillUIName ( sal_uInt16 nId, String& rFillName );
    SW_DLLPUBLIC static const String& GetUIName  ( sal_uInt16 nId, const String& rName );

    // This gets the programmatic Name from the Pool ID
    static          void FillProgName( sal_uInt16 nId, String& rFillName );
    SW_DLLPUBLIC static const String& GetProgName ( sal_uInt16 nId, const String& rName );

    // This gets the PoolId from the UI Name
    SW_DLLPUBLIC static sal_uInt16 GetPoolIdFromUIName( const String& rName, SwGetPoolIdFromName );

    // Get the Pool ID from the programmatic name
    static sal_uInt16 GetPoolIdFromProgName( const String& rName, SwGetPoolIdFromName );

    // used to convert the 4 special ExtraProg/UINames for
    // RES_POOLCOLL_LABEL_DRAWING,  RES_POOLCOLL_LABEL_ABB,
    // RES_POOLCOLL_LABEL_TABLE, RES_POOLCOLL_LABEL_FRAME
    // forth and back.
    // Non-matching names remain unchanged.
    SW_DLLPUBLIC static const String GetSpecialExtraProgName( const String& rExtraUIName );
    static const String GetSpecialExtraUIName( const String& rExtraProgName );

    static const boost::ptr_vector<String>& GetTextUINameArray();
    static const boost::ptr_vector<String>& GetListsUINameArray();
    static const boost::ptr_vector<String>& GetExtraUINameArray();
    static const boost::ptr_vector<String>& GetRegisterUINameArray();
    static const boost::ptr_vector<String>& GetDocUINameArray();
    static const boost::ptr_vector<String>& GetHTMLUINameArray();
    static const boost::ptr_vector<String>& GetFrmFmtUINameArray();
    static const boost::ptr_vector<String>& GetChrFmtUINameArray();
    static const boost::ptr_vector<String>& GetHTMLChrFmtUINameArray();
    static const boost::ptr_vector<String>& GetPageDescUINameArray();
    static const boost::ptr_vector<String>& GetNumRuleUINameArray();

    static const boost::ptr_vector<String>& GetTextProgNameArray();
    static const boost::ptr_vector<String>& GetListsProgNameArray();
    static const boost::ptr_vector<String>& GetExtraProgNameArray();
    static const boost::ptr_vector<String>& GetRegisterProgNameArray();
    static const boost::ptr_vector<String>& GetDocProgNameArray();
    static const boost::ptr_vector<String>& GetHTMLProgNameArray();
    static const boost::ptr_vector<String>& GetFrmFmtProgNameArray();
    static const boost::ptr_vector<String>& GetChrFmtProgNameArray();
    static const boost::ptr_vector<String>& GetHTMLChrFmtProgNameArray();
    static const boost::ptr_vector<String>& GetPageDescProgNameArray();
    static const boost::ptr_vector<String>& GetNumRuleProgNameArray();
};
#endif // _NAME_MAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
