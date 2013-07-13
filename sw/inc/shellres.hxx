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
#ifndef _SHELLRES_HXX
#define _SHELLRES_HXX

#include <swdllapi.h>
#include <tools/string.hxx>
#include <tools/rc.hxx>
#include <vcl/bitmap.hxx>

struct SW_DLLPUBLIC ShellResource : public Resource
{
    String          aPostItAuthor;
    String          aPostItPage;
    String          aPostItLine;

    // Calc error-strings.
    String          aCalc_Syntax;
    String          aCalc_ZeroDiv;
    String          aCalc_Brack;
    String          aCalc_Pow;
    String          aCalc_VarNFnd;
    String          aCalc_Overflow;
    String          aCalc_WrongTime;
    String          aCalc_Default;
    String          aCalc_Error;

    // For GetRefField - up/down.
    String          aGetRefFld_Up;
    String          aGetRefFld_Down;
    // For GetRefField - referenced item not found.
    String          aGetRefFld_RefItemNotFound;
    // For dynamic menu - string "all".
    String          aStrAllPageHeadFoot;
    // For some list boxes - string "none"
    String          aStrNone;
    // For fixed fields.
    String          aFixedStr;
    // Custom fields of type css::util::Duration.
    String          sDurationFormat;

    //Names of TOXs.
    String          aTOXIndexName;
    String          aTOXUserName;
    String          aTOXContentName;
    String          aTOXIllustrationsName;
    String          aTOXObjectsName;
    String          aTOXTablesName;
    String          aTOXAuthoritiesName;

    OUString        aHyperlinkClick;

    std::vector<String> aDocInfoLst;

    // The autoFormat-Redline comments.
    inline const std::vector<String>& GetAutoFmtNameLst() const;

    enum PageNameMode
    {
        NORMAL_PAGE,
        FIRST_PAGE,
        FOLLOW_PAGE
    };
    // Returns for the specific filter the new names of pagedescs
    // This method is for the old code of the specific filters with
    // now localized names.
    String GetPageDescName( sal_uInt16 nNo, PageNameMode eMode );

    ShellResource();
    ~ShellResource();

private:
    void _GetAutoFmtNameLst() const;
    mutable std::vector<String> *pAutoFmtNameLst;
    String          sPageDescFirstName;
    String          sPageDescFollowName;
    String          sPageDescName;
};

inline const std::vector<String>& ShellResource::GetAutoFmtNameLst() const
{
    if( !pAutoFmtNameLst )
        _GetAutoFmtNameLst();
    return *pAutoFmtNameLst;
}

#endif //_SHELLRES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
