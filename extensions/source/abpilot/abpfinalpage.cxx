/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abpfinalpage.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:37:12 $
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
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

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
        ,m_aExplanation         ( this, ResId( FT_FINISH_EXPL ) )
        ,m_aLocationLabel       ( this, ResId( FT_LOCATION ) )
        ,m_aLocation            ( this, ResId( CBB_LOCATION ) )
        ,m_aBrowse              ( this, ResId( PB_BROWSE ) )
        ,m_aRegisterName        ( this, ResId( CB_REGISTER_DS ) )
        ,m_aNameLabel           ( this, ResId( FT_NAME_EXPL ) )
        ,m_aName                ( this, ResId( ET_DATASOURCENAME ) )
        ,m_aDuplicateNameError  ( this, ResId( FT_DUPLICATENAME ) )
        ,m_bCheckFileName       (sal_True)
    {
        FreeResource();

        m_aName.SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
        m_aLocation.SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
        m_aBrowse.SetClickHdl( LINK( this, FinalPage, OnBrowse ) );
        m_aRegisterName.SetClickHdl( LINK( this, FinalPage, OnRegister ) );
        m_aRegisterName.Check(TRUE);

        const SfxFilter* pFilter = lcl_getBaseFilter();
        if ( pFilter )
        {
            m_aLocation.SetFilter(pFilter->GetDefaultExtension());
        }
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
        OFileNotation aFileNotation( rSettings.sDataSourceName );
        m_aLocation.SetText(aFileNotation.get(OFileNotation::N_SYSTEM));
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
    sal_Bool FinalPage::commitPage(COMMIT_REASON _eReason)
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        OFileNotation aFileNotation( m_aLocation.GetText() );
        ::rtl::OUString sURL = aFileNotation.get(OFileNotation::N_URL);
        // check if the name exists
        if ( m_bCheckFileName )
        {
            if ( ::utl::UCBContentHelper::Exists( sURL ) )
            {
                QueryBox aBox( this, WB_YES_NO, ModuleRes( RID_STR_ALREADYEXISTOVERWRITE ) );
                if ( aBox.Execute() != RET_YES )
                    return sal_False;
            }
        }

        AddressSettings& rSettings = getSettings();
        rSettings.sDataSourceName = sURL;
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

        // in real, the data source which this dialog should create, is already part of the context
        // as it's name is - of course - a valid name, we have to remove it from the bag
//      DBG_ASSERT( getDialog()->getDataSource().getName() == getSettings().sDataSourceName,
//          "FinalPage::ActivatePage: inconsistent names!" );

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
    sal_Bool FinalPage::determineNextButtonState()
    {
        return sal_False;
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
    IMPL_LINK( FinalPage, OnNameModified, Edit*, _pEdit)
    {
        if ( _pEdit == &m_aLocation )
            m_bCheckFileName = sal_True;

        implCheckName();
        return 0L;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( FinalPage, OnBrowse, PushButton*, EMPTYARG )
    {
        OFileNotation aOldFile( m_aLocation.GetText() );
        WinBits nBits(WB_STDMODAL|WB_SAVEAS);
        ::sfx2::FileDialogHelper aFileDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,static_cast<sal_uInt32>(nBits) ,this);
        aFileDlg.SetDisplayDirectory( aOldFile.get(OFileNotation::N_URL) );

        static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
        const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetFilterName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetFilterName());
        }

        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aFileDlg.GetPath() );
            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            {
                OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                m_aLocation.SetText(aFileNotation.get(OFileNotation::N_SYSTEM));
                implCheckName();
                m_bCheckFileName = sal_False;
            }
        }

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

