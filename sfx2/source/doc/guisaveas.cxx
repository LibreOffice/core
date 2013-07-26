/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/io/IOException.hpp>

#include "guisaveas.hxx"

#include <unotools/pathoptions.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/miscopt.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <sfx2/sfxsids.hrc>
#include <doc.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/request.hxx>
#include <sfxtypes.hxx>
#include "alienwarn.hxx"

#include <sfx2/docmacromode.hxx>
#include <comphelper/interaction.hxx>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <rtl/ref.hxx>
#include <framework/interaction.hxx>
#include <svtools/sfxecode.hxx>
#include "../appl/app.hrc"

#include <boost/scoped_ptr.hpp>

#include <com/sun/star/frame/Desktop.hpp>

#include <officecfg/Office/Common.hxx>

// flags that specify requested operation
#define EXPORT_REQUESTED            1
#define PDFEXPORT_REQUESTED         2
#define PDFDIRECTEXPORT_REQUESTED   4
#define WIDEEXPORT_REQUESTED        8
#define SAVE_REQUESTED              16
#define SAVEAS_REQUESTED            32
#define SAVEACOPY_REQUESTED         64

// possible statuses of save operation
#define STATUS_NO_ACTION            0
#define STATUS_SAVE                 1
#define STATUS_SAVEAS               2
#define STATUS_SAVEAS_STANDARDNAME  3

const char aFilterNameString[] = "FilterName";
const char aFilterOptionsString[] = "FilterOptions";
const char aFilterDataString[] = "FilterData";
const char aFilterFlagsString[] = "FilterFlags";

using namespace ::com::sun::star;

namespace {
//-------------------------------------------------------------------------
static sal_uInt16 getSlotIDFromMode( sal_Int8 nStoreMode )
{
    // This is a temporary hardcoded solution must be removed when
    // dialogs do not need parameters in SidSet representation any more

    sal_uInt16 nResult = 0;
    if ( nStoreMode == EXPORT_REQUESTED || nStoreMode == ( EXPORT_REQUESTED | SAVEACOPY_REQUESTED | WIDEEXPORT_REQUESTED ) )
        nResult = SID_EXPORTDOC;
    else if ( nStoreMode == ( EXPORT_REQUESTED | PDFEXPORT_REQUESTED ) )
        nResult = SID_EXPORTDOCASPDF;
    else if ( nStoreMode == ( EXPORT_REQUESTED | PDFEXPORT_REQUESTED | PDFDIRECTEXPORT_REQUESTED ) )
        nResult = SID_DIRECTEXPORTDOCASPDF;
    else if ( nStoreMode == SAVEAS_REQUESTED || nStoreMode == ( EXPORT_REQUESTED | WIDEEXPORT_REQUESTED ) )
        nResult = SID_SAVEASDOC;
    else {
        DBG_ASSERT( sal_False, "Unacceptable slot name is provided!\n" );
    }

    return nResult;
}

//-------------------------------------------------------------------------
static sal_uInt8 getStoreModeFromSlotName( const OUString& aSlotName )
{
    sal_uInt8 nResult = 0;
    if ( aSlotName == "ExportTo" )
        nResult = EXPORT_REQUESTED;
    else if ( aSlotName == "ExportToPDF" )
        nResult = EXPORT_REQUESTED | PDFEXPORT_REQUESTED;
    else if ( aSlotName == "ExportDirectToPDF" )
        nResult = EXPORT_REQUESTED | PDFEXPORT_REQUESTED | PDFDIRECTEXPORT_REQUESTED;
    else if ( aSlotName == "Save" )
        nResult = SAVE_REQUESTED;
    else if ( aSlotName == "SaveAs" )
        nResult = SAVEAS_REQUESTED;
    else
        throw task::ErrorCodeIOException( OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );

    return nResult;
}

//-------------------------------------------------------------------------
static sal_Int32 getMustFlags( sal_Int8 nStoreMode )
{
    return ( SFX_FILTER_EXPORT
            | ( ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) ) ? 0 : SFX_FILTER_IMPORT ) );
}

//-------------------------------------------------------------------------
static sal_Int32 getDontFlags( sal_Int8 nStoreMode )
{
    return ( SFX_FILTER_INTERNAL
            | SFX_FILTER_NOTINFILEDLG
            | ( ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) ) ? SFX_FILTER_IMPORT : 0 ) );
}

//=========================================================================
// class DocumentSettingsGuard
//=========================================================================

class DocumentSettingsGuard
{
    uno::Reference< beans::XPropertySet > m_xDocumentSettings;
    sal_Bool m_bPreserveReadOnly;
    sal_Bool m_bReadOnlySupported;

    sal_Bool m_bRestoreSettings;
public:
    DocumentSettingsGuard( const uno::Reference< frame::XModel >& xModel, sal_Bool bReadOnly, sal_Bool bRestore )
    : m_bPreserveReadOnly( sal_False )
    , m_bReadOnlySupported( sal_False )
    , m_bRestoreSettings( bRestore )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xDocSettingsSupplier( xModel, uno::UNO_QUERY_THROW );
            m_xDocumentSettings.set(
                xDocSettingsSupplier->createInstance(
                    OUString( "com.sun.star.document.Settings" ) ),
                uno::UNO_QUERY_THROW );

            OUString aLoadReadonlyString( "LoadReadonly" );

            try
            {
                m_xDocumentSettings->getPropertyValue( aLoadReadonlyString ) >>= m_bPreserveReadOnly;
                m_xDocumentSettings->setPropertyValue( aLoadReadonlyString, uno::makeAny( bReadOnly ) );
                m_bReadOnlySupported = sal_True;
            }
            catch( const uno::Exception& )
            {}
        }
        catch( const uno::Exception& )
        {}

        if ( ( bReadOnly && !m_bReadOnlySupported ) )
            throw uno::RuntimeException(); // the user could provide the data, so it must be stored
    }

    ~DocumentSettingsGuard()
    {
        if ( m_bRestoreSettings )
        {
            OUString aLoadReadonlyString( "LoadReadonly" );

            try
            {
                if ( m_bReadOnlySupported )
                    m_xDocumentSettings->setPropertyValue( aLoadReadonlyString, uno::makeAny( m_bPreserveReadOnly ) );
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "Unexpected exception!" );
            }
        }
    }
};
} // anonymous namespace

//=========================================================================
// class ModelData_Impl
//=========================================================================
class ModelData_Impl
{
    SfxStoringHelper* m_pOwner;
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference< frame::XStorable > m_xStorable;
    uno::Reference< frame::XStorable2 > m_xStorable2;
    uno::Reference< util::XModifiable > m_xModifiable;

    OUString m_aModuleName;
    ::comphelper::SequenceAsHashMap* m_pDocumentPropsHM;
    ::comphelper::SequenceAsHashMap* m_pModulePropsHM;

    ::comphelper::SequenceAsHashMap m_aMediaDescrHM;

    sal_Bool m_bRecommendReadOnly;

public:
    ModelData_Impl( SfxStoringHelper& aOwner,
                    const uno::Reference< frame::XModel >& xModel,
                    const uno::Sequence< beans::PropertyValue >& aMediaDescr );

    ~ModelData_Impl();

    void FreeDocumentProps();

    uno::Reference< frame::XModel > GetModel();
    uno::Reference< frame::XStorable > GetStorable();
    uno::Reference< frame::XStorable2 > GetStorable2();
    uno::Reference< util::XModifiable > GetModifiable();

    ::comphelper::SequenceAsHashMap& GetMediaDescr() { return m_aMediaDescrHM; }

    sal_Bool IsRecommendReadOnly() const { return m_bRecommendReadOnly; }

    const ::comphelper::SequenceAsHashMap& GetDocProps();

    OUString GetModuleName();
    const ::comphelper::SequenceAsHashMap& GetModuleProps();

    void CheckInteractionHandler();


    OUString GetDocServiceName();
    uno::Sequence< beans::PropertyValue > GetDocServiceDefaultFilterCheckFlags( sal_Int32 nMust, sal_Int32 nDont );
    uno::Sequence< beans::PropertyValue > GetDocServiceAnyFilter( sal_Int32 nMust, sal_Int32 nDont );
    uno::Sequence< beans::PropertyValue > GetPreselectedFilter_Impl( sal_Int8 nStoreMode );
    uno::Sequence< beans::PropertyValue > GetDocServiceDefaultFilter();

    sal_Bool ExecuteFilterDialog_Impl( const OUString& aFilterName );

    sal_Int8 CheckSaveAcceptable( sal_Int8 nCurStatus );
    sal_Int8 CheckStateForSave();

    sal_Int8 CheckFilter( const OUString& );

    sal_Bool CheckFilterOptionsDialogExistence();

