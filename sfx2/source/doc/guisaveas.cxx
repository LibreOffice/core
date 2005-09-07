/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guisaveas.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:43:47 $
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

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

#ifndef  _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE2_HPP_
#include <com/sun/star/frame/XStorable2.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif


#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

#include "guisaveas.hxx"

#include <svtools/pathoptions.hxx>
#include <svtools/itemset.hxx>
#include <svtools/adrparse.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/saveopt.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <sfxsids.hrc>
#include <doc.hrc>
#include <sfxresid.hxx>
#include <docfilt.hxx>
#include <filedlghelper.hxx>
#include <app.hxx>
#include <objuno.hxx>
#include <objsh.hxx>
#include <dinfdlg.hxx>
#include <sfxtypes.hxx>
#include "alienwarn.hxx"

#define DOCPROPSNUM 17

// flags that specify requested operation
#define EXPORT_REQUESTED            1
#define PDFEXPORT_REQUESTED         2
#define PDFDIRECTEXPORT_REQUESTED   4
#define WIDEEXPORT_REQUESTED        8
#define SAVE_REQUESTED              16
#define SAVEAS_REQUESTED            32

// possible statuses of save operation
#define STATUS_NO_ACTION            0
#define STATUS_SAVE                 1
#define STATUS_SAVEAS               2
#define STATUS_SAVEAS_STANDARDNAME  3

const ::rtl::OUString aFilterOptionsString = ::rtl::OUString::createFromAscii( "FilterOptions" );
const ::rtl::OUString aFilterDataString = ::rtl::OUString::createFromAscii( "FilterData" );

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
static sal_uInt16 getSlotIDFromMode( sal_Int8 nStoreMode )
{
    // This is a temporary hardcoded solution must be removed when
    // dialogs do not need parameters in SidSet representation any more

    sal_uInt16 nResult = 0;
    if ( nStoreMode == EXPORT_REQUESTED )
        nResult = SID_EXPORTDOC;
    else if ( nStoreMode == ( EXPORT_REQUESTED | PDFEXPORT_REQUESTED ) )
        nResult = SID_EXPORTDOCASPDF;
    else if ( nStoreMode == ( EXPORT_REQUESTED | PDFEXPORT_REQUESTED | PDFDIRECTEXPORT_REQUESTED ) )
        nResult = SID_DIRECTEXPORTDOCASPDF;
    else if ( nStoreMode == SAVEAS_REQUESTED || nStoreMode == ( EXPORT_REQUESTED | WIDEEXPORT_REQUESTED ) )
        nResult = SID_SAVEASDOC;
    else
        DBG_ASSERT( sal_False, "Unacceptable slot name is provided!\n" );

    return nResult;
}

