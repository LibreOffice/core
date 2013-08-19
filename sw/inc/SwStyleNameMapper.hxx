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
#ifndef SWSTYLENAMEMAPPER_HXX
#define SWSTYLENAMEMAPPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <SwGetPoolIdFromName.hxx>
#include "swdllapi.h"

#include <boost/unordered_map.hpp>
#include <stringhash.hxx>

#include <vector>

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

struct SwTableEntry;

typedef ::boost::unordered_map<const OUString, sal_uInt16, OUStringHash>
    NameToIdHash;

class SwStyleNameMapper
{
    friend void _InitCore();
    friend void _FinitCore();

protected:
    // UI Name tables
    static ::std::vector<OUString> *pTextUINameArray,
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

    static void fillNameFromId(sal_uInt16 nId, OUString &rName, bool bProgName);
    static const OUString& getNameFromId(sal_uInt16 nId, const OUString &rName,
                                         bool bProgName);
    static const NameToIdHash& getHashTable ( SwGetPoolIdFromName, bool bProgName );

public:
    // This gets the UI Name from the programmatic name
    static const OUString& GetUIName(const OUString& rName, SwGetPoolIdFromName);
    static         void FillUIName(const OUString& rName, OUString& rFillName,
                            SwGetPoolIdFromName, bool bDisambiguate = false);

    // Get the programmatic Name from the UI name
    static const OUString& GetProgName(const OUString& rName,
                                       SwGetPoolIdFromName);
    static         void FillProgName(const OUString& rName, OUString& rFillName,
                            SwGetPoolIdFromName, bool bDisambiguate = false);

    // This gets the UI Name from the Pool ID
    SW_DLLPUBLIC static void FillUIName(sal_uInt16 nId, OUString& rFillName);
    SW_DLLPUBLIC static const OUString& GetUIName(sal_uInt16 nId,
                                                  const OUString& rName);

    // This gets the programmatic Name from the Pool ID
    static         void FillProgName(sal_uInt16 nId, OUString& rFillName);
    SW_DLLPUBLIC static const OUString& GetProgName(sal_uInt16 nId,
                                                    const OUString& rName);

    // This gets the PoolId from the UI Name
    SW_DLLPUBLIC static sal_uInt16 GetPoolIdFromUIName(const OUString& rName,
                                                       SwGetPoolIdFromName);

    // Get the Pool ID from the programmatic name
    static sal_uInt16 GetPoolIdFromProgName(const OUString& rName,
                                            SwGetPoolIdFromName);

    // used to convert the 4 special ExtraProg/UINames for
    // RES_POOLCOLL_LABEL_DRAWING,  RES_POOLCOLL_LABEL_ABB,
    // RES_POOLCOLL_LABEL_TABLE, RES_POOLCOLL_LABEL_FRAME
    // forth and back.
    // Non-matching names remain unchanged.
    SW_DLLPUBLIC static const OUString GetSpecialExtraProgName(
                    const OUString& rExtraUIName);
    static const OUString GetSpecialExtraUIName(const OUString& rExtraProgName);

    static const ::std::vector<OUString>& GetTextUINameArray();
    static const ::std::vector<OUString>& GetListsUINameArray();
    static const ::std::vector<OUString>& GetExtraUINameArray();
    static const ::std::vector<OUString>& GetRegisterUINameArray();
    static const ::std::vector<OUString>& GetDocUINameArray();
    static const ::std::vector<OUString>& GetHTMLUINameArray();
    static const ::std::vector<OUString>& GetFrmFmtUINameArray();
    static const ::std::vector<OUString>& GetChrFmtUINameArray();
    static const ::std::vector<OUString>& GetHTMLChrFmtUINameArray();
    static const ::std::vector<OUString>& GetPageDescUINameArray();
    static const ::std::vector<OUString>& GetNumRuleUINameArray();

    static const ::std::vector<OUString>& GetTextProgNameArray();
    static const ::std::vector<OUString>& GetListsProgNameArray();
    static const ::std::vector<OUString>& GetExtraProgNameArray();
    static const ::std::vector<OUString>& GetRegisterProgNameArray();
    static const ::std::vector<OUString>& GetDocProgNameArray();
    static const ::std::vector<OUString>& GetHTMLProgNameArray();
    static const ::std::vector<OUString>& GetFrmFmtProgNameArray();
    static const ::std::vector<OUString>& GetChrFmtProgNameArray();
    static const ::std::vector<OUString>& GetHTMLChrFmtProgNameArray();
    static const ::std::vector<OUString>& GetPageDescProgNameArray();
    static const ::std::vector<OUString>& GetNumRuleProgNameArray();
};
#endif // _NAME_MAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
