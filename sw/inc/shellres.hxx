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
#ifndef _SHELLRES_HXX
#define _SHELLRES_HXX

#include <tools/string.hxx>
#include <vcl/bitmap.hxx>
#include <svl/svstdarr.hxx>

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

    String          aHyperlinkClick;

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