//-------------------------------------------------------------------------
static sal_uInt8 getStoreModeFromSlotName( const ::rtl::OUString& aSlotName )
{
    sal_uInt8 nResult = 0;
    if ( aSlotName.equalsAscii( "ExportTo" ) )
        nResult = EXPORT_REQUESTED;
    else if ( aSlotName.equalsAscii( "ExportToPDF" ) )
        nResult = EXPORT_REQUESTED | PDFEXPORT_REQUESTED;
    else if ( aSlotName.equalsAscii( "ExportDirectToPDF" ) )
        nResult = EXPORT_REQUESTED | PDFEXPORT_REQUESTED | PDFDIRECTEXPORT_REQUESTED;
    else if ( aSlotName.equalsAscii( "Save" ) )
        nResult = SAVE_REQUESTED;
    else if ( aSlotName.equalsAscii( "SaveAs" ) )
        nResult = SAVEAS_REQUESTED;
    else
        throw task::ErrorCodeIOException( ::rtl::OUString(),
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
// class ModelData_Impl
//=========================================================================
class ModelData_Impl
{
    SfxStoringHelper* m_pOwner;
    uno::Reference< frame::XModel > m_xModel;
    uno::Reference< frame::XStorable > m_xStorable;
    uno::Reference< frame::XStorable2 > m_xStorable2;
    uno::Reference< util::XModifiable > m_xModifiable;

    ::rtl::OUString m_aDocumentServiceName;
    ::comphelper::SequenceAsHashMap* m_pDocumentPropsHM;
    ::comphelper::SequenceAsHashMap* m_pDocServicePropsHM;

    ::comphelper::SequenceAsHashMap m_aMediaDescrHM;

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

    const ::comphelper::SequenceAsHashMap& GetDocProps();

    ::rtl::OUString GetDocumentServiceName();
    const ::comphelper::SequenceAsHashMap& GetDocumentServiceProps();

    void CheckInteractionHandler();


    uno::Sequence< beans::PropertyValue > GetDocServiceDefaultFilterCheckFlags( sal_Int32 nMust, sal_Int32 nDont );
    uno::Sequence< beans::PropertyValue > GetDocServiceAnyFilter( sal_Int32 nMust, sal_Int32 nDont );
    uno::Sequence< beans::PropertyValue > GetPreselectedFilter_Impl( sal_Int8 nStoreMode );

    sal_Bool ExecuteFilterDialog_Impl( const ::rtl::OUString& aFilterName );

    sal_Int8 CheckStateForSave();
    sal_Int8 CheckFilter( const ::rtl::OUString& );

    sal_Bool CheckFilterOptionsDialogExistence();

    uno::Sequence< beans::PropertyValue > GetDocServiceDefaultFilter();

    sal_Bool OutputFileDialog( sal_Int8 nStoreMode,
                                const ::comphelper::SequenceAsHashMap& aPreselectedFilterPropsHM,
                                sal_Bool bSetStandardName );

    sal_Bool ShowDocumentInfoDialog();
};

//-------------------------------------------------------------------------
ModelData_Impl::ModelData_Impl( SfxStoringHelper& aOwner,
                                const uno::Reference< frame::XModel >& xModel,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescr )
: m_pOwner( &aOwner )
, m_xModel( xModel )
, m_aMediaDescrHM( aMediaDescr )
, m_pDocumentPropsHM( NULL )
, m_pDocServicePropsHM( NULL )
{
    CheckInteractionHandler();
}

//-------------------------------------------------------------------------
ModelData_Impl::~ModelData_Impl()
{
    FreeDocumentProps();
    if ( m_pDocumentPropsHM )
        delete m_pDocumentPropsHM;

    if ( m_pDocServicePropsHM )
        delete m_pDocServicePropsHM;
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
::rtl::OUString ModelData_Impl::GetDocumentServiceName()
{
    if ( !m_aDocumentServiceName.getLength() )
    {
        m_aDocumentServiceName = m_pOwner->GetModuleManager()->identify(
                                                uno::Reference< uno::XInterface >( m_xModel, uno::UNO_QUERY ) );
        if ( !m_aDocumentServiceName.getLength() )
            throw uno::RuntimeException(); // TODO:
    }
    return m_aDocumentServiceName;
}

//-------------------------------------------------------------------------
const ::comphelper::SequenceAsHashMap& ModelData_Impl::GetDocumentServiceProps()
{
    if ( !m_pDocServicePropsHM )
    {
        uno::Sequence< beans::PropertyValue > aDocServiceProps;
        m_pOwner->GetNamedModuleManager()->getByName( GetDocumentServiceName() ) >>= aDocServiceProps;
        if ( !aDocServiceProps.getLength() )
            throw uno::RuntimeException(); // TODO;
        m_pDocServicePropsHM = new ::comphelper::SequenceAsHashMap( aDocServiceProps );
    }

    return *m_pDocServicePropsHM;
}

//-------------------------------------------------------------------------
void ModelData_Impl::CheckInteractionHandler()
{
    ::comphelper::SequenceAsHashMap::const_iterator aInteractIter =
            m_aMediaDescrHM.find( ::rtl::OUString::createFromAscii( "InteractionHandler" ) );

    if ( aInteractIter == m_aMediaDescrHM.end() )
    {
        try {
            m_aMediaDescrHM[ ::rtl::OUString::createFromAscii( "InteractionHandler" ) ]
                <<= uno::Reference< task::XInteractionHandler >(
                            m_pOwner->GetServiceFactory()->createInstance(
                                            DEFINE_CONST_UNICODE("com.sun.star.task.InteractionHandler") ),
                            uno::UNO_QUERY );
        }
        catch( uno::Exception& )
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

    ::rtl::OUString aFilterName = GetDocumentServiceProps().getUnpackedValueOrDefault(
                                                                ::rtl::OUString::createFromAscii( "ooSetupFactoryDefaultFilter" ),
                                                                ::rtl::OUString() );

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
        sal_Int32 nFlags = aFiltHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Flags" ),
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
    aSearchRequest[0].Name = ::rtl::OUString::createFromAscii( "DocumentService" );
    aSearchRequest[0].Value <<= GetDocumentServiceName();

    return SfxStoringHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
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
        aSearchRequest[0].Name = ::rtl::OUString::createFromAscii( "Type" );
        aSearchRequest[0].Value <<= ::rtl::OUString::createFromAscii( "pdf_Portable_Document_Format" );
        aSearchRequest[1].Name = ::rtl::OUString::createFromAscii( "DocumentService" );
        aSearchRequest[1].Value <<= GetDocumentServiceName();

        aFilterProps = SfxStoringHelper::SearchForFilter( m_pOwner->GetFilterQuery(), aSearchRequest, nMust, nDont );
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
sal_Bool ModelData_Impl::ExecuteFilterDialog_Impl( const ::rtl::OUString& aFilterName )
{
    sal_Bool bDialogUsed = sal_False;

    try {
        uno::Sequence < beans::PropertyValue > aProps;
          uno::Any aAny = m_pOwner->GetFilterConfiguration()->getByName( aFilterName );
           if ( aAny >>= aProps )
           {
               ::rtl::OUString aServiceName;
               sal_Int32 nPropertyCount = aProps.getLength();
               for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                   if( aProps[nProperty].Name.equals( ::rtl::OUString::createFromAscii("UIComponent")) )
                   {
                    ::rtl::OUString aServiceName;
                   aProps[nProperty].Value >>= aServiceName;
                    if( aServiceName.getLength() )
                    {
                        uno::Reference< ui::dialogs::XExecutableDialog > xFilterDialog(
                                                    m_pOwner->GetServiceFactory()->createInstance( aServiceName ), uno::UNO_QUERY );
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
                                throw task::ErrorCodeIOException( ::rtl::OUString(),
                                                                    uno::Reference< uno::XInterface >(),
                                                                    ERRCODE_IO_ABORT );
                            }
                        }
                    }

                    break;
                }
        }
    }
    catch( container::NoSuchElementException& )
    {
        // the filter name is unknown
        throw task::ErrorCodeIOException( ::rtl::OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );
    }
    catch( task::ErrorCodeIOException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
    }

    return bDialogUsed;
}

//-------------------------------------------------------------------------
sal_Int8 ModelData_Impl::CheckStateForSave()
{
    // check acceptable entries for media descriptor
    sal_Bool bVersInfoNeedsStore = sal_False;
    ::comphelper::SequenceAsHashMap aAcceptedArgs;

    ::rtl::OUString aVersionCommentString = ::rtl::OUString::createFromAscii( "VersionComment" );
    ::rtl::OUString aAuthorString = ::rtl::OUString::createFromAscii( "Author" );
    ::rtl::OUString aInteractionHandlerString = ::rtl::OUString::createFromAscii( "InteractionHandler" );
    ::rtl::OUString aStatusIndicatorString = ::rtl::OUString::createFromAscii( "StatusIndicator" );

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

    // remove unacceptable entry if there is any
    DBG_ASSERT( GetMediaDescr().size() == aAcceptedArgs.size(),
                "Unacceptable parameters are provided in Save request!\n" );
    if ( GetMediaDescr().size() != aAcceptedArgs.size() )
        GetMediaDescr() = aAcceptedArgs;

    // the document must be modified
    if ( !GetModifiable()->isModified() && !bVersInfoNeedsStore )
        return STATUS_NO_ACTION;

    // if the document is readonly or a new one a SaveAs operation must be used
    if ( !GetStorable()->hasLocation() || GetStorable()->isReadonly() )
        return STATUS_SAVEAS;

    // check that the old filter is acceptable
    ::rtl::OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "FilterName" ),
                                                    ::rtl::OUString() );
    return CheckFilter( aOldFilterName );
}

sal_Int8 ModelData_Impl::CheckFilter( const ::rtl::OUString& aFilterName )
{
    ::comphelper::SequenceAsHashMap aFiltPropsHM;
    sal_Int32 nFiltFlags = 0;
    if ( aFilterName.getLength() )
    {
        // get properties of filter
        uno::Sequence< beans::PropertyValue > aFilterProps;
        if ( aFilterName.getLength() )
            m_pOwner->GetFilterConfiguration()->getByName( aFilterName ) >>= aFilterProps;

        aFiltPropsHM = ::comphelper::SequenceAsHashMap( aFilterProps );
        nFiltFlags = aFiltPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Flags" ), (sal_Int32)0 );
    }

    // only a temporary solution until default filter retrieving feature is implemented
    // then GetDocServiceDefaultFilter() must be used
    ::comphelper::SequenceAsHashMap aDefFiltPropsHM = GetDocServiceDefaultFilterCheckFlags( 3, 0 );
    sal_Int32 nDefFiltFlags = aDefFiltPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Flags" ), (sal_Int32)0 );

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
           && !( nFiltFlags & SFX_FILTER_SILENTEXPORT ) && aDefFiltPropsHM.size()
           && ( nDefFiltFlags & SFX_FILTER_EXPORT ) && !( nDefFiltFlags & SFX_FILTER_INTERNAL ))
    {
        // the default filter is acceptable and the old filter is alian one
        // so ask to make a saveAs operation
        ::rtl::OUString aUIName = aFiltPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "UIName" ),
                                                                                ::rtl::OUString() );
        ::rtl::OUString aDefUIName = aDefFiltPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "UIName" ),
                                                                                ::rtl::OUString() );
        ::rtl::OUString aPreusedFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "PreusedFilterName" ),
                                                    ::rtl::OUString() );
        if ( !aPreusedFilterName.equals( aFilterName ) && !aUIName.equals( aDefUIName ) )
        {
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
    aSearchRequest[0].Name = ::rtl::OUString::createFromAscii( "DocumentService" );
    aSearchRequest[0].Value <<= GetDocumentServiceName();

    uno::Reference< container::XEnumeration > xFilterEnum =
                                    m_pOwner->GetFilterQuery()->createSubSetEnumerationByProperties( aSearchRequest );

    while ( xFilterEnum->hasMoreElements() )
    {
        uno::Sequence< beans::PropertyValue > pProps;
        if ( xFilterEnum->nextElement() >>= pProps )
        {
            ::comphelper::SequenceAsHashMap aPropsHM( pProps );
            ::rtl::OUString aUIServName = aPropsHM.getUnpackedValueOrDefault(
                                            ::rtl::OUString::createFromAscii( "UIComponent" ),
                                            ::rtl::OUString() );
            if ( aUIServName.getLength() )
                return sal_True;
        }
    }

    return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool ModelData_Impl::OutputFileDialog( sal_Int8 nStoreMode,
                                            const ::comphelper::SequenceAsHashMap& aPreselectedFilterPropsHM,
                                            sal_Bool bSetStandardName )
{
    sal_Bool bUseFilterOptions = sal_False;

    ::comphelper::SequenceAsHashMap::const_iterator aOverwriteIter =
                GetMediaDescr().find( ::rtl::OUString::createFromAscii( "Overwrite" ) );

    // the file name must be specified if overwrite option is set
    if ( aOverwriteIter != GetMediaDescr().end() )
           throw task::ErrorCodeIOException( ::rtl::OUString(),
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
    sal_Int16  aDialogMode = bAllowOptions ?
                                ::sfx2::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS :
                                ::sfx2::FILESAVE_AUTOEXTENSION_PASSWORD;
    sal_uInt32 aDialogFlags = 0;

    if( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        aDialogMode  = ::sfx2::FILESAVE_AUTOEXTENSION_SELECTION;
        aDialogFlags = SFXWB_EXPORT;
    }

    sfx2::FileDialogHelper* pFileDlg = NULL;

    // TODO/LATER: ooSetupFactoryShortName should be used, but for star web it returns sweb
    ::rtl::OUString aDocServiceShortName = GetDocumentServiceProps().getUnpackedValueOrDefault(
                                                ::rtl::OUString::createFromAscii( "ooSetupFactoryEmptyDocumentURL" ),
                                                ::rtl::OUString() );
    ::rtl::OUString aPrefix = ::rtl::OUString::createFromAscii( "private:factory/" );
    if ( aDocServiceShortName.match( aPrefix ) )
        aDocServiceShortName = aDocServiceShortName.copy( aPrefix.getLength() );
    sal_uInt32 nParamPos = aDocServiceShortName.indexOf( '?' );
    if ( nParamPos != -1 )
        aDocServiceShortName = aDocServiceShortName.copy( 0, nParamPos );

    DBG_ASSERT( aDocServiceShortName.getLength(), "No short name for document service!" );


    sal_Int32 nMust = getMustFlags( nStoreMode );
    sal_Int32 nDont = getDontFlags( nStoreMode );

    if ( ( nStoreMode & EXPORT_REQUESTED ) && !( nStoreMode & WIDEEXPORT_REQUESTED ) )
    {
        // This is the normal dialog
        pFileDlg = new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aDocServiceShortName, nMust, nDont );

        if( aDocServiceShortName.equalsAscii( "sdraw" ) )
            pFileDlg->SetContext( sfx2::FileDialogHelper::SD_EXPORT );
        if( aDocServiceShortName.equalsAscii( "simpress" ) )
            pFileDlg->SetContext( sfx2::FileDialogHelper::SI_EXPORT );

        pFileDlg->CreateMatcher( aDocServiceShortName );

        uno::Reference< ui::dialogs::XFilePicker > xFilePicker = pFileDlg->GetFilePicker();
        uno::Reference< ui::dialogs::XFilePickerControlAccess > xControlAccess =
        uno::Reference< ui::dialogs::XFilePickerControlAccess >( xFilePicker, uno::UNO_QUERY );

        if ( xControlAccess.is() )
        {
            ::rtl::OUString aCtrlText = String( SfxResId( STR_EXPORTBUTTON ) );
            xControlAccess->setLabel( ui::dialogs::CommonFilePickerElementIds::PUSHBUTTON_OK, aCtrlText );

            aCtrlText = ::rtl::OUString( String( SfxResId( STR_LABEL_FILEFORMAT ) ) );
            xControlAccess->setLabel( ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER_LABEL, aCtrlText );
        }

        if ( ( nStoreMode & PDFEXPORT_REQUESTED ) && aPreselectedFilterPropsHM.size() )
            pFileDlg->SetCurrentFilter( aPreselectedFilterPropsHM.getUnpackedValueOrDefault(
                                                        ::rtl::OUString::createFromAscii( "UIName" ),
                                                        ::rtl::OUString() ) );
    }
    else
    {
        // This is the normal dialog
        pFileDlg = new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, aDocServiceShortName, nMust, nDont );
        pFileDlg->CreateMatcher( aDocServiceShortName );
    }

    if ( GetStorable()->hasLocation()
      && !GetMediaDescr().getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "RepairPackage" ),
                                                                      sal_False ) )
    {
        // --> PB 2004-11-05 #i36524# - aLastName must be an URL, not only a filename
        /*
        ::rtl::OUString aLastName = INetURLObject( GetStorable()->getLocation() ).getName(
                                                        INetURLObject::LAST_SEGMENT,
                                                        sal_True,
                                                        INetURLObject::DECODE_WITH_CHARSET);
        */
        ::rtl::OUString aLastName = GetStorable()->getLocation();
        // <--

        if ( !aLastName.getLength() )
        {
            aLastName = GetDocProps().getUnpackedValueOrDefault(
                                                        ::rtl::OUString::createFromAscii( "Title" ),
                                                        ::rtl::OUString() );
            INetURLObject aObj( INetURLObject::GetAbsURL( SvtPathOptions().GetWorkPath(), aLastName ) );
            aLastName = aObj.GetMainURL( INetURLObject::NO_DECODE );
        }

        uno::Sequence< beans::PropertyValue > aOldFilterProps;
        sal_Int32 nOldFiltFlags = 0;
        ::rtl::OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "FilterName" ),
                                                    ::rtl::OUString() );

        if ( aOldFilterName.getLength() )
            m_pOwner->GetFilterConfiguration()->getByName( aOldFilterName ) >>= aOldFilterProps;

        ::comphelper::SequenceAsHashMap aOldFiltPropsHM( aOldFilterProps );
        nOldFiltFlags = aOldFiltPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Flags" ),
                                                                    (sal_Int32)0 );

        // bSetStandardName == true means that user agreed to store document in the default (default default ;-)) format
        if ( bSetStandardName || ( nOldFiltFlags & nMust ) != nMust || nOldFiltFlags & nDont )
        {
            ::rtl::OUString aTypeName;
            ::comphelper::SequenceAsHashMap aPropsHM( GetDocServiceAnyFilter( 294, 80 ) );  // import,export,default,!alien,!templatepath
            ::rtl::OUString aFilterUIName = aPropsHM.getUnpackedValueOrDefault(
                                            ::rtl::OUString::createFromAscii( "UIName" ),
                                            ::rtl::OUString() );
            aTypeName = aPropsHM.getUnpackedValueOrDefault(
                                            ::rtl::OUString::createFromAscii( "Type" ),
                                            ::rtl::OUString() );

            if( aLastName.getLength() )
            {
                INetURLObject aObj( aLastName );
                if ( aTypeName.getLength() )
                {
                    uno::Reference< container::XNameAccess > xTypeDetection = uno::Reference< container::XNameAccess >(
                                    m_pOwner->GetServiceFactory()->createInstance(
                                            ::rtl::OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
                                    uno::UNO_QUERY );
                    if ( xTypeDetection.is() )
                    {
                        uno::Sequence< beans::PropertyValue > aTypeNameProps;
                        if ( ( xTypeDetection->getByName( aTypeName ) >>= aTypeNameProps ) && aTypeNameProps.getLength() )
                        {
                            ::comphelper::SequenceAsHashMap aTypeNamePropsHM( aTypeNameProps );
                            uno::Sequence< ::rtl::OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                                            ::rtl::OUString::createFromAscii( "Extension" ),
                                                            ::uno::Sequence< ::rtl::OUString >() );
                            if ( aExtensions.getLength() )
                                aObj.SetExtension( aExtensions[0] );
                        }
                    }
                }

                pFileDlg->SetDisplayDirectory( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
            }

            pFileDlg->SetCurrentFilter( aFilterUIName );
        }
        else
        {
            if( aLastName.getLength() )
                pFileDlg->SetDisplayDirectory( aLastName );

            pFileDlg->SetCurrentFilter( aOldFiltPropsHM.getUnpackedValueOrDefault(
                                                        ::rtl::OUString::createFromAscii( "UIName" ),
                                                        ::rtl::OUString() ) );
        }
    }
    else
    {
        pFileDlg->SetDisplayDirectory( SvtPathOptions().GetWorkPath() );
    }

    uno::Reference < view::XSelectionSupplier > xSel( GetModel()->getCurrentController(), uno::UNO_QUERY );
    if ( xSel.is() && xSel->getSelection().hasValue() )
        GetMediaDescr()[::rtl::OUString::createFromAscii( "SelectionOnly" )] <<= sal_True;

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

    // aStringTypeFN is a pure output parameter, pDialogParams is an in/out parameter
    String aStringTypeFN;
    if ( pFileDlg->Execute( pDialogParams, aStringTypeFN ) != ERRCODE_NONE )
    {
        delete pFileDlg;
        throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_ABORT );
    }

    ::rtl::OUString aFilterName = aStringTypeFN;

    uno::Sequence< beans::PropertyValue > aPropsFromDialog;
    TransformItems( nSlotID, *pDialogParams, aPropsFromDialog, NULL );
    GetMediaDescr() << aPropsFromDialog;

    // get the path from the dialog
    INetURLObject aURL( pFileDlg->GetPath() );

    // old filter options should be cleared in case different filter is used

    ::rtl::OUString aFilterFromMediaDescr = GetMediaDescr().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "FilterName" ),
                                                    ::rtl::OUString() );
    ::rtl::OUString aOldFilterName = GetDocProps().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "FilterName" ),
                                                    ::rtl::OUString() );
    if ( aFilterName.equals( aFilterFromMediaDescr ) )
    {
        // preserv current settings if any
        // if there no current settings and the name is the same
        // as old filter name use old filter settings

        if ( aFilterFromMediaDescr.equals( aOldFilterName ) )
        {
            ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                        GetDocProps().find( aFilterOptionsString );
            if ( aIter != GetDocProps().end()
              && GetMediaDescr().find( aFilterOptionsString ) == GetMediaDescr().end() )
                GetMediaDescr()[aIter->first] = aIter->second;

            aIter = GetDocProps().find( aFilterDataString );
            if ( aIter != GetDocProps().end()
              && GetMediaDescr().find( aFilterDataString ) == GetMediaDescr().end() )
                GetMediaDescr()[aIter->first] = aIter->second;
        }
    }
    else
    {
        GetMediaDescr().erase( aFilterDataString );
        GetMediaDescr().erase( aFilterOptionsString );

        if ( aFilterName.equals( aOldFilterName ) )
        {
            // merge filter option of the document filter

            ::rtl::OUString aFilterOptionsString = ::rtl::OUString::createFromAscii( "FilterOptions" );
            ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                GetDocProps().find( aFilterOptionsString );
            if ( aIter != GetDocProps().end() )
                GetMediaDescr()[aIter->first] = aIter->second;

            ::rtl::OUString aFilterDataString = ::rtl::OUString::createFromAscii( "FilterData" );
            aIter = GetDocProps().find( aFilterDataString );
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
                      ( GetMediaDescr().find( aFilterDataString ) == GetMediaDescr().end()
                      && GetMediaDescr().find( aFilterOptionsString ) == GetMediaDescr().end() );
            }
            catch( lang::IllegalArgumentException& )
            {}
        }
    }

    delete pFileDlg;

    // merge in results of the dialog execution
    GetMediaDescr()[::rtl::OUString::createFromAscii( "URL" )] <<=
                                                ::rtl::OUString( aURL.GetMainURL( INetURLObject::NO_DECODE ));
    GetMediaDescr()[::rtl::OUString::createFromAscii( "FilterName" )] <<= aFilterName;

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
                aURL.Complete = ::rtl::OUString::createFromAscii( ".uno:SetDocumentProperties" );

                uno::Reference< util::XURLTransformer > xTransformer(
                            m_pOwner->GetServiceFactory()->createInstance(
                                            DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer") ),
                            uno::UNO_QUERY );
                if ( xTransformer.is() && xTransformer->parseStrict( aURL ) )
                {
                    uno::Reference< frame::XDispatch > xDispatch = xFrameDispatch->queryDispatch(
                                                                                aURL,
                                                                                ::rtl::OUString::createFromAscii( "_self" ),
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
    catch ( uno::Exception& )
    {
    }

    return bDialogUsed;
}

//=========================================================================
// class SfxStoringHelper
//=========================================================================
//-------------------------------------------------------------------------
SfxStoringHelper::SfxStoringHelper( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_xFactory( xFactory )
{
}

//-------------------------------------------------------------------------
uno::Reference< lang::XMultiServiceFactory > SfxStoringHelper::GetServiceFactory()
{
    if ( !m_xFactory.is() )
    {
        m_xFactory = ::comphelper::getProcessServiceFactory();
        if( !m_xFactory.is() )
            throw uno::RuntimeException(); // TODO:
    }

    return m_xFactory;
}

//-------------------------------------------------------------------------
uno::Reference< container::XNameAccess > SfxStoringHelper::GetFilterConfiguration()
{
    if ( !m_xFilterCFG.is() )
    {
        m_xFilterCFG = uno::Reference< container::XNameAccess >(
            GetServiceFactory()->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
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
uno::Reference< ::com::sun::star::frame::XModuleManager > SfxStoringHelper::GetModuleManager()
{
    if ( !m_xModuleManager.is() )
    {
        m_xModuleManager = uno::Reference< ::com::sun::star::frame::XModuleManager >(
            GetServiceFactory()->createInstance(
                    ::rtl::OUString::createFromAscii( "com.sun.star.frame.ModuleManager" ) ),
            uno::UNO_QUERY );

        if ( !m_xModuleManager.is() )
            throw uno::RuntimeException();
    }

    return m_xModuleManager;
}

//-------------------------------------------------------------------------
uno::Reference< container::XNameAccess > SfxStoringHelper::GetNamedModuleManager()
{
    if ( !m_xNamedModManager.is() )
    {
        m_xNamedModManager = uno::Reference< container::XNameAccess >( GetModuleManager(), uno::UNO_QUERY );
        if ( !m_xNamedModManager.is() )
            throw uno::RuntimeException();
    }

    return m_xNamedModManager;
}

//-------------------------------------------------------------------------
sal_Bool SfxStoringHelper::GUIStoreModel( const uno::Reference< frame::XModel >& xModel,
                                            const ::rtl::OUString& aSlotName,
                                            uno::Sequence< beans::PropertyValue >& aArgsSequence )
{
    ModelData_Impl aModelData( *this, xModel, aArgsSequence );

    sal_Bool bDialogUsed = sal_False;

    INetURLObject aURL;

    sal_Bool bSetStandardName = sal_False; // can be set only for SaveAs

    // parse the slot name
    sal_Int8 nStoreMode = getStoreModeFromSlotName( aSlotName );

    // handle the special cases
    if ( nStoreMode & SAVEAS_REQUESTED )
    {
        ::comphelper::SequenceAsHashMap::const_iterator aSaveToIter =
                        aModelData.GetMediaDescr().find( ::rtl::OUString::createFromAscii( "SaveTo" ) );
        if ( aSaveToIter != aModelData.GetMediaDescr().end() )
        {
            sal_Bool bWideExport = sal_False;
            aSaveToIter->second >>= bWideExport;
            if ( bWideExport )
                nStoreMode = EXPORT_REQUESTED | WIDEEXPORT_REQUESTED;
        }
    }
    else if ( nStoreMode & SAVE_REQUESTED )
    {
        sal_Int8 nStatusSave = aModelData.CheckStateForSave();
        if ( nStatusSave == STATUS_NO_ACTION )
        {
            throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), ERRCODE_IO_ABORT );
        }
        else if ( nStatusSave == STATUS_SAVE )
        {
            // Document properties can contain streams that should be freed before storing
            aModelData.FreeDocumentProps();

            if ( aModelData.GetStorable2().is() )
            {
                try
                {
                    aModelData.GetStorable2()->storeSelf( aModelData.GetMediaDescr().getAsConstPropertyValueList() );
                }
                catch( lang::IllegalArgumentException& )
                {
                    OSL_ENSURE( sal_False, "ModelData didn't handle illegal parameters, all the parameters are ignored!\n" );
                    aModelData.GetStorable()->store();
                }
            }
            else
            {
                OSL_ENSURE( sal_False, "XStorable2 is not supported by the model!\n" );
                aModelData.GetStorable()->store();
            }

            return sal_False;
        }
        else
        {
            // this should be a usual SaveAs operation
            nStoreMode = SAVEAS_REQUESTED;
            if ( nStatusSave == STATUS_SAVEAS_STANDARDNAME )
                bSetStandardName = sal_True;
        }
    }

    // preselect a filter for the storing process
    uno::Sequence< beans::PropertyValue > aFilterProps = aModelData.GetPreselectedFilter_Impl( nStoreMode );

    DBG_ASSERT( aFilterProps.getLength(), "No filter for storing!\n" );
    if ( !aFilterProps.getLength() )
        throw task::ErrorCodeIOException( ::rtl::OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );

    ::comphelper::SequenceAsHashMap aFilterPropsHM( aFilterProps );

    ::rtl::OUString aFilterFromMediaDescr = aModelData.GetMediaDescr().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "FilterName" ),
                                                    ::rtl::OUString() );
    ::rtl::OUString aOldFilterName = aModelData.GetDocProps().getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "FilterName" ),
                                                    ::rtl::OUString() );

    sal_Bool bUseFilterOptions = sal_False;
    ::comphelper::SequenceAsHashMap::const_iterator aFileNameIter = aModelData.GetMediaDescr().find( ::rtl::OUString::createFromAscii( "URL" ) );
    if ( aFileNameIter == aModelData.GetMediaDescr().end() )
    {
        sal_Bool bExit = sal_False;
        while ( !bExit )
        {
            bUseFilterOptions = aModelData.OutputFileDialog( nStoreMode, aFilterProps, bSetStandardName );
            if ( nStoreMode == SAVEAS_REQUESTED )
            {
                // in case of saving check filter for possible alien warning
                ::rtl::OUString aFilterName = aModelData.GetMediaDescr().getUnpackedValueOrDefault(
                                                                                ::rtl::OUString::createFromAscii( "FilterName" ),
                                                                                ::rtl::OUString() );
                sal_Int8 nStatusSave = aModelData.CheckFilter( aFilterName );
                if ( nStatusSave == STATUS_SAVEAS_STANDARDNAME )
                {
                    // switch to best filter
                    bSetStandardName = sal_True;
                }
                else if ( nStatusSave == STATUS_SAVE )
                {
                    // user confirmed alien filter or "good" filter is used
                    bExit = sal_True;
                }
            }
            else
                bExit = sal_True;
        }

        bDialogUsed = sal_True;
        aFileNameIter = aModelData.GetMediaDescr().find( ::rtl::OUString::createFromAscii( "URL" ) );
    }
    else
    {
        // the target file name is provided so check if new filter options
        // are provided or old options can be used
        if ( aFilterFromMediaDescr.equals( aOldFilterName ) )
        {
            ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                            aModelData.GetDocProps().find( aFilterOptionsString );
            if ( aIter != aModelData.GetDocProps().end()
              && aModelData.GetMediaDescr().find( aFilterOptionsString ) == aModelData.GetMediaDescr().end() )
                aModelData.GetMediaDescr()[aIter->first] = aIter->second;

            aIter = aModelData.GetDocProps().find( aFilterDataString );
            if ( aIter != aModelData.GetDocProps().end()
              && aModelData.GetMediaDescr().find( aFilterDataString ) == aModelData.GetMediaDescr().end() )
                aModelData.GetMediaDescr()[aIter->first] = aIter->second;
        }
    }

    // now we can get the filename from the SfxRequest
    // if it is not set the preselected filter is used
    ::rtl::OUString aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                                    ::rtl::OUString::createFromAscii( "Name" ),
                                                                    ::rtl::OUString() );
    if ( aFileNameIter != aModelData.GetMediaDescr().end() )
    {
        ::rtl::OUString aFileName;
        aFileNameIter->second >>= aFileName;
        aURL.SetURL( aFileName );
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );

        ::comphelper::SequenceAsHashMap::const_iterator aIter =
                                aModelData.GetMediaDescr().find( ::rtl::OUString::createFromAscii( "FilterName" ) );

        if ( aIter != aModelData.GetMediaDescr().end() )
            aIter->second >>= aFilterName;
        else
            aModelData.GetMediaDescr()[::rtl::OUString::createFromAscii( "FilterName" )] <<= aFilterName;

        DBG_ASSERT( aFilterName.getLength(), "Illegal filter!" );
    }
    else
    {
        DBG_ASSERT( sal_False, "This code must be unreachable!\n" );
        throw task::ErrorCodeIOException( ::rtl::OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            ERRCODE_IO_INVALIDPARAMETER );
    }

    ::comphelper::SequenceAsHashMap::const_iterator aIter =
                            aModelData.GetMediaDescr().find( ::rtl::OUString::createFromAscii( "FilterFlags" ) );
    sal_Bool bFilterFlagsSet = ( aIter != aModelData.GetMediaDescr().end() );

    if( !( nStoreMode & PDFDIRECTEXPORT_REQUESTED ) && !bFilterFlagsSet
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

    if ( aOptions.IsDocInfoSave()
      && ( !aModelData.GetStorable()->hasLocation()
          || INetURLObject( aModelData.GetStorable()->getLocation() ) != aURL ) )
    {
        // this is defenitly not a Save operation
        // so the document info can be updated

        // on export document info must be preserved
        SfxDocumentInfo aPreservDocInfo;
        FillCopy( aModelData.GetModel(), aPreservDocInfo );

        // use dispatch API to show document info dialog
        if ( aModelData.ShowDocumentInfoDialog() )
            bDialogUsed = sal_True;
        else
        {
            DBG_ERROR( "Can't execute document info dialog!\n" );
        }

        try {
            // Document properties can contain streams that should be freed before storing
            aModelData.FreeDocumentProps();
            if ( ( nStoreMode & EXPORT_REQUESTED ) )
                aModelData.GetStorable()->storeToURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
            else
                aModelData.GetStorable()->storeAsURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
        }
        catch( uno::Exception& )
        {
            if ( ( nStoreMode & EXPORT_REQUESTED ) )
                SetDocInfoState( aModelData.GetModel(), aPreservDocInfo, sal_True );

            throw;
        }

        if ( ( nStoreMode & EXPORT_REQUESTED ) )
            SetDocInfoState( aModelData.GetModel(), aPreservDocInfo, sal_True );
    }
    else
    {
        // Document properties can contain streams that should be freed before storing
        aModelData.FreeDocumentProps();

        // this is actually a save operation with different parameters
        // so storeTo or storeAs without DocInfo operations are used
        if ( ( nStoreMode & EXPORT_REQUESTED ) )
            aModelData.GetStorable()->storeToURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
        else
            aModelData.GetStorable()->storeAsURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), aArgsSequence );
    }

    return bDialogUsed;
}

