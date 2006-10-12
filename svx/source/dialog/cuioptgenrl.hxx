/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cuioptgenrl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:10:03 $
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
#ifndef _SVX_CUIOPTGENRL_HXX
#define _SVX_CUIOPTGENRL_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_OPTGENRL_HXX
#include <optgenrl.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

// forward ---------------------------------------------------------------

struct GeneralTabPage_Impl;

#define SfxGeneralTabPage SvxGeneralTabPage

// class SvxGeneralTabPage -----------------------------------------------

class SvxGeneralTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    FixedText           aCompanyLbl;
    Edit                aCompanyEdit;
    FixedText           aNameLbl;
    FixedText           aNameLblRuss;
    Edit                aFirstName;
    Edit                aFatherName;
    Edit                aName;
    Edit                aShortName;
    FixedText           aStreetLbl;
    FixedText           aStreetLblRuss;
    Edit                aStreetEdit;
    Edit                aApartmentNrEdit;
    FixedText           aCityLbl;
    Edit                aPLZEdit;
    Edit                aCityEdit;
    Edit                aUsCityEdit;
    Edit                aUsStateEdit;
    Edit                aUsZipEdit;
    FixedText           aCountryLbl;
    Edit                aCountryEdit;
    FixedText           aTitlePosLbl;
    Edit                aTitleEdit;
    Edit                aPositionEdit;
    FixedText           aPhoneLbl;
    Edit                aTelPrivEdit;
    Edit                aTelCompanyEdit;
    FixedText           aFaxMailLbl;
    Edit                aFaxEdit;
    Edit                aEmailEdit;
    FixedLine           aAddrFrm;
    CheckBox            aUseDataCB;

    GeneralTabPage_Impl*    pImpl;


#ifdef _SVX_OPTGENRL_CXX
    DECL_LINK( ModifyHdl_Impl, Edit * );

    sal_Bool            GetAddress_Impl();
    void                SetAddress_Impl();
#endif

protected:
    virtual int         DeactivatePage( SfxItemSet* pSet );

public:
    SvxGeneralTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxGeneralTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_CUIOPTGENRL_HXX


