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



#include <vcl/bitmap.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#endif
namespace binfilter {

struct ShellResource : public Resource
{
    String			aPostItAuthor;
    String 			aPostItPage;
    String			aPostItLine;

    // Calc Fehlerstrings
    String			aCalc_Syntax;
    String			aCalc_ZeroDiv;
    String			aCalc_Brack;
    String			aCalc_Pow;
    String			aCalc_VarNFnd;
    String			aCalc_Overflow;
    String			aCalc_WrongTime;
    String			aCalc_Default;
    String			aCalc_Error;

    // fuers GetRefFeld - oben/unten
    String			aGetRefFld_Up, aGetRefFld_Down;
    // fuer dynamisches Menu - String "alle"
    String			aStrAllPageHeadFoot;
    // fuer einige Listboxen - String "keine"
    String			aStrNone;
    // fuer Felder, die Fixiert sind
    String			aFixedStr;

    //names of TOXs
    String 			aTOXIndexName;
    String          aTOXUserName;
    String          aTOXContentName;
    String          aTOXIllustrationsName;
    String          aTOXObjectsName;
    String          aTOXTablesName;
    String          aTOXAuthoritiesName;

    SvStringsDtor	aDocInfoLst;

    // Fly-Anker Bmps
//	Bitmap			aAnchorBmp;
//	Bitmap			aDragAnchorBmp;

    // die AutoFormat-Redline-Kommentare
    inline const SvStringsDtor&	GetAutoFmtNameLst() const;

    // returns for the specific filter the new names of pagedescs
    // This method is for the old code of the specific filters with
    // now localized names
    String GetPageDescName( USHORT nNo, BOOL bFirst = FALSE,
                                        BOOL bFollow = FALSE );

    ShellResource();
    ~ShellResource();

private:
    void _GetAutoFmtNameLst() const;
    SvStringsDtor	*pAutoFmtNameLst;
    String 			sPageDescFirstName, sPageDescFollowName, sPageDescName;
};

inline const SvStringsDtor& ShellResource::GetAutoFmtNameLst() const
{
    if( !pAutoFmtNameLst )
        _GetAutoFmtNameLst();
    return *pAutoFmtNameLst;
}


} //namespace binfilter
#endif //_SHELLRES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