//-------------------------------------------------------------------------
// static
uno::Sequence< beans::PropertyValue > SfxStoringHelper::SearchForFilter(
                                                        const uno::Reference< container::XContainerQuery >& xFilterQuery,
                                                        const uno::Sequence< beans::NamedValue >& aSearchRequest,
                                                        sal_Int32 nMustFlags,
                                                        sal_Int32 nDontFlags )
{
    uno::Sequence< beans::PropertyValue > aFilterProps;
    uno::Reference< container::XEnumeration > xFilterEnum =
                                            xFilterQuery->createSubSetEnumerationByProperties( aSearchRequest );

    // use the first filter that is found
    if ( xFilterEnum.is() )
        while ( xFilterEnum->hasMoreElements() )
        {
            uno::Sequence< beans::PropertyValue > aProps;
            if ( xFilterEnum->nextElement() >>= aProps )
            {
                ::comphelper::SequenceAsHashMap aPropsHM( aProps );
                sal_Int32 nFlags = aPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Flags" ),
                                                                        (sal_Int32)0 );
                if ( ( ( nFlags & nMustFlags ) == nMustFlags ) && !( nFlags & nDontFlags ) )
                {
                    aFilterProps = aProps;
                    break;
                }
            }
        }

    return aFilterProps;
}

