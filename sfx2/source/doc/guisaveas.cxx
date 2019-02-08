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
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
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

#include <guisaveas.hxx>

#include <sal/log.hxx>
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
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/lok.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/asyncfunc.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfxtypes.hxx>
#include <alienwarn.hxx>

#include <sfx2/docmacromode.hxx>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <rtl/ref.hxx>
#include <framework/interaction.hxx>
#include <svtools/sfxecode.hxx>

#include <memory>

#include <com/sun/star/frame/Desktop.hpp>

#include <officecfg/Office/Common.hxx>

#include <vcl/FilterConfigItem.hxx>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <osl/file.hxx>

#ifdef _WIN32
#include <Shlobj.h>
#ifdef GetTempPath
#undef GetTempPath
#endif
#endif

// flags that specify requested operation
#define EXPORT_REQUESTED            1
#define PDFEXPORT_REQUESTED         2
#define PDFDIRECTEXPORT_REQUESTED   4
#define WIDEEXPORT_REQUESTED        8
#define SAVE_REQUESTED              16
#define SAVEAS_REQUESTED            32
#define SAVEACOPY_REQUESTED         64
#define EPUBEXPORT_REQUESTED       128
#define EPUBDIRECTEXPORT_REQUESTED 256
#define SAVEASREMOTE_REQUESTED      -1

// possible statuses of save operation
#define STATUS_NO_ACTION            0
#define STATUS_SAVE                 1
#define STATUS_SAVEAS               2
#define STATUS_SAVEAS_STANDARDNAME  3

const char aFilterNameString[] = "FilterName";
const char aFilterOptionsString[] = "FilterOptions";
const char aFilterDataString[] = "FilterData";

using namespace ::com::sun::star;
using namespace css::system;

namespace {

sal_uInt16 getSlotIDFromMode( sal_Int16 nStoreMode )
{
    // This is a temporary hardcoded solution must be removed when
    // dialogs do not need parameters in SidSet representation any more

    sal_uInt16 nResult = 0;
    if ( nStoreMode == EXPORT_REQUESTED || nStoreMode == ( EXPORT_REQUESTED | SAVEACOPY_REQUESTED | WIDEEXPORT_REQUESTED ) )
        nResult = SID_EXPORTDOC;
    else if ( nStoreMode == ( EXPORT_REQUESTED | PDFEXPORT_REQUESTED ) )
        nResult = SID_EXPORTDOCASPDF;
    else if ( nStoreMode == ( EXPORT_REQUESTED | EPUBEXPORT_REQUESTED ) )
        nResult = SID_EXPORTDOCASEPUB;
    else if ( nStoreMode == ( EXPORT_REQUESTED | PDFEXPORT_REQUESTED | PDFDIRECTEXPORT_REQUESTED ) )
        nResult = SID_DIRECTEXPORTDOCASPDF;
    else if ( nStoreMode == ( EXPORT_REQUESTED | EPUBEXPORT_REQUESTED | EPUBDIRECTEXPORT_REQUESTED ) )
        nResult = SID_DIRECTEXPORTDOCASEPUB;
    else if ( nStoreMode == SAVEAS_REQUESTED || nStoreMode == ( EXPORT_REQUESTED | WIDEEXPORT_REQUESTED ) )
        nResult = SID_SAVEASDOC;
    else if ( nStoreMode == SAVEASREMOTE_REQUESTED )
        nResult = SID_SAVEASREMOTE;
    else {
        SAL_WARN( "sfx.doc", "Unacceptable slot name is provided!" );
    }

    return nResult;
}


sal_Int16 getStoreModeFromSlotName( const OUString& aSlotName )
{
    sal_Int16 nResult = 0;
    if ( aSlotName == "ExportTo" )
        nResult = EXPORT_REQUESTED;
    else if ( aSlotName == "ExportToPDF" )
        nResult = EXPORT_REQUESTED | PDFEXPORT_REQUESTED;
    else if ( aSlotName == "ExportDirectToPDF" )
        nResult = EXPORT_REQUESTED | PDFEXPORT_REQUESTED | PDFDIRECTEXPORT_REQUESTED;
    else if ( aSlotName == "ExportToEPUB" )
        nResult = EXPORT_REQUESTED | EPUBEXPORT_REQUESTED;
    else if ( aSlotName == "ExportDirectToEPUB" )
        nResult = EXPORT_REQUESTED | EPUBEXPORT_REQUESTED | EPUBDIRECTEXPORT_REQUESTED;
    else if ( aSlotName == "Save" )
        nResult = SAVE_REQUESTED;
    else if ( aSlotName == "SaveAs" )
        nResult = SAVEAS_REQUESTED;
    else if ( aSlotName == "SaveAsRemote" )
        nResult = SAVEASREMOTE_REQUESTED;
    else
        throw task::ErrorCodeIOException(
            ("getStoreModeFromSlotName(\"" + aSlotName
             + "): ERRCODE_IO_INVALIDPARAMETER"),
            uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_INVALIDPARAMETER) );

    return nResult;
}


SfxFilterFlags getMustFlags( sal_Int16 nStoreMode )
{
    return ( SfxFilterFlags::EXPORT
            | ( ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) ) ? SfxFilterFlags::NONE : SfxFilterFlags::IMPORT ) );
}


SfxFilterFlags getDontFlags( sal_Int16 nStoreMode )
{
    return ( SfxFilterFlags::INTERNAL
            | SfxFilterFlags::NOTINFILEDLG
            | ( ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) ) ? SfxFilterFlags::IMPORT : SfxFilterFlags::NONE ) );
}


// class DocumentSettingsGuard


class DocumentSettingsGuard
{
    uno::Reference< beans::XPropertySet > m_xDocumentSettings;
    bool m_bPreserveReadOnly;
    bool m_bReadOnlySupported;

    bool const m_bRestoreSettings;
public:
    DocumentSettingsGuard( const uno::Reference< frame::XModel >& xModel, bool bReadOnly, bool bRestore )
    : m_bPreserveReadOnly( false )
    , m_bReadOnlySupported( false )
    , m_bRestoreSettings( bRestore )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xDocSettingsSupplier( xModel, uno::UNO_QUERY_THROW );
            m_xDocumentSettings.set(
                xDocSettingsSupplier->createInstance( "com.sun.star.document.Settings" ),
                uno::UNO_QUERY_THROW );

            OUString aLoadReadonlyString( "LoadReadonly" );

            try
            {
                m_xDocumentSettings->getPropertyValue( aLoadReadonlyString ) >>= m_bPreserveReadOnly;
                m_xDocumentSettings->setPropertyValue( aLoadReadonlyString, uno::makeAny( bReadOnly ) );
                m_bReadOnlySupported = true;
            }
            catch( const uno::Exception& )
            {}
        }
        catch( const uno::Exception& )
        {}

        if ( bReadOnly && !m_bReadOnlySupported )
            throw uno::RuntimeException(); // the user could provide the data, so it must be stored
    }

    ~DocumentSettingsGuard()
    {
        if ( m_bRestoreSettings )
        {
            try
            {
                if ( m_bReadOnlySupported )
                    m_xDocumentSettings->setPropertyValue( "LoadReadonly", uno::makeAny( m_bPreserveReadOnly ) );
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "Unexpected exception!" );
            }
        }
    }
};
} // anonymous namespace


// class ModelData_Impl

class ModelData_Impl
{
    SfxStoringHelper* m_pOwner;
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference< frame::XStorable > m_xStorable;
    uno::Reference< frame::XStorable2 > m_xStorable2;

    OUString m_aModuleName;
    std::unique_ptr<::comphelper::SequenceAsHashMap> m_pDocumentPropsHM;
    std::unique_ptr<::comphelper::SequenceAsHashMap> m_pModulePropsHM;

    ::comphelper::SequenceAsHashMap m_aMediaDescrHM;

    bool m_bRecommendReadOnly;

public:
    ModelData_Impl( SfxStoringHelper& aOwner,
                    const uno::Reference< frame::XModel >& xModel,
                    const uno::Sequence< beans::PropertyValue >& aMediaDescr );

