/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prntopts.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:47:31 $
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

#ifndef _SD_PRNTOPTS_HXX
#define _SD_PRNTOPTS_HXX

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

/*
enum PrintType
{
    PT_DRAWING,
    PT_NOTES,
    PT_HANDOUT,
    PT_OUTLINE
};
*/
class SdModule;
class SdPrintOptions : public SfxTabPage
{
 friend class SdOptionsDlg;
 friend class SdModule;

private:
    FixedLine           aGrpPrint;
    CheckBox            aCbxDraw;
    CheckBox            aCbxNotes;
    CheckBox            aCbxHandout;
    CheckBox            aCbxOutline;

    FixedLine           aSeparator1FL;
    FixedLine           aGrpOutput;
    RadioButton         aRbtColor;
    RadioButton         aRbtGrayscale;
    RadioButton         aRbtBlackWhite;

    FixedLine           aGrpPrintExt;
    CheckBox            aCbxPagename;
    CheckBox            aCbxDate;
    CheckBox            aCbxTime;
    CheckBox            aCbxHiddenPages;

    FixedLine           aSeparator2FL;
    FixedLine           aGrpPageoptions;
    RadioButton         aRbtDefault;
    RadioButton         aRbtPagesize;
    RadioButton         aRbtPagetile;
    RadioButton         aRbtBooklet;
    CheckBox            aCbxFront;
    CheckBox            aCbxBack;

    CheckBox            aCbxPaperbin;

    const SfxItemSet&   rOutAttrs;

    DECL_LINK( ClickCheckboxHdl, CheckBox * );
    DECL_LINK( ClickBookletHdl, CheckBox * );

    void updateControls();

    using OutputDevice::SetDrawMode;
public:
            SdPrintOptions( Window* pParent, const SfxItemSet& rInAttrs);
            ~SdPrintOptions();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    static  USHORT*     GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    void    SetDrawMode();
    virtual void        PageCreated (SfxAllItemSet aSet);
};



#endif // _SD_PRNTOPTS_HXX