//-------------------------------------------------------------------------
// static
sal_Bool SfxStoringHelper::CheckFilterOptionsAppearence(
                                                    const uno::Reference< container::XNameAccess >& xFilterCFG,
                                                    const ::rtl::OUString& aFilterName )
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
                   ::rtl::OUString aServiceName = aPropsHM.getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "UIComponent" ),
                                                    ::rtl::OUString() );
                if( aServiceName.getLength() )
                       bUseFilterOptions = sal_True;
            }
        }
        catch( uno::Exception& )
        {
        }
    }

    return bUseFilterOptions;
}

//-------------------------------------------------------------------------
// static
void SfxStoringHelper::FillCopy( const uno::Reference< frame::XModel >& xModel,
                                 SfxDocumentInfo& aDocInfoToFill )
{
    uno::Reference< document::XDocumentInfoSupplier > xModelDocInfoSupplier( xModel, uno::UNO_QUERY );
    if ( !xModelDocInfoSupplier.is() )
        throw uno::RuntimeException(); // TODO

    uno::Reference< document::XDocumentInfo > xDocInfo = xModelDocInfoSupplier->getDocumentInfo();
    uno::Reference< beans::XPropertySet > xPropSet( xDocInfo, uno::UNO_QUERY );
    DBG_ASSERT( xPropSet.is(), "No access to the document info!\n" )
    if ( !xPropSet.is() )
        throw uno::RuntimeException();

    uno::Any aVal;
    ::rtl::OUString aStrVal;
    util::DateTime aDateTime;
    sal_Int32 nInt32Val;

    // ==== Author property ====
    ::rtl::OUString aStringAuthor = ::rtl::OUString::createFromAscii( "Author" );
    aVal = xPropSet->getPropertyValue( aStringAuthor );
    if ( aVal >>= aStrVal )
    {
        const SfxStamp& rStamp = aDocInfoToFill.GetCreated();
        if ( aStrVal.getLength() > TIMESTAMP_MAXLENGTH )
        {
            SvAddressParser aParser( aStrVal );
            if ( aParser.Count() > 0 )
            {
                ::rtl::OUString aEmail = aParser.GetEmailAddress(0);
                ::rtl::OUString aRealname = aParser.GetRealName(0);

                if ( aRealname.getLength() <= TIMESTAMP_MAXLENGTH )
                    aStrVal = aRealname;
                else if ( aEmail.getLength() <= TIMESTAMP_MAXLENGTH )
                    aStrVal = aEmail;
            }
        }

        aDocInfoToFill.SetCreated( SfxStamp( aStrVal, rStamp.GetTime() ) );
    }
    else
        DBG_ERROR( "The type of parameter \"Author\" is wrong!\n" );

    // ==== Generator property is not supported by SfxDocumentInfo ====


    // ==== CreationDate property ====
    ::rtl::OUString aStringCreationDate = ::rtl::OUString::createFromAscii( "CreationDate" );
    SfxStamp rStamp = aDocInfoToFill.GetCreated();
    aVal = xPropSet->getPropertyValue( aStringCreationDate );
    if ( aVal >>= aDateTime  )
    {
        aDocInfoToFill.SetCreated( SfxStamp( rStamp.GetName(),
                                    SfxDocumentInfoObject::impl_DateTime_Struct2Object( aDateTime ) ) );
    }
    else
        aDocInfoToFill.SetCreated( SfxStamp( rStamp.GetName(), TIMESTAMP_INVALID_DATETIME ) );


    // ==== Title property ====
    ::rtl::OUString aStringTitle = ::rtl::OUString::createFromAscii( "Title" );
    aVal = xPropSet->getPropertyValue( aStringTitle );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetTitle( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"Title\" is wrong!\n" );

    // ==== Subject property ====
    ::rtl::OUString aStringSubject = ::rtl::OUString::createFromAscii( "Subject" );
    aVal = xPropSet->getPropertyValue( aStringSubject );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetTheme( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"Subject\" is wrong!\n" );

    // ==== Description property ====
    ::rtl::OUString aStringDescription = ::rtl::OUString::createFromAscii( "Description" );
    aVal = xPropSet->getPropertyValue( aStringDescription );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetComment( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"Description\" is wrong!\n" );

    // ==== Keywords property ====
    ::rtl::OUString aStringKeywords = ::rtl::OUString::createFromAscii( "Keywords" );
    aVal = xPropSet->getPropertyValue( aStringKeywords );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetKeywords( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"Keywords\" is wrong!\n" );

    // ==== Language property is not supported by SfxDocumentInfo ====


    // ==== ModifiedBy property ====
    ::rtl::OUString aStringModifiedBy = ::rtl::OUString::createFromAscii( "ModifiedBy" );
    aVal = xPropSet->getPropertyValue( aStringModifiedBy );
    if ( aVal >>= aStrVal )
    {
        const SfxStamp& rStamp = aDocInfoToFill.GetChanged();
        aDocInfoToFill.SetChanged( SfxStamp( aStrVal, rStamp.GetTime() ) );
    }
    else
        DBG_ERROR( "The type of parameter \"ModifiedBy\" is wrong!\n" );

    // ==== ModifyDate property ====
    ::rtl::OUString aStringModifyDate = ::rtl::OUString::createFromAscii( "ModifyDate" );
    aVal = xPropSet->getPropertyValue( aStringModifyDate );
    rStamp = aDocInfoToFill.GetChanged();
    if ( aVal >>= aDateTime  )
    {
        aDocInfoToFill.SetChanged( SfxStamp( rStamp.GetName(),
                                    SfxDocumentInfoObject::impl_DateTime_Struct2Object(aDateTime) ) );
    }
    else
        aDocInfoToFill.SetChanged( SfxStamp( rStamp.GetName(), TIMESTAMP_INVALID_DATETIME ) );


    // ==== PrintedBy property ====
    ::rtl::OUString aStringPrintedBy = ::rtl::OUString::createFromAscii( "PrintedBy" );
    aVal = xPropSet->getPropertyValue( aStringPrintedBy );
    if ( aVal >>= aStrVal )
    {
        const SfxStamp& rStamp = aDocInfoToFill.GetPrinted();
        aDocInfoToFill.SetPrinted( SfxStamp( aStrVal, rStamp.GetTime() ) );
    }
    else
        DBG_ERROR( "The type of parameter \"PrintedBy\" is wrong!\n" );

    // ==== PrintDate property ====
    ::rtl::OUString aStringPrintDate = ::rtl::OUString::createFromAscii( "PrintDate" );
    rStamp = aDocInfoToFill.GetPrinted();
    aVal = xPropSet->getPropertyValue( aStringPrintDate );
    if ( aVal >>= aDateTime  )
    {
        aDocInfoToFill.SetPrinted( SfxStamp( rStamp.GetName(),
                                    SfxDocumentInfoObject::impl_DateTime_Struct2Object(aDateTime) ) );
    }
    else
        aDocInfoToFill.SetPrinted( SfxStamp( rStamp.GetName(), TIMESTAMP_INVALID_DATETIME ) );

    // ==== Template property ====
    ::rtl::OUString aStringTemplate = ::rtl::OUString::createFromAscii( "Template" );
    aVal = xPropSet->getPropertyValue( aStringTemplate );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetTemplateName( aStrVal ); // ???: aDocInfoToFill.SetTemplateFileName( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"Template\" is wrong!\n" );

    // ==== TemplateDate property ====
    ::rtl::OUString aStringTemplateDate = ::rtl::OUString::createFromAscii( "TemplateDate" );
    aVal = xPropSet->getPropertyValue( aStringTemplateDate );
    if ( aVal >>= aDateTime  )
        aDocInfoToFill.SetTemplateDate( SfxDocumentInfoObject::impl_DateTime_Struct2Object(aDateTime) );
    else
        aDocInfoToFill.SetTemplateDate( TIMESTAMP_INVALID_DATETIME );

    // ==== AutoloadURL property ====
    ::rtl::OUString aStringAutoloadURL = ::rtl::OUString::createFromAscii( "AutoloadURL" );
    aVal = xPropSet->getPropertyValue( aStringAutoloadURL );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetReloadURL( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"AutoloadURL\" is wrong!\n" );


    // ==== AutoloadSecs property ====
    ::rtl::OUString aStringAutoloadSecs = ::rtl::OUString::createFromAscii( "AutoloadSecs" );
    aVal = xPropSet->getPropertyValue( aStringAutoloadSecs );
    if ( aVal >>= nInt32Val )
        aDocInfoToFill.SetReloadDelay( nInt32Val );
    else
        DBG_ERROR( "The type of parameter \"AutoloadSecs\" is wrong!\n" );

    // ==== DefaultTarget property ====
    ::rtl::OUString aStringDefaultTarget = ::rtl::OUString::createFromAscii( "DefaultTarget" );
    aVal = xPropSet->getPropertyValue( aStringDefaultTarget );
    if ( aVal >>= aStrVal )
        aDocInfoToFill.SetDefaultTarget( aStrVal );
    else
        DBG_ERROR( "The type of parameter \"DefaultTarget\" is wrong!\n" );


    // now handle special properties
    sal_Int16 nCount = xDocInfo->getUserFieldCount();
    sal_Int16 nSupportedCount = aDocInfoToFill.GetUserKeyCount();
    OSL_ENSURE( nCount <= nSupportedCount, "Not all user property can be stored in sfx implementation!\n" );

    for ( sal_Int16 nInd = 0; nInd < nCount && nInd < nSupportedCount; nInd++ )
    {
        ::rtl::OUString aPropName = xDocInfo->getUserFieldName( nInd );
        ::rtl::OUString aPropVal = xDocInfo->getUserFieldValue( nInd );
        aDocInfoToFill.SetUserKey( SfxDocUserKey( aPropName, aPropVal ), nInd );
    }
}