    sal_Bool OutputFileDialog( sal_Int8 nStoreMode,
                                const ::comphelper::SequenceAsHashMap& aPreselectedFilterPropsHM,
                                sal_Bool bSetStandardName,
                                OUString& aSuggestedName,
                                sal_Bool bPreselectPassword,
                                OUString& aSuggestedDir,
                                sal_Int16 nDialog,
                                const OUString& rStandardDir,
                                const ::com::sun::star::uno::Sequence< OUString >& rBlackList
                                );

    sal_Bool ShowDocumentInfoDialog();

    OUString GetRecommendedDir( const OUString& aSuggestedDir );
    OUString GetRecommendedName( const OUString& aSuggestedName,
                                        const OUString& aTypeName );

};

//-------------------------------------------------------------------------
ModelData_Impl::ModelData_Impl( SfxStoringHelper& aOwner,
                                const uno::Reference< frame::XModel >& xModel,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescr )
: m_pOwner( &aOwner )
, m_xModel( xModel )
, m_pDocumentPropsHM( NULL )
, m_pModulePropsHM( NULL )
, m_aMediaDescrHM( aMediaDescr )
, m_bRecommendReadOnly( sal_False )
{
    CheckInteractionHandler();
    OUString sModuleName;
    try
    {
        uno::Reference< lang::XComponent > xCurrentComponent = frame::Desktop::create( comphelper::getProcessComponentContext() )->getCurrentComponent();
        sModuleName = aOwner.GetModuleManager()->identify(xCurrentComponent);
        if(sModuleName == "com.sun.star.chart2.ChartDocument")
        {
            // let us switch the model and set the xStorable and
            // XStorable2 to the old model.
            // This is an ugly hack because we have no SfxObjectShell for chart2 yet.
            // We need SfxObjectShell for the heavy work around ODF document creation
            // because chart2 only writes the basic stream out.
            // In future in might make sense to implement a full scale object shell in
            // chart2 and make chart2 an own program.
            m_xModel = uno::Reference< frame::XModel >(xCurrentComponent, uno::UNO_QUERY_THROW );
            m_xStorable = uno::Reference< frame::XStorable >(xModel, uno::UNO_QUERY_THROW );
            m_xStorable2 = uno::Reference< frame::XStorable2 >(xModel, uno::UNO_QUERY_THROW );
        }
    }
    catch(...)
    {
        // we don't want to pass on any errors;
    }
}

//-------------------------------------------------------------------------
ModelData_Impl::~ModelData_Impl()
{
    FreeDocumentProps();
    if ( m_pDocumentPropsHM )
        delete m_pDocumentPropsHM;

    if ( m_pModulePropsHM )
        delete m_pModulePropsHM;
}

//-------------------------------------------------------------------------
void ModelData_Impl::FreeDocumentProps()
{
    if ( m_pDocumentPropsHM )
    {
        delete m_pDocumentPropsHM;
        m_pDocumentPropsHM = NULL;
    }
}

//-------------------------------------------------------------------------
uno::Reference< frame::XModel > ModelData_Impl::GetModel()
{
    if ( !m_xModel.is() )
        throw uno::RuntimeException();

    return m_xModel;
}

//-------------------------------------------------------------------------
uno::Reference< frame::XStorable > ModelData_Impl::GetStorable()
{
    if ( !m_xStorable.is() )
    {
        m_xStorable = uno::Reference< frame::XStorable >( m_xModel, uno::UNO_QUERY );
        if ( !m_xStorable.is() )
            throw uno::RuntimeException();
    }

    return m_xStorable;
}

//-------------------------------------------------------------------------
uno::Reference< frame::XStorable2 > ModelData_Impl::GetStorable2()
{
    if ( !m_xStorable2.is() )
    {
        m_xStorable2 = uno::Reference< frame::XStorable2 >( m_xModel, uno::UNO_QUERY );
        if ( !m_xStorable2.is() )
            throw uno::RuntimeException();
    }

    return m_xStorable2;
}

//-------------------------------------------------------------------------
uno::Reference< util::XModifiable > ModelData_Impl::GetModifiable()
{
    if ( !m_xModifiable.is() )
    {
        m_xModifiable = uno::Reference< util::XModifiable >( m_xModel, uno::UNO_QUERY );
        if ( !m_xModifiable.is() )
            throw uno::RuntimeException();
    }

    return m_xModifiable;
}

//-------------------------------------------------------------------------
const ::comphelper::SequenceAsHashMap& ModelData_Impl::GetDocProps()
{
    if ( !m_pDocumentPropsHM )
        m_pDocumentPropsHM = new ::comphelper::SequenceAsHashMap( GetModel()->getArgs() );

    return *m_pDocumentPropsHM;
}

//-------------------------------------------------------------------------
OUString ModelData_Impl::GetModuleName()
{
    if ( m_aModuleName.isEmpty() )
    {
        m_aModuleName = m_pOwner->GetModuleManager()->identify(
                                                uno::Reference< uno::XInterface >( m_xModel, uno::UNO_QUERY ) );
        if ( m_aModuleName.isEmpty() )
            throw uno::RuntimeException(); // TODO:
    }
    return m_aModuleName;
}

//-------------------------------------------------------------------------
const ::comphelper::SequenceAsHashMap& ModelData_Impl::GetModuleProps()
{
    if ( !m_pModulePropsHM )
    {
        uno::Sequence< beans::PropertyValue > aModuleProps;
        m_pOwner->GetModuleManager()->getByName( GetModuleName() ) >>= aModuleProps;
        if ( !aModuleProps.getLength() )
            throw uno::RuntimeException(); // TODO;
        m_pModulePropsHM = new ::comphelper::SequenceAsHashMap( aModuleProps );
    }

    return *m_pModulePropsHM;
}

//-------------------------------------------------------------------------
OUString ModelData_Impl::GetDocServiceName()
{
    return GetModuleProps().getUnpackedValueOrDefault("ooSetupFactoryDocumentService", OUString());
}

//-------------------------------------------------------------------------
void ModelData_Impl::CheckInteractionHandler()
{
    ::comphelper::SequenceAsHashMap::const_iterator aInteractIter =
            m_aMediaDescrHM.find( OUString("InteractionHandler") );

    if ( aInteractIter == m_aMediaDescrHM.end() )
    {
        try {
            m_aMediaDescrHM[ OUString("InteractionHandler") ]
                <<= task::InteractionHandler::createWithParent( comphelper::getProcessComponentContext(), 0);
        }
        catch( const uno::Exception& )
        {
        }
    }
    else
    {
        uno::Reference< task::XInteractionHandler > xInteract;
        DBG_ASSERT( ( aInteractIter->second >>= xInteract ) && xInteract.is(), "Broken interaction handler is provided!\n" );
    }
}

//-------------------------------------------------------------------------
uno::Sequence< beans::PropertyValue > ModelData_Impl::GetDocServiceDefaultFilter()
{
    uno::Sequence< beans::PropertyValue > aProps;

    OUString aFilterName = GetModuleProps().getUnpackedValueOrDefault(
                                                                OUString("ooSetupFactoryDefaultFilter"),
                                                                OUString() );

    m_pOwner->GetFilterConfiguration()->getByName( aFilterName ) >>= aProps;

    return aProps;
}

//-------------------------------------------------------------------------
uno::Sequence< beans::PropertyValue > ModelData_Impl::GetDocServiceDefaultFilterCheckFlags( sal_Int32 nMust,
                                                                                                sal_Int32 nDont )
{
    uno::Sequence< beans::PropertyValue > aFilterProps;
    uno::Sequence< beans::PropertyValue > aProps = GetDocServiceDefaultFilter();
    if ( aProps.getLength() )
    {
        ::comphelper::SequenceAsHashMap aFiltHM( aProps );
        sal_Int32 nFlags = aFiltHM.getUnpackedValueOrDefault("Flags",
                                                        (sal_Int32)0 );
        if ( ( ( nFlags & nMust ) == nMust ) && !( nFlags & nDont ) )
            aFilterProps = aProps;
    }

    return aFilterProps;
}


//-------------------------------------------------------------------------
uno::Sequence< beans::PropertyValue > ModelData_Impl::GetDocServiceAnyFilter( sal_Int32 nMust, sal_Int32 nDont )
{
    uno::Sequence< beans::NamedValue > aSearchRequest( 1 );
    aSearchRequest[0].Name = OUString("DocumentService");
    aSearchRequest[0].Value <<= GetDocServiceName();

    return ::comphelper::MimeConfigurationHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
}

//-------------------------------------------------------------------------
uno::Sequence< beans::PropertyValue > ModelData_Impl::GetPreselectedFilter_Impl( sal_Int8 nStoreMode )
{
    uno::Sequence< beans::PropertyValue > aFilterProps;

    sal_Int32 nMust = getMustFlags( nStoreMode );
    sal_Int32 nDont = getDontFlags( nStoreMode );

    if ( nStoreMode & PDFEXPORT_REQUESTED )
    {
        // Preselect PDF-Filter for EXPORT
        uno::Sequence< beans::NamedValue > aSearchRequest( 2 );
        aSearchRequest[0].Name = OUString("Type");
        aSearchRequest[0].Value <<= OUString("pdf_Portable_Document_Format");
        aSearchRequest[1].Name = OUString("DocumentService");
        aSearchRequest[1].Value <<= GetDocServiceName();

        aFilterProps = ::comphelper::MimeConfigurationHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
    }
    else
    {
        aFilterProps = GetDocServiceDefaultFilterCheckFlags( nMust, nDont );

        if ( !aFilterProps.getLength() )
        {
            // the default filter was not faund, use just the first acceptable one
            aFilterProps = GetDocServiceAnyFilter( nMust, nDont );
        }
    }

    return aFilterProps;
}