    ~ModelData_Impl();

    void FreeDocumentProps();

    uno::Reference< frame::XModel > const & GetModel();
    uno::Reference< frame::XStorable > const & GetStorable();
    uno::Reference< frame::XStorable2 > const & GetStorable2();

    ::comphelper::SequenceAsHashMap& GetMediaDescr() { return m_aMediaDescrHM; }

    bool IsRecommendReadOnly() const { return m_bRecommendReadOnly; }

    const ::comphelper::SequenceAsHashMap& GetDocProps();

    OUString const & GetModuleName();
    const ::comphelper::SequenceAsHashMap& GetModuleProps();

    void CheckInteractionHandler();


    OUString GetDocServiceName();
    uno::Sequence< beans::PropertyValue > GetDocServiceDefaultFilterCheckFlags( SfxFilterFlags nMust, SfxFilterFlags nDont );
    uno::Sequence< beans::PropertyValue > GetDocServiceAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont );
    uno::Sequence< beans::PropertyValue > GetPreselectedFilter_Impl( sal_Int16 nStoreMode );
    uno::Sequence< beans::PropertyValue > GetDocServiceDefaultFilter();

    bool ExecuteFilterDialog_Impl( const OUString& aFilterName );

    sal_Int8 CheckSaveAcceptable( sal_Int8 nCurStatus );
    sal_Int8 CheckStateForSave();

    sal_Int8 CheckFilter( const OUString& );

    bool CheckFilterOptionsDialogExistence();

    bool OutputFileDialog( sal_Int16 nStoreMode,
                                const ::comphelper::SequenceAsHashMap& aPreselectedFilterPropsHM,
                                bool bSetStandardName,
                                OUString& aSuggestedName,
                                bool bPreselectPassword,
                                OUString& aSuggestedDir,
                                sal_Int16 nDialog,
                                const OUString& rStandardDir,
                                const css::uno::Sequence< OUString >& rBlackList
                                );

    bool ShowDocumentInfoDialog(const std::function< void () >&);

    static OUString GetRecommendedExtension( const OUString& aTypeName );
    OUString GetRecommendedDir( const OUString& aSuggestedDir );
    OUString GetRecommendedName( const OUString& aSuggestedName,
                                        const OUString& aTypeName );

};


ModelData_Impl::ModelData_Impl( SfxStoringHelper& aOwner,
                                const uno::Reference< frame::XModel >& xModel,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescr )
: m_pOwner( &aOwner )
, m_xModel( xModel )
, m_aMediaDescrHM( aMediaDescr )
, m_bRecommendReadOnly( false )
{
    CheckInteractionHandler();
}


ModelData_Impl::~ModelData_Impl()
{
    FreeDocumentProps();
    m_pDocumentPropsHM.reset();
    m_pModulePropsHM.reset();
}


void ModelData_Impl::FreeDocumentProps()
{
    m_pDocumentPropsHM.reset();
}


uno::Reference< frame::XModel > const & ModelData_Impl::GetModel()
{
    if ( !m_xModel.is() )
        throw uno::RuntimeException();

    return m_xModel;
}


uno::Reference< frame::XStorable > const & ModelData_Impl::GetStorable()
{
    if ( !m_xStorable.is() )
    {
        m_xStorable.set( m_xModel, uno::UNO_QUERY_THROW );
    }

    return m_xStorable;
}


uno::Reference< frame::XStorable2 > const & ModelData_Impl::GetStorable2()
{
    if ( !m_xStorable2.is() )
    {
        m_xStorable2.set( m_xModel, uno::UNO_QUERY_THROW );
    }

    return m_xStorable2;
}


const ::comphelper::SequenceAsHashMap& ModelData_Impl::GetDocProps()
{
    if ( !m_pDocumentPropsHM )
        m_pDocumentPropsHM.reset( new ::comphelper::SequenceAsHashMap( GetModel()->getArgs() ) );

    return *m_pDocumentPropsHM;
}


OUString const & ModelData_Impl::GetModuleName()
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


const ::comphelper::SequenceAsHashMap& ModelData_Impl::GetModuleProps()
{
    if ( !m_pModulePropsHM )
    {
        uno::Sequence< beans::PropertyValue > aModuleProps;
        m_pOwner->GetModuleManager()->getByName( GetModuleName() ) >>= aModuleProps;
        if ( !aModuleProps.getLength() )
            throw uno::RuntimeException(); // TODO;
        m_pModulePropsHM.reset( new ::comphelper::SequenceAsHashMap( aModuleProps ) );
    }

    return *m_pModulePropsHM;
}


OUString ModelData_Impl::GetDocServiceName()
{
    return GetModuleProps().getUnpackedValueOrDefault("ooSetupFactoryDocumentService", OUString());
}


