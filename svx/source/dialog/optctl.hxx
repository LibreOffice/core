/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optctl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 10:03:07 $
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
#ifndef _SVX_OPTCTL_HXX
#define _SVX_OPTCTL_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

// class SvxCTLOptionsPage -----------------------------------------------------

class SvxCTLOptionsPage : public SfxTabPage
{
private:
    FixedLine           m_aSequenceCheckingFL;
    CheckBox            m_aSequenceCheckingCB;
    CheckBox            m_aRestrictedCB;
    CheckBox            m_aTypeReplaceCB;

    FixedLine           m_aCursorControlFL;
    FixedText           m_aMovementFT;
    RadioButton         m_aMovementLogicalRB;
    RadioButton         m_aMovementVisualRB;

    FixedLine           m_aGeneralFL;
    FixedText           m_aNumeralsFT;
    ListBox             m_aNumeralsLB;

    DECL_LINK( SequenceCheckingCB_Hdl, void* );

    SvxCTLOptionsPage( Window* pParent, const SfxItemSet& rSet );

public:

    virtual ~SvxCTLOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_OPTCTL_HXX

