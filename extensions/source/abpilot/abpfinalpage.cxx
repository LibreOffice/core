/*************************************************************************
 *
 *  $RCSfile: abpfinalpage.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:00:40 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABP_ABPFINALPAGE_HXX
#include "abpfinalpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#include "addresssettings.hxx"
#endif
#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= FinalPage
    //=====================================================================
    //---------------------------------------------------------------------
    FinalPage::FinalPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_FINAL))
        ,m_aExplanation         ( this, ResId( FT_FINISH_EXPL ) )
        ,m_aNameLabel           ( this, ResId( FT_NAME_EXPL ) )
        ,m_aName                ( this, ResId( ET_DATASOURCENAME ) )
        ,m_aDuplicateNameError  ( this, ResId( FT_DUPLICATENAME ) )
    {
        FreeResource();

        m_aName.SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
    }

    //---------------------------------------------------------------------
    sal_Bool FinalPage::isValidName() const
    {
        ::rtl::OUString sCurrentName(m_aName.GetText());

        if (0 == sCurrentName.getLength())
            // the name must not be empty
            return sal_False;

        if ( m_aInvalidDataSourceNames.find( sCurrentName ) != m_aInvalidDataSourceNames.end() )
            // there already is a data source with this name
            return sal_False;

        return sal_True;
    }

    //---------------------------------------------------------------------
    void FinalPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();
        m_aName.SetText(rSettings.sDataSourceName);

        implCheckName();
    }

    //---------------------------------------------------------------------
    sal_Bool FinalPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        AddressSettings& rSettings = getSettings();
        rSettings.sDataSourceName = m_aName.GetText();

        return sal_True;
    }

    //---------------------------------------------------------------------
    void FinalPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        // get the names of all data sources
        ODataSourceContext aContext( getORB() );
        aContext.getDataSourceNames( m_aInvalidDataSourceNames );

        // in real, the data source which this dialog should create, is already part of the context
        // as it's name is - of course - a valid name, we have to remove it from the bag
        DBG_ASSERT( getDialog()->getDataSource().getName() == getSettings().sDataSourceName,
            "FinalPage::ActivatePage: inconsistent names!" );
            // only we ourself or the "admin dialog invokation" page are allowed to produce such an inconsistence,
            // and both should - immediately after doing so - rename the data source
        StringBagIterator aPos = m_aInvalidDataSourceNames.find( getSettings().sDataSourceName );
        DBG_ASSERT( m_aInvalidDataSourceNames.end() != aPos, "FinalPage::ActivatePage: invalid settings!" );
        if ( m_aInvalidDataSourceNames.end() != aPos )
            m_aInvalidDataSourceNames.erase( aPos );

        // give the name edit the focus
        m_aName.GrabFocus();

        // default the finish button
        getDialog()->defaultButton( WZB_FINISH );
    }

    //---------------------------------------------------------------------
    void FinalPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();

        // default the "next" button, again
        getDialog()->defaultButton( WZB_NEXT );
        // disable the finish button
        getDialog()->enableButtons( WZB_FINISH, sal_False );
    }

    //---------------------------------------------------------------------
    sal_Bool FinalPage::determineNextButtonState()
    {
        return sal_False;
    }

    //---------------------------------------------------------------------
    void FinalPage::implCheckName()
    {
        sal_Bool bValidName = isValidName();
        sal_Bool bEmptyName = 0 == m_aName.GetText().Len();

        // enable or disable the finish button
        getDialog()->enableButtons( WZB_FINISH, bValidName );

        // show the error message for an invalid name
        m_aDuplicateNameError.Show( !bValidName && !bEmptyName );
    }

    //---------------------------------------------------------------------
    IMPL_LINK( FinalPage, OnNameModified, void*, NOTINTERESTEDIN )
    {
        implCheckName();
        return 0L;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