void ModelData_Impl::CheckInteractionHandler()
{
    const OUString sInteractionHandler {"InteractionHandler"};
    ::comphelper::SequenceAsHashMap::const_iterator aInteractIter =
            m_aMediaDescrHM.find( sInteractionHandler );

    if ( aInteractIter == m_aMediaDescrHM.end() )
    {
        try {
            m_aMediaDescrHM[ sInteractionHandler ]
                <<= task::InteractionHandler::createWithParent( comphelper::getProcessComponentContext(), nullptr);
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


uno::Sequence< beans::PropertyValue > ModelData_Impl::GetDocServiceDefaultFilter()
{
    uno::Sequence< beans::PropertyValue > aProps;

    const OUString aFilterName = GetModuleProps().getUnpackedValueOrDefault( "ooSetupFactoryDefaultFilter", OUString() );

    m_pOwner->GetFilterConfiguration()->getByName( aFilterName ) >>= aProps;

    return aProps;
}


uno::Sequence< beans::PropertyValue > ModelData_Impl::GetDocServiceDefaultFilterCheckFlags( SfxFilterFlags nMust,
                                                                                            SfxFilterFlags nDont )
{
    uno::Sequence< beans::PropertyValue > aFilterProps;
    uno::Sequence< beans::PropertyValue > aProps = GetDocServiceDefaultFilter();
    if ( aProps.getLength() )
    {
        ::comphelper::SequenceAsHashMap aFiltHM( aProps );
        SfxFilterFlags nFlags = static_cast<SfxFilterFlags>(aFiltHM.getUnpackedValueOrDefault("Flags",
                                                        sal_Int32(0) ));
        if ( ( ( nFlags & nMust ) == nMust ) && !( nFlags & nDont ) )
            aFilterProps = aProps;
    }

    return aFilterProps;
}


uno::Sequence< beans::PropertyValue > ModelData_Impl::GetDocServiceAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    uno::Sequence< beans::NamedValue > aSearchRequest { { "DocumentService", css::uno::makeAny(GetDocServiceName()) } };

    return ::comphelper::MimeConfigurationHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
}


uno::Sequence< beans::PropertyValue > ModelData_Impl::GetPreselectedFilter_Impl( sal_Int16 nStoreMode )
{
    if ( nStoreMode == SAVEASREMOTE_REQUESTED )
        nStoreMode = SAVEAS_REQUESTED;

    uno::Sequence< beans::PropertyValue > aFilterProps;

    SfxFilterFlags nMust = getMustFlags( nStoreMode );
    SfxFilterFlags nDont = getDontFlags( nStoreMode );

    if ( ( nStoreMode != SAVEASREMOTE_REQUESTED ) && ( nStoreMode & PDFEXPORT_REQUESTED ) )
    {
        // Preselect PDF-Filter for EXPORT
        uno::Sequence< beans::NamedValue > aSearchRequest
        {
            { "Type", css::uno::makeAny(OUString("pdf_Portable_Document_Format")) },
            { "DocumentService", css::uno::makeAny(GetDocServiceName()) }
        };

        aFilterProps = ::comphelper::MimeConfigurationHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
    }
    else if ( ( nStoreMode != SAVEASREMOTE_REQUESTED ) && ( nStoreMode & EPUBEXPORT_REQUESTED ) )
    {
        // Preselect EPUB filter for export.
        uno::Sequence<beans::NamedValue> aSearchRequest
        {
            { "Type", css::uno::makeAny(OUString("writer_EPUB_Document")) },
            { "DocumentService", css::uno::makeAny(GetDocServiceName()) }
        };

        aFilterProps = ::comphelper::MimeConfigurationHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
    }
    else
    {
        aFilterProps = GetDocServiceDefaultFilterCheckFlags( nMust, nDont );

        if ( !aFilterProps.getLength() )
        {
            // the default filter was not found, use just the first acceptable one
            aFilterProps = GetDocServiceAnyFilter( nMust, nDont );
        }
    }

    return aFilterProps;
}


bool ModelData_Impl::ExecuteFilterDialog_Impl( const OUString& aFilterName )
{
    bool bDialogUsed = false;

    try {
        uno::Sequence < beans::PropertyValue > aProps;
        uno::Any aAny = m_pOwner->GetFilterConfiguration()->getByName( aFilterName );
        if ( aAny >>= aProps )
        {
            const sal_Int32 nPropertyCount = aProps.getLength();
            for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
            {
                if( aProps[nProperty].Name == "UIComponent" )
                {
                    OUString aServiceName;
                    aProps[nProperty].Value >>= aServiceName;
                    if( !aServiceName.isEmpty() )
                    {
                        uno::Sequence<uno::Any> aDialogArgs(comphelper::InitAnyPropertySequence(
                        {
                            {"ParentWindow", uno::Any(SfxStoringHelper::GetModelXWindow(m_xModel))},
                        }));

                        uno::Reference< ui::dialogs::XExecutableDialog > xFilterDialog(
                                                    comphelper::getProcessServiceFactory()->createInstanceWithArguments(aServiceName, aDialogArgs), uno::UNO_QUERY );
                        uno::Reference< beans::XPropertyAccess > xFilterProperties( xFilterDialog, uno::UNO_QUERY );

                        if( xFilterDialog.is() && xFilterProperties.is() )
                        {
                            bDialogUsed = true;

                            uno::Reference< document::XExporter > xExporter( xFilterDialog, uno::UNO_QUERY );
                            if( xExporter.is() )
                                xExporter->setSourceDocument(
                                    uno::Reference< lang::XComponent >( GetModel(), uno::UNO_QUERY ) );

                            uno::Sequence< beans::PropertyValue > aPropsForDialog;
                            GetMediaDescr() >> aPropsForDialog;
                            xFilterProperties->setPropertyValues( aPropsForDialog );

                            if( !xFilterDialog->execute() )
                            {
                                throw task::ErrorCodeIOException(
                                    ("ModelData_Impl::ExecuteFilterDialog_Impl:"
                                     " ERRCODE_IO_ABORT"),
                                    uno::Reference< uno::XInterface >(),
                                    sal_uInt32(ERRCODE_IO_ABORT));
                            }

                            uno::Sequence< beans::PropertyValue > aPropsFromDialog =
                                                                        xFilterProperties->getPropertyValues();
                            const sal_Int32 nPropsLen {aPropsFromDialog.getLength()};
                            for ( sal_Int32 nInd = 0; nInd < nPropsLen; ++nInd )
                                GetMediaDescr()[aPropsFromDialog[nInd].Name] = aPropsFromDialog[nInd].Value;
                        }
                    }

                    break;
                }
            }
        }
    }
    catch( const container::NoSuchElementException& e )
    {
        // the filter name is unknown
        throw task::ErrorCodeIOException(
            ("ModelData_Impl::ExecuteFilterDialog_Impl: NoSuchElementException"
             " \"" + e.Message + "\": ERRCODE_IO_ABORT"),
            uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_INVALIDPARAMETER));
    }
    catch( const task::ErrorCodeIOException& )
    {
        throw;
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("sfx.doc", "ignoring " << e);
    }

    return bDialogUsed;
}


sal_Int8 ModelData_Impl::CheckSaveAcceptable( sal_Int8 nCurStatus )
{
    sal_Int8 nResult = nCurStatus;

    if ( nResult != STATUS_NO_ACTION && GetStorable()->hasLocation() )
    {
        // the saving is acceptable
        // in case the configuration entry is not set or set to false
        // or in case of version creation
        if ( officecfg::Office::Common::Save::Document::AlwaysSaveAs::get()
          && GetMediaDescr().find( OUString("VersionComment") ) == GetMediaDescr().end() )
        {
            // notify the user that SaveAs is going to be done
            vcl::Window* pWin = SfxStoringHelper::GetModelWindow(m_xModel);
            std::unique_ptr<weld::MessageDialog> xMessageBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                             VclMessageType::Question, VclButtonsType::OkCancel, SfxResId(STR_NEW_FILENAME_SAVE)));
            if (xMessageBox->run() == RET_OK)
                nResult = STATUS_SAVEAS;
            else
                nResult = STATUS_NO_ACTION;
        }
    }

    return nResult;
}


