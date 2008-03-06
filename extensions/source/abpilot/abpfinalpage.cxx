/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abpfinalpage.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:34:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

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
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif

//.........................................................................
namespace abp
{
//.........................................................................
    using namespace ::svt;
    using namespace ::utl;

    const SfxFilter* lcl_getBaseFilter()
    {
        static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
        const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        return pFilter;
    }
    //=====================================================================
    //= FinalPage
    //=====================================================================
    //---------------------------------------------------------------------
    FinalPage::FinalPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_FINAL))
        ,m_aExplanation         ( this, ModuleRes( FT_FINISH_EXPL ) )
        ,m_aLocationLabel       ( this, ModuleRes( FT_LOCATION ) )
        ,m_aLocation            ( this, ModuleRes( CBB_LOCATION ) )
        ,m_aBrowse              ( this, ModuleRes( PB_BROWSE ) )
        ,m_aRegisterName        ( this, ModuleRes( CB_REGISTER_DS ) )
        ,m_aNameLabel           ( this, ModuleRes( FT_NAME_EXPL ) )
        ,m_aName                ( this, ModuleRes( ET_DATASOURCENAME ) )
        ,m_aDuplicateNameError  ( this, ModuleRes( FT_DUPLICATENAME ) )
        ,m_aLocationController( ::comphelper::ComponentContext( _pParent->getORB() ), m_aLocation, m_aBrowse )
    {
        FreeResource();

        m_aName.SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
        m_aLocation.SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
        m_aRegisterName.SetClickHdl( LINK( this, FinalPage, OnRegister ) );
        m_aRegisterName.Check(TRUE);
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
    void FinalPage::setFields()
    {
        AddressSettings& rSettings = getSettings();

        INetURLObject aURL( rSettings.sDataSourceName );
        if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
        {
            String sPath = SvtPathOptions().GetWorkPath();
            sPath += '/';
            sPath += String(rSettings.sDataSourceName);

            const SfxFilter* pFilter = lcl_getBaseFilter();
            if ( pFilter )
            {
                String sExt = pFilter->GetDefaultExtension();
                sPath += sExt.GetToken(1,'*');
            }

            aURL.SetURL(sPath);
        }
        OSL_ENSURE( aURL.GetProtocol() != INET_PROT_NOT_VALID ,"No valid file name!");
        rSettings.sDataSourceName = aURL.GetMainURL( INetURLObject::NO_DECODE );
        m_aLocationController.setURL( rSettings.sDataSourceName );
        String sName = aURL.getName( );
        xub_StrLen nPos = sName.Search(String(aURL.GetExtension()));
        if ( nPos != STRING_NOTFOUND )
        {
            sName.Erase(nPos-1,4);
        }
        m_aName.SetText(sName);

        OnRegister(&m_aRegisterName);
    }

    //---------------------------------------------------------------------
    void FinalPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        setFields();
    }

    //---------------------------------------------------------------------
    sal_Bool FinalPage::commitPage( CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        if ( !m_aLocationController.prepareCommit() )
            return sal_False;

        AddressSettings& rSettings = getSettings();
        rSettings.sDataSourceName = m_aLocationController.getURL();
        rSettings.bRegisterDataSource = m_aRegisterName.IsChecked();
        if ( rSettings.bRegisterDataSource )
            rSettings.sRegisteredDataSourceName = m_aName.GetText();

        return sal_True;
    }

    //---------------------------------------------------------------------
    void FinalPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        // get the names of all data sources
        ODataSourceContext aContext( getORB() );
        aContext.getDataSourceNames( m_aInvalidDataSourceNames );

        // give the name edit the focus
        m_aLocation.GrabFocus();

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
    bool FinalPage::canAdvance() const
    {
        return false;
    }

    //---------------------------------------------------------------------
    void FinalPage::implCheckName()
    {
        sal_Bool bValidName = isValidName();
        sal_Bool bEmptyName = 0 == m_aName.GetText().Len();
        sal_Bool bEmptyLocation = 0 == m_aLocation.GetText().Len();

        // enable or disable the finish button
        getDialog()->enableButtons( WZB_FINISH, !bEmptyLocation && (!m_aRegisterName.IsChecked() || bValidName) );

        // show the error message for an invalid name
        m_aDuplicateNameError.Show( !bValidName && !bEmptyName );
    }

    //---------------------------------------------------------------------
    IMPL_LINK( FinalPage, OnNameModified, Edit*, /**/ )
    {
        implCheckName();
        return 0L;
    }

    // -----------------------------------------------------------------------------
    IMPL_LINK( FinalPage, OnRegister, CheckBox*, EMPTYARG )
    {
        BOOL bEnable = m_aRegisterName.IsChecked();
        m_aNameLabel.Enable(bEnable);
        m_aName.Enable(bEnable);
        implCheckName();
        return 0L;
    }
//.........................................................................
}   // namespace abp
//.........................................................................

