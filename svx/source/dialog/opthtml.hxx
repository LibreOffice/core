/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: opthtml.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 11:56:36 $
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
#ifndef _OFA_OPTHTML_HXX
#define _OFA_OPTHTML_HXX


#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _VCL_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_TXENCBOX_HXX
#include "txencbox.hxx"
#endif


class OfaHtmlTabPage : public SfxTabPage
{
    FixedLine       aFontSizeGB;
    FixedText       aSize1FT;
    NumericField    aSize1NF;
    FixedText       aSize2FT;
    NumericField    aSize2NF;
    FixedText       aSize3FT;
    NumericField    aSize3NF;
    FixedText       aSize4FT;
    NumericField    aSize4NF;
    FixedText       aSize5FT;
    NumericField    aSize5NF;
    FixedText       aSize6FT;
    NumericField    aSize6NF;
    FixedText       aSize7FT;
    NumericField    aSize7NF;

    FixedLine       aImportGB;
    CheckBox        aNumbersEnglishUSCB;
    CheckBox        aUnknownTagCB;
    CheckBox        aIgnoreFontNamesCB;

    FixedLine       aExportGB;
    ListBox         aExportLB;
    CheckBox        aStarBasicCB;
    CheckBox        aStarBasicWarningCB;
    CheckBox        aPrintExtensionCB;
    CheckBox        aSaveGrfLocalCB;
    FixedText       aCharSetFT;
    SvxTextEncodingBox aCharSetLB;

    DECL_LINK(ExportHdl_Impl, ListBox*);
    DECL_LINK(CheckBoxHdl_Impl, CheckBox*);

    OfaHtmlTabPage(Window* pParent, const SfxItemSet& rSet);
    virtual ~OfaHtmlTabPage();
public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};


#endif //