//-------------------------------------------------------------------------
sal_Bool ModelData_Impl::ExecuteFilterDialog_Impl( const OUString& aFilterName )
{
    sal_Bool bDialogUsed = sal_False;

    try {
        uno::Sequence < beans::PropertyValue > aProps;
        uno::Any aAny = m_pOwner->GetFilterConfiguration()->getByName( aFilterName );
        if ( aAny >>= aProps )
        {
            sal_Int32 nPropertyCount = aProps.getLength();
            for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
            {
                if( aProps[nProperty].Name == "UIComponent" )
                {
                    OUString aServiceName;
                    aProps[nProperty].Value >>= aServiceName;
                    if( !aServiceName.isEmpty() )
                    {
                        uno::Reference< ui::dialogs::XExecutableDialog > xFilterDialog(
                                                    comphelper::getProcessServiceFactory()->createInstance( aServiceName ), uno::UNO_QUERY );
                        uno::Reference< beans::XPropertyAccess > xFilterProperties( xFilterDialog, uno::UNO_QUERY );

                        if( xFilterDialog.is() && xFilterProperties.is() )
                        {
                            bDialogUsed = sal_True;

                            uno::Reference< document::XExporter > xExporter( xFilterDialog, uno::UNO_QUERY );
                            if( xExporter.is() )
                                xExporter->setSourceDocument(
                                    uno::Reference< lang::XComponent >( GetModel(), uno::UNO_QUERY ) );

                            uno::Sequence< beans::PropertyValue > aPropsForDialog;
                            GetMediaDescr() >> aPropsForDialog;
                            xFilterProperties->setPropertyValues( aPropsForDialog );

                            if( xFilterDialog->execute() )
                            {
                                uno::Sequence< beans::PropertyValue > aPropsFromDialog =
                                                                            xFilterProperties->getPropertyValues();
                                for ( sal_Int32 nInd = 0; nInd < aPropsFromDialog.getLength(); nInd++ )
                                    GetMediaDescr()[aPropsFromDialog[nInd].Name] = aPropsFromDialog[nInd].Value;
                            }
                            else
                            {
                                throw task::ErrorCodeIOException( OUString(),
                                                                    uno::Reference< uno::XInterface >(),
                                                                    ERRCODE_IO_ABORT );
                            }
                        }
                    }

                    break;
                }
            }
        }
    }
    catch( const container::NoSuchElementException& )
    {
        // the filter name is unknown
        throw task::ErrorCodeIOException( OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );
    }
    catch( const task::ErrorCodeIOException& )
    {
        throw;
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("sfx.doc", "ignoring UNO exception " << e.Message);
    }

    return bDialogUsed;
}

//-------------------------------------------------------------------------
sal_Int8 ModelData_Impl::CheckSaveAcceptable( sal_Int8 nCurStatus )
{
    sal_Int8 nResult = nCurStatus;

    if ( nResult != STATUS_NO_ACTION && GetStorable()->hasLocation() )
    {
        // the saving is acceptable
        // in case the configuration entry is not set or set to false
        // or in case of version creation
        OUString aVersionCommentString = OUString("VersionComment");
        if ( officecfg::Office::Common::Save::Document::AlwaysSaveAs::get()
          && GetMediaDescr().find( aVersionCommentString ) == GetMediaDescr().end() )
        {
            // notify the user that SaveAs is going to be done
            Window* pWin = SfxStoringHelper::GetModelWindow( m_xModel );
            QueryBox aMessageBox( pWin, WB_OK_CANCEL | WB_DEF_OK, SfxResId(STR_NEW_FILENAME_SAVE).toString() );
            if ( aMessageBox.Execute() == RET_OK )
                nResult = STATUS_SAVEAS;
            else
                nResult = STATUS_NO_ACTION;
        }
    }

    return nResult;
}

