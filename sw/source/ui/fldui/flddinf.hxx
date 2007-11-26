/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flddinf.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 15:32:14 $
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
#ifndef _SWFLDDINF_HXX
#define _SWFLDDINF_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#include "numfmtlb.hxx"
#include "fldpage.hxx"

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/
class SfxDocumentInfoItem;
class SwFldDokInfPage : public SwFldPage
{
    FixedText           aTypeFT;
    SvTreeListBox       aTypeTLB;
    FixedText           aSelectionFT;
    ListBox             aSelectionLB;
    FixedText           aFormatFT;
    NumFormatListBox    aFormatLB;
    CheckBox            aFixedCB;

    SvLBoxEntry*        pSelEntry;
    com::sun::star::uno::Sequence < ::rtl::OUString > aPropertyNames;

    String              aInfoStr;

    USHORT              nOldSel;
    ULONG               nOldFormat;

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );

    USHORT              FillSelectionLB(USHORT nSubTypeId);

protected:
    virtual USHORT      GetGroup();

public:
                        SwFldDokInfPage(Window* pWindow, const SfxItemSet& rSet);

                        ~SwFldDokInfPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

