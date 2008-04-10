/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cuicharmap.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _CUI_CHARMAP_HXX
#define _CUI_CHARMAP_HXX

// include ---------------------------------------------------------------

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/scrbar.hxx>
#include <sfx2/basedlgs.hxx>
#include <map>

class SubsetMap;
class SvxCharMapData;

// define ----------------------------------------------------------------

#define CHARMAP_MAXLEN  32
#define COLUMN_COUNT    16
#define ROW_COUNT        8

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetVirtualAcc;
}
// class SvxCharacterMap -------------------------------------------------
// the main purpose of this dialog is to enable the use of characters
// that are not easily accesible from the keyboard

class SvxCharacterMap : public SfxModalDialog
{
private:
    SvxCharMapData* const mpCharMapData;

public:
                    SvxCharacterMap( Window* pParent, BOOL bOne = TRUE );
                    ~SvxCharacterMap();

    void            DisableFontSelection();

    const Font&     GetCharFont() const;
    void            SetCharFont( const Font& rFont );
    void            SetFont( const Font& rFont ) { SetCharFont( rFont ); } //! irgendwann entfernen

    void            SetChar( sal_UCS4 );
    sal_UCS4        GetChar() const;

    String          GetCharacters() const;
};

#endif