sal_Int8 ModelData_Impl::CheckStateForSave()
{
    // if the document is readonly or a new one a SaveAs operation must be used
    if ( !GetStorable()->hasLocation() || GetStorable()->isReadonly() )
        return STATUS_SAVEAS;

    // check acceptable entries for media descriptor
    ::comphelper::SequenceAsHashMap aAcceptedArgs;

    const OUString aVersionCommentString("VersionComment");
    const OUString aAuthorString("Author");
    const OUString aDontTerminateEdit("DontTerminateEdit");
    const OUString aInteractionHandlerString("InteractionHandler");
    const OUString aStatusIndicatorString("StatusIndicator");
    const OUString aFailOnWarningString("FailOnWarning");
    const OUString aNoFileSync("NoFileSync");

    if ( GetMediaDescr().find( aVersionCommentString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aVersionCommentString ] = GetMediaDescr()[ aVersionCommentString ];
    if ( GetMediaDescr().find( aAuthorString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aAuthorString ] = GetMediaDescr()[ aAuthorString ];
    if ( GetMediaDescr().find( aDontTerminateEdit ) != GetMediaDescr().end() )
        aAcceptedArgs[ aDontTerminateEdit ] = GetMediaDescr()[ aDontTerminateEdit ];
    if ( GetMediaDescr().find( aInteractionHandlerString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aInteractionHandlerString ] = GetMediaDescr()[ aInteractionHandlerString ];
    if ( GetMediaDescr().find( aStatusIndicatorString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aStatusIndicatorString ] = GetMediaDescr()[ aStatusIndicatorString ];
    if ( GetMediaDescr().find( aFailOnWarningString ) != GetMediaDescr().end() )
        aAcceptedArgs[ aFailOnWarningString ] = GetMediaDescr()[ aFailOnWarningString ];
    if (GetMediaDescr().find(aNoFileSync) != GetMediaDescr().end())
        aAcceptedArgs[aNoFileSync] = GetMediaDescr()[aNoFileSync];

    // remove unacceptable entry if there is any
    DBG_ASSERT( GetMediaDescr().size() == aAcceptedArgs.size(),
                "Unacceptable parameters are provided in Save request!\n" );
    if ( GetMediaDescr().size() != aAcceptedArgs.size() )
        GetMediaDescr() = aAcceptedArgs;

    // check that the old filter is acceptable
    return CheckFilter( GetDocProps().getUnpackedValueOrDefault(aFilterNameString, OUString()) );
}

sal_Int8 ModelData_Impl::CheckFilter( const OUString& aFilterName )
{
    ::comphelper::SequenceAsHashMap aFiltPropsHM;
    SfxFilterFlags nFiltFlags = SfxFilterFlags::NONE;
    if ( !aFilterName.isEmpty() )
    {
        // get properties of filter
        uno::Sequence< beans::PropertyValue > aFilterProps;
        m_pOwner->GetFilterConfiguration()->getByName( aFilterName ) >>= aFilterProps;

        aFiltPropsHM = ::comphelper::SequenceAsHashMap( aFilterProps );
        nFiltFlags = static_cast<SfxFilterFlags>(aFiltPropsHM.getUnpackedValueOrDefault("Flags", sal_Int32(0) ));
    }

    // only a temporary solution until default filter retrieving feature is implemented
    // then GetDocServiceDefaultFilter() must be used
    ::comphelper::SequenceAsHashMap aDefFiltPropsHM = GetDocServiceDefaultFilterCheckFlags( SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT, SfxFilterFlags::NONE );
    SfxFilterFlags nDefFiltFlags = static_cast<SfxFilterFlags>(aDefFiltPropsHM.getUnpackedValueOrDefault("Flags", sal_Int32(0) ));

    // if the old filter is not acceptable
    // and there is no default filter or it is not acceptable for requested parameters then proceed with saveAs
    if ( ( aFiltPropsHM.empty() || !( nFiltFlags & SfxFilterFlags::EXPORT ) )
      && ( aDefFiltPropsHM.empty() || !( nDefFiltFlags & SfxFilterFlags::EXPORT ) || nDefFiltFlags & SfxFilterFlags::INTERNAL ) )
        return STATUS_SAVEAS;

    // so at this point there is either an acceptable old filter or default one
    if ( aFiltPropsHM.empty() || !( nFiltFlags & SfxFilterFlags::EXPORT ) )
    {
        // so the default filter must be acceptable
        return STATUS_SAVEAS_STANDARDNAME;
    }
    else if ( ( !( nFiltFlags & SfxFilterFlags::OWN ) || ( nFiltFlags & SfxFilterFlags::ALIEN ) )
           && !aDefFiltPropsHM.empty()
           && ( nDefFiltFlags & SfxFilterFlags::EXPORT ) && !( nDefFiltFlags & SfxFilterFlags::INTERNAL ))
    {
        // the default filter is acceptable and the old filter is alien one
        // so ask to make a saveAs operation
        const OUString aUIName = aFiltPropsHM.getUnpackedValueOrDefault("UIName", OUString() );
        const OUString aDefUIName = aDefFiltPropsHM.getUnpackedValueOrDefault("UIName", OUString() );
        const OUString aPreusedFilterName = GetDocProps().getUnpackedValueOrDefault("PreusedFilterName", OUString() );
        const OUString aDefType = aDefFiltPropsHM.getUnpackedValueOrDefault( "Type", OUString() );
        const OUString aDefExtension = GetRecommendedExtension( aDefType );

        if ( aPreusedFilterName != aFilterName && aUIName != aDefUIName )
        {
            if ( !SfxStoringHelper::WarnUnacceptableFormat( GetModel(), aUIName, aDefExtension,
                                                            static_cast<bool>( nDefFiltFlags & SfxFilterFlags::ALIEN ) ) )
                return STATUS_SAVEAS_STANDARDNAME;
        }
    }

    return STATUS_SAVE;
}


bool ModelData_Impl::CheckFilterOptionsDialogExistence()
{
    uno::Sequence< beans::NamedValue > aSearchRequest { { "DocumentService", css::uno::makeAny(GetDocServiceName()) } };

    uno::Reference< container::XEnumeration > xFilterEnum =
                                    m_pOwner->GetFilterQuery()->createSubSetEnumerationByProperties( aSearchRequest );

    while ( xFilterEnum->hasMoreElements() )
    {
        uno::Sequence< beans::PropertyValue > aProps;
        if ( xFilterEnum->nextElement() >>= aProps )
        {
            ::comphelper::SequenceAsHashMap aPropsHM( aProps );
            if ( !aPropsHM.getUnpackedValueOrDefault("UIComponent", OUString()).isEmpty() )
                return true;
        }
    }

    return false;
}


bool ModelData_Impl::OutputFileDialog( sal_Int16 nStoreMode,
                                            const ::comphelper::SequenceAsHashMap& aPreselectedFilterPropsHM,
                                            bool bSetStandardName,
                                            OUString& aSuggestedName,
                                            bool bPreselectPassword,
                                            OUString& aSuggestedDir,
                                            sal_Int16 nDialog,
                                            const OUString& rStandardDir,
                                            const css::uno::Sequence< OUString >& rBlackList)
{
    if ( nStoreMode == SAVEASREMOTE_REQUESTED )
        nStoreMode = SAVEAS_REQUESTED;

    bool bUseFilterOptions = false;

    ::comphelper::SequenceAsHashMap::const_iterator aOverwriteIter =
                GetMediaDescr().find( OUString("Overwrite") );

    // the file name must be specified if overwrite option is set
    if ( aOverwriteIter != GetMediaDescr().end() )
           throw task::ErrorCodeIOException(
               "ModelData_Impl::OutputFileDialog: ERRCODE_IO_INVALIDPARAMETER",
               uno::Reference< uno::XInterface >(),
               sal_uInt32(ERRCODE_IO_INVALIDPARAMETER));

    // no target file name is specified
    // we need to show the file dialog

    // check if we have a filter which allows for filter options, so we need a corresponding checkbox in the dialog
    bool bAllowOptions = false;

    // in case of Export, filter options dialog is used if available
    if( !( nStoreMode & EXPORT_REQUESTED ) || ( nStoreMode & WIDEEXPORT_REQUESTED ) )
        bAllowOptions = CheckFilterOptionsDialogExistence();

    // get the filename by dialog ...
    // create the file dialog
    sal_Int16  aDialogMode = bAllowOptions
        ? css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS
        : css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD;
    FileDialogFlags aDialogFlags = FileDialogFlags::NONE;

    if( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        if ( (nStoreMode & PDFEXPORT_REQUESTED) || (nStoreMode & EPUBEXPORT_REQUESTED) )
            aDialogMode = css::ui::dialogs::TemplateDescription::
                FILESAVE_AUTOEXTENSION;
        else
            aDialogMode = css::ui::dialogs::TemplateDescription::
                FILESAVE_AUTOEXTENSION_SELECTION;
        aDialogFlags = FileDialogFlags::Export;
    }

    if( ( nStoreMode & EXPORT_REQUESTED ) && ( nStoreMode & SAVEACOPY_REQUESTED ) && ( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        aDialogFlags = FileDialogFlags::SaveACopy;
    }

    std::unique_ptr<sfx2::FileDialogHelper> pFileDlg;

    const OUString aDocServiceName {GetDocServiceName()};
    DBG_ASSERT( !aDocServiceName.isEmpty(), "No document service for this module set!" );

    SfxFilterFlags nMust = getMustFlags( nStoreMode );
    SfxFilterFlags nDont = getDontFlags( nStoreMode );
    vcl::Window* pWin = SfxStoringHelper::GetModelWindow( m_xModel );
    weld::Window* pFrameWin = pWin ? pWin->GetFrameWeld() : nullptr;
    if ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        if ( ( nStoreMode & PDFEXPORT_REQUESTED ) && !aPreselectedFilterPropsHM.empty() )
        {
            // this is a PDF export
            // the filter options has been shown already
            const OUString aFilterUIName = aPreselectedFilterPropsHM.getUnpackedValueOrDefault( "UIName", OUString() );
            pFileDlg.reset(new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aFilterUIName, "pdf", rStandardDir, rBlackList, pFrameWin ));
            pFileDlg->SetCurrentFilter( aFilterUIName );
        }
        else if ((nStoreMode & EPUBEXPORT_REQUESTED) && !aPreselectedFilterPropsHM.empty())
        {
            // This is an EPUB export, the filter options has been shown already.
            const OUString aFilterUIName = aPreselectedFilterPropsHM.getUnpackedValueOrDefault( "UIName", OUString() );
            pFileDlg.reset(new sfx2::FileDialogHelper(aDialogMode, aDialogFlags, aFilterUIName, "epub", rStandardDir, rBlackList, pFrameWin));
            pFileDlg->SetCurrentFilter(aFilterUIName);
        }
        else
        {
            // This is the normal dialog
            pFileDlg.reset(new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aDocServiceName, nDialog, nMust, nDont, rStandardDir, rBlackList, pFrameWin ));
        }

        sfx2::FileDialogHelper::Context eCtxt = sfx2::FileDialogHelper::UNKNOWN_CONTEXT;
        if ( aDocServiceName == "com.sun.star.drawing.DrawingDocument" )
            eCtxt = sfx2::FileDialogHelper::SD_EXPORT;
        else if ( aDocServiceName == "com.sun.star.presentation.PresentationDocument" )
            eCtxt = sfx2::FileDialogHelper::SI_EXPORT;
        else if ( aDocServiceName == "com.sun.star.text.TextDocument" )
            eCtxt = sfx2::FileDialogHelper::SW_EXPORT;

        if ( eCtxt != sfx2::FileDialogHelper::UNKNOWN_CONTEXT )
               pFileDlg->SetContext( eCtxt );

        pFileDlg->CreateMatcher( aDocServiceName );

        uno::Reference< ui::dialogs::XFilePicker3 > xFilePicker = pFileDlg->GetFilePicker();
        uno::Reference< ui::dialogs::XFilePickerControlAccess > xControlAccess =
        uno::Reference< ui::dialogs::XFilePickerControlAccess >( xFilePicker, uno::UNO_QUERY );

        if ( xControlAccess.is() )
        {
            xControlAccess->setLabel( ui::dialogs::CommonFilePickerElementIds::PUSHBUTTON_OK, SfxResId(STR_EXPORTBUTTON) );
            xControlAccess->setLabel( ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER_LABEL, SfxResId(STR_LABEL_FILEFORMAT) );
        }
    }
    else
    {
        // This is the normal dialog
        pFileDlg.reset(new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aDocServiceName, nDialog,
            nMust, nDont, rStandardDir, rBlackList, pFrameWin ));
        pFileDlg->CreateMatcher( aDocServiceName );
    }

    OUString aAdjustToType;

    const OUString sFilterNameString(aFilterNameString);

    if ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        // it is export, set the preselected filter
        pFileDlg->SetCurrentFilter( aPreselectedFilterPropsHM.getUnpackedValueOrDefault( "UIName", OUString() ) );
        aAdjustToType = aPreselectedFilterPropsHM.getUnpackedValueOrDefault( "Type", OUString() );
    }
    // it is no export, bSetStandardName == true means that user agreed to store document in the default (default default ;-)) format
    else if ( bSetStandardName || GetStorable()->hasLocation() )
    {
        uno::Sequence< beans::PropertyValue > aOldFilterProps;
        const OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault( sFilterNameString, OUString() );

        if ( !aOldFilterName.isEmpty() )
            m_pOwner->GetFilterConfiguration()->getByName( aOldFilterName ) >>= aOldFilterProps;

        ::comphelper::SequenceAsHashMap aOldFiltPropsHM( aOldFilterProps );
        SfxFilterFlags nOldFiltFlags = static_cast<SfxFilterFlags>(aOldFiltPropsHM.getUnpackedValueOrDefault("Flags", sal_Int32(0) ));

        if ( bSetStandardName || ( nOldFiltFlags & nMust ) != nMust || bool(nOldFiltFlags & nDont) )
        {
            // the suggested type will be changed, the extension should be adjusted
            aAdjustToType = aPreselectedFilterPropsHM.getUnpackedValueOrDefault( "Type", OUString() );
            pFileDlg->SetCurrentFilter( aPreselectedFilterPropsHM.getUnpackedValueOrDefault( "UIName", OUString() ) );
        }
        else
        {
            pFileDlg->SetCurrentFilter( aOldFiltPropsHM.getUnpackedValueOrDefault(
                                                        "UIName",
                                                        OUString() ) );
        }
    }

    const OUString aRecommendedDir {GetRecommendedDir( aSuggestedDir )};
    if ( !aRecommendedDir.isEmpty() )
        pFileDlg->SetDisplayFolder( aRecommendedDir );
    const OUString aRecommendedName {GetRecommendedName( aSuggestedName, aAdjustToType )};
    if ( !aRecommendedName.isEmpty() )
        pFileDlg->SetFileName( aRecommendedName );

    uno::Reference < view::XSelectionSupplier > xSel( GetModel()->getCurrentController(), uno::UNO_QUERY );
    if ( xSel.is() && xSel->getSelection().hasValue() )
        GetMediaDescr()[OUString("SelectionOnly")] <<= true;

    // This is a temporary hardcoded solution must be removed when
    // dialogs do not need parameters in SidSet representation any more
    sal_uInt16 nSlotID = getSlotIDFromMode( nStoreMode );
    if ( !nSlotID )
        throw lang::IllegalArgumentException(); // TODO:

    // generate SidSet from MediaDescriptor and provide it into FileDialog
    // than merge changed SidSet back
    std::unique_ptr<SfxItemSet> pDialogParams(new SfxAllItemSet( SfxGetpApp()->GetPool() ));
    TransformParameters( nSlotID,
                         GetMediaDescr().getAsConstPropertyValueList(),
                         static_cast<SfxAllItemSet&>(*pDialogParams) );

    const SfxPoolItem* pItem = nullptr;
    if ( bPreselectPassword && pDialogParams->GetItemState( SID_ENCRYPTIONDATA, true, &pItem ) != SfxItemState::SET )
    {
        // the file dialog preselects the password checkbox if the provided mediadescriptor has encryption data entry
        // after dialog execution the password interaction flag will be either removed or not
        pDialogParams->Put( SfxBoolItem( SID_PASSWORDINTERACTION, true ) );
    }

    // aFilterName is a pure output parameter, pDialogParams is an in/out parameter
    OUString aFilterName;
    if ( pFileDlg->Execute( pDialogParams, aFilterName ) != ERRCODE_NONE )
    {
        throw task::ErrorCodeIOException(
            "ModelData_Impl::OutputFileDialog: ERRCODE_IO_ABORT",
            uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_ABORT));
    }

    // the following two arguments can not be converted in MediaDescriptor,
    // so they should be removed from the ItemSet after retrieving
    const SfxBoolItem* pRecommendReadOnly = SfxItemSet::GetItem<SfxBoolItem>(pDialogParams.get(), SID_RECOMMENDREADONLY, false);
    m_bRecommendReadOnly = ( pRecommendReadOnly && pRecommendReadOnly->GetValue() );
    pDialogParams->ClearItem( SID_RECOMMENDREADONLY );

    uno::Sequence< beans::PropertyValue > aPropsFromDialog;
    TransformItems( nSlotID, *pDialogParams, aPropsFromDialog );
    GetMediaDescr() << aPropsFromDialog;

    // get the path from the dialog
    INetURLObject aURL( pFileDlg->GetPath() );
    // the path should be provided outside since it might be used for further calls to the dialog
    aSuggestedName = aURL.GetName( INetURLObject::DecodeMechanism::WithCharset );
    aSuggestedDir = pFileDlg->GetDisplayDirectory();

    // old filter options should be cleared in case different filter is used

    const OUString aFilterFromMediaDescr = GetMediaDescr().getUnpackedValueOrDefault( sFilterNameString, OUString() );
    const OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault( sFilterNameString, OUString() );

    const OUString sFilterOptionsString(aFilterOptionsString);
    const OUString sFilterDataString(aFilterDataString);

    if ( aFilterName == aFilterFromMediaDescr )
    {
        // preserve current settings if any
        // if there no current settings and the name is the same
        // as old filter name use old filter settings

        if ( aFilterFromMediaDescr == aOldFilterName )
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

        if ( aFilterName == aOldFilterName )
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
        if ( SfxStoringHelper::CheckFilterOptionsAppearance( m_pOwner->GetFilterConfiguration(), aFilterName ) )
            bUseFilterOptions = true;

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
    GetMediaDescr()[OUString("URL")] <<= aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    GetMediaDescr()[sFilterNameString] <<= aFilterName;

    return bUseFilterOptions;
}


