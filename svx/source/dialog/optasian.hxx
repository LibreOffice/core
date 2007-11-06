/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optasian.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-06 16:30:28 $
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
#ifndef _SVX_OPTASIAN_HXX
#define _SVX_OPTASIAN_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
struct SvxAsianLayoutPage_Impl;
class SvxAsianLayoutPage : public SfxTabPage
{
    FixedLine    aKerningGB;
    RadioButton aCharKerningRB;
    RadioButton aCharPunctKerningRB;

    FixedLine    aCharDistGB;
    RadioButton aNoCompressionRB;
    RadioButton aPunctCompressionRB;
    RadioButton aPunctKanaCompressionRB;

    FixedLine    aStartEndGB;

    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;
    CheckBox        aStandardCB;

    FixedText   aStartFT;
    Edit        aStartED;
    FixedText   aEndFT;
    Edit        aEndED;
    FixedText   aHintFT;

    SvxAsianLayoutPage_Impl* pImpl;

    DECL_LINK(LanguageHdl, SvxLanguageBox*);
    DECL_LINK(ChangeStandardHdl, CheckBox*);
    DECL_LINK(ModifyHdl, Edit*);

    SvxAsianLayoutPage( Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAsianLayoutPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static UINT16*      GetRanges();
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