//-------------------------------------------------------------------------
sal_Int8 ModelData_Impl::CheckStateForSave()
{
    // if the document is readonly or a new one a SaveAs operation must be used
    if ( !GetStorable()->hasLocation() || GetStorable()->isReadonly() )
        return STATUS_SAVEAS;

    // check acceptable entries for media descriptor
    sal_Bool bVersInfoNeedsStore = sal_False;
    ::comphelper::SequenceAsHashMap aAcceptedArgs;

    OUString aVersionCommentString("VersionComment");
    OUString aAuthorString("Author");
    OUString aInteractionHandlerString("InteractionHandler");
    OUString aStatusIndicatorString("StatusIndicator");
    OUString aFailOnWarningString("FailOnWarning");

    if ( GetMediaDescr().find( aVersionCommentString ) != GetMediaDescr().end() )
    {
        bVersInfoNeedsStore = sal_True;
        aAcceptedArgs[ aVersionCommentString ] = GetMediaDescr()[ aVersionCommentString ];
    }
    if ( GetMediaDescr().find( aAuthorString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aAuthorString ] = GetMediaDescr()[ aAuthorString ];
    if ( GetMediaDescr().find( aInteractionHandlerString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aInteractionHandlerString ] = GetMediaDescr()[ aInteractionHandlerString ];
    if ( GetMediaDescr().find( aStatusIndicatorString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aStatusIndicatorString ] = GetMediaDescr()[ aStatusIndicatorString ];
	if ( GetMediaDescr().find( aFailOnWarningString ) != GetMediaDescr().end() )
		aAcceptedArgs[ aFailOnWarningString ] = GetMediaDescr()[ aFailOnWarningString ];

    // remove unacceptable entry if there is any
    DBG_ASSERT( GetMediaDescr().size() == aAcceptedArgs.size(),
                "Unacceptable parameters are provided in Save request!\n" );
    if ( GetMediaDescr().size() != aAcceptedArgs.size() )
        GetMediaDescr() = aAcceptedArgs;

    // the document must be modified unless the always-save flag is set.
    SvtMiscOptions aMiscOptions;
    sal_Bool bAlwaysAllowSave = aMiscOptions.IsSaveAlwaysAllowed();
    if (!bAlwaysAllowSave)
    {
        if ( !GetModifiable()->isModified() && !bVersInfoNeedsStore )
            return STATUS_NO_ACTION;
    }

    // check that the old filter is acceptable
    OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    OUString(aFilterNameString),
                                                    OUString() );
    sal_Int8 nResult = CheckFilter( aOldFilterName );

    return nResult;
}

sal_Bool hasMacros( const uno::Reference< frame::XModel >& xModel  )
{
    sal_Bool bHasMacros = sal_False;
    uno::Reference< script::XLibraryContainer > xContainer;
    uno::Reference< beans::XPropertySet > xProps( xModel, uno::UNO_QUERY );
    if ( xProps.is() )
    {
        xProps->getPropertyValue( "BasicLibraries" ) >>= xContainer;
        bHasMacros = sfx2::DocumentMacroMode::containerHasBasicMacros( xContainer );
    }
    return bHasMacros;
}

sal_Int8 ModelData_Impl::CheckFilter( const OUString& aFilterName )
{
    ::comphelper::SequenceAsHashMap aFiltPropsHM;
    sal_Int32 nFiltFlags = 0;
    if ( !aFilterName.isEmpty() )
    {
        // get properties of filter
        uno::Sequence< beans::PropertyValue > aFilterProps;
        if ( !aFilterName.isEmpty() )
            m_pOwner->GetFilterConfiguration()->getByName( aFilterName ) >>= aFilterProps;

        aFiltPropsHM = ::comphelper::SequenceAsHashMap( aFilterProps );
        nFiltFlags = aFiltPropsHM.getUnpackedValueOrDefault("Flags", (sal_Int32)0 );
    }

    // only a temporary solution until default filter retrieving feature is implemented
    // then GetDocServiceDefaultFilter() must be used
    ::comphelper::SequenceAsHashMap aDefFiltPropsHM = GetDocServiceDefaultFilterCheckFlags( 3, 0 );
    sal_Int32 nDefFiltFlags = aDefFiltPropsHM.getUnpackedValueOrDefault("Flags", (sal_Int32)0 );

    // if the old filter is not acceptable
    // and there is no default filter or it is not acceptable for requested parameters then proceed with saveAs
    if ( ( !aFiltPropsHM.size() || !( nFiltFlags & SFX_FILTER_EXPORT ) )
      && ( !aDefFiltPropsHM.size() || !( nDefFiltFlags & SFX_FILTER_EXPORT ) || nDefFiltFlags & SFX_FILTER_INTERNAL ) )
        return STATUS_SAVEAS;

    // so at this point there is either an acceptable old filter or default one
    if ( !aFiltPropsHM.size() || !( nFiltFlags & SFX_FILTER_EXPORT ) )
    {
        // so the default filter must be acceptable
        return STATUS_SAVEAS_STANDARDNAME;
    }
    else if ( ( !( nFiltFlags & SFX_FILTER_OWN ) || ( nFiltFlags & SFX_FILTER_ALIEN ) )
           && aDefFiltPropsHM.size()
           && ( nDefFiltFlags & SFX_FILTER_EXPORT ) && !( nDefFiltFlags & SFX_FILTER_INTERNAL ))
    {
        // the default filter is acceptable and the old filter is alian one
        // so ask to make a saveAs operation
        OUString aUIName = aFiltPropsHM.getUnpackedValueOrDefault("UIName",
                                                                                OUString() );
        OUString aDefUIName = aDefFiltPropsHM.getUnpackedValueOrDefault("UIName",
                                                                                OUString() );
        OUString aPreusedFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    OUString("PreusedFilterName"),
                                                    OUString() );
        if ( !aPreusedFilterName.equals( aFilterName ) && !aUIName.equals( aDefUIName ) )
        {
            // is it possible to get these names from somewhere and not just
            // hardcode them?
            OUString sXLSXFilter("Calc MS Excel 2007 XML");
            OUString sOtherXLSXFilter("Calc Office Open XML");
            bool bHasMacros = hasMacros( GetModel() );
            if ( bHasMacros && (  aFilterName == sXLSXFilter  ||  aFilterName == sOtherXLSXFilter ) )
            {
                uno::Reference< task::XInteractionHandler > xHandler;
                GetMediaDescr()[ OUString( "InteractionHandler" ) ] >>= xHandler;
                bool bResult = false;
                if ( xHandler.is() )
                {
                    try
                    {
                        task::ErrorCodeRequest aErrorCode;
                        aErrorCode.ErrCode = ERRCODE_SFX_VBASIC_CANTSAVE_STORAGE;

                        uno::Any aRequest = uno::makeAny( aErrorCode );
                        uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( 2 );

                        ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
                        aContinuations[ 0 ] = pApprove.get();

                        ::rtl::Reference< ::comphelper::OInteractionAbort > pAbort( new ::comphelper::OInteractionAbort );
                        aContinuations[ 1 ] = pAbort.get();
                        xHandler->handle(::framework::InteractionRequest::CreateRequest (aRequest,aContinuations));
                        bResult = pApprove->wasSelected();
                    }
                    catch( const uno::Exception& )
                    {
                    }
                    if ( !bResult )
                        return STATUS_SAVEAS;
                }
            }
            if ( !SfxStoringHelper::WarnUnacceptableFormat( GetModel(), aUIName, aDefUIName, sal_True ) )
                return STATUS_SAVEAS_STANDARDNAME;
        }
    }

    return STATUS_SAVE;
}

//-------------------------------------------------------------------------
sal_Bool ModelData_Impl::CheckFilterOptionsDialogExistence()
{
    uno::Sequence< beans::NamedValue > aSearchRequest( 1 );
    aSearchRequest[0].Name = OUString("DocumentService");
    aSearchRequest[0].Value <<= GetDocServiceName();

    uno::Reference< container::XEnumeration > xFilterEnum =
                                    m_pOwner->GetFilterQuery()->createSubSetEnumerationByProperties( aSearchRequest );

    while ( xFilterEnum->hasMoreElements() )
    {
        uno::Sequence< beans::PropertyValue > pProps;
        if ( xFilterEnum->nextElement() >>= pProps )
        {
            ::comphelper::SequenceAsHashMap aPropsHM( pProps );
            OUString aUIServName = aPropsHM.getUnpackedValueOrDefault(
                                            OUString("UIComponent"),
                                            OUString() );
            if ( !aUIServName.isEmpty() )
                return sal_True;
        }
    }

    return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool ModelData_Impl::OutputFileDialog( sal_Int8 nStoreMode,
                                            const ::comphelper::SequenceAsHashMap& aPreselectedFilterPropsHM,
                                            sal_Bool bSetStandardName,
                                            OUString& aSuggestedName,
                                            sal_Bool bPreselectPassword,
                                            OUString& aSuggestedDir,
                                            sal_Int16 nDialog,
                                            const OUString& rStandardDir,
                                            const ::com::sun::star::uno::Sequence< OUString >& rBlackList)
{
    sal_Bool bUseFilterOptions = sal_False;

    ::comphelper::SequenceAsHashMap::const_iterator aOverwriteIter =
                GetMediaDescr().find( OUString("Overwrite") );

    // the file name must be specified if overwrite option is set
    if ( aOverwriteIter != GetMediaDescr().end() )
           throw task::ErrorCodeIOException( OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );

    // no target file name is specified
    // we need to show the file dialog

    // check if we have a filter which allows for filter options, so we need a corresponding checkbox in the dialog
    sal_Bool bAllowOptions = sal_False;

    // in case of Export, filter options dialog is used if available
    if( !( nStoreMode & EXPORT_REQUESTED ) || ( nStoreMode & WIDEEXPORT_REQUESTED ) )
        bAllowOptions = CheckFilterOptionsDialogExistence();

    // get the filename by dialog ...
    // create the file dialog
    sal_Int16  aDialogMode = bAllowOptions
        ? (com::sun::star::ui::dialogs::TemplateDescription::
           FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS)
        : (com::sun::star::ui::dialogs::TemplateDescription::
           FILESAVE_AUTOEXTENSION_PASSWORD);
    sal_Int64 aDialogFlags = 0;

    if( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        if ( nStoreMode & PDFEXPORT_REQUESTED )
            aDialogMode = com::sun::star::ui::dialogs::TemplateDescription::
                FILESAVE_AUTOEXTENSION;
        else
            aDialogMode = com::sun::star::ui::dialogs::TemplateDescription::
                FILESAVE_AUTOEXTENSION_SELECTION;
        aDialogFlags = SFXWB_EXPORT;
    }

    if( ( nStoreMode & EXPORT_REQUESTED ) && ( nStoreMode & SAVEACOPY_REQUESTED ) && ( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        aDialogFlags = SFXWB_SAVEACOPY;
    }

    boost::scoped_ptr<sfx2::FileDialogHelper> pFileDlg;

    OUString aDocServiceName = GetDocServiceName();
    DBG_ASSERT( !aDocServiceName.isEmpty(), "No document service for this module set!" );

    sal_Int32 nMust = getMustFlags( nStoreMode );
    sal_Int32 nDont = getDontFlags( nStoreMode );
    sfx2::FileDialogHelper::Context eCtxt = sfx2::FileDialogHelper::UNKNOWN_CONTEXT;

    if ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        if ( ( nStoreMode & PDFEXPORT_REQUESTED ) && aPreselectedFilterPropsHM.size() )
        {
            // this is a PDF export
            // the filter options has been shown already
            OUString aFilterUIName = aPreselectedFilterPropsHM.getUnpackedValueOrDefault(
                                                        OUString("UIName"),
                                                        OUString() );

            pFileDlg.reset(new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aFilterUIName, OUString( "pdf" ), rStandardDir, rBlackList ));
            pFileDlg->SetCurrentFilter( aFilterUIName );
        }
        else
        {
            // This is the normal dialog
            pFileDlg.reset(new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aDocServiceName, nDialog, nMust, nDont, rStandardDir, rBlackList ));
        }

        if ( aDocServiceName == "com.sun.star.drawing.DrawingDocument" )
            eCtxt = sfx2::FileDialogHelper::SD_EXPORT;
        else if ( aDocServiceName == "com.sun.star.presentation.PresentationDocument" )
            eCtxt = sfx2::FileDialogHelper::SI_EXPORT;
        else if ( aDocServiceName == "com.sun.star.text.TextDocument" )
            eCtxt = sfx2::FileDialogHelper::SW_EXPORT;

        if ( eCtxt != sfx2::FileDialogHelper::UNKNOWN_CONTEXT )
               pFileDlg->SetContext( eCtxt );

        pFileDlg->CreateMatcher( aDocServiceName );

        uno::Reference< ui::dialogs::XFilePicker > xFilePicker = pFileDlg->GetFilePicker();
        uno::Reference< ui::dialogs::XFilePickerControlAccess > xControlAccess =
        uno::Reference< ui::dialogs::XFilePickerControlAccess >( xFilePicker, uno::UNO_QUERY );

        if ( xControlAccess.is() )
        {
            OUString aCtrlText = SfxResId(STR_EXPORTBUTTON).toString();
            xControlAccess->setLabel( ui::dialogs::CommonFilePickerElementIds::PUSHBUTTON_OK, aCtrlText );

            aCtrlText = SfxResId(STR_LABEL_FILEFORMAT).toString();
            xControlAccess->setLabel( ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER_LABEL, aCtrlText );
        }
    }
    else
    {
        // This is the normal dialog
        pFileDlg.reset(new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aDocServiceName, nDialog, nMust, nDont, rStandardDir, rBlackList ));
        pFileDlg->CreateMatcher( aDocServiceName );
    }

    OUString aAdjustToType;

    const OUString sFilterNameString(aFilterNameString);

    if ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        // it is export, set the preselected filter
        OUString aFilterUIName = aPreselectedFilterPropsHM.getUnpackedValueOrDefault(
                                        OUString("UIName"),
                                        OUString() );
        pFileDlg->SetCurrentFilter( aFilterUIName );
    }
    // it is no export, bSetStandardName == true means that user agreed to store document in the default (default default ;-)) format
    else if ( bSetStandardName || GetStorable()->hasLocation() )
    {
        uno::Sequence< beans::PropertyValue > aOldFilterProps;
        OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                        sFilterNameString,
                                                        OUString() );

        if ( !aOldFilterName.isEmpty() )
            m_pOwner->GetFilterConfiguration()->getByName( aOldFilterName ) >>= aOldFilterProps;

        ::comphelper::SequenceAsHashMap aOldFiltPropsHM( aOldFilterProps );
        sal_Int32 nOldFiltFlags = aOldFiltPropsHM.getUnpackedValueOrDefault("Flags", (sal_Int32)0 );

        if ( bSetStandardName || ( nOldFiltFlags & nMust ) != nMust || nOldFiltFlags & nDont )
        {
            // the suggested type will be changed, the extension should be adjusted
            aAdjustToType = aPreselectedFilterPropsHM.getUnpackedValueOrDefault(
                                            OUString("Type"),
                                            OUString() );

            OUString aFilterUIName = aPreselectedFilterPropsHM.getUnpackedValueOrDefault(
                                            OUString("UIName"),
                                            OUString() );
            pFileDlg->SetCurrentFilter( aFilterUIName );
        }
        else
        {
            pFileDlg->SetCurrentFilter( aOldFiltPropsHM.getUnpackedValueOrDefault(
                                                        OUString("UIName"),
                                                        OUString() ) );
        }
    }

    OUString aRecommendedDir = GetRecommendedDir( aSuggestedDir );
    if ( !aRecommendedDir.isEmpty() )
        pFileDlg->SetDisplayFolder( aRecommendedDir );
    OUString aRecommendedName = GetRecommendedName( aSuggestedName, aAdjustToType );
    if ( !aRecommendedName.isEmpty() )
        pFileDlg->SetFileName( aRecommendedName );

    uno::Reference < view::XSelectionSupplier > xSel( GetModel()->getCurrentController(), uno::UNO_QUERY );
    if ( xSel.is() && xSel->getSelection().hasValue() )
        GetMediaDescr()[OUString("SelectionOnly")] <<= sal_True;

    // This is a temporary hardcoded solution must be removed when
    // dialogs do not need parameters in SidSet representation any more
    sal_uInt16 nSlotID = getSlotIDFromMode( nStoreMode );
    if ( !nSlotID )
        throw lang::IllegalArgumentException(); // TODO:

    // generate SidSet from MediaDescriptor and provide it into FileDialog
    // than merge changed SidSet back
    SfxAllItemSet aDialogParams( SFX_APP()->GetPool() );
    SfxItemSet* pDialogParams = &aDialogParams;
    TransformParameters( nSlotID,
                         GetMediaDescr().getAsConstPropertyValueList(),
                         aDialogParams,
                         NULL );

    const SfxPoolItem* pItem = NULL;
    if ( bPreselectPassword && aDialogParams.GetItemState( SID_ENCRYPTIONDATA, sal_True, &pItem ) != SFX_ITEM_SET )
    {
        // the file dialog preselects the password checkbox if the provided mediadescriptor has encryption data entry
        // after dialog execution the password interaction flag will be either removed or not
        aDialogParams.Put( SfxBoolItem( SID_PASSWORDINTERACTION, sal_True ) );
    }

    // aStringTypeFN is a pure output parameter, pDialogParams is an in/out parameter
    String aStringTypeFN;
    if ( pFileDlg->Execute( pDialogParams, aStringTypeFN ) != ERRCODE_NONE )
    {
        throw task::ErrorCodeIOException( OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_ABORT );
    }

    OUString aFilterName = aStringTypeFN;

    // the following two arguments can not be converted in MediaDescriptor,
    // so they should be removed from the ItemSet after retrieving
    SFX_ITEMSET_ARG( pDialogParams, pRecommendReadOnly, SfxBoolItem, SID_RECOMMENDREADONLY, sal_False );
    m_bRecommendReadOnly = ( pRecommendReadOnly && pRecommendReadOnly->GetValue() );
    pDialogParams->ClearItem( SID_RECOMMENDREADONLY );

    uno::Sequence< beans::PropertyValue > aPropsFromDialog;
    TransformItems( nSlotID, *pDialogParams, aPropsFromDialog, NULL );
    GetMediaDescr() << aPropsFromDialog;

    // get the path from the dialog
    INetURLObject aURL( pFileDlg->GetPath() );
    // the path should be provided outside since it might be used for further calls to the dialog
    aSuggestedName = aURL.GetName( INetURLObject::DECODE_WITH_CHARSET );
       aSuggestedDir = pFileDlg->GetDisplayDirectory();

    // old filter options should be cleared in case different filter is used

    OUString aFilterFromMediaDescr = GetMediaDescr().getUnpackedValueOrDefault(
                                                    sFilterNameString,
                                                    OUString() );
    OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    sFilterNameString,
                                                    OUString() );

    const OUString sFilterOptionsString(aFilterOptionsString);
    const OUString sFilterDataString(aFilterDataString);

    if ( aFilterName.equals( aFilterFromMediaDescr ) )
    {
        // preserv current settings if any
        // if there no current settings and the name is the same
        // as old filter name use old filter settings

        if ( aFilterFromMediaDescr.equals( aOldFilterName ) )
        {
            ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                        GetDocProps().find( sFilterOptionsString );
            if ( aIter != GetDocProps().end()
              && GetMediaDescr().find( sFilterOptionsString ) == GetMediaDescr().end() )
                GetMediaDescr()[aIter->first] = aIter->second;

            aIter = GetDocProps().find( sFilterDataString );
            if ( aIter != GetDocProps().end()
              && GetMediaDescr().find( sFilterDataString ) == GetMediaDescr().end() )
                GetMediaDescr()[aIter->first] = aIter->second;
        }
    }
    else
    {
        GetMediaDescr().erase( sFilterDataString );
        GetMediaDescr().erase( sFilterOptionsString );

        if ( aFilterName.equals( aOldFilterName ) )
        {
            // merge filter option of the document filter

            ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                GetDocProps().find( sFilterOptionsString );
            if ( aIter != GetDocProps().end() )
                GetMediaDescr()[aIter->first] = aIter->second;

            aIter = GetDocProps().find( sFilterDataString );
            if ( aIter != GetDocProps().end() )
                GetMediaDescr()[aIter->first] = aIter->second;
        }
    }

    uno::Reference< ui::dialogs::XFilePickerControlAccess > xExtFileDlg( pFileDlg->GetFilePicker(), uno::UNO_QUERY );
    if ( xExtFileDlg.is() )
    {
        if ( SfxStoringHelper::CheckFilterOptionsAppearence( m_pOwner->GetFilterConfiguration(), aFilterName ) )
            bUseFilterOptions = sal_True;

        if ( ( !( nStoreMode & EXPORT_REQUESTED ) || ( nStoreMode & WIDEEXPORT_REQUESTED ) ) && bUseFilterOptions )
        {
            try
            {
                // for exporters: always show dialog if format uses options
                // for save: show dialog if format uses options and no options given or if forced by user
                uno::Any aVal =
                        xExtFileDlg->getValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS, 0 );

                aVal >>= bUseFilterOptions;
                if ( !bUseFilterOptions )
                    bUseFilterOptions =
                      ( GetMediaDescr().find( sFilterDataString ) == GetMediaDescr().end()
                      && GetMediaDescr().find( sFilterOptionsString ) == GetMediaDescr().end() );
            }
            catch( const lang::IllegalArgumentException& )
            {}
        }
    }

    // merge in results of the dialog execution
    GetMediaDescr()[OUString("URL")] <<=
                                                OUString( aURL.GetMainURL( INetURLObject::NO_DECODE ));
    GetMediaDescr()[sFilterNameString] <<= aFilterName;

    return bUseFilterOptions;
}

