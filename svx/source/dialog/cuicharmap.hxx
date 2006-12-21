/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cuicharmap.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-21 12:00:29 $
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
#ifndef _CUI_CHARMAP_HXX
#define _CUI_CHARMAP_HXX

// include ---------------------------------------------------------------

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#include <map>

class SubsetMap;
class SvxCharMapData;

// define ----------------------------------------------------------------

#ifdef MAC
#define CHARMAP_MAXLEN  28
#define COLUMN_COUNT    28
#define ROW_COUNT        8
#else
#define CHARMAP_MAXLEN  32
#define COLUMN_COUNT    16
#define ROW_COUNT        8
#endif

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

