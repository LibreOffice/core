/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeselectionpage.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-29 10:41:08 $
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

#ifndef EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX
#include "typeselectionpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#include "addresssettings.hxx"
#endif
#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif

//.........................................................................
namespace abp
{
    using namespace ::com::sun::star::uno;
//.........................................................................

    //=====================================================================
    //= TypeSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
    TypeSelectionPage::TypeSelectionPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_SELECTABTYPE))
        ,m_aHint            (this,  ResId(FT_TYPE_HINTS))
        ,m_aTypeSep         (this,  ResId(FL_TYPE))
        ,m_aMORK            (this,  ResId(RB_MORK))
        ,m_aThunderbird     (this,  ResId(RB_THUNDERBIRD))
        ,m_aEvolution       (this,  ResId(RB_EVOLUTION))
        ,m_aLDAP            (this,  ResId(RB_LDAP))
        ,m_aOutlook         (this,  ResId(RB_OUTLOOK))
        ,m_aOE              (this,  ResId(RB_OUTLOOKEXPRESS))
        ,m_aOther           (this,  ResId(RB_OTHER))
    {
        FreeResource();

        Link aTypeSelectionHandler = LINK(this, TypeSelectionPage, OnTypeSelected );
        m_aMORK.SetClickHdl( aTypeSelectionHandler );
        m_aThunderbird.SetClickHdl( aTypeSelectionHandler );
        m_aEvolution.SetClickHdl( aTypeSelectionHandler );
        m_aLDAP.SetClickHdl( aTypeSelectionHandler );
        m_aOutlook.SetClickHdl( aTypeSelectionHandler );
        m_aOE.SetClickHdl( aTypeSelectionHandler );
        m_aOther.SetClickHdl( aTypeSelectionHandler );

#ifndef UNX
        sal_Int32 nMoveControlsUp = m_aLDAP.GetPosPixel().Y() - m_aEvolution.GetPosPixel().Y();
        m_aEvolution.Hide();

        Control* pMoveControls[] = {
            &m_aLDAP, &m_aOutlook, &m_aOE, &m_aOther, NULL
        };
        for ( Control** pMoveIt = pMoveControls; *pMoveIt; ++pMoveIt )
        {
            Point aPos = (*pMoveIt)->GetPosPixel();
            aPos.Y() -= nMoveControlsUp;
            (*pMoveIt)->SetPosPixel( aPos );
        }
#endif

#ifdef UNX
        // no Outlook / Outlook Express for ~NIX systems
        sal_Int32 nMoveControlsUp = m_aOther.GetPosPixel().Y() - m_aOutlook.GetPosPixel().Y();

        m_aOutlook.Hide();
        m_aOE.Hide();

        Point aPos = m_aOther.GetPosPixel();
        aPos.Y() -= nMoveControlsUp;
        m_aOther.SetPosPixel( aPos );

        Reference< ::com::sun::star::sdbc::XDriverAccess> xManager(_pParent->getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.DriverManager"))), UNO_QUERY);
        if(!(xManager->getDriverByURL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:evolution"))).is()))
        {
            nMoveControlsUp = m_aLDAP.GetPosPixel().Y() - m_aEvolution.GetPosPixel().Y();
            m_aEvolution.Hide();

            aPos = m_aLDAP.GetPosPixel();
            aPos.Y() -= nMoveControlsUp;
            m_aLDAP.SetPosPixel( aPos );

            aPos = m_aOther.GetPosPixel();
            aPos.Y() -= nMoveControlsUp;
            m_aOther.SetPosPixel( aPos );
        }
#endif
    }

    //---------------------------------------------------------------------
    void TypeSelectionPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        RadioButton* pOptions[] = {
            &m_aMORK, &m_aThunderbird, &m_aEvolution, &m_aLDAP, &m_aOutlook, &m_aOE, &m_aOther, NULL
        };
        for ( RadioButton** pCheck = pOptions; *pCheck; ++pCheck )
            if ( (*pCheck)->IsChecked() )
            {
                (*pCheck)->GrabFocus();
                break;
            }

        getDialog()->enableButtons(WZB_PREVIOUS, sal_False);
    }

    //---------------------------------------------------------------------
    void TypeSelectionPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
        getDialog()->enableButtons(WZB_PREVIOUS, sal_True);
    }

    //---------------------------------------------------------------------
    void TypeSelectionPage::selectType( AddressSourceType _eType )
    {
        m_aMORK.Check(AST_MORK == _eType);
        m_aThunderbird.Check(AST_THUNDERBIRD == _eType);
        m_aEvolution.Check(AST_EVOLUTION == _eType);
        m_aLDAP.Check(AST_LDAP == _eType);
        m_aOutlook.Check(AST_OUTLOOK == _eType);
        m_aOE.Check(AST_OE == _eType);
        m_aOther.Check(AST_OTHER == _eType);
    }

    //---------------------------------------------------------------------
    AddressSourceType TypeSelectionPage::getSelectedType( )
    {
        if (m_aMORK.IsChecked())
            return AST_MORK;
        else if (m_aThunderbird.IsChecked())
            return AST_THUNDERBIRD;
        else if (m_aEvolution.IsChecked())
            return AST_EVOLUTION;
        else if (m_aLDAP.IsChecked())
            return AST_LDAP;
        else if (m_aOutlook.IsChecked())
            return AST_OUTLOOK;
        else if (m_aOE.IsChecked())
            return AST_OE;
        else if (m_aOther.IsChecked())
            return AST_OTHER;

        return AST_INVALID;
    }

    //---------------------------------------------------------------------
    void TypeSelectionPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();
        selectType(rSettings.eType);
    }

    //---------------------------------------------------------------------
    sal_Bool TypeSelectionPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        if (AST_INVALID == getSelectedType( ))
        {
            if (_eReason != IWizardPage::CR_VALIDATE_NOUI)
            {
                ErrorBox aError(this, ModuleRes(RID_ERR_NEEDTYPESELECTION));
                aError.Execute();
            }
            return sal_False;
        }

        AddressSettings& rSettings = getSettings();
        rSettings.eType = getSelectedType();

        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool TypeSelectionPage::determineNextButtonState()
    {
        return  AddressBookSourcePage::determineNextButtonState()
            &&  (AST_INVALID != getSelectedType());
    }

    //---------------------------------------------------------------------
    IMPL_LINK( TypeSelectionPage, OnTypeSelected, void*, NOTINTERESTEDIN )
    {
        getDialog()->typeSelectionChanged( getSelectedType() );
        implCheckNextButton();
        return 0L;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