bool ModelData_Impl::ShowDocumentInfoDialog(const std::function< void () >& aFunc)
{
    bool bDialogUsed = false;

    try {
        uno::Reference< frame::XController > xController = GetModel()->getCurrentController();
        if ( xController.is() )
        {
            uno::Reference< frame::XDispatchProvider > xFrameDispatch( xController->getFrame(), uno::UNO_QUERY );
            if ( xFrameDispatch.is() )
            {
                util::URL aURL;
                aURL.Complete = ".uno:SetDocumentProperties";

                uno::Reference < util::XURLTransformer > xTransformer( util::URLTransformer::create( comphelper::getProcessComponentContext() ) );
                if ( xTransformer->parseStrict( aURL ) )
                {
                    uno::Reference< frame::XDispatch > xDispatch = xFrameDispatch->queryDispatch(
                                                                                aURL,
                                                                                "_self",
                                                                                0 );
                    if ( xDispatch.is() )
                    {
                        uno::Sequence< beans::PropertyValue > aProperties(1);
                        uno::Reference< lang::XUnoTunnel > aAsyncFunc(new AsyncFunc(aFunc));
                        aProperties[0].Name = "AsyncFunc";
                        aProperties[0].Value <<= aAsyncFunc;
                        xDispatch->dispatch( aURL, aProperties );
                        bDialogUsed = true;
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


OUString ModelData_Impl::GetRecommendedExtension( const OUString& aTypeName )
{
    if ( aTypeName.isEmpty() )
       return OUString();

    uno::Reference< container::XNameAccess > xTypeDetection(
       comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.TypeDetection"),
       uno::UNO_QUERY );
    if ( xTypeDetection.is() )
    {
       uno::Sequence< beans::PropertyValue > aTypeNameProps;
       if ( ( xTypeDetection->getByName( aTypeName ) >>= aTypeNameProps ) && aTypeNameProps.getLength() )
       {
           ::comphelper::SequenceAsHashMap aTypeNamePropsHM( aTypeNameProps );
           uno::Sequence< OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                           "Extensions",
                                           ::uno::Sequence< OUString >() );
           if ( aExtensions.getLength() )
               return aExtensions[0];
       }
    }

    return OUString();
}


OUString ModelData_Impl::GetRecommendedDir( const OUString& aSuggestedDir )
{
    if ( ( !aSuggestedDir.isEmpty() || GetStorable()->hasLocation() )
      && !GetMediaDescr().getUnpackedValueOrDefault("RepairPackage", false ) )
    {
        INetURLObject aLocation;
        if ( !aSuggestedDir.isEmpty() )
            aLocation = INetURLObject( aSuggestedDir );
        else
        {
            const OUString aOldURL = GetStorable()->getLocation();
            if ( !aOldURL.isEmpty() )
            {
                INetURLObject aTmp( aOldURL );
                if ( aTmp.removeSegment() )
                    aLocation = aTmp;
            }

            if ( aLocation.HasError() )
                aLocation = INetURLObject( SvtPathOptions().GetWorkPath() );
        }

        OUString sLocationURL( aLocation.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        bool bIsInTempPath( false );
        OUString sSysTempPath;
        if( osl::FileBase::getTempDirURL( sSysTempPath ) == osl::FileBase::E_None )
            bIsInTempPath = !sSysTempPath.isEmpty() && sLocationURL.startsWith( sSysTempPath );
#ifdef _WIN32
        if( !bIsInTempPath )
        {
            wchar_t sPath[MAX_PATH+1];
            HRESULT hRes = SHGetFolderPathW( nullptr, CSIDL_INTERNET_CACHE, nullptr, SHGFP_TYPE_CURRENT, sPath );
            if( SUCCEEDED(hRes) )
            {
                OUString sTempINetFiles;
                if( osl::FileBase::getFileURLFromSystemPath(o3tl::toU(sPath), sTempINetFiles) == osl::FileBase::E_None )
                    bIsInTempPath = !sTempINetFiles.isEmpty() && sLocationURL.startsWith( sTempINetFiles );
            }
        }
#endif
        // Suggest somewhere other than the system's temp directory
        if( bIsInTempPath )
            aLocation = INetURLObject( SvtPathOptions().GetWorkPath() );

        aLocation.setFinalSlash();
        if ( !aLocation.HasError() )
            return aLocation.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        return OUString();
    }

    return INetURLObject( SvtPathOptions().GetWorkPath() ).GetMainURL( INetURLObject::DecodeMechanism::NONE );
}


OUString ModelData_Impl::GetRecommendedName( const OUString& aSuggestedName, const OUString& aTypeName )
{
    // the last used name might be provided by aSuggestedName from the old selection, or from the MediaDescriptor
    if ( !aSuggestedName.isEmpty() )
        return aSuggestedName;

    OUString aRecommendedName {INetURLObject( GetStorable()->getLocation() ).GetName( INetURLObject::DecodeMechanism::WithCharset )};
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
        uno::Reference< container::XNameAccess > xTypeDetection(
            comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.TypeDetection"),
            uno::UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            INetURLObject aObj( "c:/" + aRecommendedName, INetProtocol::File,
                    INetURLObject::EncodeMechanism::All, RTL_TEXTENCODING_UTF8, FSysStyle::Dos );

            const OUString aExtension = GetRecommendedExtension( aTypeName );
            if ( !aExtension.isEmpty() )
                aObj.SetExtension( aExtension );

            aRecommendedName = aObj.GetName( INetURLObject::DecodeMechanism::WithCharset );
        }
    }

    return aRecommendedName;
}


// class SfxStoringHelper


SfxStoringHelper::SfxStoringHelper()
{
}


uno::Reference< container::XNameAccess > const & SfxStoringHelper::GetFilterConfiguration()
{
    if ( !m_xFilterCFG.is() )
    {
        m_xFilterCFG.set( comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.FilterFactory"),
                          uno::UNO_QUERY_THROW );
    }

    return m_xFilterCFG;
}


uno::Reference< container::XContainerQuery > const & SfxStoringHelper::GetFilterQuery()
{
    if ( !m_xFilterQuery.is() )
    {
        m_xFilterQuery.set( GetFilterConfiguration(), uno::UNO_QUERY_THROW );
    }

    return m_xFilterQuery;
}


uno::Reference< css::frame::XModuleManager2 > const & SfxStoringHelper::GetModuleManager()
{
    if ( !m_xModuleManager.is() )
    {
        m_xModuleManager = frame::ModuleManager::create(
            comphelper::getProcessComponentContext() );
    }

    return m_xModuleManager;
}


bool SfxStoringHelper::GUIStoreModel( const uno::Reference< frame::XModel >& xModel,
                                            const OUString& aSlotName,
                                            uno::Sequence< beans::PropertyValue >& aArgsSequence,
                                            bool bPreselectPassword,
                                            SignatureState nDocumentSignatureState )
{
    ModelData_Impl aModelData( *this, xModel, aArgsSequence );

    bool bDialogUsed = false;

    INetURLObject aURL;

    bool bSetStandardName = false; // can be set only for SaveAs

    // parse the slot name
    bool bRemote = false;
    sal_Int16 nStoreMode = getStoreModeFromSlotName( aSlotName );

    if ( nStoreMode == SAVEASREMOTE_REQUESTED )
    {
        nStoreMode = SAVEAS_REQUESTED;
        bRemote = true;
    }

    sal_Int8 nStatusSave = STATUS_NO_ACTION;

    ::comphelper::SequenceAsHashMap::const_iterator aSaveACopyIter =
                        aModelData.GetMediaDescr().find( OUString("SaveACopy") );
    if ( aSaveACopyIter != aModelData.GetMediaDescr().end() )
    {
        bool bSaveACopy = false;
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
            bool bWideExport = false;
            aSaveToIter->second >>= bWideExport;
            if ( bWideExport )
                nStoreMode = EXPORT_REQUESTED | WIDEEXPORT_REQUESTED;
        }

        // if saving is not acceptable the warning must be shown even in case of SaveAs operation
        if ( ( nStoreMode & SAVEAS_REQUESTED ) && aModelData.CheckSaveAcceptable( STATUS_SAVEAS ) == STATUS_NO_ACTION )
            throw task::ErrorCodeIOException(
                "SfxStoringHelper::GUIStoreModel: ERRCODE_IO_ABORT",
                uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_ABORT));
    }
    else if ( nStoreMode & SAVE_REQUESTED )
    {
        // if saving is not acceptable by the configuration the warning must be shown
        nStatusSave = aModelData.CheckSaveAcceptable( STATUS_SAVE );

        if ( nStatusSave == STATUS_NO_ACTION )
            throw task::ErrorCodeIOException(
                "SfxStoringHelper::GUIStoreModel: ERRCODE_IO_ABORT",
                uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_ABORT));
        else if ( nStatusSave == STATUS_SAVE )
        {
            // check whether it is possible to use save operation
            nStatusSave = aModelData.CheckStateForSave();
        }

        if ( nStatusSave == STATUS_NO_ACTION )
        {
            throw task::ErrorCodeIOException(
                "SfxStoringHelper::GUIStoreModel: ERRCODE_IO_ABORT",
                uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_ABORT));
        }
        else if ( nStatusSave != STATUS_SAVE )
        {
            // this should be a usual SaveAs operation
            nStoreMode = SAVEAS_REQUESTED;
            if ( nStatusSave == STATUS_SAVEAS_STANDARDNAME )
                bSetStandardName = true;
        }
    }

    if (!comphelper::LibreOfficeKit::isActive() && !( nStoreMode & EXPORT_REQUESTED ) )
    {
        // if it is no export, warn user that the signature will be removed
        if (  SignatureState::OK == nDocumentSignatureState
           || SignatureState::INVALID == nDocumentSignatureState
           || SignatureState::NOTVALIDATED == nDocumentSignatureState
           || SignatureState::PARTIAL_OK == nDocumentSignatureState)
        {
            vcl::Window* pWin = SfxStoringHelper::GetModelWindow( xModel );
            std::unique_ptr<weld::MessageDialog> xMessageBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                             VclMessageType::Question, VclButtonsType::YesNo, SfxResId(RID_SVXSTR_XMLSEC_QUERY_LOSINGSIGNATURE)));
            if (xMessageBox->run() != RET_YES)
            {
                // the user has decided not to store the document
                throw task::ErrorCodeIOException(
                    "SfxStoringHelper::GUIStoreModel: ERRCODE_IO_ABORT (Preserve Signature)",
                    uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_ABORT));
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
                SAL_WARN("sfx.doc", "Ignoring parameters! "
                    "ModelData considers this illegal:  " << e);
                aModelData.GetStorable()->store();
            }
        }
        else
        {
            OSL_FAIL( "XStorable2 is not supported by the model!" );
            aModelData.GetStorable()->store();
        }

        return false;
    }

    // preselect a filter for the storing process
    uno::Sequence< beans::PropertyValue > aFilterProps = aModelData.GetPreselectedFilter_Impl( nStoreMode );

    DBG_ASSERT( aFilterProps.getLength(), "No filter for storing!\n" );
    if ( !aFilterProps.getLength() )
        throw task::ErrorCodeIOException(
            "SfxStoringHelper::GUIStoreModel: ERRCODE_IO_INVALIDPARAMETER",
            uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_INVALIDPARAMETER));

    ::comphelper::SequenceAsHashMap aFilterPropsHM( aFilterProps );
    OUString aFilterName = aFilterPropsHM.getUnpackedValueOrDefault( "Name", OUString() );

    const OUString sFilterNameString(aFilterNameString);

    const OUString aFilterFromMediaDescr = aModelData.GetMediaDescr().getUnpackedValueOrDefault( sFilterNameString, OUString() );
    const OUString aOldFilterName = aModelData.GetDocProps().getUnpackedValueOrDefault( sFilterNameString, OUString() );

    bool bUseFilterOptions = false;
    ::comphelper::SequenceAsHashMap::const_iterator aFileNameIter = aModelData.GetMediaDescr().find( OUString("URL") );

    const OUString sFilterOptionsString(aFilterOptionsString);
    const OUString sFilterDataString(aFilterDataString);
    const OUString sFilterFlagsString("FilterFlags");

    bool bPDFOptions = (nStoreMode & PDFEXPORT_REQUESTED) && !(nStoreMode & PDFDIRECTEXPORT_REQUESTED);
    bool bEPUBOptions = (nStoreMode & EPUBEXPORT_REQUESTED) && !(nStoreMode & EPUBDIRECTEXPORT_REQUESTED);
    if ( ( nStoreMode & EXPORT_REQUESTED ) && (bPDFOptions || bEPUBOptions) )
    {
        // this is PDF or EPUB export, the filter options dialog should be shown before the export
        aModelData.GetMediaDescr()[sFilterNameString] <<= aFilterName;
        if ( aModelData.GetMediaDescr().find( sFilterFlagsString ) == aModelData.GetMediaDescr().end()
          && aModelData.GetMediaDescr().find( sFilterOptionsString ) == aModelData.GetMediaDescr().end()
          && aModelData.GetMediaDescr().find( sFilterDataString ) == aModelData.GetMediaDescr().end() )
        {
            // execute filter options dialog since no options are set in the media descriptor
            if ( aModelData.ExecuteFilterDialog_Impl( aFilterName ) )
                bDialogUsed = true;
        }
    }

    if ( aFileNameIter == aModelData.GetMediaDescr().end() )
    {
        sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG;

        if( bRemote )
        {
            nDialog = SFX2_IMPL_DIALOG_REMOTE;
        }
        else
        {
            ::comphelper::SequenceAsHashMap::const_iterator aDlgIter =
                aModelData.GetMediaDescr().find( OUString("UseSystemDialog") );
            if ( aDlgIter != aModelData.GetMediaDescr().end() )
            {
                bool bUseSystemDialog = true;
                if ( aDlgIter->second >>= bUseSystemDialog )
                {
                    if ( bUseSystemDialog )
                        nDialog = SFX2_IMPL_DIALOG_SYSTEM;
                    else
                        nDialog = SFX2_IMPL_DIALOG_OOO;
                }
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

        OUString aSuggestedName = aModelData.GetMediaDescr().getUnpackedValueOrDefault("SuggestedSaveAsName", OUString() );
        if ( aSuggestedName.isEmpty() )
            aSuggestedName = aModelData.GetDocProps().getUnpackedValueOrDefault("SuggestedSaveAsName", OUString() );

        OUString sStandardDir;
        ::comphelper::SequenceAsHashMap::const_iterator aStdDirIter =
            aModelData.GetMediaDescr().find( OUString("StandardDir") );
        if ( aStdDirIter != aModelData.GetMediaDescr().end() )
            aStdDirIter->second >>= sStandardDir;

        css::uno::Sequence< OUString >  aBlackList;

        ::comphelper::SequenceAsHashMap::const_iterator aBlackListIter =
            aModelData.GetMediaDescr().find( OUString("BlackList") );
        if ( aBlackListIter != aModelData.GetMediaDescr().end() )
            aBlackListIter->second >>= aBlackList;

        for (;;)
        {
            // in case the dialog is opened a second time the folder should be the same as previously navigated to by the user, not what was handed over by initial parameters
            bUseFilterOptions = aModelData.OutputFileDialog( nStoreMode, aFilterProps, bSetStandardName, aSuggestedName, bPreselectPassword, aSuggestedDir, nDialog, sStandardDir, aBlackList );
            if ( nStoreMode == SAVEAS_REQUESTED )
            {
                // in case of saving check filter for possible alien warning
                const OUString aSelFilterName = aModelData.GetMediaDescr().getUnpackedValueOrDefault( sFilterNameString, OUString() );
                sal_Int8 nStatusFilterSave = aModelData.CheckFilter( aSelFilterName );
                if ( nStatusFilterSave == STATUS_SAVEAS_STANDARDNAME )
                {
                    // switch to best filter
                    bSetStandardName = true;
                }
                else if ( nStatusFilterSave == STATUS_SAVE )
                {
                    // user confirmed alien filter or "good" filter is used
                    break;
                }
            }
            else
                break;
        }

        bDialogUsed = true;
        aFileNameIter = aModelData.GetMediaDescr().find( OUString("URL") );
    }
    else
    {
        // the target file name is provided so check if new filter options
        // are provided or old options can be used
        if ( aFilterFromMediaDescr == aOldFilterName )
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
        DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "Illegal URL!" );

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
        SAL_WARN( "sfx.doc", "This code must be unreachable!" );
        throw task::ErrorCodeIOException(
            "SfxStoringHelper::GUIStoreModel: ERRCODE_IO_INVALIDPARAMETER",
            uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_INVALIDPARAMETER));
    }

    ::comphelper::SequenceAsHashMap::const_iterator aIter =
                            aModelData.GetMediaDescr().find( OUString("FilterFlags") );
    bool bFilterFlagsSet = ( aIter != aModelData.GetMediaDescr().end() );

    if( !( nStoreMode & PDFEXPORT_REQUESTED ) && !( nStoreMode & EPUBEXPORT_REQUESTED ) && !bFilterFlagsSet
        && ( ( nStoreMode & EXPORT_REQUESTED ) || bUseFilterOptions ) )
    {
        // execute filter options dialog
        if ( aModelData.ExecuteFilterDialog_Impl( aFilterName ) )
            bDialogUsed = true;
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
        // this is definitely not a Save operation
        // so the document info can be updated

        // on export document info must be preserved
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            aModelData.GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<util::XCloneable> xCloneable(
            xDPS->getDocumentProperties(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xOldDocProps(
            xCloneable->createClone(), uno::UNO_QUERY_THROW);

        std::function< void () > aFunc = [xModel, xOldDocProps, nStoreMode, aURL, aArgsSequence]() {
            SfxStoringHelper aStoringHelper;
            ModelData_Impl aModel(aStoringHelper, xModel, aArgsSequence );

            try
            {
                if ( nStoreMode & EXPORT_REQUESTED )
                    aModel.GetStorable()->storeToURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aArgsSequence );
                else
                    aModel.GetStorable()->storeAsURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aArgsSequence );
            }
            catch( const uno::Exception& )
            {
            }

            if ( nStoreMode & EXPORT_REQUESTED )
            {
                SfxStoringHelper::SetDocInfoState(aModel.GetModel(), xOldDocProps);
            }
        };

        // use dispatch API to show document info dialog
        if ( aModelData.ShowDocumentInfoDialog(aFunc) )
            bDialogUsed = true;
        else
        {
            OSL_FAIL( "Can't execute document info dialog!" );
        }
    }
    else
    {
        // Document properties can contain streams that should be freed before storing
        aModelData.FreeDocumentProps();

        // this is actually a save operation with different parameters
        // so storeTo or storeAs without DocInfo operations are used
        if ( nStoreMode & EXPORT_REQUESTED )
            aModelData.GetStorable()->storeToURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aArgsSequence );
        else
            aModelData.GetStorable()->storeAsURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aArgsSequence );
    }

    // Launch PDF viewer
    if ( nStoreMode & PDFEXPORT_REQUESTED )
    {
        FilterConfigItem aItem( "Office.Common/Filter/PDF/Export/" );
        bool aViewPDF = aItem.ReadBool( "ViewPDFAfterExport", false );

        if ( aViewPDF )
        {
            uno::Reference<XSystemShellExecute> xSystemShellExecute(SystemShellExecute::create( ::comphelper::getProcessComponentContext() ) );
            xSystemShellExecute->execute( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), "", SystemShellExecuteFlags::URIS_ONLY );
        }
    }

    return bDialogUsed;
}


