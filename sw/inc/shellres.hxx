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

#include <swdllapi.h>
#include <rtl/ustring.hxx>
#include <tools/rc.hxx>
#include <vcl/bitmap.hxx>

struct SW_DLLPUBLIC ShellResource : public Resource
{
    OUString        aPostItAuthor;
    OUString        aPostItPage;
    OUString        aPostItLine;

    // Calc error-strings.
    OUString        aCalc_Syntax;
    OUString        aCalc_ZeroDiv;
    OUString        aCalc_Brack;
    OUString        aCalc_Pow;
    OUString        aCalc_VarNFnd;
    OUString        aCalc_Overflow;
    OUString        aCalc_WrongTime;
    OUString        aCalc_Default;
    OUString        aCalc_Error;

    // For GetRefField - referenced item not found.
    OUString        aGetRefField_RefItemNotFound;
    // For some list boxes - string "none"
    OUString        aStrNone;
    // For fixed fields.
    OUString        aFixedStr;
    // Custom fields of type css::util::Duration.
    OUString        sDurationFormat;

    //Names of TOXs.
    OUString        aTOXIndexName;
    OUString        aTOXUserName;
    OUString        aTOXContentName;
    OUString        aTOXIllustrationsName;
    OUString        aTOXObjectsName;
    OUString        aTOXTablesName;
    OUString        aTOXAuthoritiesName;
    OUString        aTOXCitationName;

    OUString        aLinkCtrlClick;
    OUString        aLinkClick;

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
    ~ShellResource();

private:
    void GetAutoFormatNameLst_() const;
    mutable std::unique_ptr<std::vector<OUString>> pAutoFormatNameLst;
    OUString        sPageDescFirstName;
    OUString        sPageDescFollowName;
    OUString        sPageDescName;
};

inline const std::vector<OUString>& ShellResource::GetAutoFormatNameLst() const
{
    if( !pAutoFormatNameLst )
        GetAutoFormatNameLst_();
    return *pAutoFormatNameLst;
}

#endif // INCLUDED_SW_INC_SHELLRES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