//-------------------------------------------------------------------------
sal_Bool ModelData_Impl::ShowDocumentInfoDialog()
{
    sal_Bool bDialogUsed = sal_False;

    try {
        uno::Reference< frame::XController > xController = GetModel()->getCurrentController();
        if ( xController.is() )
        {
            uno::Reference< frame::XDispatchProvider > xFrameDispatch( xController->getFrame(), uno::UNO_QUERY );
            if ( xFrameDispatch.is() )
            {
                util::URL aURL;
                aURL.Complete = OUString(".uno:SetDocumentProperties");

                uno::Reference < util::XURLTransformer > xTransformer( util::URLTransformer::create( comphelper::getProcessComponentContext() ) );
                if ( xTransformer->parseStrict( aURL ) )
                {
                    uno::Reference< frame::XDispatch > xDispatch = xFrameDispatch->queryDispatch(
                                                                                aURL,
                                                                                OUString("_self"),
                                                                                0 );
                    if ( xDispatch.is() )
                    {
                        xDispatch->dispatch( aURL, uno::Sequence< beans::PropertyValue >() );
                        bDialogUsed = sal_True;
                    }
                }
            }
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return bDialogUsed;
}

//-------------------------------------------------------------------------
OUString ModelData_Impl::GetRecommendedDir( const OUString& aSuggestedDir )
{
    OUString aRecommendedDir;

    if ( ( !aSuggestedDir.isEmpty() || GetStorable()->hasLocation() )
      && !GetMediaDescr().getUnpackedValueOrDefault("RepairPackage",
                                                                      sal_False ) )
    {
        INetURLObject aLocation;
        if ( !aSuggestedDir.isEmpty() )
            aLocation = INetURLObject( aSuggestedDir );
        else
        {
            OUString aOldURL = GetStorable()->getLocation();
            if ( !aOldURL.isEmpty() )
            {
                INetURLObject aTmp( aOldURL );
                if ( aTmp.removeSegment() )
                    aLocation = aTmp;
            }

            if ( aLocation.HasError() )
                aLocation = INetURLObject( SvtPathOptions().GetWorkPath() );
        }

        aLocation.setFinalSlash();
        if ( !aLocation.HasError() )
            aRecommendedDir = aLocation.GetMainURL( INetURLObject::NO_DECODE );
    }
    else
    {
        aRecommendedDir = INetURLObject( SvtPathOptions().GetWorkPath() ).GetMainURL( INetURLObject::NO_DECODE );
    }

    return aRecommendedDir;
}

//-------------------------------------------------------------------------
OUString ModelData_Impl::GetRecommendedName( const OUString& aSuggestedName, const OUString& aTypeName )
{
    // the last used name might be provided by aSuggestedName from the old selection, or from the MediaDescriptor
    OUString aRecommendedName;

    if ( !aSuggestedName.isEmpty() )
        aRecommendedName = aSuggestedName;
    else
    {
        aRecommendedName = INetURLObject( GetStorable()->getLocation() ).GetName( INetURLObject::DECODE_WITH_CHARSET );
        if ( aRecommendedName.isEmpty() )
        {
            try {
                uno::Reference< frame::XTitle > xTitle( GetModel(), uno::UNO_QUERY_THROW );
                aRecommendedName = xTitle->getTitle();
            } catch( const uno::Exception& ) {}
        }

        if ( !aRecommendedName.isEmpty() && !aTypeName.isEmpty() )
        {
            // adjust the extension to the type
            uno::Reference< container::XNameAccess > xTypeDetection = uno::Reference< container::XNameAccess >(
                comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.TypeDetection"),
                uno::UNO_QUERY );
            if ( xTypeDetection.is() )
            {
                INetURLObject aObj( OUString( "file:///c:/" ) + aRecommendedName );

                uno::Sequence< beans::PropertyValue > aTypeNameProps;
                if ( ( xTypeDetection->getByName( aTypeName ) >>= aTypeNameProps ) && aTypeNameProps.getLength() )
                {
                    ::comphelper::SequenceAsHashMap aTypeNamePropsHM( aTypeNameProps );
                    uno::Sequence< OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                                    OUString("Extensions"),
                                                    ::uno::Sequence< OUString >() );
                    if ( aExtensions.getLength() )
                        aObj.SetExtension( aExtensions[0] );
                }

                aRecommendedName = aObj.GetName( INetURLObject::DECODE_WITH_CHARSET );
            }
        }
    }

    return aRecommendedName;
}


//=========================================================================
// class SfxStoringHelper
//=========================================================================
//-------------------------------------------------------------------------
SfxStoringHelper::SfxStoringHelper()
{
}

//-------------------------------------------------------------------------
uno::Reference< container::XNameAccess > SfxStoringHelper::GetFilterConfiguration()
{
    if ( !m_xFilterCFG.is() )
    {
        m_xFilterCFG = uno::Reference< container::XNameAccess >(
            comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.FilterFactory"),
            uno::UNO_QUERY );

        if ( !m_xFilterCFG.is() )
            throw uno::RuntimeException();
    }

    return m_xFilterCFG;
}

//-------------------------------------------------------------------------
uno::Reference< container::XContainerQuery > SfxStoringHelper::GetFilterQuery()
{
    if ( !m_xFilterQuery.is() )
    {
        m_xFilterQuery = uno::Reference< container::XContainerQuery >( GetFilterConfiguration(), uno::UNO_QUERY );
        if ( !m_xFilterQuery.is() )
            throw uno::RuntimeException();
    }

    return m_xFilterQuery;
}

//-------------------------------------------------------------------------
uno::Reference< ::com::sun::star::frame::XModuleManager2 > SfxStoringHelper::GetModuleManager()
{
    if ( !m_xModuleManager.is() )
    {
        m_xModuleManager = frame::ModuleManager::create(
            comphelper::getProcessComponentContext() );
    }

    return m_xModuleManager;
}

//-------------------------------------------------------------------------
sal_Bool SfxStoringHelper::GUIStoreModel( uno::Reference< frame::XModel > xModel,
                                            const OUString& aSlotName,
                                            uno::Sequence< beans::PropertyValue >& aArgsSequence,
                                            sal_Bool bPreselectPassword,
                                            OUString aSuggestedName,
                                            sal_uInt16 nDocumentSignatureState )
{
    ModelData_Impl aModelData( *this, xModel, aArgsSequence );

    sal_Bool bDialogUsed = sal_False;

    INetURLObject aURL;

    sal_Bool bSetStandardName = sal_False; // can be set only for SaveAs

    // parse the slot name
    sal_Int8 nStoreMode = getStoreModeFromSlotName( aSlotName );
    sal_Int8 nStatusSave = STATUS_NO_ACTION;

    ::comphelper::SequenceAsHashMap::const_iterator aSaveACopyIter =
                        aModelData.GetMediaDescr().find( ::rtl::OUString("SaveACopy") );
    if ( aSaveACopyIter != aModelData.GetMediaDescr().end() )
    {
        sal_Bool bSaveACopy = sal_False;
        aSaveACopyIter->second >>= bSaveACopy;
        if ( bSaveACopy )
            nStoreMode = EXPORT_REQUESTED | SAVEACOPY_REQUESTED | WIDEEXPORT_REQUESTED;
    }
    // handle the special cases
    if ( nStoreMode & SAVEAS_REQUESTED )
    {
        ::comphelper::SequenceAsHashMap::const_iterator aSaveToIter =
                        aModelData.GetMediaDescr().find( OUString("SaveTo") );
        if ( aSaveToIter != aModelData.GetMediaDescr().end() )
        {
            sal_Bool bWideExport = sal_False;
            aSaveToIter->second >>= bWideExport;
            if ( bWideExport )
                nStoreMode = EXPORT_REQUESTED | WIDEEXPORT_REQUESTED;
        }

        // if saving is not acceptable the warning must be shown even in case of SaveAs operation
        if ( ( nStoreMode & SAVEAS_REQUESTED ) && aModelData.CheckSaveAcceptable( STATUS_SAVEAS ) == STATUS_NO_ACTION )
            throw task::ErrorCodeIOException( OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_ABORT );
    }
    else if ( nStoreMode & SAVE_REQUESTED )
    {
        // if saving is not acceptable by the configuration the warning must be shown
        nStatusSave = aModelData.CheckSaveAcceptable( STATUS_SAVE );

        if ( nStatusSave == STATUS_NO_ACTION )
            throw task::ErrorCodeIOException( OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_ABORT );
        else if ( nStatusSave == STATUS_SAVE )
        {
            // check whether it is possible to use save operation
            nStatusSave = aModelData.CheckStateForSave();
        }

        if ( nStatusSave == STATUS_NO_ACTION )
        {
            throw task::ErrorCodeIOException( OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_ABORT );
        }
        else if ( nStatusSave != STATUS_SAVE )
        {
            // this should be a usual SaveAs operation
            nStoreMode = SAVEAS_REQUESTED;
            if ( nStatusSave == STATUS_SAVEAS_STANDARDNAME )
                bSetStandardName = sal_True;
        }
    }

    if ( !( nStoreMode & EXPORT_REQUESTED ) )
    {
        // if it is no export, warn user that the signature will be removed
        if (  SIGNATURESTATE_SIGNATURES_OK == nDocumentSignatureState
           || SIGNATURESTATE_SIGNATURES_INVALID == nDocumentSignatureState
           || SIGNATURESTATE_SIGNATURES_NOTVALIDATED == nDocumentSignatureState
           || SIGNATURESTATE_SIGNATURES_PARTIAL_OK == nDocumentSignatureState)
        {
            if ( QueryBox( NULL, SfxResId( RID_XMLSEC_QUERY_LOSINGSIGNATURE ) ).Execute() != RET_YES )
            {
                // the user has decided not to store the document
                throw task::ErrorCodeIOException( OUString(),
                                                  uno::Reference< uno::XInterface >(),
                                                  ERRCODE_IO_ABORT );
            }
        }
    }

    if ( nStoreMode & SAVE_REQUESTED && nStatusSave == STATUS_SAVE )
    {
        // Document properties can contain streams that should be freed before storing
        aModelData.FreeDocumentProps();

        if ( aModelData.GetStorable2().is() )
        {
            try
            {
                aModelData.GetStorable2()->storeSelf( aModelData.GetMediaDescr().getAsConstPropertyValueList() );
            }
            catch (const lang::IllegalArgumentException& e)
            {
                SAL_WARN("sfx", "Ignoring parameters! "
                    "ModelData considers this illegal:  " << e.Message);
                aModelData.GetStorable()->store();
            }
        }
        else
        {
            OSL_FAIL( "XStorable2 is not supported by the model!\n" );
            aModelData.GetStorable()->store();
        }

        return sal_False;
    }

    // preselect a filter for the storing process
    uno::Sequence< beans::PropertyValue > aFilterProps = aModelData.GetPreselectedFilter_Impl( nStoreMode );

    DBG_ASSERT( aFilterProps.getLength(), "No filter for storing!\n" );
    if ( !aFilterProps.getLength() )
        throw task::ErrorCodeIOException( OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );

    ::comphelper::SequenceAsHashMap aFilterPropsHM( aFilterProps );
    OUString aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                                    OUString("Name"),
                                                                    OUString() );

    const OUString sFilterNameString(aFilterNameString);

    OUString aFilterFromMediaDescr = aModelData.GetMediaDescr().getUnpackedValueOrDefault(
                                                    sFilterNameString,
                                                    OUString() );
    OUString aOldFilterName = aModelData.GetDocProps().getUnpackedValueOrDefault(
                                                    sFilterNameString,
                                                    OUString() );

    sal_Bool bUseFilterOptions = sal_False;
    ::comphelper::SequenceAsHashMap::const_iterator aFileNameIter = aModelData.GetMediaDescr().find( OUString("URL") );

    const OUString sFilterOptionsString(aFilterOptionsString);
    const OUString sFilterDataString(aFilterDataString);
    const OUString sFilterFlagsString(aFilterFlagsString);

    if ( ( nStoreMode & EXPORT_REQUESTED ) && ( nStoreMode & PDFEXPORT_REQUESTED ) && !( nStoreMode & PDFDIRECTEXPORT_REQUESTED ) )
    {
        // this is PDF export, the filter options dialog should be shown before the export
        aModelData.GetMediaDescr()[sFilterNameString] <<= aFilterName;
        if ( aModelData.GetMediaDescr().find( sFilterFlagsString ) == aModelData.GetMediaDescr().end()
          && aModelData.GetMediaDescr().find( sFilterOptionsString ) == aModelData.GetMediaDescr().end()
          && aModelData.GetMediaDescr().find( sFilterDataString ) == aModelData.GetMediaDescr().end() )
        {
            // execute filter options dialog since no options are set in the media descriptor
            if ( aModelData.ExecuteFilterDialog_Impl( aFilterName ) )
                bDialogUsed = sal_True;
        }
    }

    if ( aFileNameIter == aModelData.GetMediaDescr().end() )
    {
        sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG;
        ::comphelper::SequenceAsHashMap::const_iterator aDlgIter =
            aModelData.GetMediaDescr().find( OUString("UseSystemDialog") );
        if ( aDlgIter != aModelData.GetMediaDescr().end() )
        {
            sal_Bool bUseSystemDialog = sal_True;
            if ( aDlgIter->second >>= bUseSystemDialog )
            {
                if ( bUseSystemDialog )
                    nDialog = SFX2_IMPL_DIALOG_SYSTEM;
                else
                    nDialog = SFX2_IMPL_DIALOG_OOO;
            }
        }

        // The Dispatch supports parameter FolderName that overwrites SuggestedSaveAsDir
        OUString aSuggestedDir = aModelData.GetMediaDescr().getUnpackedValueOrDefault("FolderName", OUString() );
        if ( aSuggestedDir.isEmpty() )
        {
            aSuggestedDir = aModelData.GetMediaDescr().getUnpackedValueOrDefault("SuggestedSaveAsDir", OUString() );
            if ( aSuggestedDir.isEmpty() )
                aSuggestedDir = aModelData.GetDocProps().getUnpackedValueOrDefault("SuggestedSaveAsDir", OUString() );
        }

        aSuggestedName = aModelData.GetMediaDescr().getUnpackedValueOrDefault("SuggestedSaveAsName", OUString() );
        if ( aSuggestedName.isEmpty() )
            aSuggestedName = aModelData.GetDocProps().getUnpackedValueOrDefault("SuggestedSaveAsName", OUString() );

        OUString sStandardDir;
        ::comphelper::SequenceAsHashMap::const_iterator aStdDirIter =
            aModelData.GetMediaDescr().find( OUString("StandardDir") );
        if ( aStdDirIter != aModelData.GetMediaDescr().end() )
            aStdDirIter->second >>= sStandardDir;

        ::com::sun::star::uno::Sequence< OUString >  aBlackList;

        ::comphelper::SequenceAsHashMap::const_iterator aBlackListIter =
            aModelData.GetMediaDescr().find( OUString("BlackList") );
        if ( aBlackListIter != aModelData.GetMediaDescr().end() )
            aBlackListIter->second >>= aBlackList;

        sal_Bool bExit = sal_False;
        while ( !bExit )
        {
            // in case the dialog is opened a second time the folder should be the same as previously navigated to by the user, not what was handed over by initial parameters
            bUseFilterOptions = aModelData.OutputFileDialog( nStoreMode, aFilterProps, bSetStandardName, aSuggestedName, bPreselectPassword, aSuggestedDir, nDialog, sStandardDir, aBlackList );
            if ( nStoreMode == SAVEAS_REQUESTED )
            {
                // in case of saving check filter for possible alien warning
                OUString aSelFilterName = aModelData.GetMediaDescr().getUnpackedValueOrDefault(
                                                                                sFilterNameString,
                                                                                OUString() );
                sal_Int8 nStatusFilterSave = aModelData.CheckFilter( aSelFilterName );
                if ( nStatusFilterSave == STATUS_SAVEAS_STANDARDNAME )
                {
                    // switch to best filter
                    bSetStandardName = sal_True;
                }
                else if ( nStatusFilterSave == STATUS_SAVE )
                {
                    // user confirmed alien filter or "good" filter is used
                    bExit = sal_True;
                }
            }
            else
                bExit = sal_True;
        }

        bDialogUsed = sal_True;
        aFileNameIter = aModelData.GetMediaDescr().find( OUString("URL") );
    }
    else
    {
        // the target file name is provided so check if new filter options
        // are provided or old options can be used
        if ( aFilterFromMediaDescr.equals( aOldFilterName ) )
        {
            ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                            aModelData.GetDocProps().find( sFilterOptionsString );
            if ( aIter != aModelData.GetDocProps().end()
              && aModelData.GetMediaDescr().find( sFilterOptionsString ) == aModelData.GetMediaDescr().end() )
                aModelData.GetMediaDescr()[aIter->first] = aIter->second;

            aIter = aModelData.GetDocProps().find( sFilterDataString );
            if ( aIter != aModelData.GetDocProps().end()
              && aModelData.GetMediaDescr().find( sFilterDataString ) == aModelData.GetMediaDescr().end() )
                aModelData.GetMediaDescr()[aIter->first] = aIter->second;
        }
    }

    if ( aFileNameIter != aModelData.GetMediaDescr().end() )
    {
        OUString aFileName;
        aFileNameIter->second >>= aFileName;
        aURL.SetURL( aFileName );
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );

        ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                aModelData.GetMediaDescr().find( sFilterNameString );

        if ( aIter != aModelData.GetMediaDescr().end() )
            aIter->second >>= aFilterName;
        else
            aModelData.GetMediaDescr()[sFilterNameString] <<= aFilterName;

        DBG_ASSERT( !aFilterName.isEmpty(), "Illegal filter!" );
    }
    else
    {
        DBG_ASSERT( sal_False, "This code must be unreachable!\n" );
        throw task::ErrorCodeIOException( OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );
    }

    ::comphelper::SequenceAsHashMap::const_iterator aIter =
                            aModelData.GetMediaDescr().find( OUString("FilterFlags") );
    sal_Bool bFilterFlagsSet = ( aIter != aModelData.GetMediaDescr().end() );

    if( !( nStoreMode & PDFEXPORT_REQUESTED ) && !bFilterFlagsSet
        && ( ( nStoreMode & EXPORT_REQUESTED ) || bUseFilterOptions ) )
    {
        // execute filter options dialog
        if ( aModelData.ExecuteFilterDialog_Impl( aFilterName ) )
            bDialogUsed = sal_True;
    }

    // so the arguments will not change any more and can be stored to the main location
    aArgsSequence = aModelData.GetMediaDescr().getAsConstPropertyValueList();

    // store the document and handle it's docinfo
    SvtSaveOptions aOptions;

    DocumentSettingsGuard aSettingsGuard( aModelData.GetModel(), aModelData.IsRecommendReadOnly(), nStoreMode & EXPORT_REQUESTED );

    OSL_ENSURE( aModelData.GetMediaDescr().find( OUString( "Password" ) ) == aModelData.GetMediaDescr().end(), "The Password property of MediaDescriptor should not be used here!" );
    if ( aOptions.IsDocInfoSave()
      && ( !aModelData.GetStorable()->hasLocation()
          || INetURLObject( aModelData.GetStorable()->getLocation() ) != aURL ) )
    {
        // this is defenitly not a Save operation
        // so the document info can be updated

        // on export document info must be preserved
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            aModelData.GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<util::XCloneable> xCloneable(
            xDPS->getDocumentProperties(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xOldDocProps(
            xCloneable->createClone(), uno::UNO_QUERY_THROW);

        // use dispatch API to show document info dialog
        if ( aModelData.ShowDocumentInfoDialog() )
            bDialogUsed = sal_True;
        else
        {
            OSL_FAIL( "Can't execute document info dialog!\n" );
        }

        try {
            // Document properties can contain streams that should be freed before storing
            aModelData.FreeDocumentProps();
            if ( nStoreMode & EXPORT_REQUESTED )
                aModelData.GetStorable()->storeToURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
            else
                aModelData.GetStorable()->storeAsURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
        }
        catch( const uno::Exception& )
        {
            if ( nStoreMode & EXPORT_REQUESTED )
            {
                SetDocInfoState(aModelData.GetModel(), xOldDocProps, sal_True);
            }
            throw;
        }

        if ( nStoreMode & EXPORT_REQUESTED )
        {
            SetDocInfoState(aModelData.GetModel(), xOldDocProps, sal_True);
        }
    }
    else
    {
        // Document properties can contain streams that should be freed before storing
        aModelData.FreeDocumentProps();

        // this is actually a save operation with different parameters
        // so storeTo or storeAs without DocInfo operations are used
        if ( nStoreMode & EXPORT_REQUESTED )
            aModelData.GetStorable()->storeToURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
        else
            aModelData.GetStorable()->storeAsURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
    }

    return bDialogUsed;
}

//-------------------------------------------------------------------------
// static
sal_Bool SfxStoringHelper::CheckFilterOptionsAppearence(
                                                    const uno::Reference< container::XNameAccess >& xFilterCFG,
                                                    const OUString& aFilterName )
{
    sal_Bool bUseFilterOptions = sal_False;

    DBG_ASSERT( xFilterCFG.is(), "No filter configuration!\n" );
    if( xFilterCFG.is() )
    {
        try {
               uno::Sequence < beans::PropertyValue > aProps;
            uno::Any aAny = xFilterCFG->getByName( aFilterName );
               if ( aAny >>= aProps )
               {
                ::comphelper::SequenceAsHashMap aPropsHM( aProps );
                   OUString aServiceName = aPropsHM.getUnpackedValueOrDefault(
                                                    OUString("UIComponent"),
                                                    OUString() );
                if( !aServiceName.isEmpty() )
                       bUseFilterOptions = sal_True;
            }
        }
        catch( const uno::Exception& )
        {
        }
    }

    return bUseFilterOptions;
}

//-------------------------------------------------------------------------
// static
void SfxStoringHelper::SetDocInfoState(
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< document::XDocumentProperties>& i_xOldDocProps,
        sal_Bool bNoModify )
{
    uno::Reference<document::XDocumentPropertiesSupplier> const
        xModelDocPropsSupplier(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> const xDocPropsToFill =
        xModelDocPropsSupplier->getDocumentProperties();
    uno::Reference< beans::XPropertySet > const xPropSet(
            i_xOldDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);

    uno::Reference< util::XModifiable > xModifiable( xModel, uno::UNO_QUERY );
    if ( bNoModify && !xModifiable.is() )
        throw uno::RuntimeException();

    sal_Bool bIsModified = bNoModify && xModifiable->isModified();

    try
    {
        uno::Reference< beans::XPropertySet > const xSet(
                xDocPropsToFill->getUserDefinedProperties(), uno::UNO_QUERY);
        uno::Reference< beans::XPropertyContainer > xContainer( xSet, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();
        uno::Sequence< beans::Property > lProps = xSetInfo->getProperties();
        const beans::Property* pProps = lProps.getConstArray();
        sal_Int32 c = lProps.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            uno::Any aValue = xPropSet->getPropertyValue( pProps[i].Name );
            if ( pProps[i].Attributes & ::com::sun::star::beans::PropertyAttribute::REMOVABLE )
            {
                try
                {
                    // QUESTION: DefaultValue?!
                    xContainer->addProperty( pProps[i].Name, pProps[i].Attributes, aValue );
                }
                catch (beans::PropertyExistException const&) {}
                try
                {
                    // it is possible that the propertysets from XML and binary files differ; we shouldn't break then
                    xSet->setPropertyValue( pProps[i].Name, aValue );
                }
                catch ( const uno::Exception& ) {}
            }
        }

        // sigh... have to set these manually i'm afraid... wonder why
        // SfxObjectShell doesn't handle this internally, should be easier
        xDocPropsToFill->setAuthor(i_xOldDocProps->getAuthor());
        xDocPropsToFill->setGenerator(i_xOldDocProps->getGenerator());
        xDocPropsToFill->setCreationDate(i_xOldDocProps->getCreationDate());
        xDocPropsToFill->setTitle(i_xOldDocProps->getTitle());
        xDocPropsToFill->setSubject(i_xOldDocProps->getSubject());
        xDocPropsToFill->setDescription(i_xOldDocProps->getDescription());
        xDocPropsToFill->setKeywords(i_xOldDocProps->getKeywords());
        xDocPropsToFill->setModifiedBy(i_xOldDocProps->getModifiedBy());
        xDocPropsToFill->setModificationDate(i_xOldDocProps->getModificationDate());
        xDocPropsToFill->setPrintedBy(i_xOldDocProps->getPrintedBy());
        xDocPropsToFill->setPrintDate(i_xOldDocProps->getPrintDate());
        xDocPropsToFill->setAutoloadURL(i_xOldDocProps->getAutoloadURL());
        xDocPropsToFill->setAutoloadSecs(i_xOldDocProps->getAutoloadSecs());
        xDocPropsToFill->setDefaultTarget(i_xOldDocProps->getDefaultTarget());
        xDocPropsToFill->setEditingCycles(i_xOldDocProps->getEditingCycles());
        xDocPropsToFill->setEditingDuration(i_xOldDocProps->getEditingDuration());
        // other attributes e.g. DocumentStatistics are not editable from dialog
    }
    catch (const uno::Exception& e)
    {
        SAL_INFO("sfx.doc", "SetDocInfoState: caught exception: " << e.Message);
    }

    // set the modified flag back if required
    if ( bNoModify && bIsModified != xModifiable->isModified() )
        xModifiable->setModified( bIsModified );
}

//-------------------------------------------------------------------------
// static
sal_Bool SfxStoringHelper::WarnUnacceptableFormat( const uno::Reference< frame::XModel >& xModel,
                                                    OUString aOldUIName,
                                                    OUString /*aDefUIName*/,
                                                    sal_Bool /*bCanProceedFurther*/ )
{
    if ( !SvtSaveOptions().IsWarnAlienFormat() )
        return sal_True;

    Window* pWin = SfxStoringHelper::GetModelWindow( xModel );
    SfxAlienWarningDialog aDlg( pWin, aOldUIName );

    return aDlg.Execute() == RET_OK;
}

Window* SfxStoringHelper::GetModelWindow( const uno::Reference< frame::XModel >& xModel )
{
    Window* pWin = 0;
    try {
        if ( xModel.is() )
        {
            uno::Reference< frame::XController > xController = xModel->getCurrentController();
            if ( xController.is() )
            {
                uno::Reference< frame::XFrame > xFrame = xController->getFrame();
                if ( xFrame.is() )
                {
                    uno::Reference< awt::XWindow > xWindow = xFrame->getContainerWindow();
                    if ( xWindow.is() )
                    {
                        VCLXWindow* pVCLWindow = VCLXWindow::GetImplementation( xWindow );
                        if ( pVCLWindow )
                            pWin = pVCLWindow->GetWindow();
                    }
                }
            }
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return pWin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
