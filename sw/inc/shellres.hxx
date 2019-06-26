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
#ifndef INCLUDED_SW_INC_SHELLRES_HXX
#define INCLUDED_SW_INC_SHELLRES_HXX

#include <memory>
#include <vector>
#include "swdllapi.h"
#include <rtl/ustring.hxx>

struct SW_DLLPUBLIC ShellResource
{
    OUString const        aPostItAuthor;
    OUString const        aPostItPage;
    OUString const        aPostItLine;

    // Calc error-strings.
    OUString const        aCalc_Syntax;
    OUString const        aCalc_ZeroDiv;
    OUString const        aCalc_Brack;
    OUString const        aCalc_Pow;
    OUString const        aCalc_Overflow;
    OUString const        aCalc_Default;
    OUString const        aCalc_Error;

    // For GetRefField - referenced item not found.
    OUString const        aGetRefField_RefItemNotFound;
    // For some list boxes - string "none"
    OUString const        aStrNone;
    // For fixed fields.
    OUString const        aFixedStr;
    // Custom fields of type css::util::Duration.
    OUString const        sDurationFormat;

    //Names of TOXs.
    OUString const        aTOXIndexName;
    OUString const        aTOXUserName;
    OUString const        aTOXContentName;
    OUString const        aTOXIllustrationsName;
    OUString const        aTOXObjectsName;
    OUString const        aTOXTablesName;
    OUString const        aTOXAuthoritiesName;
    OUString const        aTOXCitationName;

    OUString        aLinkCtrlClick;
    OUString const        aLinkClick;

    std::vector<OUString> aDocInfoLst;

    // The autoFormat-Redline comments.
    inline const std::vector<OUString>& GetAutoFormatNameLst() const;

    enum PageNameMode
    {
        NORMAL_PAGE,
        FIRST_PAGE,
        FOLLOW_PAGE
    };
    // Returns for the specific filter the new names of pagedescs
    // This method is for the old code of the specific filters with
    // now localized names.
    OUString GetPageDescName(sal_uInt16 nNo, PageNameMode eMode);

    ShellResource();

private:
    void GetAutoFormatNameLst_() const;
    mutable std::unique_ptr<std::vector<OUString>> pAutoFormatNameLst;
    OUString const        sPageDescFirstName;
    OUString const        sPageDescFollowName;
    OUString const        sPageDescName;
};

inline const std::vector<OUString>& ShellResource::GetAutoFormatNameLst() const
{
    if( !pAutoFormatNameLst )
        GetAutoFormatNameLst_();
    return *pAutoFormatNameLst;
}

#endif // INCLUDED_SW_INC_SHELLRES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
