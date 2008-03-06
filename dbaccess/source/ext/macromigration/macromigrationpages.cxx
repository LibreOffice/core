/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macromigrationpages.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:04:03 $
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
#include "precompiled_dbaccess.hxx"

#include "dbmm_module.hxx"
#include "dbmm_global.hrc"
#include "macromigration.hrc"
#include "macromigrationpages.hxx"
#include "macromigrationdialog.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbmm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::frame::XModel;
    /** === end UNO using === **/

    //====================================================================
    //= MacroMigrationPage
    //====================================================================
    //--------------------------------------------------------------------
    MacroMigrationPage::MacroMigrationPage( MacroMigrationDialog& _rParentDialog, const ResId& _rRes )
        :MacroMigrationPage_Base( &_rParentDialog, _rRes )
        ,m_aHeader( this, MacroMigrationResId( FT_HEADER ) )
    {
        Font aFont( m_aHeader.GetFont() );
        aFont.SetWeight( WEIGHT_BOLD );
        m_aHeader.SetFont( aFont );
    }

    //--------------------------------------------------------------------
    MacroMigrationPage::~MacroMigrationPage()
    {
    }

    //--------------------------------------------------------------------
    const MacroMigrationDialog& MacroMigrationPage::getDialog() const
    {
        return *dynamic_cast< const MacroMigrationDialog* >( GetParent() );
    }

    //--------------------------------------------------------------------
    MacroMigrationDialog& MacroMigrationPage::getDialog()
    {
        return *dynamic_cast< MacroMigrationDialog* >( GetParent() );
    }

    //====================================================================
    //= PreparationPage
    //====================================================================
    //--------------------------------------------------------------------
    PreparationPage::PreparationPage( MacroMigrationDialog& _rParentDialog )
        :MacroMigrationPage( _rParentDialog, MacroMigrationResId( TP_PREPARE ) )
        ,m_aIntroduction ( this, MacroMigrationResId( FT_INTRODUCTION    ) )
        ,m_aCloseDocError( this, MacroMigrationResId( FT_CLOSE_DOC_ERROR ) )
    {
        FreeResource();
    }

    //--------------------------------------------------------------------
    void PreparationPage::showCloseDocsError( bool _bShow )
    {
        m_aCloseDocError.Show( _bShow );
    }

    //--------------------------------------------------------------------
    TabPage* PreparationPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new PreparationPage( dynamic_cast< MacroMigrationDialog& >( _rParentDialog ) );
    }

    //====================================================================
    //= SaveDBDocPage
    //====================================================================
    //--------------------------------------------------------------------
    SaveDBDocPage::SaveDBDocPage( MacroMigrationDialog& _rParentDialog )
        :MacroMigrationPage( _rParentDialog, MacroMigrationResId( TP_SAVE_DBDOC_AS ) )
        ,m_aExplanation         ( this, MacroMigrationResId( FT_EXPLANATION             ) )
        ,m_aSaveAsLabel         ( this, MacroMigrationResId( FT_SAVE_AS_LABEL           ) )
        ,m_aSaveAsLocation      ( this, MacroMigrationResId( ED_SAVE_AS_LOCATION        ) )
        ,m_aBrowseSaveAsLocation( this, MacroMigrationResId( PB_BROWSE_SAVE_AS_LOCATION ) )
        ,m_aStartMigration      ( this, MacroMigrationResId( FT_START_MIGRATION         ) )
        ,m_aLocationController( _rParentDialog.getComponentContext(), m_aSaveAsLocation, m_aBrowseSaveAsLocation )
    {
        FreeResource();

        m_aSaveAsLocation.SetModifyHdl( LINK( this, SaveDBDocPage, OnLocationModified ) );
        m_aSaveAsLocation.SetDropDownLineCount( 20 );

        impl_updateLocationDependentItems();
    }

    //--------------------------------------------------------------------
    void SaveDBDocPage::impl_updateLocationDependentItems()
    {
        updateDialogTravelUI();
        m_aStartMigration.Show( m_aSaveAsLocation.GetText().Len() > 0 );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( SaveDBDocPage, OnLocationModified, Edit*, /**/ )
    {
        impl_updateLocationDependentItems();
        return 0L;
    }

    //--------------------------------------------------------------------
    void SaveDBDocPage::initializePage()
    {
        OWizardPage::initializePage();

        try
        {
            // get the document's current URL
            Reference< XModel > xDocument( getDialog().getDocument(), UNO_QUERY_THROW );
            INetURLObject aURLParser( xDocument->getURL() );
            OSL_ENSURE( aURLParser.GetProtocol() != INET_PROT_NOT_VALID, "SaveDBDocPage::initializePage: illegal document URL!" );

            ::rtl::OUStringBuffer aBaseName( aURLParser.getBase() );
            aBaseName.appendAscii( ".backup" );
            aURLParser.setBase( aBaseName.makeStringAndClear() );

            m_aLocationController.setURL( aURLParser.GetMainURL( INetURLObject::NO_DECODE ) );
            impl_updateLocationDependentItems();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    bool SaveDBDocPage::canAdvance() const
    {
        if ( !MacroMigrationPage::canAdvance() )
            return false;

        return m_aSaveAsLocation.GetText().Len() > 0;
    }

    //--------------------------------------------------------------------
    sal_Bool SaveDBDocPage::commitPage( CommitPageReason _eReason )
    {
        if ( !MacroMigrationPage::commitPage( _eReason ) )
            return sal_False;

        if ( eTravelBackward == _eReason )
            return sal_True;

        if ( !m_aLocationController.prepareCommit() )
            return sal_False;

        return sal_True;
    }

    //--------------------------------------------------------------------
    TabPage* SaveDBDocPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new SaveDBDocPage( dynamic_cast< MacroMigrationDialog& >( _rParentDialog ) );
    }

    //====================================================================
    //= ProgressPage
    //====================================================================
    //--------------------------------------------------------------------
    ProgressPage::ProgressPage( MacroMigrationDialog& _rParentDialog )
        :MacroMigrationPage( _rParentDialog, MacroMigrationResId( TP_MIGRATE ) )
        ,m_aObjectCount             ( this, MacroMigrationResId( FT_OBJECT_COUNT            ) )
        ,m_aCurrentObjectLabel      ( this, MacroMigrationResId( FT_CURRENT_OBJECT_LABEL    ) )
        ,m_aCurrentObject           ( this, MacroMigrationResId( FT_CURRENT_OBJECT          ) )
        ,m_aCurrentActionLabel      ( this, MacroMigrationResId( FT_CURRENT_PROGRESS_LABEL    ) )
        ,m_aCurrentAction           ( this, MacroMigrationResId( FT_CURRENT_PROGRESS          ) )
        ,m_aCurrentProgress         ( this, MacroMigrationResId( WND_CURRENT_PROGRESS       ) )
        ,m_aAllProgressLabel        ( this, MacroMigrationResId( FT_ALL_PROGRESS_LABEL      ) )
        ,m_aAllProgressText         ( this, MacroMigrationResId( FT_OBJECT_COUNT_PROGRESS   ) )
        ,m_aAllProgress             ( this, MacroMigrationResId( WND_ALL_PROGRESS           ) )
        ,m_aMigrationDone           ( this, MacroMigrationResId( FT_MIGRATION_DONE          ) )
    {
        FreeResource();
    }

    //--------------------------------------------------------------------
    TabPage* ProgressPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new ProgressPage( dynamic_cast< MacroMigrationDialog& >( _rParentDialog ) );
    }

    //--------------------------------------------------------------------
    void ProgressPage::setDocumentCounts( const sal_Int32 _nForms, const sal_Int32 _nReports )
    {
        String sText( m_aObjectCount.GetText() );
        sText.SearchAndReplaceAscii( "$forms$", String::CreateFromInt32( _nForms ) );
        sText.SearchAndReplaceAscii( "$reports$", String::CreateFromInt32( _nReports ) );
        m_aObjectCount.SetText( sText );
    }

    //--------------------------------------------------------------------
    void ProgressPage::startObject( const ::rtl::OUString& _rObjectName, const ::rtl::OUString& _rCurrentAction, const sal_uInt32 _nRange )
    {
        m_aCurrentObject.SetText( _rObjectName );
        m_aCurrentAction.SetText( _rCurrentAction );
        m_aCurrentProgress.SetRange( _nRange );
        m_aCurrentProgress.SetValue( (sal_uInt32)0 );

        // since this is currently called from the main thread, which does not have the chance
        // to re-schedule, we need to explicitly update the display
        Update();
    }

    //--------------------------------------------------------------------
    void ProgressPage::setObjectProgressText( const ::rtl::OUString& _rText )
    {
        m_aCurrentAction.SetText( _rText );
        Update();
    }

    //--------------------------------------------------------------------
    void ProgressPage::setObjectProgressValue( const sal_uInt32 _nValue )
    {
        m_aCurrentProgress.SetValue( _nValue );
        Update();
    }

    //--------------------------------------------------------------------
    void ProgressPage::endObject()
    {
        m_aCurrentObject.SetText( String() );
        m_aCurrentAction.SetText( String() );
        m_aCurrentProgress.SetValue( (sal_uInt32)0 );
        Update();
    }

    //--------------------------------------------------------------------
    void ProgressPage::start( const sal_uInt32 _nOverallRange )
    {
        m_aAllProgress.SetRange( _nOverallRange );
        Update();
    }

    //--------------------------------------------------------------------
    void ProgressPage::setOverallProgressText( const ::rtl::OUString& _rText )
    {
        m_aAllProgressText.SetText( _rText );
        Update();
    }

    //--------------------------------------------------------------------
    void ProgressPage::setOverallProgressValue( const sal_uInt32 _nValue )
    {
        m_aAllProgress.SetValue( _nValue );
        Update();
    }

    //====================================================================
    //= ResultPage
    //====================================================================
    //--------------------------------------------------------------------
    ResultPage::ResultPage( MacroMigrationDialog& _rParentDialog )
        :MacroMigrationPage( _rParentDialog, MacroMigrationResId( TP_SUMMARY ) )
        ,m_aChangesLabel( this, MacroMigrationResId( FT_CHANGES_LABEL ) )
        ,m_aChanges     ( this, MacroMigrationResId( ED_CHANGES       ) )
    {
        FreeResource();
    }

    //--------------------------------------------------------------------
    TabPage* ResultPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new ResultPage( dynamic_cast< MacroMigrationDialog& >( _rParentDialog ) );
    }

    //--------------------------------------------------------------------
    void ResultPage::displaySummary( const String& _rSummary )
    {
        m_aChanges.SetText( _rSummary );
    }

//........................................................................
} // namespace dbmm
//........................................................................
