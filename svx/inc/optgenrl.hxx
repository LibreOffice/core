/*************************************************************************
 *
 *  $RCSfile: optgenrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-03-16 12:37:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_OPTGENRL_HXX
#define _SVX_OPTGENRL_HXX

// include ---------------------------------------------------------------

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

class SvxAddressItem;
struct GeneralTabPage_Impl;

// define ----------------------------------------------------------------

#define SfxGeneralTabPage SvxGeneralTabPage

// defines um von Aussen "uber einen Slot einem bestimmten Edit
// den Fokus zu setzen.
#define UNKNOWN_EDIT    ((USHORT)0)
#define COMPANY_EDIT    ((USHORT)1)
#define FIRSTNAME_EDIT  ((USHORT)2)
#define LASTNAME_EDIT   ((USHORT)3)
#define STREET_EDIT     ((USHORT)4)
#define COUNTRY_EDIT    ((USHORT)5)
#define PLZ_EDIT        ((USHORT)6)
#define CITY_EDIT       ((USHORT)7)
#define STATE_EDIT      ((USHORT)8)
#define TITLE_EDIT      ((USHORT)9)
#define POSITION_EDIT   ((USHORT)10)
#define SHORTNAME_EDIT  ((USHORT)11)
#define TELPRIV_EDIT    ((USHORT)12)
#define TELCOMPANY_EDIT ((USHORT)13)
#define FAX_EDIT        ((USHORT)14)
#define EMAIL_EDIT      ((USHORT)15)

// class SvxGeneralTabPage -----------------------------------------------

class SvxGeneralTabPage : public SfxTabPage
{
private:
    FixedText           aCompanyLbl;
    Edit                aCompanyEdit;
    FixedText           aNameLbl;
    FixedText           aNameLblRuss;
    Edit                aFatherName;
    Edit                aFirstName;
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

    String              GetAddress_Impl();
    void                SetAddress_Impl( const SvxAddressItem& rAddress );
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

#endif // #ifndef _SVX_OPTGENRL_HXX


