/*************************************************************************
 *
 *  $RCSfile: pages.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 15:49:20 $
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

#ifndef _PAGES_HXX_
#define _PAGES_HXX_

#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/scrbar.hxx>
#include <svtools/wizardmachine.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/lstner.hxx>
#include <svtools/xtextedt.hxx>

namespace desktop
{
class WelcomePage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    enum OEMType
    {
        OEM_NONE, OEM_NORMAL, OEM_EXTENDED
    };
    OEMType checkOEM();

public:
    WelcomePage( svt::OWizardMachine* parent, const ResId& resid);
};

class LicenseView : public MultiLineEdit, public SfxListener
{
    BOOL            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, const ResId& rResId );
    ~LicenseView();

    void ScrollDown( ScrollType eScroll );

    BOOL IsEndReached() const;
    BOOL EndReached() const { return mbEndReached; }
    void SetEndReached( BOOL bEnd ) { mbEndReached = bEnd; }

    void SetEndReachedHdl( const Link& rHdl )  { maEndReachedHdl = rHdl; }
    const Link& GetAutocompleteHdl() const { return maEndReachedHdl; }

    void SetScrolledHdl( const Link& rHdl )  { maScrolledHdl = rHdl; }
    const Link& GetScrolledHdl() const { return maScrolledHdl; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

class LicensePage : public svt::OWizardPage
{
private:
    svt::OWizardMachine *m_pParent;
    FixedText m_ftHead;
    FixedText m_ftBody1;
    FixedText m_ftBody1Txt;
    FixedText m_ftBody2;
    FixedText m_ftBody2Txt;
    LicenseView m_mlLicense;
    PushButton m_pbDown;
    sal_Bool m_bLicenseRead;
public:
    LicensePage( svt::OWizardMachine* parent, const ResId& resid);
private:
    DECL_LINK(PageDownHdl, PushButton*);
    DECL_LINK(EndReachedHdl, LicenseView*);
    DECL_LINK(ScrolledHdl, LicenseView*);
protected:
    virtual sal_Bool determineNextButtonState();
};

class MigrationPage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    CheckBox m_cbMigration;
    sal_Bool m_bMigrationDone;
public:
    MigrationPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);

};

class UserPage : public svt::OWizardPage
{
private:
    FixedText m_ftHead;
    FixedText m_ftBody;
    FixedText m_ftFirst;
    FixedText m_ftLast;
    FixedText m_ftInitials;
    FixedText m_ftFather;
    Edit m_edFirst;
    Edit m_edLast;
    Edit m_edInitials;
    Edit m_edFather;
    LanguageType m_lang;

public:
    UserPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);
};


class RegistrationPage : public svt::OWizardPage
{
private:
    FixedText m_ftHeader;
    FixedText m_ftBody;
    FixedImage m_fiImage;
    RadioButton m_rbNow;
    RadioButton m_rbLater;
    RadioButton m_rbNever;
    RadioButton m_rbReg;
    FixedLine m_flSeparator;
    FixedText m_ftEnd;
public:
    RegistrationPage( svt::OWizardMachine* parent, const ResId& resid);
    virtual sal_Bool commitPage(COMMIT_REASON _eReason);
protected:
    virtual sal_Bool determineNextButtonState();
};

}
#endif