//-------------------------------------------------------------------------
// static
void SfxStoringHelper::PrepareDocInfoForStore( SfxDocumentInfo& aDocInfoToClear )
{
    // TODO/LATER: how preset SetUseUserData?
    // TODO/LATER: aDocInfoToClear.SetTemplateConfig( HasTemplateConfig() ) is not covered by UNO API
    // HasTemplateConfig() reterned false always and was removed

    ::rtl::OUString aUserName = SvtUserOptions().GetFullName();
    if ( !aDocInfoToClear.IsUseUserData() )
       {
           SfxStamp aCreated = aDocInfoToClear.GetCreated();
           if ( aUserName.equals( aCreated.GetName() ) )
           {
               aCreated.SetName( String() );
              aDocInfoToClear.SetCreated( aCreated );
           }

           SfxStamp aPrinted = aDocInfoToClear.GetPrinted();
           if ( aUserName.equals( aPrinted.GetName() ) )
           {
               aPrinted.SetName( String() );
              aDocInfoToClear.SetPrinted( aPrinted );
           }

        aUserName = ::rtl::OUString();
       }

    aDocInfoToClear.SetChanged( aUserName );
    // TODO/LATER: the editing duration can't be updated anyway
    // TODO/LATER: the message can not be broadcasted but it looks like nobody needs it
}

