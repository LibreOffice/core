/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cuioptgenrl.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SVX_CUIOPTGENRL_HXX
#define _SVX_CUIOPTGENRL_HXX

// include ---------------------------------------------------------------

#include <optgenrl.hxx>
#include <sfx2/tabdlg.hxx>
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


