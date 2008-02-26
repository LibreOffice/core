/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shellres.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:31:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SHELLRES_HXX
#define _SHELLRES_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

struct ShellResource : public Resource
{
    String          aPostItAuthor;
    String          aPostItPage;
    String          aPostItLine;

    // Calc Fehlerstrings
    String          aCalc_Syntax;
    String          aCalc_ZeroDiv;
    String          aCalc_Brack;
    String          aCalc_Pow;
    String          aCalc_VarNFnd;
    String          aCalc_Overflow;
    String          aCalc_WrongTime;
    String          aCalc_Default;
    String          aCalc_Error;

    // fuers GetRefFeld - oben/unten
    String          aGetRefFld_Up;
    String          aGetRefFld_Down;
    // --> OD 2007-09-13 #i81002#
    // for GetRefField - referenced item not found
    String          aGetRefFld_RefItemNotFound;
    // <--
    // fuer dynamisches Menu - String "alle"
    String          aStrAllPageHeadFoot;
    // fuer einige Listboxen - String "keine"
    String          aStrNone;
    // fuer Felder, die Fixiert sind
    String          aFixedStr;

    //names of TOXs
    String          aTOXIndexName;
    String          aTOXUserName;
    String          aTOXContentName;
    String          aTOXIllustrationsName;
    String          aTOXObjectsName;
    String          aTOXTablesName;
    String          aTOXAuthoritiesName;

    String          aHyperlinkClick;

    SvStringsDtor   aDocInfoLst;

    // Fly-Anker Bmps
//  Bitmap          aAnchorBmp;
//  Bitmap          aDragAnchorBmp;

    // die AutoFormat-Redline-Kommentare
    inline const SvStringsDtor& GetAutoFmtNameLst() const;

    // returns for the specific filter the new names of pagedescs
    // This method is for the old code of the specific filters with
    // now localized names
    String GetPageDescName( USHORT nNo, BOOL bFirst = FALSE,
                                        BOOL bFollow = FALSE );

    ShellResource();
    ~ShellResource();

private:
    void _GetAutoFmtNameLst() const;
    SvStringsDtor   *pAutoFmtNameLst;
    String          sPageDescFirstName;
    String          sPageDescFollowName;
    String          sPageDescName;
};

inline const SvStringsDtor& ShellResource::GetAutoFmtNameLst() const
{
    if( !pAutoFmtNameLst )
        _GetAutoFmtNameLst();
    return *pAutoFmtNameLst;
}


#endif //_SHELLRES_HXX