// static
bool SfxStoringHelper::CheckFilterOptionsAppearance(
                                                    const uno::Reference< container::XNameAccess >& xFilterCFG,
                                                    const OUString& aFilterName )
{
    bool bUseFilterOptions = false;

    DBG_ASSERT( xFilterCFG.is(), "No filter configuration!\n" );
    if( xFilterCFG.is() )
    {
        try {
            uno::Sequence < beans::PropertyValue > aProps;
            uno::Any aAny = xFilterCFG->getByName( aFilterName );
            if ( aAny >>= aProps )
            {
                ::comphelper::SequenceAsHashMap aPropsHM( aProps );
                if( !aPropsHM.getUnpackedValueOrDefault( "UIComponent", OUString() ).isEmpty() )
                    bUseFilterOptions = true;
            }
        }
        catch( const uno::Exception& )
        {
        }
    }

    return bUseFilterOptions;
}


// static
void SfxStoringHelper::SetDocInfoState(
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< document::XDocumentProperties>& i_xOldDocProps )
{
    uno::Reference<document::XDocumentPropertiesSupplier> const
        xModelDocPropsSupplier(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> const xDocPropsToFill =
        xModelDocPropsSupplier->getDocumentProperties();
    uno::Reference< beans::XPropertySet > const xPropSet(
            i_xOldDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);

    uno::Reference< util::XModifiable > xModifiable( xModel, uno::UNO_QUERY );
    if ( !xModifiable.is() )
        throw uno::RuntimeException();

    bool bIsModified = xModifiable->isModified();

    try
    {
        uno::Reference< beans::XPropertySet > const xSet(
                xDocPropsToFill->getUserDefinedProperties(), uno::UNO_QUERY);
        uno::Reference< beans::XPropertyContainer > xContainer( xSet, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();
        uno::Sequence< beans::Property > lProps = xSetInfo->getProperties();
        const beans::Property* pProps = lProps.getConstArray();
        const sal_Int32 nPropLen = lProps.getLength();
        for (sal_Int32 i=0; i<nPropLen; ++i)
        {
            uno::Any aValue = xPropSet->getPropertyValue( pProps[i].Name );
            if ( pProps[i].Attributes & css::beans::PropertyAttribute::REMOVABLE )
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

        // sigh... have to set these manually I'm afraid... wonder why
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
        SAL_INFO("sfx.doc", "SetDocInfoState: caught " << e);
    }

    // set the modified flag back if required
    if ( bIsModified != bool(xModifiable->isModified()) )
        xModifiable->setModified( bIsModified );
}


// static
bool SfxStoringHelper::WarnUnacceptableFormat( const uno::Reference< frame::XModel >& xModel,
                                                    const OUString& aOldUIName,
                                                    const OUString& aDefExtension,
                                                    bool bDefIsAlien )
{
    if ( !SvtSaveOptions().IsWarnAlienFormat() )
        return true;

    vcl::Window* pWin = SfxStoringHelper::GetModelWindow( xModel );
    SfxAlienWarningDialog aDlg(pWin ? pWin->GetFrameWeld() : nullptr, aOldUIName, aDefExtension, bDefIsAlien);

    return aDlg.run() == RET_OK;
}

uno::Reference<awt::XWindow> SfxStoringHelper::GetModelXWindow(const uno::Reference<frame::XModel>& xModel)
{
    try {
        if ( xModel.is() )
        {
            uno::Reference< frame::XController > xController = xModel->getCurrentController();
            if ( xController.is() )
            {
                uno::Reference< frame::XFrame > xFrame = xController->getFrame();
                if ( xFrame.is() )
                {
                    return xFrame->getContainerWindow();
                }
            }
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return uno::Reference<awt::XWindow>();
}

vcl::Window* SfxStoringHelper::GetModelWindow( const uno::Reference< frame::XModel >& xModel )
{
    VclPtr<vcl::Window> pWin;

    try {
        uno::Reference<awt::XWindow> xWindow = GetModelXWindow(xModel);
        if ( xWindow.is() )
        {
            VCLXWindow* pVCLWindow = VCLXWindow::GetImplementation( xWindow );
            if ( pVCLWindow )
                pWin = pVCLWindow->GetWindow();
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return pWin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