//-------------------------------------------------------------------------
// static
void SfxStoringHelper::SetDocInfoState( const uno::Reference< frame::XModel >& xModel,
                                        const SfxDocumentInfo& aDocInfoState,
                                        sal_Bool bNoModify )
{
    uno::Reference< document::XDocumentInfoSupplier > xModelDocInfoSupplier( xModel, uno::UNO_QUERY );
    if ( !xModelDocInfoSupplier.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< document::XDocumentInfo > xDocInfo = xModelDocInfoSupplier->getDocumentInfo();
    uno::Reference< beans::XPropertySet > xPropSet( xDocInfo, uno::UNO_QUERY );
    DBG_ASSERT( xPropSet.is(), "No access to the document info!\n" )
    if ( !xPropSet.is() )
        throw uno::RuntimeException();

    uno::Reference< util::XModifiable > xModifiable( xModel, uno::UNO_QUERY );
    if ( bNoModify && !xModifiable.is() )
        throw uno::RuntimeException();

    sal_Bool bIsModified = bNoModify && xModifiable->isModified();


    // ==== Author property ====
    ::rtl::OUString aStringAuthor = ::rtl::OUString::createFromAscii( "Author" );
    const SfxStamp& rStamp = aDocInfoState.GetCreated();
    if ( rStamp.IsValid() )
        xPropSet->setPropertyValue( aStringAuthor, uno::makeAny( ::rtl::OUString( rStamp.GetName() ) ) );

    // ==== Generator property is not supported by SfxDocumentInfo ====


    // ==== CreationDate property ====
    ::rtl::OUString aStringCreationDate = ::rtl::OUString::createFromAscii( "CreationDate" );
    xPropSet->setPropertyValue( aStringCreationDate,
                                uno::makeAny( SfxDocumentInfoObject::impl_DateTime_Object2Struct( rStamp.GetTime() ) ) );

    // ==== Title property ====
    ::rtl::OUString aStringTitle = ::rtl::OUString::createFromAscii( "Title" );
    xPropSet->setPropertyValue( aStringTitle, uno::makeAny( ::rtl::OUString( aDocInfoState.GetTitle() ) ) );

    // ==== Subject property ====
    ::rtl::OUString aStringSubject = ::rtl::OUString::createFromAscii( "Subject" );
    xPropSet->setPropertyValue( aStringSubject, uno::makeAny( ::rtl::OUString( aDocInfoState.GetTheme() ) ) );

    // ==== Description property ====
    ::rtl::OUString aStringDescription = ::rtl::OUString::createFromAscii( "Description" );
    xPropSet->setPropertyValue( aStringDescription, uno::makeAny( ::rtl::OUString( aDocInfoState.GetComment() ) ) );

    // ==== Keywords property ====
    ::rtl::OUString aStringKeywords = ::rtl::OUString::createFromAscii( "Keywords" );
    xPropSet->setPropertyValue( aStringKeywords, uno::makeAny( ::rtl::OUString( aDocInfoState.GetKeywords() ) ) );

    // ==== Language property is not supported by SfxDocumentInfo ====

    // ==== ModifiedBy property ====
    const SfxStamp& rChangedStamp = aDocInfoState.GetChanged();
    ::rtl::OUString aStringModifiedBy = ::rtl::OUString::createFromAscii( "ModifiedBy" );
    if ( rChangedStamp.IsValid() )
        xPropSet->setPropertyValue( aStringModifiedBy, uno::makeAny( ::rtl::OUString( rChangedStamp.GetName() ) ) );

    // ==== ModifyDate property ====
    ::rtl::OUString aStringModifyDate = ::rtl::OUString::createFromAscii( "ModifyDate" );
    if ( rChangedStamp.IsValid() )
        xPropSet->setPropertyValue( aStringModifyDate,
                            uno::makeAny( SfxDocumentInfoObject::impl_DateTime_Object2Struct( rChangedStamp.GetTime() ) ) );

    // ==== PrintedBy property ====
    const SfxStamp& rPrintedStamp = aDocInfoState.GetPrinted();
    ::rtl::OUString aStringPrintedBy = ::rtl::OUString::createFromAscii( "PrintedBy" );
    if ( rPrintedStamp.IsValid() )
        xPropSet->setPropertyValue( aStringPrintedBy, uno::makeAny( ::rtl::OUString( rPrintedStamp.GetName() ) ) );

    // ==== PrintDate property ====
    ::rtl::OUString aStringPrintDate = ::rtl::OUString::createFromAscii( "PrintDate" );
    if ( rPrintedStamp.IsValid() )
        xPropSet->setPropertyValue( aStringPrintDate,
                            uno::makeAny( SfxDocumentInfoObject::impl_DateTime_Object2Struct( rPrintedStamp.GetTime() ) ) );

    // ==== Template property ====
    ::rtl::OUString aStringTemplate = ::rtl::OUString::createFromAscii( "Template" );
    xPropSet->setPropertyValue( aStringTemplate, uno::makeAny( ::rtl::OUString( aDocInfoState.GetTemplateName() ) ) );

    // ==== TemplateDate property ====
    ::rtl::OUString aStringTemplateDate = ::rtl::OUString::createFromAscii( "TemplateDate" );
    xPropSet->setPropertyValue( aStringTemplateDate,
                    uno::makeAny( SfxDocumentInfoObject::impl_DateTime_Object2Struct( aDocInfoState.GetTemplateDate() ) ) );

    // ==== AutoloadURL property ====
    ::rtl::OUString aStringAutoloadURL = ::rtl::OUString::createFromAscii( "AutoloadURL" );
    xPropSet->setPropertyValue( aStringAutoloadURL, uno::makeAny( ::rtl::OUString( aDocInfoState.GetReloadURL() ) ) );

    // ==== AutoloadSecs property ====
    ::rtl::OUString aStringAutoloadSecs = ::rtl::OUString::createFromAscii( "AutoloadSecs" );
    xPropSet->setPropertyValue( aStringAutoloadSecs, uno::makeAny( aDocInfoState.GetReloadDelay() ) );

    // ==== DefaultTarget property ====
    ::rtl::OUString aStringDefaultTarget = ::rtl::OUString::createFromAscii( "DefaultTarget" );
    xPropSet->setPropertyValue( aStringDefaultTarget, uno::makeAny( ::rtl::OUString( aDocInfoState.GetDefaultTarget() ) ) );


    // now handle special properties
    sal_Int16 nCount = xDocInfo->getUserFieldCount();
    sal_Int16 nSupportedCount = aDocInfoState.GetUserKeyCount();
    OSL_ENSURE( nCount <= nSupportedCount, "Not all user property can be stored in sfx implementation!\n" );

    for ( sal_Int16 nInd = 0; nInd < nCount && nInd < nSupportedCount; nInd++ )
    {
        xDocInfo->setUserFieldName( nInd, aDocInfoState.GetUserKey( nInd ).GetTitle() );
        xDocInfo->setUserFieldValue( nInd, aDocInfoState.GetUserKey( nInd ).GetWord() );
    }

    // set the modified flag back if required
    if ( bNoModify && bIsModified != xModifiable->isModified() )
        xModifiable->setModified( bIsModified );
}

//-------------------------------------------------------------------------
// static
void SfxStoringHelper::ExecuteInfoDlg( const ::rtl::OUString& aTargetURL,
                                        const ::rtl::OUString& aTitle, const String& rBaseURL,
                                        SfxDocumentInfo &aDocInfo )
{
    // Itemset f"ur Dialog aufbereiten
    SfxDocumentInfoItem aDocInfoItem( aTargetURL, aDocInfo );
    SfxItemSet aSet( SFX_APP()->GetPool(), SID_DOCINFO, SID_DOCINFO,
                                        SID_EXPLORER_PROPS_START, SID_EXPLORER_PROPS_START, SID_BASEURL, SID_BASEURL,
                                        0L );
    aSet.Put( aDocInfoItem );
    aSet.Put( SfxStringItem( SID_EXPLORER_PROPS_START, aTitle ) );
    aSet.Put( SfxStringItem( SID_BASEURL, rBaseURL ) );

    // Dialog via Factory erzeugen und ausf"uhren
    SfxDocumentInfoDialog aDlg( 0, aSet );
    if ( RET_OK == aDlg.Execute() )
    {
        // neue DocInfo aus Dialog holen
        const SfxPoolItem *pItem = 0;
        if ( SFX_ITEM_SET == aDlg.GetOutputItemSet()->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            aDocInfo = (*(const SfxDocumentInfoItem *)pItem)();
        }
    }
}
//-------------------------------------------------------------------------
// static
sal_Bool SfxStoringHelper::WarnUnacceptableFormat( const uno::Reference< frame::XModel >& xModel,
                                                    ::rtl::OUString aOldUIName,
                                                    ::rtl::OUString aDefUIName,
                                                    sal_Bool bCanProceedFurther )
{
    if ( !SvtSaveOptions().IsWarnAlienFormat() )
        return sal_True;

    Window* pWin = 0;
    try {
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
    catch ( uno::Exception& )
    {
    }

    SfxAlienWarningDialog aDlg( pWin, aOldUIName );

    /*String aWarn;
    if ( bCanProceedFurther )
        aWarn = String(SfxResId(STR_QUERY_SAVEOWNFORMAT));
    else
        aWarn = String(SfxResId(STR_QUERY_MUSTOWNFORMAT));

    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FILENAME)" ), aCurrentName );
    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FORMAT)" ), aOldUIName );
    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FORMAT)" ), aOldUIName );
    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(OWNFORMAT)" ), aDefUIName );

    QueryBox aWarnBox( pWin, WB_YES_NO|WB_DEF_OK, aWarn );
    return ( aWarnBox.Execute() == RET_YES );*/
    return aDlg.Execute() == RET_OK;
}

// static
void SfxStoringHelper::ExecuteFilterDialog( SfxStoringHelper& _rStorageHelper
                                            ,const ::rtl::OUString& _sFilterName
                                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _xModel
                                            ,/*OUT*/::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgsSequence)
{
    ModelData_Impl aModelData( _rStorageHelper, _xModel, _rArgsSequence );
    if ( aModelData.ExecuteFilterDialog_Impl( _sFilterName ) )
        _rArgsSequence = aModelData.GetMediaDescr().getAsConstPropertyValueList();
}
