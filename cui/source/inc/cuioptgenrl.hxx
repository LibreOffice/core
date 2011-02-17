/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svx/optgenrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>

// forward ---------------------------------------------------------------

struct GeneralTabPage_Impl;

#define SfxGeneralTabPage   SvxGeneralTabPage
#define INDEX_NOTSET        ((sal_Int16)-1)

// class SvxUserEdit -----------------------------------------------------

class SvxUserEdit : public Edit
{
private:
    sal_Int16   m_nIndex;
    FixedText*  m_pLabel;

public:
    SvxUserEdit( Window* pParent, const ResId& rResId,
                 sal_Int16 nIndex = INDEX_NOTSET, FixedText* pLabel = NULL ) :
        Edit( pParent, rResId, true ), m_nIndex( nIndex ), m_pLabel( pLabel ) {}

    inline void         SetIndex( sal_Int16 nIndex ) { m_nIndex = nIndex; }
    inline sal_Int16    GetIndex() const { return m_nIndex; }
    inline void         SetLabel( FixedText* pLabel ) { m_pLabel = pLabel; }
    inline FixedText*   GetLabel() const { return m_pLabel; }
};

// class SvxGeneralTabPage -----------------------------------------------

class SvxGeneralTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    FixedLine           aAddrFrm;
    FixedText           aCompanyLbl;
    SvxUserEdit         aCompanyEdit;
    FixedText           aNameLbl;
    FixedText           aNameLblRuss;
    FixedText           aNameLblEastern;
    SvxUserEdit         aFirstName;
    SvxUserEdit         aFatherName;
    SvxUserEdit         aName;
    SvxUserEdit         aShortName;
    FixedText           aStreetLbl;
    FixedText           aStreetLblRuss;
    SvxUserEdit         aStreetEdit;
    SvxUserEdit         aApartmentNrEdit;
    FixedText           aCityLbl;
    SvxUserEdit         aPLZEdit;
    SvxUserEdit         aCityEdit;
    SvxUserEdit         aUsCityEdit;
    SvxUserEdit         aUsStateEdit;
    SvxUserEdit         aUsZipEdit;
    FixedText           aCountryLbl;
    SvxUserEdit         aCountryEdit;
    FixedText           aTitlePosLbl;
    SvxUserEdit         aTitleEdit;
    SvxUserEdit         aPositionEdit;
    FixedText           aPhoneLbl;
    SvxUserEdit         aTelPrivEdit;
    SvxUserEdit         aTelCompanyEdit;
    FixedText           aFaxMailLbl;
    SvxUserEdit         aFaxEdit;
    SvxUserEdit         aEmailEdit;
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

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_CUIOPTGENRL_HXX


