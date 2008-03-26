/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objstor.cxx,v $
 *
 *  $Revision: 1.206 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-26 10:44:56 $
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
#include "precompiled_sfx2.hxx"

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _ZCODEC_HXX //autogen
#include <tools/zcodec.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_FILTEROPTIONSREQUEST_HPP_
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XINTERACTIONFILTEROPTIONS_HPP_
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XOPTIMIZEDSTORAGE_HPP_
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_SECURITY_XDOCUMENTDIGITALSIGNATURES_HPP_
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#endif

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <comphelper/configurationhelper.hxx>

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _CACHESTR_HXX
#include <tools/cachestr.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ADDXMLTOSTORAGEOPTIONS_HXX
#include <svtools/addxmltostorageoptions.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#include <svtools/saveopt.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/docinfohelper.hxx>
#include <ucbhelper/content.hxx>
#include <sot/storinfo.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <shell/systemshell.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/string.hxx>
#include <vcl/bitmapex.hxx>
#include <svtools/embedhlp.hxx>
#include <rtl/logfile.hxx>
#include <basic/modsizeexceeded.hxx>

#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/request.hxx>
#include "sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include "fltfnc.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include "doc.hrc"
#include <sfx2/sfxsids.hrc>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include "openflag.hxx"
#include "helper.hxx"
#include <sfx2/filedlghelper.hxx>
#include <sfx2/event.hxx>
#include "fltoptint.hxx"
#include <sfx2/viewfrm.hxx>
#include "graphhelp.hxx"
#include "appbaslib.hxx"
#include "newerverwarn.hxx"
#include "appdata.hxx"

#include "../appl/app.hrc"

extern sal_uInt32 CheckPasswd_Impl( SfxObjectShell*, SfxItemPool&, SfxMedium* );

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::document;
using namespace ::rtl;
using namespace ::cppu;

namespace css = ::com::sun::star;

//=========================================================================
void impl_addToModelCollection(const css::uno::Reference< css::frame::XModel >& xModel)
{
    if (!xModel.is())
        return;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    css::uno::Reference< css::container::XSet > xModelCollection(
        xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster")),
        css::uno::UNO_QUERY);
    if (xModelCollection.is())
    {
        try
        {
            xModelCollection->insert(css::uno::makeAny(xModel));
        }
        catch ( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "The document seems to be in the collection already!\n" );
        }
    }
}

//=========================================================================

sal_Bool SfxObjectShell::Save()
{
    return SaveChildren();
}

//--------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveAs( SfxMedium& rMedium )
{
    return SaveAsChildren( rMedium );
}

//-------------------------------------------------------------------------

sal_Bool GetPasswd_Impl( const SfxItemSet* pSet, ::rtl::OUString& rPasswd )
{
    const SfxPoolItem* pItem = NULL;
    if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_PASSWORD, sal_True, &pItem ) )
    {
        DBG_ASSERT( pItem->IsA( TYPE(SfxStringItem) ), "wrong item type" );
        rPasswd = ( (const SfxStringItem*)pItem )->GetValue();
        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::NoDependencyFromManifest_Impl( const uno::Reference< embed::XStorage >& xStorage )
{
    uno::Sequence< ::rtl::OUString > aElements = xStorage->getElementNames();
    for ( sal_Int32 nInd = 0; nInd < aElements.getLength(); nInd++ )
    {
        if ( xStorage->isStorageElement( aElements[nInd] ) )
        {
            // if there are other standard elements that do not need manifest.xml the following
            // list can be extended
            if ( !aElements[nInd].equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pictures" ) ) )
              && !aElements[nInd].equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Configurations" ) ) )
              && !aElements[nInd].equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Configurations2" ) ) )
              && !aElements[nInd].equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Thumbnails" ) ) )
              && !aElements[nInd].equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Basic" ) ) ) )
            {
                // the substorage is not know as one that does not need manifest.xml
                return sal_False;
            }
        }
    }

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::PutURLContentsToVersionStream_Impl(
                                            ::rtl::OUString aURL,
                                            const uno::Reference< embed::XStorage >& xDocStorage,
                                            ::rtl::OUString aStreamName )
{
    sal_Bool bResult = sal_False;
    try
    {
        uno::Reference< embed::XStorage > xVersion = xDocStorage->openStorageElement(
                                                        ::rtl::OUString::createFromAscii( "Versions" ),
                                                        embed::ElementModes::READWRITE );

        DBG_ASSERT( xVersion.is(),
                "The method must throw an exception if the storage can not be opened!\n" );
        if ( !xVersion.is() )
            throw uno::RuntimeException();

        uno::Reference< io::XStream > xVerStream = xVersion->openStreamElement(
                                                                aStreamName,
                                                                embed::ElementModes::READWRITE );
        DBG_ASSERT( xVerStream.is(), "The method must throw an exception if the storage can not be opened!\n" );
        if ( !xVerStream.is() )
            throw uno::RuntimeException();

        uno::Reference< io::XOutputStream > xOutStream = xVerStream->getOutputStream();
        uno::Reference< io::XTruncate > xTrunc( xOutStream, uno::UNO_QUERY );

        DBG_ASSERT( xTrunc.is(), "The output stream must exist and implement XTruncate interface!\n" );
        if ( !xTrunc.is() )
            throw RuntimeException();

        uno::Reference< io::XInputStream > xTmpInStream =
            ::comphelper::OStorageHelper::GetInputStreamFromURL( aURL );
        DBG_ASSERT( xTmpInStream.is(), "The method must create the stream or throw an exception!\n" );
        if ( !xTmpInStream.is() )
            throw uno::RuntimeException();

        xTrunc->truncate();
        ::comphelper::OStorageHelper::CopyInputToOutput( xTmpInStream, xOutStream );
        xOutStream->closeOutput();

        uno::Reference< embed::XTransactedObject > xTransact( xVersion, uno::UNO_QUERY );
        DBG_ASSERT( xTransact.is(), "The storage must implement XTransacted interface!\n" );
        if ( xTransact.is() )
            xTransact->commit();

        bResult = sal_True;
    }
    catch( uno::Exception& )
    {
        // TODO/LATER: handle the error depending on exception
        SetError( ERRCODE_IO_GENERAL );
    }

    return bResult;
}

//-------------------------------------------------------------------------
::rtl::OUString SfxObjectShell::CreateTempCopyOfStorage_Impl( const uno::Reference< embed::XStorage >& xStorage )
{
    ::rtl::OUString aTempURL = ::utl::TempFile().GetURL();

    DBG_ASSERT( aTempURL.getLength(), "Can't create a temporary file!\n" );
    if ( aTempURL.getLength() )
    {
        try
        {
            uno::Reference< embed::XStorage > xTempStorage =
                ::comphelper::OStorageHelper::GetStorageFromURL( aTempURL, embed::ElementModes::READWRITE );

            // the password will be transfered from the xStorage to xTempStorage by storage implemetation
            xStorage->copyToStorage( xTempStorage );

            // the temporary storage was commited by the previous method and it will die by refcount
        }
        catch ( uno::Exception& )
        {
            DBG_ERROR( "Creation of a storage copy is failed!" );
            ::utl::UCBContentHelper::Kill( aTempURL );

            aTempURL = ::rtl::OUString();

            // TODO/LATER: may need error code setting based on exception
            SetError( ERRCODE_IO_GENERAL );
        }
    }

    return aTempURL;
}

//-------------------------------------------------------------------------
SvGlobalName SfxObjectShell::GetClassName() const
{
    return GetFactory().GetClassId();
}

//-------------------------------------------------------------------------
void SfxObjectShell::SetupStorage( const uno::Reference< embed::XStorage >& xStorage,
                                   sal_Int32 nVersion,
                                   sal_Bool bTemplate ) const
{
    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );

    if ( xProps.is() )
    {
        SvGlobalName aName;
        String aFullTypeName, aShortTypeName, aAppName;
        sal_uInt32 nClipFormat=0;

        FillClass( &aName, &nClipFormat, &aAppName, &aFullTypeName, &aShortTypeName, nVersion );
        if ( nClipFormat )
        {
            // basic doesn't have a ClipFormat
            // without MediaType the storage is not really usable, but currently the BasicIDE still
            // is an SfxObjectShell and so we can't take this as an error
            datatransfer::DataFlavor aDataFlavor;
            SotExchange::GetFormatDataFlavor( nClipFormat, aDataFlavor );
            if ( aDataFlavor.MimeType.getLength() )
            {
                if ( bTemplate )
                {
                    // TODO/LATER: this is a temporary solution for BETA to avoid incompatible change
                    if ( aDataFlavor.MimeType.equals( MIMETYPE_OASIS_OPENDOCUMENT_TEXT ) )
                        aDataFlavor.MimeType = MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE;
                    else if ( aDataFlavor.MimeType.equals( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING ) )
                        aDataFlavor.MimeType = MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE;
                    else if ( aDataFlavor.MimeType.equals( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION ) )
                        aDataFlavor.MimeType = MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE;
                    else if ( aDataFlavor.MimeType.equals( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET ) )
                        aDataFlavor.MimeType = MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE;
                    else if ( aDataFlavor.MimeType.equals( MIMETYPE_OASIS_OPENDOCUMENT_CHART ) )
                        aDataFlavor.MimeType = MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE;
                    else if ( aDataFlavor.MimeType.equals( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA ) )
                        aDataFlavor.MimeType = MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE;
                }

                try
                {
                    xProps->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ), uno::makeAny( aDataFlavor.MimeType ) );
                }
                catch( uno::Exception& )
                {
                    // TODO/LATER: ErrorHandling
                }
            }
        }
    }
}

//-------------------------------------------------------------------------
void SfxObjectShell::PrepareSecondTryLoad_Impl()
{
    // only for internal use
    pImp->m_xDocStorage = uno::Reference< embed::XStorage >();
    pImp->m_bIsInit = sal_False;
    ResetError();
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::GeneralInit_Impl( const uno::Reference< embed::XStorage >& xStorage,
                                            sal_Bool bTypeMustBeSetAlready )
{
    if ( pImp->m_bIsInit )
        return sal_False;

    pImp->m_bIsInit = sal_True;
    if ( xStorage.is() )
    {
        // no notification is required the storage is set the first time
        pImp->m_xDocStorage = xStorage;

        try {
            uno::Reference < beans::XPropertySet > xPropSet( xStorage, uno::UNO_QUERY_THROW );
            Any a = xPropSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) );
            ::rtl::OUString aMediaType;
            if ( !(a>>=aMediaType) || !aMediaType.getLength() )
            {
                if ( bTypeMustBeSetAlready )
                {
                    SetError( ERRCODE_IO_BROKENPACKAGE );
                    return sal_False;
                }

                SetupStorage( xStorage, SOFFICE_FILEFORMAT_CURRENT, sal_False );
            }
        }
        catch ( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Can't check storage's mediatype!\n" );
        }
    }
    else
        pImp->m_bCreateTempStor = sal_True;

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::InitNew( const uno::Reference< embed::XStorage >& xStorage )
{
    return GeneralInit_Impl( xStorage, sal_False );
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::Load( SfxMedium& rMedium )
{
    return GeneralInit_Impl( rMedium.GetStorage(), sal_True );
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::DoInitNew_Impl( const ::rtl::OUString& rName )

/*  [Beschreibung]
*/

{
    if ( rName.getLength() )
    {
        DBG_ERROR( "This code is intended to be removed, the caller part must be checked!\n" );
        return DoInitNew(0);
    }
    else
        return DoInitNew(0);
}


sal_Bool SfxObjectShell::DoInitNew( SfxMedium* pMed )
/*  [Beschreibung]

    Diese von SvPersist geerbte virtuelle Methode wird gerufen, um
    die SfxObjectShell-Instanz aus einem Storage (pStor != 0) bzw.
    (pStor == 0) ganz neu zu initialisieren.

    Wie alle Do...-Methoden liegt hier eine Steuerung vor, die eigentliche
    Implementierung erfolgt, indem die ebenfalls virtuellen Methode
    InitNew(SvStorate*) von der SfxObjectShell-Subclass implementiert wird.

    F"ur pStor == 0 wird ein die SfxObjectShell-Instanz mit einem leeren
    SfxMedium verbunden, sonst mit einem SfxMedium, welches auf den
    als Parameter "ubergeben SvStorage verweist.

    Erst nach InitNew() oder Load() ist das Objekt korrekt initialisiert.

    [R"uckgabewert]
    sal_True            Das Objekt wurde initialisiert.
    sal_False           Das Objekt konnte nicht initialisiert werden
*/

{
    ModifyBlocker_Impl aBlock( this );
    pMedium = pMed;
    if ( !pMedium )
    {
        bIsTmp = sal_True;
        pMedium = new SfxMedium;
    }

    pMedium->CanDisposeStorage_Impl( sal_True );

    if ( InitNew( pMed ? pMed->GetStorage() : uno::Reference < embed::XStorage >() ) )
    {
        // empty documents always get their macros from the user, so there is no reason to restrict access
        pImp->aMacroMode.allowMacroExecution();
        if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
            SetTitle( String( SfxResId( STR_NONAME ) ));

        uno::Reference< frame::XModel >  xModel ( GetModel(), uno::UNO_QUERY );
        if ( xModel.is() )
        {
            SfxItemSet *pSet = GetMedium()->GetItemSet();
            uno::Sequence< beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pSet, aArgs );
            sal_Int32 nLength = aArgs.getLength();
            aArgs.realloc( nLength + 1 );
            aArgs[nLength].Name = DEFINE_CONST_UNICODE("Title");
            aArgs[nLength].Value <<= ::rtl::OUString( GetTitle( SFX_TITLE_DETECT ) );
            xModel->attachResource( ::rtl::OUString(), aArgs );
            impl_addToModelCollection(xModel);
        }

        pImp->bInitialized = sal_True;
        SetActivateEvent_Impl( SFX_EVENT_CREATEDOC );
        SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_DOCCREATED, this ) );
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ImportFromGeneratedStream_Impl(
                    const uno::Reference< io::XStream >& xStream,
                    const uno::Sequence< beans::PropertyValue >& aMediaDescr )
{
    if ( !xStream.is() )
        return sal_False;

    if ( pMedium && pMedium->HasStorage_Impl() )
        pMedium->CloseStorage();

    sal_Bool bResult = sal_False;

    try
    {
        uno::Reference< embed::XStorage > xStorage =
            ::comphelper::OStorageHelper::GetStorageFromStream( xStream, embed::ElementModes::READWRITE );

        if ( !xStorage.is() )
            throw uno::RuntimeException();

        if ( !pMedium )
            pMedium = new SfxMedium( xStorage, String() );
        else
            pMedium->SetStorage_Impl( xStorage );

        SfxAllItemSet aSet( SFX_APP()->GetPool() );
        TransformParameters( SID_OPENDOC, aMediaDescr, aSet );
        pMedium->GetItemSet()->Put( aSet );
        pMedium->CanDisposeStorage_Impl( sal_False );

        // allow the subfilter to reinit the model
        if ( pImp->m_bIsInit )
            pImp->m_bIsInit = sal_False;

        if ( LoadOwnFormat( *pMedium ) )
        {
            bHasName = sal_True;
            if ( !IsReadOnly() && IsLoadReadonly() )
                SetReadOnlyUI();

            bResult = sal_True;
            OSL_ENSURE( pImp->m_xDocStorage == xStorage, "Wrong storage is used!\n" );
        }

        // now the medium can be disconnected from the storage
        // the medium is not allowed to dispose the storage so CloseStorage() can be used
        pMedium->CloseStorage();
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoLoad( SfxMedium *pMed )
{
    ModifyBlocker_Impl aBlock( this );

    if ( SFX_CREATE_MODE_EMBEDDED != eCreateMode )
        GetpApp()->ShowStatusText( SfxResId(STR_DOC_LOADING) );

    pMedium = pMed;
    pMedium->CanDisposeStorage_Impl( sal_True );

    sal_Bool bOk = sal_False;
    const SfxFilter* pFilter = pMed->GetFilter();
    SfxItemSet* pSet = pMedium->GetItemSet();
    if( !pImp->nEventId )
    {
        SFX_ITEMSET_ARG(
            pSet, pTemplateItem, SfxBoolItem,
            SID_TEMPLATE, sal_False);
        SetActivateEvent_Impl(
            ( pTemplateItem && pTemplateItem->GetValue() )
            ? SFX_EVENT_CREATEDOC : SFX_EVENT_OPENDOC );
    }


    SFX_ITEMSET_ARG( pSet, pBaseItem, SfxStringItem,
                     SID_BASEURL, sal_False);
    String aBaseURL;
    SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if( pBaseItem )
        aBaseURL = pBaseItem->GetValue();
    else
    {
        if ( pSalvageItem )
        {
            String aName( pMed->GetPhysicalName() );
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aBaseURL );
        }
        else
            aBaseURL = pMed->GetBaseURL();
    }

    pImp->nLoadedFlags = 0;
    pImp->bModelInitialized = sal_False;

    //TODO/LATER: make a clear strategy how to handle "UsesStorage" etc.
    sal_Bool bOwnStorageFormat = IsOwnStorageFormat_Impl( *pMedium );
    sal_Bool bHasStorage = IsPackageStorageFormat_Impl( *pMedium );
    if ( pMedium->GetFilter() )
    {
        sal_uInt32 nError = HandleFilter( pMedium, this );
        if ( nError != ERRCODE_NONE )
            SetError( nError );
    }

    EnableSetModified( sal_False );

    if ( GetError() == ERRCODE_NONE && bOwnStorageFormat && ( !pFilter || !( pFilter->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) ) )
    {
        uno::Reference< embed::XStorage > xStorage = pMed->GetStorage();
        if( xStorage.is() && pMed->GetLastStorageCreationState() == ERRCODE_NONE )
        {
            DBG_ASSERT( pFilter, "No filter for storage found!" );

            try
            {
                sal_Bool bWarnMediaTypeFallback = sal_False;
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pRepairPackageItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False);

                // treat the package as broken if the mediatype was retrieved as a fallback
                uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );
                xStorProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaTypeFallbackUsed" ) ) )
                                                                    >>= bWarnMediaTypeFallback;

                if ( bWarnMediaTypeFallback && pRepairPackageItem && pRepairPackageItem->GetValue() )
                {
                    // the mediatype was retrieved by using fallback solution but this is a repairing mode
                    // so it is acceptable to open the document if there is no contents that required manifest.xml
                    bWarnMediaTypeFallback = sal_False; //!NoDependencyFromManifest_Impl( xStorage );
                }

                if ( bWarnMediaTypeFallback || !xStorage->getElementNames().getLength() )
                    SetError( ERRCODE_IO_BROKENPACKAGE );
            }
            catch( uno::Exception& )
            {
                // TODO/LATER: may need error code setting based on exception
                SetError( ERRCODE_IO_GENERAL );
            }

            // Load
            if ( !GetError() )
            {
                pImp->nLoadedFlags = 0;
                pImp->bModelInitialized = sal_False;
                bOk = xStorage.is() && LoadOwnFormat( *pMed );
                if ( bOk )
                {
                    // the document loaded from template has no name
                    SFX_ITEMSET_ARG( pMedium->GetItemSet(), pTemplateItem, SfxBoolItem, SID_TEMPLATE, sal_False);
                    if ( !pTemplateItem || !pTemplateItem->GetValue() )
                        bHasName = sal_True;

                    if ( !IsReadOnly() && IsLoadReadonly() )
                        SetReadOnlyUI();
                }
                else
                    SetError( ERRCODE_ABORT );
            }
        }
        else
            SetError( pMed->GetLastStorageCreationState() );
    }
    else if ( GetError() == ERRCODE_NONE && InitNew(0) )
    {
        // Name vor ConvertFrom setzen, damit GetSbxObject() schon funktioniert
        bHasName = sal_True;
        SetName( SfxResId( STR_NONAME ) );

        if( !bHasStorage )
            pMedium->GetInStream();
        else
            pMedium->GetStorage();

        if ( GetError() == ERRCODE_NONE )
        {
            pImp->nLoadedFlags = 0;
            pImp->bModelInitialized = sal_False;
            if ( pMedium->GetFilter() && ( pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
            {
                uno::Reference < beans::XPropertySet > xSet( GetModel(), uno::UNO_QUERY );
                ::rtl::OUString sLockUpdates(::rtl::OUString::createFromAscii("LockUpdates"));
                bool bSetProperty = true;
                try
                {
                    xSet->setPropertyValue( sLockUpdates, makeAny( (sal_Bool) sal_True ) );
                }
                catch(const beans::UnknownPropertyException& )
                {
                    bSetProperty = false;
                }
                bOk = ImportFrom(*pMedium);
                if(bSetProperty)
                {
                    try
                    {
                        xSet->setPropertyValue( sLockUpdates, makeAny( (sal_Bool) sal_False ) );
                    }
                    catch(const beans::UnknownPropertyException& )
                    {}
                }
                UpdateLinks();
                FinishedLoading( SFX_LOADED_ALL );
            }
            else
            {
                bOk = ConvertFrom(*pMedium);
                InitOwnModel_Impl();
            }
        }
    }

    if ( bOk )
    {
        try
        {
            ::ucbhelper::Content aContent( pMedium->GetName(), com::sun::star::uno::Reference < XCommandEnvironment >() );
            com::sun::star::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                ::rtl::OUString aAuthor( RTL_CONSTASCII_USTRINGPARAM("Author") );
                ::rtl::OUString aKeywords( RTL_CONSTASCII_USTRINGPARAM("Keywords") );
                ::rtl::OUString aSubject( RTL_CONSTASCII_USTRINGPARAM("Subject") );
                Any aAny;
                ::rtl::OUString aValue;
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();
                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aAny = aContent.getPropertyValue( aAuthor );
                    if ( ( aAny >>= aValue ) )
                        xDocProps->setAuthor(aValue);
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    aAny = aContent.getPropertyValue( aKeywords );
                    if ( ( aAny >>= aValue ) )
                        xDocProps->setKeywords(
                          ::comphelper::string::convertCommaSeparated(aValue));
;
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aAny = aContent.getPropertyValue( aSubject );
                    if ( ( aAny >>= aValue ) ) {
                        xDocProps->setSubject(aValue);
                    }
                }
            }
        }
        catch( Exception& )
        {
        }

        // Falls nicht asynchron geladen wird selbst FinishedLoading aufrufen
        if ( !( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) &&
            ( !pMedium->GetFilter() || pMedium->GetFilter()->UsesStorage() )
            )
            FinishedLoading( SFX_LOADED_MAINDOCUMENT );

        if( IsOwnStorageFormat_Impl(*pMed) && pMed->GetFilter() )
        {
//???? dv           DirEntry aDirEntry( pMed->GetPhysicalName() );
//???? dv           SetFileName( aDirEntry.GetFull() );
        }
        Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );

        if ( SFX_CREATE_MODE_EMBEDDED != eCreateMode )
        {
            GetpApp()->HideStatusText();

            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pAsTempItem, SfxBoolItem, SID_TEMPLATE, sal_False);
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pPreviewItem, SfxBoolItem, SID_PREVIEW, sal_False);
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False);
            if( bOk && pMedium->GetOrigURL().Len()
            && !( pAsTempItem && pAsTempItem->GetValue() )
            && !( pPreviewItem && pPreviewItem->GetValue() )
            && !( pHiddenItem && pHiddenItem->GetValue() ) )
            {
                INetURLObject aUrl( pMedium->GetOrigURL() );

                if ( aUrl.GetProtocol() == INET_PROT_FILE )
                {
                    const SfxFilter* pOrgFilter = pMedium->GetOrigFilter();
                    SystemShell::AddToRecentDocumentList(
                        aUrl.GetURLNoPass( INetURLObject::NO_DECODE ),
                        (pOrgFilter) ? pOrgFilter->GetMimeType() : String() );
                }
            }
        }

        // --> PB 2007-11-20 #i83653# Give warning if document contains features from new ODF version
        // Online Update Check enabled?
        sal_Bool bOnlineUpdateEnabled = sal_False;
        uno::Any aVal;
        uno::Reference< lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
        try
        {
            uno::Reference < container::XNameAccess > xNameAccess(
                xSMGR->createInstance( DEFINE_CONST_UNICODE("com.sun.star.setup.UpdateCheckConfig") ),
                uno::UNO_QUERY );
            if ( xNameAccess.is() )
            {
                aVal = xNameAccess->getByName( DEFINE_CONST_UNICODE("AutoCheckEnabled") );
                aVal >>= bOnlineUpdateEnabled;
            }
        }
        catch( const uno::Exception& )
        {
            // property "AutoCheckEnabled" not set
        }

        if ( pMedium->GetInteractionHandler().is() && !SFX_APP()->Get_Impl()->bODFVersionWarningLater )
        {
            // scan the generator string (within meta.xml)
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps
                = xDPS->getDocumentProperties();
            if ( xDocProps.is() )
            {
                uno::Reference<beans::XPropertySet> xUserDefinedProps(
                    xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
                //FIXME: this should be removed before 3.0!
                // the new property "ODFVersion" is a temporary solution for OOo2.4
                uno::Any aAny;
                try
                {
                    aAny = xUserDefinedProps->getPropertyValue(
                            DEFINE_CONST_UNICODE("ODFVersion"));
                }
                catch( const uno::Exception& )
                {
                    // Custom Property "ODFVersion" does not exist
                }

                ::rtl::OUString sTemp;
                if ( (aAny >>= sTemp) && sTemp.getLength() )
                {
                    double nVersion = sTemp.toDouble();
                    if ( nVersion > 1.10001 )
                    {
                        // ODF version greater than 1.1 - added some decimal places to be safe against floating point conversion errors (hack)
                        sfx2::NewerVersionWarningDialog aDlg( NULL );
                        aDlg.Execute();
                    }
                }
            }
        }
    }
    else
        GetpApp()->HideStatusText();

    return bOk;
}

sal_uInt32 SfxObjectShell::HandleFilter( SfxMedium* pMedium, SfxObjectShell* pDoc )
{
    sal_uInt32 nError = ERRCODE_NONE;
    SfxItemSet* pSet = pMedium->GetItemSet();
    SFX_ITEMSET_ARG( pSet, pOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
    SFX_ITEMSET_ARG( pSet, pData, SfxUnoAnyItem, SID_FILTER_DATA, sal_False );
    if ( !pData && !pOptions )
    {
        com::sun::star::uno::Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        com::sun::star::uno::Reference< XNameAccess > xFilterCFG;
        if( xServiceManager.is() )
        {
            xFilterCFG = com::sun::star::uno::Reference< XNameAccess >(
                xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
                UNO_QUERY );
        }

        if( xFilterCFG.is() )
        {
            BOOL bAbort = FALSE;
            try {
                const SfxFilter* pFilter = pMedium->GetFilter();
                Sequence < PropertyValue > aProps;
                Any aAny = xFilterCFG->getByName( pFilter->GetName() );
                if ( aAny >>= aProps )
                {
                    sal_Int32 nPropertyCount = aProps.getLength();
                    for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                        if( aProps[nProperty].Name.equals( ::rtl::OUString::createFromAscii("UIComponent")) )
                        {
                            ::rtl::OUString aServiceName;
                            aProps[nProperty].Value >>= aServiceName;
                            if( aServiceName.getLength() )
                            {
                                com::sun::star::uno::Reference< XInteractionHandler > rHandler = pMedium->GetInteractionHandler();
                                if( rHandler.is() )
                                {
                                    // we need some properties in the media descriptor, so we have to make sure that they are in
                                    Any aStreamAny;
                                    aStreamAny <<= pMedium->GetInputStream();
                                    if ( pSet->GetItemState( SID_INPUTSTREAM ) < SFX_ITEM_SET )
                                        pSet->Put( SfxUnoAnyItem( SID_INPUTSTREAM, aStreamAny ) );
                                    if ( pSet->GetItemState( SID_FILE_NAME ) < SFX_ITEM_SET )
                                        pSet->Put( SfxStringItem( SID_FILE_NAME, pMedium->GetName() ) );
                                    if ( pSet->GetItemState( SID_FILTER_NAME ) < SFX_ITEM_SET )
                                        pSet->Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );

                                    Sequence< PropertyValue > rProperties;
                                    TransformItems( SID_OPENDOC, *pSet, rProperties, NULL );
                                    RequestFilterOptions* pFORequest = new RequestFilterOptions( pDoc->GetModel(), rProperties );

                                    com::sun::star::uno::Reference< XInteractionRequest > rRequest( pFORequest );
                                    rHandler->handle( rRequest );

                                    if ( !pFORequest->isAbort() )
                                    {
                                           SfxAllItemSet aNewParams( pDoc->GetPool() );
                                           TransformParameters( SID_OPENDOC,
                                                             pFORequest->getFilterOptions(),
                                                             aNewParams,
                                                             NULL );

                                           SFX_ITEMSET_ARG( &aNewParams,
                                                         pFilterOptions,
                                                         SfxStringItem,
                                                         SID_FILE_FILTEROPTIONS,
                                                         sal_False );
                                           if ( pFilterOptions )
                                               pSet->Put( *pFilterOptions );

                                           SFX_ITEMSET_ARG( &aNewParams,
                                                         pFilterData,
                                                         SfxUnoAnyItem,
                                                         SID_FILTER_DATA,
                                                         sal_False );
                                           if ( pFilterData )
                                               pSet->Put( *pFilterData );
                                    }
                                    else
                                        bAbort = TRUE;
                                }
                            }

                            break;
                        }
                }

                if( bAbort )
                {
                    // filter options were not entered
                    nError = ERRCODE_ABORT;
                }
            }
            catch( NoSuchElementException& )
            {
                // the filter name is unknown
                nError = ERRCODE_IO_INVALIDPARAMETER;
            }
            catch( Exception& )
            {
                nError = ERRCODE_ABORT;
            }
        }
    }

    return nError;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsOwnStorageFormat_Impl(const SfxMedium &rMedium) const
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->IsOwnFormat() &&
             rMedium.GetFilter()->UsesStorage() &&
             rMedium.GetFilter()->GetVersion() >= SOFFICE_FILEFORMAT_60 );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsPackageStorageFormat_Impl(const SfxMedium &rMedium) const
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->UsesStorage() &&
             rMedium.GetFilter()->GetVersion() >= SOFFICE_FILEFORMAT_60 );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSave()
// DoSave wird nur noch ueber OLE aufgerufen. Sichern eigener Dokumente im SFX
// laeuft uber DoSave_Impl, um das Anlegen von Backups zu ermoeglichen.
// Save in eigenes Format jetzt auch wieder Hierueber
{
    sal_Bool bOk = sal_False ;
    {
        ModifyBlocker_Impl aBlock( this );

        pImp->bIsSaving = sal_True;

        ::rtl::OUString aPasswd;
        if ( IsPackageStorageFormat_Impl( *GetMedium() ) )
        {
            if ( GetPasswd_Impl( GetMedium()->GetItemSet(), aPasswd ) )
            {
                try
                {
                    //TODO/MBA: GetOutputStorage?! Special mode, because it's "Save"?!
                    ::comphelper::OStorageHelper::SetCommonStoragePassword( GetMedium()->GetStorage(), aPasswd );
                    bOk = sal_True;
                }
                catch( uno::Exception& )
                {
                    SetError( ERRCODE_IO_GENERAL );
                }

                DBG_ASSERT( bOk, "The root storage must allow to set common password!\n" );
            }
            else
                bOk = sal_True;

            if ( HasBasic() )
            {
                try
                {
                    // The basic and dialogs related contents are still not able to proceed with save operation ( saveTo only )
                    // so since the document storage is locked a workaround has to be used

                    uno::Reference< embed::XStorage > xTmpStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                    DBG_ASSERT( xTmpStorage.is(), "If a storage can not be created an exception must be thrown!\n" );
                    if ( !xTmpStorage.is() )
                        throw uno::RuntimeException();

                    ::rtl::OUString aBasicStorageName( RTL_CONSTASCII_USTRINGPARAM( "Basic" ) );
                    ::rtl::OUString aDialogsStorageName( RTL_CONSTASCII_USTRINGPARAM( "Dialogs" ) );
                    if ( GetMedium()->GetStorage()->hasByName( aBasicStorageName ) )
                        GetMedium()->GetStorage()->copyElementTo( aBasicStorageName, xTmpStorage, aBasicStorageName );
                    if ( GetMedium()->GetStorage()->hasByName( aDialogsStorageName ) )
                        GetMedium()->GetStorage()->copyElementTo( aDialogsStorageName, xTmpStorage, aDialogsStorageName );

                    GetBasicManager();

                    // disconnect from the current storage
                    pImp->pBasicManager->setStorage( xTmpStorage );

                    // store to the current storage
                    pImp->pBasicManager->storeLibrariesToStorage( GetMedium()->GetStorage() );

                    // connect to the current storage back
                    pImp->pBasicManager->setStorage( GetMedium()->GetStorage() );
                }
                catch( uno::Exception& )
                {
                    SetError( ERRCODE_IO_GENERAL );
                    bOk = sal_False;
                }
            }
        }

        if ( bOk )
            bOk = Save();

        bOk = pMedium->Commit();
    }

//#88046
//    if ( bOk )
//        SetModified( sal_False );
    return bOk;
}

void Lock_Impl( SfxObjectShell* pDoc, BOOL bLock )
{
    SfxViewFrame *pFrame= SfxViewFrame::GetFirst( pDoc );
    while ( pFrame )
    {
        pFrame->GetDispatcher()->Lock( bLock );
        pFrame->Enable( !bLock );
        pFrame = SfxViewFrame::GetNext( *pFrame, pDoc );
    }

}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveTo_Impl
(
     SfxMedium &rMedium, // Medium, in das gespeichert werden soll
     const SfxItemSet* pSet
)

/*  [Beschreibung]

    Schreibt den aktuellen Inhalt in das Medium rMedium.
    Ist das Zielmedium kein Storage, so wird ueber ein temporaeres
    Medium gespeichert, sonst direkt, da das Medium transacted
    geschaltet ist, wenn wir es selbst geoeffnet haben und falls wir
    Server sind entweder der Container einen transacted Storage zur
    Verfuegung stellt oder selbst einen temporaeren Storage erzeugt hat.
*/

{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveTo_Impl" );

    ModifyBlocker_Impl aMod(this);

    const SfxFilter *pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        // if no filter was set, use the default filter
        // this should be changed in the feature, it should be an error!
        DBG_ERROR("No filter set!");
        pFilter = GetFactory().GetFilterContainer()->GetAnyFilter( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT );
        rMedium.SetFilter(pFilter);
    }

    sal_Bool bStorageBasedSource = IsPackageStorageFormat_Impl( *pMedium );
    sal_Bool bStorageBasedTarget = IsPackageStorageFormat_Impl( rMedium );
    sal_Bool bOwnSource = IsOwnStorageFormat_Impl( *pMedium );
    sal_Bool bOwnTarget = IsOwnStorageFormat_Impl( rMedium );

    // Examine target format to determine whether to query if any password
    // protected libraries exceed the size we can handler
    if ( bOwnTarget && !QuerySaveSizeExceededModules_Impl( rMedium.GetInteractionHandler() ) )
    {
        SetError( ERRCODE_IO_ABORT );
        return sal_False;
    }

    sal_Bool bNeedsDisconnectionOnFail = sal_False;

    sal_Bool bStoreToSameLocation = sal_False;

    // the detection whether the script is changed should be done before saving
    sal_Bool bTryToPreservScriptSignature = sal_False;
    if ( bOwnSource && bOwnTarget
      && ( pImp->nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_OK
        || pImp->nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED
        || pImp->nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_INVALID ) )
    {
        // the checking of the library modified state iterates over the libraries, should be done only when required
        bTryToPreservScriptSignature = !pImp->pBasicManager->isAnyContainerModified();
    }

    sal_Bool bCopyTo = sal_False;
    SfxItemSet *pMedSet = rMedium.GetItemSet();
    if( pMedSet )
    {
        SFX_ITEMSET_ARG( pMedSet, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
        bCopyTo =   GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
                    pSaveToItem && pSaveToItem->GetValue();
    }

    // use UCB for case sensitive/insensitive file name comparison
    if ( pMedium
      && pMedium->GetName().CompareIgnoreCaseToAscii( "private:stream", 14 ) != COMPARE_EQUAL
      && rMedium.GetName().CompareIgnoreCaseToAscii( "private:stream", 14 ) != COMPARE_EQUAL
      && SfxMedium::EqualURLs( pMedium->GetName(), rMedium.GetName() ) )
    {
        bStoreToSameLocation = sal_True;

        if ( bCopyTo && GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        {
            // export to the same location is vorbidden
            SetError( ERRCODE_IO_CANTWRITE );
        }
        else
        {
            // before we overwrite the original file, we will make a backup if there is a demand for that
            // if the backup is not created here it will be created internally and will be removed in case of successful saving
            const sal_Bool bDoBackup = SvtSaveOptions().IsBackup();
            if ( bDoBackup )
            {
                rMedium.DoBackup_Impl();
                if ( rMedium.GetError() )
                {
                    SetError( rMedium.GetErrorCode() );
                    rMedium.ResetError();
                }
            }

            if ( bStorageBasedSource && bStorageBasedTarget )
            {
                // The active storage must be switched. The simple saving is not enough.
                // The problem is that the target medium contains target MediaDescriptor.

                    // In future the switch of the persistance could be done on stream level:
                    // a new wrapper service will be implemented that allows to exchange
                    // persistance on the fly. So the real persistance will be set
                    // to that stream only after successful commit of the storage.
                    // TODO/LATER:
                    // create wrapper stream based on the URL
                    // create a new storage based on this stream
                    // store to this new storage
                    // commit the new storage
                    // call saveCompleted based with this new storage ( get rid of old storage and "frees" URL )
                    // commit the wrapper stream ( the stream will connect the URL only on commit, after that it will hold it )
                    // if the last step is failed the stream should stay to be transacted and should be commited on any flush
                    // so we can forget the stream in any way and the next storage commit will flush it
                bNeedsDisconnectionOnFail = DisconnectStorage_Impl(
                    *pMedium, rMedium );
                if ( bNeedsDisconnectionOnFail
                  || ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                {
                    pMedium->CloseAndRelease();

                    // TODO/LATER: for now the medium must be closed since it can already contain streams from old medium
                    //             in future those streams should not be copied in case a valid target url is provided,
                    //             if the url is not provided ( means the document is based on a stream ) this code is not
                    //             reachable.
                    rMedium.CloseAndRelease();
                    rMedium.GetOutputStorage();
                }
            }
            else if ( !bStorageBasedSource && !bStorageBasedTarget )
            {
                // the source and the target formats are alien
                // just disconnect the stream from the source format
                // so that the target medium can use it
                pMedium->CloseAndRelease();
                rMedium.CloseAndRelease();
                rMedium.CreateTempFileNoCopy();
                rMedium.GetOutStream();
            }
            else if ( !bStorageBasedSource && bStorageBasedTarget )
            {
                // the source format is an alien one but the target
                // format is an own one so just disconnect the source
                // medium
                pMedium->CloseAndRelease();
                rMedium.CloseAndRelease();
                rMedium.GetOutputStorage();
            }
            else // means if ( bStorageBasedSource && !bStorageBasedTarget )
            {
                // the source format is an own one but the target is
                // an alien format, just connect the source to temporary
                // storage
                bNeedsDisconnectionOnFail = DisconnectStorage_Impl(
                    *pMedium, rMedium );
                if ( bNeedsDisconnectionOnFail
                  || ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                {
                    pMedium->CloseAndRelease();
                    rMedium.CloseAndRelease();
                    rMedium.CreateTempFileNoCopy();
                    rMedium.GetOutStream();
                }
            }
        }
    }

    // TODO/LATER: error handling
    if( rMedium.GetErrorCode() || pMedium->GetErrorCode() || GetErrorCode() )
        return sal_False;

    if ( bStorageBasedTarget )
    {
        rMedium.GetOutputStorage();

        // If the filter is a "cross export" filter ( f.e. a filter for exporting an impress document from
        // a draw document ), the ClassId of the destination storage is different from the ClassId of this
        // document. It can be retrieved from the default filter for the desired target format
        long nFormat = rMedium.GetFilter()->GetFormat();
        SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        const SfxFilter *pFilt = rMatcher.GetFilter4ClipBoardId( nFormat );
        if ( pFilt )
        {
            if ( pFilt->GetServiceName() != rMedium.GetFilter()->GetServiceName() )
            {
//REMOVE            rMedium.GetStorage()->SetClass( SvFactory::GetServerName( nFormat ), nFormat, pFilt->GetTypeName() );
                datatransfer::DataFlavor aDataFlavor;
                SotExchange::GetFormatDataFlavor( nFormat, aDataFlavor );

                try
                {
                    uno::Reference< beans::XPropertySet > xProps( rMedium.GetStorage(), uno::UNO_QUERY );
                    DBG_ASSERT( xProps.is(), "The storage implementation must implement XPropertySet!" );
                    if ( !xProps.is() )
                        throw uno::RuntimeException();

                    xProps->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ),
                                            uno::makeAny( aDataFlavor.MimeType ) );
                }
                catch( uno::Exception& )
                {
                }
            }
        }
    }

    // TODO/LATER: error handling
    if( rMedium.GetErrorCode() || pMedium->GetErrorCode() || GetErrorCode() )
        return sal_False;

    sal_Bool bOldStat = pImp->bForbidReload;
    pImp->bForbidReload = sal_True;

    // lock user interface while saving the document
    Lock_Impl( this, sal_True );

    sal_Bool bOk = sal_False;
    // TODO/LATER: get rid of bOk

    if( bOwnTarget && !( pFilter->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
    {
        uno::Reference< embed::XStorage > xMedStorage = rMedium.GetStorage();
        if ( !xMedStorage.is() )
        {
            // no saving without storage, unlock UI and return
            Lock_Impl( this, sal_False );
            pImp->bForbidReload = bOldStat;
            return sal_False;
        }

        // transfer password from the parameters to the storage
        ::rtl::OUString aPasswd;
        sal_Bool bPasswdProvided = sal_False;
        if ( GetPasswd_Impl( rMedium.GetItemSet(), aPasswd ) )
        {
            bPasswdProvided = sal_True;
            try {
                ::comphelper::OStorageHelper::SetCommonStoragePassword( xMedStorage, aPasswd );
                bOk = sal_True;
            }
            catch( uno::Exception& )
            {
                DBG_ERROR( "Setting of common encryption key failed!" );
                SetError( ERRCODE_IO_GENERAL );
            }
        }
        else
            bOk = sal_True;

        pFilter = rMedium.GetFilter();

        const SfxStringItem *pVersionItem = pSet ? (const SfxStringItem*)
                SfxRequest::GetItem( pSet, SID_DOCINFO_COMMENTS, sal_False, TYPE(SfxStringItem) ) : NULL;
        ::rtl::OUString aTmpVersionURL;

        if ( bOk )
        {
            bOk = sal_False;
            // currently the case that the storage is the same should be impossible
            if ( xMedStorage == GetStorage() )
            {
                OSL_ENSURE( !pVersionItem, "This scenario is impossible currently!\n" );
                // usual save procedure
                bOk = Save();
            }
            else
            {
                // save to target
                bOk = SaveAsOwnFormat( rMedium );
                if ( bOk && pVersionItem )
                {
                    aTmpVersionURL = CreateTempCopyOfStorage_Impl( xMedStorage );
                    bOk = ( aTmpVersionURL.getLength() > 0 );
                }
            }
        }


        if ( bOk && GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        {
            // store the thumbnail representation image
            // TODO: handle the case when document is encrypted and/or signed
            if ( !GenerateAndStoreThumbnail( bPasswdProvided,
                                            sal_False,
                                            pFilter->IsOwnTemplateFormat(),
                                            xMedStorage ) )
            {
                // TODO: error handling
                OSL_ENSURE( sal_False, "Couldn't store thumbnail representation!" );
            }
        }

        if ( bOk )
        {
            if ( pImp->bIsSaving || pImp->bPreserveVersions )
            {
                try
                {
                    Sequence < util::RevisionTag > aVersions = rMedium.GetVersionList();
                    if ( aVersions.getLength() )
                    {
                        // copy the version streams
                        ::rtl::OUString aVersionsName( RTL_CONSTASCII_USTRINGPARAM( "Versions" ) );
                        uno::Reference< embed::XStorage > xNewVerStor = xMedStorage->openStorageElement(
                                                        aVersionsName,
                                                        embed::ElementModes::READWRITE );
                        uno::Reference< embed::XStorage > xOldVerStor = GetStorage()->openStorageElement(
                                                        aVersionsName,
                                                        embed::ElementModes::READ );
                        if ( !xNewVerStor.is() || !xOldVerStor.is() )
                            throw uno::RuntimeException();

                        for ( sal_Int32 n=0; n<aVersions.getLength(); n++ )
                        {
                            if ( xOldVerStor->hasByName( aVersions[n].Identifier ) )
                                xOldVerStor->copyElementTo( aVersions[n].Identifier, xNewVerStor, aVersions[n].Identifier );
                        }

                        uno::Reference< embed::XTransactedObject > xTransact( xNewVerStor, uno::UNO_QUERY );
                        if ( xTransact.is() )
                            xTransact->commit();
                    }
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "Couldn't copy versions!\n" );
                    bOk = sal_False;
                    // TODO/LATER: a specific error could be set
                }
            }

            if ( bOk && pVersionItem )
            {
                // store a version also
                const SfxStringItem *pAuthorItem = pSet ? (const SfxStringItem*)
                    SfxRequest::GetItem( pSet, SID_DOCINFO_AUTHOR, sal_False, TYPE(SfxStringItem) ) : NULL;

                // version comment
                util::RevisionTag aInfo;
                aInfo.Comment = pVersionItem->GetValue();

                // version author
                String aAuthor;
                if ( pAuthorItem )
                    aInfo.Author = pAuthorItem->GetValue();
                else
                    // if not transferred as a parameter, get it from user settings
                    aInfo.Author = SvtUserOptions().GetFullName();

                DateTime aTime;
                aInfo.TimeStamp.Day = aTime.GetDay();
                aInfo.TimeStamp.Month = aTime.GetMonth();
                aInfo.TimeStamp.Year = aTime.GetYear();
                aInfo.TimeStamp.Hours = aTime.GetHour();
                aInfo.TimeStamp.Minutes = aTime.GetMin();
                aInfo.TimeStamp.Seconds = aTime.GetSec();

                if ( bOk )
                {
                    // add new version information into the versionlist and save the versionlist
                    // the version list must have been transferred from the "old" medium before
                    rMedium.AddVersion_Impl( aInfo );
                    rMedium.SaveVersionList_Impl( sal_True );
                    bOk = PutURLContentsToVersionStream_Impl( aTmpVersionURL, xMedStorage, aInfo.Identifier );
                }
            }
            else if ( bOk && ( pImp->bIsSaving || pImp->bPreserveVersions ) )
            {
                rMedium.SaveVersionList_Impl( sal_True );
            }
        }

        if ( aTmpVersionURL.getLength() )
            ::utl::UCBContentHelper::Kill( aTmpVersionURL );
    }
    else
    {
        // it's a "SaveAs" in an alien format
        if ( rMedium.GetFilter() && ( rMedium.GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
            bOk = ExportTo( rMedium );
        else
            bOk = ConvertTo( rMedium );

        // after saving the document, the temporary object storage must be updated
        // if the old object storage was not a temporary one, it will be updated also, because it will be used
        // as a source for copying the objects into the new temporary storage that will be created below
        // updating means: all child objects must be stored into it
        // ( same as on loading, where these objects are copied to the temporary storage )
        // but don't commit these changes, because in the case when the old object storage is not a temporary one,
        // all changes will be written into the original file !

        if( bOk && !bCopyTo )
            // we also don't touch any graphical replacements here
            bOk = SaveChildren( TRUE );
    }

    uno::Reference< security::XDocumentDigitalSignatures > xDDSigns;
    sal_Bool bScriptSignatureIsCopied = sal_False;
    if ( bOk && bTryToPreservScriptSignature )
    {
        // if the scripting code was not changed and it is signed the signature should be preserved
        // unfortunately at this point we have only information whether the basic code has changed or not
        // so the only way is to check the signature if the basic was not changed
        try
        {
            xDDSigns = uno::Reference< security::XDocumentDigitalSignatures >(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ),
                    uno::UNO_QUERY_THROW );

            ::rtl::OUString aScriptSignName = xDDSigns->getScriptingContentSignatureDefaultStreamName();

            if ( aScriptSignName.getLength() )
            {
                uno::Reference< embed::XStorage > xMetaInf = GetStorage()->openStorageElement(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) ),
                            embed::ElementModes::READ );
                uno::Reference< embed::XStorage > xTargetMetaInf = rMedium.GetStorage()->openStorageElement(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) ),
                            embed::ElementModes::WRITE );

                if ( xMetaInf.is() && xTargetMetaInf.is() )
                {
                    xMetaInf->copyElementTo( aScriptSignName, xTargetMetaInf, aScriptSignName );

                    // after loading the UseCommonStoragePassword property might be set to true
                    // set it to false here, since this is a rare case when it must be so
                    // TODO/LATER: in future it should be done on loading probably
                    uno::Reference< beans::XPropertySet > xTargetSignPropSet(
                        xTargetMetaInf->openStreamElement( aScriptSignName, embed::ElementModes::WRITE ),
                        uno::UNO_QUERY_THROW );
                    xTargetSignPropSet->setPropertyValue(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseCommonStoragePasswordEncryption" ) ),
                        uno::makeAny( (sal_Bool)sal_False ) );

                    uno::Reference< embed::XTransactedObject > xTransact( xTargetMetaInf, uno::UNO_QUERY );
                    if ( xTransact.is() )
                        xTransact->commit();
                    bScriptSignatureIsCopied = sal_True;
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }

    if ( bOk )
    {
        // transfer data to its destinated location
        // the medium commits the storage or the stream it is based on
        RegisterTransfer( rMedium );
        bOk = rMedium.Commit();

        if ( bOk && bScriptSignatureIsCopied )
        {
            // if the script signature was copied it should be checked now
            // usually it should be ok, so no additional actions will be done
            // but if for any reasong ( f.e. binshell change ) it is broken it should be removed here
            // in result the behaviour will work in optimized way in most cases, means in case of signed basic scripts
            OSL_ENSURE( !bScriptSignatureIsCopied || xDDSigns.is(), "The signing could not be done without the service!\n" );
            if ( xDDSigns.is() )
            {
                try
                {
                    bOk = sal_False;
                    ::com::sun::star::uno::Sequence< security::DocumentSignatureInformation > aInfos =
                        xDDSigns->verifyScriptingContentSignatures( rMedium.GetLastCommitReadStorage_Impl(),
                                                                    uno::Reference< io::XInputStream >() );
                    sal_uInt16 nState = ImplCheckSignaturesInformation( aInfos );
                    if ( nState == SIGNATURESTATE_SIGNATURES_OK || nState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED )
                    {
                        rMedium.SetCachedSignatureState_Impl( nState );
                        bOk = sal_True;
                    }
                    else
                    {
                        // the signature is broken, remove it
                        rMedium.SetCachedSignatureState_Impl( SIGNATURESTATE_NOSIGNATURES );
                        uno::Reference< embed::XStorage > xTargetMetaInf = rMedium.GetStorage()->openStorageElement(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF" ) ),
                            embed::ElementModes::WRITE );

                        if ( xTargetMetaInf.is() )
                        {
                            xTargetMetaInf->removeElement( xDDSigns->getScriptingContentSignatureDefaultStreamName() );
                            uno::Reference< embed::XTransactedObject > xTransact( xTargetMetaInf, uno::UNO_QUERY );
                            if ( xTransact.is() )
                                xTransact->commit();

                            bOk = rMedium.Commit();
                        }
                    }
                }
                catch( uno::Exception )
                {
                    OSL_ENSURE( sal_False, "This exception must not happen!" );
                }
            }
        }

        if ( bOk )
        {
            // if the target medium is an alien format and the "old" medium was an own format and the "old" medium
            // has a name, the object storage must be exchanged, because now we need a new temporary storage
            // as object storage
            if ( !bCopyTo && bStorageBasedSource && !bStorageBasedTarget )
            {
                if ( bStoreToSameLocation )
                {
                    // if the old medium already disconnected from document storage, the storage still must
                    // be switched if backup file is used
                    if ( bNeedsDisconnectionOnFail )
                        ConnectTmpStorage_Impl( pImp->m_xDocStorage, NULL );
                }
                else if ( pMedium->GetName().Len()
                  || ( pMedium->HasStorage_Impl() && pMedium->WillDisposeStorageOnClose_Impl() ) )
                {
                    OSL_ENSURE( pMedium->GetName().Len(), "Fallback is used, the medium without name should not dispose the storage!\n" );
                    // copy storage of old medium to new temporary storage and take this over
                    if( !ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                        bOk = sal_False;
                }
            }
        }
        else
        {
            // in case the document storage was connected to backup temporarely it must be disconnected now
            if ( bNeedsDisconnectionOnFail )
                ConnectTmpStorage_Impl( pImp->m_xDocStorage, NULL );
        }
    }

    // unlock user interface
    Lock_Impl( this, sal_False );
    pImp->bForbidReload = bOldStat;

    if ( bOk )
    {
        try
        {
            ::ucbhelper::Content aContent( rMedium.GetName(), com::sun::star::uno::Reference < XCommandEnvironment >() );
            com::sun::star::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                ::rtl::OUString aAuthor( RTL_CONSTASCII_USTRINGPARAM("Author") );
                ::rtl::OUString aKeywords( RTL_CONSTASCII_USTRINGPARAM("Keywords") );
                ::rtl::OUString aSubject( RTL_CONSTASCII_USTRINGPARAM("Subject") );
                Any aAny;

                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();

                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aAny <<= xDocProps->getAuthor();
                    aContent.setPropertyValue( aAuthor, aAny );
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    aAny <<= ::comphelper::string::convertCommaSeparated(
                                xDocProps->getKeywords());
                    aContent.setPropertyValue( aKeywords, aAny );
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aAny <<= xDocProps->getSubject();
                    aContent.setPropertyValue( aSubject, aAny );
                }
            }
        }
        catch( Exception& )
        {
        }
    }

    return bOk;
}

//------------------------------------------------------------------------
sal_Bool SfxObjectShell::DisconnectStorage_Impl( SfxMedium& rSrcMedium, SfxMedium& rTargetMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::DisconnectStorage_Impl" );

    // this method disconnects the storage from source medium, and attaches it to the backup created by the target medium

    uno::Reference< embed::XStorage > xStorage = rSrcMedium.GetStorage();

    sal_Bool bResult = sal_False;
    if ( xStorage == pImp->m_xDocStorage )
    {
        try
        {
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY_THROW );
            ::rtl::OUString aBackupURL = rTargetMedium.GetBackup_Impl();
            if ( aBackupURL.getLength() )
            {
                // the following call will only compare stream sizes
                // TODO/LATER: this is a very risky part, since if the URL contents are different from the storage
                // contents, the storag will be broken
                xOptStorage->attachToURL( aBackupURL, sal_True );

                // the storage is successfuly attached to backup, thus it it owned by the document not by the medium
                rSrcMedium.CanDisposeStorage_Impl( sal_False );
                bResult = sal_True;
            }
        }
        catch ( uno::Exception& )
        {}
    }

    OSL_ENSURE( bResult, "Storage disconnecting has failed - affects performance!" );

    return bResult;
}

//------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConnectTmpStorage_Impl(
    const uno::Reference< embed::XStorage >& xStorage,
    SfxMedium* pMediumArg )

/*   [Beschreibung]

     Arbeitet die Applikation auf einem temporaeren Storage,
     so darf der temporaere Storage nicht aus dem SaveCompleted
     genommen werden. Daher wird in diesem Fall schon hier an
     den neuen Storage connected. SaveCompleted tut dann nichts.

     */

{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::ConnectTmpStorage_Impl" );

    sal_Bool bResult = sal_False;

    if ( xStorage.is() )
    {
        try
        {
            // the empty argument means that the storage will create temporary stream itself
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY_THROW );
            xOptStorage->writeAndAttachToStream( uno::Reference< io::XStream >() );

            // the storage is successfuly disconnected from the original sources, thus the medium must not dispose it
            if ( pMediumArg )
                pMediumArg->CanDisposeStorage_Impl( sal_False );

            bResult = sal_True;
        }
        catch( uno::Exception& )
        {
        }

        // if switching of the storage does not work for any reason ( nonroot storage for example ) use the old method
        if ( !bResult ) try
        {
            uno::Reference< embed::XStorage > xTmpStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();

            DBG_ASSERT( xTmpStorage.is(), "If a storage can not be created an exception must be thrown!\n" );
            if ( !xTmpStorage.is() )
                throw uno::RuntimeException();

            // TODO/LATER: may be it should be done in SwitchPersistence also
            // TODO/LATER: find faster way to copy storage; perhaps sharing with backup?!
            xStorage->copyToStorage( xTmpStorage );
            //CopyStoragesOfUnknownMediaType( xStorage, xTmpStorage );
            bResult = SaveCompleted( xTmpStorage );

            if ( bResult )
                pImp->pBasicManager->setStorage( xTmpStorage );
        }
        catch( uno::Exception& )
        {}

        if ( !bResult )
        {
            // TODO/LATER: may need error code setting based on exception
            SetError( ERRCODE_IO_GENERAL );
        }
    }

    return bResult;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveObjectAs( SfxMedium& rMedium, BOOL bCommit )
{
    sal_Bool bOk = sal_False;
    {
        ModifyBlocker_Impl aBlock( this );

        uno::Reference < embed::XStorage > xNewStor = rMedium.GetStorage();
        if ( !xNewStor.is() )
            return sal_False;

        uno::Reference < beans::XPropertySet > xPropSet( xNewStor, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            Any a = xPropSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) );
            ::rtl::OUString aMediaType;
            if ( !(a>>=aMediaType) || !aMediaType.getLength() )
            {
                OSL_ENSURE( sal_False, "The mediatype must be set already!\n" );
                SetupStorage( xNewStor, SOFFICE_FILEFORMAT_CURRENT, sal_False );
            }

            pImp->bIsSaving = sal_False;
            bOk = SaveAsOwnFormat( rMedium );

            if ( bCommit )
            {
                try {
                    uno::Reference< embed::XTransactedObject > xTransact( xNewStor, uno::UNO_QUERY_THROW );
                    xTransact->commit();
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "The strotage was not commited on DoSaveAs!\n" );
                }
            }
        }
    }

    return bOk;
}

//-------------------------------------------------------------------------
// TODO/LATER: may be the call must be removed completelly
sal_Bool SfxObjectShell::DoSaveAs( SfxMedium& rMedium )
{
    // hier kommen nur Root-Storages rein, die via Temp-File gespeichert werden
    rMedium.CreateTempFileNoCopy();
    SetError(rMedium.GetErrorCode());
    if ( GetError() )
        return sal_False;

    // copy version list from "old" medium to target medium, so it can be used on saving
    if ( pImp->bPreserveVersions )
        rMedium.TransferVersionList_Impl( *pMedium );

    sal_Bool bRet = SaveTo_Impl( rMedium, NULL );
    if ( !bRet )
        SetError(rMedium.GetErrorCode());
    return bRet;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveCompleted( SfxMedium* pNewMed )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::DoSaveCompleted" );

    sal_Bool bOk = sal_True;
    sal_Bool bMedChanged = pNewMed && pNewMed!=pMedium;
/*  sal_Bool bCreatedTempStor = pNewMed && pMedium &&
        IsPackageStorageFormat_Impl(*pMedium) &&
        !IsPackageStorageFormat_Impl(*pNewMed) &&
        pMedium->GetName().Len();
*/
    DBG_ASSERT( !pNewMed || pNewMed->GetError() == ERRCODE_NONE, "DoSaveCompleted: Medium has error!" );

    // delete Medium (and Storage!) after all notifications
    SfxMedium* pOld = pMedium;
    if ( bMedChanged )
    {
        pMedium = pNewMed;
        pMedium->CanDisposeStorage_Impl( sal_True );
    }

    const SfxFilter *pFilter = pMedium ? pMedium->GetFilter() : 0;
    if ( pNewMed )
    {
        if( bMedChanged )
        {
            if( pNewMed->GetName().Len() )
                bHasName = sal_True;
            Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );
            getDocProperties()->setGenerator(
               ::utl::DocInfoHelper::GetGeneratorString() );
        }

        uno::Reference< embed::XStorage > xStorage;
        if ( !pFilter || IsPackageStorageFormat_Impl( *pMedium ) )
        {
            uno::Reference < embed::XStorage > xOld = GetStorage();

            // when the package based medium is broken and has no storage or if the storage
            // is the same as the document storage the current document storage should be preserved
            xStorage = pMedium->GetStorage();
            bOk = SaveCompleted( xStorage );
            if ( bOk && xStorage.is() && xOld != xStorage
              && (!pOld || !pOld->HasStorage_Impl() || xOld != pOld->GetStorage() ) )
            {
                // old own storage was not controlled by old Medium -> dispose it
                try {
                    xOld->dispose();
                } catch( uno::Exception& )
                {
                    // the storage is disposed already
                    // can happen during reload scenario when the medium has disposed it during the closing
                    // will be fixed in one of the next milestones
                }
            }
        }
        else
        {
//REMOVE                if( pFilter->UsesStorage() )
//REMOVE                    pMedium->GetStorage();
//REMOVE                else if( pMedium->GetOpenMode() & STREAM_WRITE )
            if( pMedium->GetOpenMode() & STREAM_WRITE )
                pMedium->GetInStream();
            xStorage = GetStorage();
        }

        // TODO/LATER: may be this code will be replaced, but not sure
        // Set storage in document library containers
        pImp->pBasicManager->setStorage( xStorage );
    }
    else
    {
        if( pMedium )
        {
            if( pFilter && !IsPackageStorageFormat_Impl( *pMedium ) && (pMedium->GetOpenMode() & STREAM_WRITE ))
            {
                pMedium->ReOpen();
                bOk = SaveCompletedChildren( sal_False );
            }
            else
                bOk = SaveCompleted( NULL );
        }
        // entweder Save oder ConvertTo
        else
            bOk = SaveCompleted( NULL );
    }

    if ( bOk && pNewMed )
    {
        if( bMedChanged )
        {
            delete pOld;

            uno::Reference< frame::XModel > xModel = GetModel();
            if ( xModel.is() )
            {
                ::rtl::OUString aURL = pNewMed->GetOrigURL();
                uno::Sequence< beans::PropertyValue > aMediaDescr;
                TransformItems( SID_OPENDOC, *pNewMed->GetItemSet(), aMediaDescr );
                try
                {
                    xModel->attachResource( aURL, aMediaDescr );
                }
                catch( uno::Exception& )
                {}
            }

            // before the title regenerated the document must loose the signatures
            pImp->nDocumentSignatureState = SIGNATURESTATE_NOSIGNATURES;
            pImp->nScriptingSignatureState = pNewMed->GetCachedSignatureState_Impl();
            OSL_ENSURE( pImp->nScriptingSignatureState != SIGNATURESTATE_SIGNATURES_BROKEN, "The signature must not be broken at this place" );
            pImp->bSignatureErrorIsShown = sal_False;

            // TODO/LATER: in future the medium must control own signature state, not the document
            pNewMed->SetCachedSignatureState_Impl( SIGNATURESTATE_NOSIGNATURES ); // set the default value back

            // Titel neu setzen
            if ( pNewMed->GetName().Len() && SFX_CREATE_MODE_EMBEDDED != eCreateMode )
                InvalidateName();
            SetModified(sal_False); // nur bei gesetztem Medium zur"ucksetzen
            Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
        }
    }

    pMedium->ClearBackup_Impl();

    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConvertFrom
(
    SfxMedium&  /*rMedium*/     /*  <SfxMedium>, welches die Quell-Datei beschreibt
                                (z.B. Dateiname, <SfxFilter>, Open-Modi etc.) */
)

/*  [Beschreibung]

    Diese Methode wird zum Laden von Dokumenten "uber alle Filter gerufen,
    die nicht SFX_FILTER_OWN sind oder f"ur die kein Clipboard-Format
    registriert wurde (also kein Storage-Format benutzen). Mit anderen Worten:
    mit dieser Methode wird importiert.

    Das hier zu "offende File sollte "uber 'rMedium' ge"offnet werden,
    um die richtigen Open-Modi zu gew"ahrleisten. Insbesondere wenn das
    Format beibehalten wird (nur m"oglich bei SFX_FILTER_SIMULATE oder
    SFX_FILTER_ONW) mu\s die Datei STREAM_SHARE_DENYWRITE ge"offnet werden.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Das Dokument konnte geladen werden.

                        sal_False
                        Das Dokument konnte nicht geladen werden, ein
                        Fehlercode ist mit <SvMedium::GetError()const> zu
                        erhalten.


    [Beispiel]

    sal_Bool DocSh::ConvertFrom( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetInStream();
        if( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream >> ...;

            // NICHT 'rMedium.CloseInStream()' rufen! File gelockt halten!
            return SVSTREAM_OK == rMedium.GetError();
        }

        return sal_False;
    }


    [Querverweise]

    <SfxObjectShell::ConvertTo(SfxMedium&)>
    <SFX_FILTER_REGISTRATION>
*/
{
    return sal_False;
}

sal_Bool SfxObjectShell::ImportFrom( SfxMedium& rMedium )
{
    ::rtl::OUString aTypeName( rMedium.GetFilter()->GetTypeName() );
    ::rtl::OUString aFilterName( rMedium.GetFilter()->GetFilterName() );

    uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
    uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), uno::UNO_QUERY );

    uno::Sequence < beans::PropertyValue > aProps;
    uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
    if ( xFilters->hasByName( aFilterName ) )
    {
        xFilters->getByName( aFilterName ) >>= aProps;
        rMedium.GetItemSet()->Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
    }

    ::rtl::OUString aFilterImplName;
    sal_Int32 nFilterProps = aProps.getLength();
    for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
    {
        const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
        if ( rFilterProp.Name.compareToAscii("FilterService") == COMPARE_EQUAL )
        {
            rFilterProp.Value >>= aFilterImplName;
            break;
        }
    }

    uno::Reference< document::XFilter > xLoader;
    if ( aFilterImplName.getLength() )
    {
        try{
        xLoader = uno::Reference< document::XFilter >
            ( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
        }catch(const uno::Exception&)
            { xLoader.clear(); }
    }
    if ( xLoader.is() )
    {
        // #131744#: it happens that xLoader does not support xImporter!
        try{
        uno::Reference< lang::XComponent >  xComp( GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< document::XImporter > xImporter( xLoader, uno::UNO_QUERY_THROW );
        xImporter->setTargetDocument( xComp );

        uno::Sequence < beans::PropertyValue > lDescriptor;
        rMedium.GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, rMedium.GetName() ) );
        TransformItems( SID_OPENDOC, *rMedium.GetItemSet(), lDescriptor );

        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aArgs ( lDescriptor.getLength() );
        com::sun::star::beans::PropertyValue * pNewValue = aArgs.getArray();
        const com::sun::star::beans::PropertyValue * pOldValue = lDescriptor.getConstArray();
        const OUString sInputStream ( RTL_CONSTASCII_USTRINGPARAM ( "InputStream" ) );

        sal_Bool bHasInputStream = sal_False;
        BOOL bHasBaseURL = FALSE;
        sal_Int32 i;
        sal_Int32 nEnd = lDescriptor.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue [i].Name == sInputStream )
                bHasInputStream = sal_True;
            else if ( pOldValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "DocumentBaseURL" ) ) )
                bHasBaseURL = sal_True;
        }

        if ( !bHasInputStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sInputStream;
            aArgs[nEnd-1].Value <<= com::sun::star::uno::Reference < com::sun::star::io::XInputStream > ( new utl::OSeekableInputStreamWrapper ( *rMedium.GetInStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "DocumentBaseURL" ) );
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL();
        }

        return xLoader->filter( aArgs );
        }catch(const uno::Exception&)
        {}
    }

    return sal_False;
}

sal_Bool SfxObjectShell::ExportTo( SfxMedium& rMedium )
{
    ::rtl::OUString aTypeName( rMedium.GetFilter()->GetTypeName() );
    ::rtl::OUString aFilterName( rMedium.GetFilter()->GetFilterName() );
    uno::Reference< document::XExporter > xExporter;

    {
        uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), uno::UNO_QUERY );

        uno::Sequence < beans::PropertyValue > aProps;
        uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
        if ( xFilters->hasByName( aFilterName ) )
            xFilters->getByName( aFilterName ) >>= aProps;

        ::rtl::OUString aFilterImplName;
        sal_Int32 nFilterProps = aProps.getLength();
        for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
        {
            const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
            if ( rFilterProp.Name.compareToAscii("FilterService") == COMPARE_EQUAL )
            {
                rFilterProp.Value >>= aFilterImplName;
                break;
            }
        }

        if ( aFilterImplName.getLength() )
        {
            try{
            xExporter = uno::Reference< document::XExporter >
                ( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
            }catch(const uno::Exception&)
                { xExporter.clear(); }
        }
    }

    if ( xExporter.is() )
    {
        try{
        uno::Reference< lang::XComponent >  xComp( GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );

        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aOldArgs;
        SfxItemSet* pItems = rMedium.GetItemSet();
        TransformItems( SID_SAVEASDOC, *pItems, aOldArgs );

        const com::sun::star::beans::PropertyValue * pOldValue = aOldArgs.getConstArray();
        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aArgs ( aOldArgs.getLength() );
        com::sun::star::beans::PropertyValue * pNewValue = aArgs.getArray();

        // put in the REAL file name, and copy all PropertyValues
        const OUString sOutputStream ( RTL_CONSTASCII_USTRINGPARAM ( "OutputStream" ) );
        BOOL bHasStream = FALSE;
        BOOL bHasBaseURL = FALSE;
        sal_Int32 i;
        sal_Int32 nEnd = aOldArgs.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "FileName" ) ) )
                pNewValue[i].Value <<= OUString ( rMedium.GetName() );
            else if ( pOldValue[i].Name == sOutputStream )
                bHasStream = sal_True;
            else if ( pOldValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "DocumentBaseURL" ) ) )
                bHasBaseURL = sal_True;
        }

        if ( !bHasStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sOutputStream;
            aArgs[nEnd-1].Value <<= com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > ( new utl::OOutputStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "DocumentBaseURL" ) );
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL( sal_True );
        }

        return xFilter->filter( aArgs );
        }catch(const uno::Exception&)
        {}
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConvertTo
(
    SfxMedium&  /*rMedium*/     /*  <SfxMedium>, welches die Ziel-Datei beschreibt
                                (z.B. Dateiname, <SfxFilter>, Open-Modi etc.) */
)

/*  [Beschreibung]

    Diese Methode wird zum Speichern von Dokumenten "uber alle Filter gerufen,
    die nicht SFX_FILTER_OWN sind oder f"ur die kein Clipboard-Format
    registriert wurde (also kein Storage-Format benutzen). Mit anderen Worten:
    mit dieser Methode wird exportiert.

    Das hier zu "offende File sollte "uber 'rMedium' ge"offnet werden,
    um die richtigen Open-Modi zu gew"ahrleisten. Insbesondere wenn das
    Format beibehalten wird (nur m"oglich bei SFX_FILTER_SIMULATE oder
    SFX_FILTER_ONW) mu\s die Datei auch nach dem Speichern im Modus
    STREAM_SHARE_DENYWRITE ge"offnet bleiben.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Das Dokument konnte gespeichert werden.

                        sal_False
                        Das Dokument konnte nicht gespeichert werden, ein
                        Fehlercode ist mit <SvMedium::GetError()const> zu
                        erhalten.


    [Beispiel]

    sal_Bool DocSh::ConvertTo( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetOutStream();
        if ( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream << ...;

            rMedium.CloseOutStream(); // "offnet automatisch wieder den InStream
            return SVSTREAM_OK == rMedium.GetError();
        }
        return sal_False ;
    }


    [Querverweise]

    <SfxObjectShell::ConvertFrom(SfxMedium&)>
    <SFX_FILTER_REGISTRATION>
*/

{
    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSave_Impl( const SfxItemSet* pArgs )
{
    SfxMedium* pRetrMedium = GetMedium();
    const SfxFilter* pFilter = pRetrMedium->GetFilter();

    // copy the original itemset, but remove the "version" item, because pMediumTmp
    // is a new medium "from scratch", so no version should be stored into it
    SfxItemSet* pSet = pRetrMedium->GetItemSet() ? new SfxAllItemSet(*pRetrMedium->GetItemSet()): 0;
    pSet->ClearItem( SID_VERSION );

    // create a medium as a copy; this medium is only for writingm, because it uses the same name as the original one
    // writing is done through a copy, that will be transferred to the target ( of course after calling HandsOff )
    SfxMedium* pMediumTmp = new SfxMedium( pRetrMedium->GetName(), pRetrMedium->GetOpenMode(), pRetrMedium->IsDirect(), pFilter, pSet );
    pMediumTmp->SetLongName( pRetrMedium->GetLongName() );
//    pMediumTmp->CreateTempFileNoCopy();
    if ( pMediumTmp->GetErrorCode() != ERRCODE_NONE )
    {
        SetError( pMediumTmp->GetError() );
        delete pMediumTmp;
        return sal_False;
    }

    // copy version list from "old" medium to target medium, so it can be used on saving
    pMediumTmp->TransferVersionList_Impl( *pRetrMedium );
/*
    if ( pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) )
        SetError( GetMedium()->Unpack_Impl( pRetrMedium->GetPhysicalName() ) );
*/

    // an interaction handler here can aquire only in case of GUI Saving
    // and should be removed after the saving is done
    com::sun::star::uno::Reference< XInteractionHandler > xInteract;
    SFX_ITEMSET_ARG( pArgs, pxInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False );
    if ( pxInteractionItem && ( pxInteractionItem->GetValue() >>= xInteract ) && xInteract.is() )
        pMediumTmp->GetItemSet()->Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, makeAny( xInteract ) ) );

    sal_Bool bSaved = sal_False;
    if( !GetError() && SaveTo_Impl( *pMediumTmp, pArgs ) )
    {
        bSaved = sal_True;

        if( pMediumTmp->GetItemSet() )
        {
            pMediumTmp->GetItemSet()->ClearItem( SID_INTERACTIONHANDLER );
            pMediumTmp->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        }

        SetError(pMediumTmp->GetErrorCode());

//REMOVE            if ( !IsHandsOff() )
//REMOVE            pMediumTmp->Close();

        sal_Bool bOpen( sal_False );
        bOpen = DoSaveCompleted( pMediumTmp );
        DBG_ASSERT(bOpen,"Fehlerbehandlung fuer DoSaveCompleted nicht implementiert");
    }
    else
    {
        // transfer error code from medium to objectshell
        SetError( pMediumTmp->GetError() );

        // reconnect to object storage
//REMOVE            if ( IsHandsOff() )
//REMOVE            {
//REMOVE                if ( !DoSaveCompleted( pRetrMedium ) )
//REMOVE                    DBG_ERROR("Case not handled - no way to get a storage!");
//REMOVE            }
//REMOVE            else
            DoSaveCompleted( 0 );

        if( pRetrMedium->GetItemSet() )
        {
            pRetrMedium->GetItemSet()->ClearItem( SID_INTERACTIONHANDLER );
            pRetrMedium->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        }

        delete pMediumTmp;
    }

    SetModified( !bSaved );
    return bSaved;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::Save_Impl( const SfxItemSet* pSet )
{
    if ( IsReadOnly() )
    {
        SetError( ERRCODE_SFX_DOCUMENTREADONLY );
        return sal_False;
    }

    DBG_CHKTHIS(SfxObjectShell, 0);

    pImp->bIsSaving = sal_True;
    sal_Bool bSaved = FALSE;
    SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if ( pSalvageItem )
    {
        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False);
        String aFilterName;
        const SfxFilter *pFilter = NULL;
        if ( pFilterItem )
            pFilter = SfxFilterMatcher( String::CreateFromAscii( GetFactory().GetShortName()) ).GetFilter4FilterName( aFilterName );

        SfxMedium *pMed = new SfxMedium(
            pSalvageItem->GetValue(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC, sal_False, pFilter );

        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False );
        if ( pPasswordItem )
            pMed->GetItemSet()->Put( *pPasswordItem );

        bSaved = DoSaveAs( *pMed );
        if ( bSaved )
            bSaved = DoSaveCompleted( pMed );
        else
            delete pMed;
    }
    else
        bSaved = DoSave_Impl( pSet );
    return bSaved;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::CommonSaveAs_Impl
(
    const INetURLObject&   aURL,
    const String&   aFilterName,
    SfxItemSet*     aParams
)
{
    if( aURL.HasError() )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER );
        return sal_False;
    }

    if ( aURL != INetURLObject( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:stream" ) ) ) )
    {
        // gibt es schon ein Doc mit dem Namen?
        SfxObjectShell* pDoc = 0;
        for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
                pTmp && !pDoc;
                pTmp = SfxObjectShell::GetNext(*pTmp) )
        {
            if( ( pTmp != this ) && pTmp->GetMedium() )
            {
                INetURLObject aCompare( pTmp->GetMedium()->GetName() );
                if ( aCompare == aURL )
                    pDoc = pTmp;
            }
        }
        if ( pDoc )
        {
            // dann Fehlermeldeung: "schon offen"
            SetError(ERRCODE_SFX_ALREADYOPEN);
            return sal_False;
        }
    }

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );
    DBG_ASSERT( aParams->Count() != 0, "fehlerhafte Parameter");

    SFX_ITEMSET_ARG( aParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
    sal_Bool bSaveTo = pSaveToItem ? pSaveToItem->GetValue() : sal_False;

    const SfxFilter* pFilter = GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName );
    if ( !pFilter
        || !pFilter->CanExport()
        || !bSaveTo && !pFilter->CanImport() )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER );
        return sal_False;
    }

    SFX_ITEMSET_ARG( aParams, pCopyStreamItem, SfxBoolItem, SID_COPY_STREAM_IF_POSSIBLE, sal_False );
    if ( bSaveTo && pCopyStreamItem && pCopyStreamItem->GetValue() && !IsModified() )
    {
        if ( pMedium->TryDirectTransfer( aURL.GetMainURL( INetURLObject::NO_DECODE ), *aParams ) )
            return sal_True;
    }
    aParams->ClearItem( SID_COPY_STREAM_IF_POSSIBLE );

    pImp->bPasswd = aParams && SFX_ITEM_SET == aParams->GetItemState(SID_PASSWORD);

    SfxMedium *pActMed = GetMedium();
    const INetURLObject aActName(pActMed->GetName());

    BOOL bWasReadonly = IsReadOnly();

    if ( aURL == aActName && aURL != INetURLObject( OUString::createFromAscii( "private:stream" ) )
        && IsReadOnly() )
    {
        SetError(ERRCODE_SFX_DOCUMENTREADONLY);
        return sal_False;
    }

//REMOVE        if ( aURL == aActName && aURL != INetURLObject( OUString::createFromAscii( "private:stream" ) ) )
//REMOVE        {
//REMOVE            if ( IsReadOnly() )
//REMOVE            {
//REMOVE                SetError(ERRCODE_SFX_DOCUMENTREADONLY);
//REMOVE                return sal_False;
//REMOVE            }
//REMOVE
//REMOVE            // gleicher Filter? -> Save()
//REMOVE            const SfxFilter *pFilter = pActMed->GetFilter();
//REMOVE            if ( pFilter && pFilter->GetFilterName() == aFilterName )
//REMOVE            {
//REMOVE                pImp->bIsSaving=sal_False;
//REMOVE                if ( aParams )
//REMOVE                {
//REMOVE                    SfxItemSet* pSet = pMedium->GetItemSet();
//REMOVE                    pSet->ClearItem( SID_PASSWORD );
//REMOVE                    pSet->Put( *aParams );
//REMOVE                }
//REMOVE
//REMOVE                SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEDOC,this));
//REMOVE                BOOL bRet = DoSave_Impl();
//REMOVE                SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEDOCDONE,this));
//REMOVE                return bRet;
//REMOVE            }
//REMOVE        }

    // this notification should be already sent by caller in sfxbasemodel
    // SFX_APP()->NotifyEvent(SfxEventHint( bSaveTo? SFX_EVENT_SAVETODOC : SFX_EVENT_SAVEASDOC,this));

    if( SFX_ITEM_SET != aParams->GetItemState(SID_UNPACK) && SvtSaveOptions().IsSaveUnpacked() )
        aParams->Put( SfxBoolItem( SID_UNPACK, sal_False ) );

    if ( PreDoSaveAs_Impl(aURL.GetMainURL( INetURLObject::NO_DECODE ),aFilterName,aParams))
    {
        pImp->bWaitingForPicklist = sal_True;

        // Daten am Medium updaten
        SfxItemSet *pSet = GetMedium()->GetItemSet();
        pSet->ClearItem( SID_INTERACTIONHANDLER );
        pSet->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );

        if ( !bSaveTo )
        {
            pSet->ClearItem( SID_REFERER );
            pSet->ClearItem( SID_POSTDATA );
            pSet->ClearItem( SID_TEMPLATE );
            pSet->ClearItem( SID_DOC_READONLY );
            pSet->ClearItem( SID_CONTENTTYPE );
            pSet->ClearItem( SID_CHARSET );
            pSet->ClearItem( SID_FILTER_NAME );
            pSet->ClearItem( SID_OPTIONS );
            //pSet->ClearItem( SID_FILE_FILTEROPTIONS );
            pSet->ClearItem( SID_VERSION );
            //pSet->ClearItem( SID_USE_FILTEROPTIONS );
            pSet->ClearItem( SID_EDITDOC );
            pSet->ClearItem( SID_OVERWRITE );

            SFX_ITEMSET_GET( (*aParams), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( pFilterItem )
                pSet->Put( *pFilterItem );

            SFX_ITEMSET_GET( (*aParams), pOptionsItem, SfxStringItem, SID_OPTIONS, sal_False );
            if ( pOptionsItem )
                pSet->Put( *pOptionsItem );

            SFX_ITEMSET_GET( (*aParams), pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
            if ( pFilterOptItem )
                pSet->Put( *pFilterOptItem );
        }

        if ( bWasReadonly && !bSaveTo )
            Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );

        return sal_True;
    }
    else
        return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::PreDoSaveAs_Impl
(
    const String&   rFileName,
    const String&   aFilterName,
    SfxItemSet*     pParams
)
{
    // copy all items stored in the itemset of the current medium
    SfxAllItemSet* pMergedParams = new SfxAllItemSet( *pMedium->GetItemSet() );

    // in "SaveAs" title and password will be cleared ( maybe the new itemset contains new values, otherwise they will be empty )
    pMergedParams->ClearItem( SID_PASSWORD );
    pMergedParams->ClearItem( SID_DOCINFO_TITLE );

    pMergedParams->ClearItem( SID_INPUTSTREAM );
    pMergedParams->ClearItem( SID_STREAM );
    pMergedParams->ClearItem( SID_CONTENT );
    pMergedParams->ClearItem( SID_DOC_READONLY );

    pMergedParams->ClearItem( SID_REPAIRPACKAGE );

    // "SaveAs" will never store any version information - it's a complete new file !
    pMergedParams->ClearItem( SID_VERSION );

    // merge the new parameters into the copy
    // all values present in both itemsets will be overwritten by the new parameters
    if( pParams )
        pMergedParams->Put( *pParams );
    //DELETEZ( pParams );

#ifdef DBG_UTIL
    if ( pMergedParams->GetItemState( SID_DOC_SALVAGE) >= SFX_ITEM_SET )
        DBG_ERROR("Salvage item present in Itemset, check the parameters!");
#endif

    // should be unneccessary - too hot to handle!
    pMergedParams->ClearItem( SID_DOC_SALVAGE );

    // take over the new merged itemset
    pParams = pMergedParams;

    // create a medium for the target URL
    SfxMedium *pNewFile = new SfxMedium( rFileName, STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC, sal_False, 0, pParams );

    // set filter; if no filter is given, take the default filter of the factory
    if ( aFilterName.Len() )
        pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName ) );
    else
        pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetAnyFilter( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT ) );

//REMOVE        // saving is alway done using a temporary file
//REMOVE        pNewFile->CreateTempFileNoCopy();
    if ( pNewFile->GetErrorCode() != ERRCODE_NONE )
    {
        // creating temporary file failed ( f.e. floppy disk not inserted! )
        SetError( pNewFile->GetError() );
        delete pNewFile;
        return sal_False;
    }

    // check if a "SaveTo" is wanted, no "SaveAs"
    SFX_ITEMSET_ARG( pParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
    sal_Bool bCopyTo = GetCreateMode() == SFX_CREATE_MODE_EMBEDDED || pSaveToItem && pSaveToItem->GetValue();

    // distinguish between "Save" and "SaveAs"
    pImp->bIsSaving = sal_False;

    // copy version list from "old" medium to target medium, so it can be used on saving
    if ( pImp->bPreserveVersions )
        pNewFile->TransferVersionList_Impl( *pMedium );

/*
    if ( GetMedium()->GetFilter() && ( GetMedium()->GetFilter()->GetFilterFlags() & SFX_FILTER_PACKED ) )
    {
        SfxMedium *pMed = bCopyTo ? pMedium : pNewFile;
        pNewFile->SetError( GetMedium()->Unpack_Impl( pMed->GetPhysicalName() ) );
    }
*/
    // Save the document ( first as temporary file, then transfer to the target URL by committing the medium )
    sal_Bool bOk = sal_False;
    if ( !pNewFile->GetErrorCode() && SaveTo_Impl( *pNewFile, NULL ) )
    {
        bOk = sal_True;

        // transfer a possible error from the medium to the document
        SetError( pNewFile->GetErrorCode() );

        // notify the document that saving was done successfully
//REMOVE            if ( bCopyTo )
//REMOVE            {
//REMOVE                if ( IsHandsOff() )
//REMOVE                    bOk = DoSaveCompleted( pMedium );
//REMOVE            }
//REMOVE            else
        if ( !bCopyTo )
        {
            // Muss !!!
//REMOVE                if ( bToOwnFormat )
//REMOVE                    SetFileName( pNewFile->GetPhysicalName() );

            bOk = DoSaveCompleted( pNewFile );
        }
        else
            bOk = DoSaveCompleted(0);

        if( bOk )
        {
            if( !bCopyTo )
                SetModified( sal_False );
        }
        else
        {
            // TODO/LATER: the code below must be dead since the storage commit makes all the stuff
            //       and the DoSaveCompleted call should not be able to fail in general

            DBG_ASSERT( !bCopyTo, "Error while reconnecting to medium, can't be handled!");
            SetError( pNewFile->GetErrorCode() );

            if ( !bCopyTo )
            {
                // reconnect to the old medium
                BOOL bRet( FALSE );
                bRet = DoSaveCompleted( pMedium );
                DBG_ASSERT( bRet, "Error in DoSaveCompleted, can't be handled!");
            }

            // TODO/LATER: disconnect the new file from the storage for the case when pure saving is done
            //       if storing has corrupted the file, probably it must be restored either here or
            //       by the storage
            DELETEZ( pNewFile );
        }

        // TODO/LATER: there is no need in the following code in case HandsOff is not used,
        //             hope we will not have to introduce it back
//REMOVE            String aPasswd;
//REMOVE            if ( IsOwnStorageFormat_Impl( *GetMedium() ) && GetPasswd_Impl( GetMedium()->GetItemSet(), aPasswd ) )
//REMOVE            {
//REMOVE                try
//REMOVE                {
//REMOVE                    // the following code must throw an exception in case of failure
//REMOVE                    ::comphelper::OStorageHelper::SetCommonStoragePassword( GetMedium->GetStorage(), aPasswd );
//REMOVE                }
//REMOVE                catch( uno::Exception& )
//REMOVE                {
//REMOVE                    // TODO: handle the error
//REMOVE                }
//REMOVE            }
    }
    else
    {
        SetError( pNewFile->GetErrorCode() );

//REMOVE            // reconnect to the old storage
//REMOVE            if ( IsHandsOff() )
//REMOVE                DoSaveCompleted( pMedium );
//REMOVE            else
        DoSaveCompleted( 0 );

        DELETEZ( pNewFile );
    }

    if ( bCopyTo )
        DELETEZ( pNewFile );
    else if( !bOk )
        SetModified( sal_True );

    return bOk;
}

//------------------------------------------------------------------------

sal_Bool SfxObjectShell::LoadFrom( SfxMedium& /*rMedium*/ )
{
    DBG_ERROR( "Base implementation, must not be called in general!" );
    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::IsInformationLost()
{
    Sequence< PropertyValue > aProps = GetModel()->getArgs();
    ::rtl::OUString aFilterName;
    ::rtl::OUString aPreusedFilterName;
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
    {
        if ( aProps[nInd].Name.equalsAscii( "FilterName" ) )
            aProps[nInd].Value >>= aFilterName;
        else if ( aProps[nInd].Name.equalsAscii( "PreusedFilterName" ) )
            aProps[nInd].Value >>= aPreusedFilterName;
    }

    // if current filter can lead to information loss and it was used
    // for the latest store then the user should be asked to store in own format
    if ( aFilterName.getLength() && aFilterName.equals( aPreusedFilterName ) )
    {
        const SfxFilter *pFilt = GetMedium()->GetFilter();
        DBG_ASSERT( pFilt && aFilterName.equals( pFilt->GetName() ), "MediaDescriptor contains wrong filter!\n" );
        return ( pFilt && pFilt->IsAlienFormat() && !(pFilt->GetFilterFlags() & SFX_FILTER_SILENTEXPORT ) );
    }

    return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::CanReload_Impl()

/*  [Beschreibung]

    Interne Methode zum Feststellen, ob eine erneutes Laden des
    Dokuments (auch als RevertToSaved oder LastVersion bekannt)
    m"oglich ist.
*/

{
    return pMedium && HasName() && !IsInModalMode() && !pImp->bForbidReload;
}

//-------------------------------------------------------------------------

sal_uInt16 SfxObjectShell::GetHiddenInformationState( sal_uInt16 nStates )
{
    sal_uInt16 nState = 0;
    if ( nStates & HIDDENINFORMATION_DOCUMENTVERSIONS )
    {
        if ( GetMedium()->GetVersionList().getLength() )
            nState |= HIDDENINFORMATION_DOCUMENTVERSIONS;
    }

    return nState;
}

sal_Int16 SfxObjectShell::QueryHiddenInformation( HiddenWarningFact eFact, Window* pParent )
{
    sal_Int16 nRet = RET_YES;
    USHORT nResId = 0;
    SvtSecurityOptions::EOption eOption = static_cast< SvtSecurityOptions::EOption >( -1 );

    switch ( eFact )
    {
        case WhenSaving :
        {
            nResId = STR_HIDDENINFO_CONTINUE_SAVING;
            eOption = SvtSecurityOptions::E_DOCWARN_SAVEORSEND;
            break;
        }
        case WhenPrinting :
        {
            nResId = STR_HIDDENINFO_CONTINUE_PRINTING;
            eOption = SvtSecurityOptions::E_DOCWARN_PRINT;
            break;
        }
        case WhenSigning :
        {
            nResId = STR_HIDDENINFO_CONTINUE_SIGNING;
            eOption = SvtSecurityOptions::E_DOCWARN_SIGNING;
            break;
        }
        case WhenCreatingPDF :
        {
            nResId = STR_HIDDENINFO_CONTINUE_CREATEPDF;
            eOption = SvtSecurityOptions::E_DOCWARN_CREATEPDF;
            break;
        }
        default:
        {
            DBG_ERRORFILE( "SfxObjectShell::DetectHiddenInformation(): what fact?" );
        }
    }

    if ( eOption != -1 && SvtSecurityOptions().IsOptionSet( eOption ) )
    {
        String sMessage( SfxResId( STR_HIDDENINFO_CONTAINS ) );
        sal_uInt16 nWantedStates = HIDDENINFORMATION_RECORDEDCHANGES | HIDDENINFORMATION_NOTES;
        if ( eFact != WhenPrinting )
            nWantedStates |= HIDDENINFORMATION_DOCUMENTVERSIONS;
        sal_uInt16 nStates = GetHiddenInformationState( nWantedStates );
        bool bWarning = false;

        if ( ( nStates & HIDDENINFORMATION_RECORDEDCHANGES ) == HIDDENINFORMATION_RECORDEDCHANGES )
        {
            sMessage += String( SfxResId( STR_HIDDENINFO_RECORDCHANGES ) );
            sMessage += '\n';
            bWarning = true;
        }
        if ( ( nStates & HIDDENINFORMATION_NOTES ) == HIDDENINFORMATION_NOTES )
        {
            sMessage += String( SfxResId( STR_HIDDENINFO_NOTES ) );
            sMessage += '\n';
            bWarning = true;
        }
        if ( ( nStates & HIDDENINFORMATION_DOCUMENTVERSIONS ) == HIDDENINFORMATION_DOCUMENTVERSIONS )
        {
            sMessage += String( SfxResId( STR_HIDDENINFO_DOCVERSIONS ) );
            sMessage += '\n';
            bWarning = true;
        }

        if ( bWarning )
        {
            sMessage += '\n';
            sMessage += String( SfxResId( nResId ) );
            WarningBox aWBox( pParent, WB_YES_NO | WB_DEF_NO, sMessage );
            nRet = aWBox.Execute();
        }
    }

    return nRet;
}

sal_Bool SfxObjectShell::HasSecurityOptOpenReadOnly() const
{
    return sal_True;
}

sal_Bool SfxObjectShell::IsSecurityOptOpenReadOnly() const
{
    return IsLoadReadonly();
}

void SfxObjectShell::SetSecurityOptOpenReadOnly( sal_Bool _b )
{
    SetLoadReadonly( _b );
}

sal_Bool SfxObjectShell::LoadOwnFormat( SfxMedium& rMedium )
{
    uno::Reference< embed::XStorage > xStorage = rMedium.GetStorage();
    if ( xStorage.is() )
    {
//REMOVE            if ( rMedium.GetFileVersion() )
//REMOVE                xStor->SetVersion( rMedium.GetFileVersion() );

        // Password
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pPasswdItem, SfxStringItem, SID_PASSWORD, sal_False );
        if ( pPasswdItem || ERRCODE_IO_ABORT != CheckPasswd_Impl( this, SFX_APP()->GetPool(), pMedium ) )
        {
            ::rtl::OUString aPasswd;
            if ( GetPasswd_Impl(pMedium->GetItemSet(), aPasswd) )
            {
                try
                {
                    // the following code must throw an exception in case of failure
                    ::comphelper::OStorageHelper::SetCommonStoragePassword( xStorage, aPasswd );
                }
                catch( uno::Exception& )
                {
                    // TODO/LATER: handle the error code
                }
            }

            // load document
            return Load( rMedium );
        }
        return sal_False;
    }
    else
        return sal_False;
}

sal_Bool SfxObjectShell::SaveAsOwnFormat( SfxMedium& rMedium )
{
    uno::Reference< embed::XStorage > xStorage = rMedium.GetStorage();
    if( xStorage.is() )
    {
        sal_Int32 nVersion = rMedium.GetFilter()->GetVersion();

        // OASIS templates have own mediatypes ( SO7 also actually, but it is to late to use them here )
        sal_Bool bTemplate = ( rMedium.GetFilter()->IsOwnTemplateFormat() && nVersion > SOFFICE_FILEFORMAT_60 );

        SetupStorage( xStorage, nVersion, bTemplate );

        if ( HasBasic() )
        {
            // Initialize Basic
            GetBasicManager();

            // Save dialog/script container
            pImp->pBasicManager->storeLibrariesToStorage( xStorage );
        }

        return SaveAs( rMedium );
    }
    else return sal_False;
}

uno::Reference< embed::XStorage > SfxObjectShell::GetStorage()
{
    if ( !pImp->m_xDocStorage.is() )
    {
        OSL_ENSURE( pImp->m_bCreateTempStor, "The storage must exist already!\n" );
        try {
            // no notification is required the storage is set the first time
            pImp->m_xDocStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
            OSL_ENSURE( pImp->m_xDocStorage.is(), "The method must either return storage or throw an exception!" );

            SetupStorage( pImp->m_xDocStorage, SOFFICE_FILEFORMAT_CURRENT, sal_False );
            pImp->m_bCreateTempStor = sal_False;
            SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_STORAGECHANGED, this ) );
        }
        catch( uno::Exception& )
        {
            // TODO/LATER: error handling?
        }
    }

    OSL_ENSURE( pImp->m_xDocStorage.is(), "The document storage must be created!" );
    return pImp->m_xDocStorage;
}


sal_Bool SfxObjectShell::SaveChildren( BOOL bObjectsOnly )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveChildren" );

    sal_Bool bResult = sal_True;
    if ( pImp->mpObjectContainer )
    {
        sal_Bool bOasis = ( SotStorage::GetVersion( GetStorage() ) > SOFFICE_FILEFORMAT_60 );
        GetEmbeddedObjectContainer().StoreChildren(bOasis,bObjectsOnly);
    }

    return bResult;
}

sal_Bool SfxObjectShell::SaveAsChildren( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveAsChildren" );

    sal_Bool bResult = sal_True;

    uno::Reference < embed::XStorage > xStorage = rMedium.GetStorage();
    if ( !xStorage.is() )
        return sal_False;

    if ( xStorage == GetStorage() )
        return SaveChildren();

    sal_Bool bOasis = sal_True;
    if ( pImp->mpObjectContainer )
    {
        bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
        GetEmbeddedObjectContainer().StoreAsChildren(bOasis,SFX_CREATE_MODE_EMBEDDED == eCreateMode,xStorage);
    }

    if ( bResult )
        bResult = CopyStoragesOfUnknownMediaType( GetStorage(), xStorage );

    return bResult;
}

sal_Bool SfxObjectShell::SaveCompletedChildren( sal_Bool bSuccess )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveCompletedChildren" );

    sal_Bool bResult = sal_True;

    if ( pImp->mpObjectContainer )
    {
        uno::Sequence < ::rtl::OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
        for ( sal_Int32 n=0; n<aNames.getLength(); n++ )
        {
            uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObjectContainer().GetEmbeddedObject( aNames[n] );
            OSL_ENSURE( xObj.is(), "An empty entry in the embedded objects list!\n" );
            if ( xObj.is() )
            {
                uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    try
                    {
                        xPersist->saveCompleted( bSuccess );
                    }
                    catch( uno::Exception& )
                    {
                        // TODO/LATER: error handling
                        bResult = sal_False;
                        break;
                    }
                }
            }
        }
    }

    return bResult;
}

sal_Bool SfxObjectShell::SwitchChildrenPersistance( const uno::Reference< embed::XStorage >& xStorage,
                                                    sal_Bool bForceNonModified )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SwitchChildrenPersistence" );

    if ( !xStorage.is() )
    {
        // TODO/LATER: error handling
        return sal_False;
    }

    sal_Bool bResult = sal_True;

    if ( pImp->mpObjectContainer )
        pImp->mpObjectContainer->SetPersistentEntries(xStorage,bForceNonModified);

    return bResult;
}

// Never call this method directly, always use the DoSaveCompleted call
sal_Bool SfxObjectShell::SaveCompleted( const uno::Reference< embed::XStorage >& xStorage )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveCompleted" );

    sal_Bool bResult = sal_False;
    sal_Bool bSendNotification = sal_False;
    uno::Reference< embed::XStorage > xOldStorageHolder;

#ifdef DBG_UTIL
    // check for wrong creation of object container
    BOOL bHasContainer = ( pImp->mpObjectContainer != 0 );
#endif

    if ( !xStorage.is() || xStorage == GetStorage() )
    {
        // no persistence change
        bResult = SaveCompletedChildren( sal_False );
    }
    else
    {
        if ( pImp->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( xStorage );

        bResult = SwitchChildrenPersistance( xStorage, sal_True );
    }

    if ( bResult )
    {
        if ( xStorage.is() && pImp->m_xDocStorage != xStorage )
        {
            // make sure that until the storage is assigned the object container is not created by accident!
            DBG_ASSERT( bHasContainer == (pImp->mpObjectContainer != 0), "Wrong storage in object container!" );
            xOldStorageHolder = pImp->m_xDocStorage;
            pImp->m_xDocStorage = xStorage;
            bSendNotification = sal_True;

            if ( IsEnableSetModified() )
                SetModified( sal_False );
        }
    }
    else
    {
        if ( pImp->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( pImp->m_xDocStorage );

        // let already successfully connected objects be switched back
        SwitchChildrenPersistance( pImp->m_xDocStorage, sal_True );
    }

    if ( bSendNotification )
    {
        SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_STORAGECHANGED, this ) );
    }

    return bResult;
}


sal_Bool StoragesOfUnknownMediaTypeAreCopied_Impl( const uno::Reference< embed::XStorage >& xSource,
                                                    const uno::Reference< embed::XStorage >& xTarget )
{
    OSL_ENSURE( xSource.is() && xTarget.is(), "Source and/or target storages are not available!\n" );
    if ( !xSource.is() || !xTarget.is() || xSource == xTarget )
        return sal_True;

    try
    {
        uno::Sequence< ::rtl::OUString > aSubElements = xSource->getElementNames();
        for ( sal_Int32 nInd = 0; nInd < aSubElements.getLength(); nInd++ )
        {
            if ( xSource->isStorageElement( aSubElements[nInd] ) )
            {
                ::rtl::OUString aMediaType;
                ::rtl::OUString aMediaTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
                sal_Bool bGotMediaType = sal_False;

                try
                {
                    uno::Reference< embed::XOptimizedStorage > xOptStorage( xSource, uno::UNO_QUERY_THROW );
                    bGotMediaType =
                        ( xOptStorage->getElementPropertyValue( aSubElements[nInd], aMediaTypePropName ) >>= aMediaType );
                }
                catch( uno::Exception& )
                {}

                if ( !bGotMediaType )
                {
                    uno::Reference< embed::XStorage > xSubStorage;
                    try {
                        xSubStorage = xSource->openStorageElement( aSubElements[nInd], embed::ElementModes::READ );
                    } catch( uno::Exception& )
                    {}

                    if ( !xSubStorage.is() )
                    {
                        xSubStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        xSource->copyStorageElementLastCommitTo( aSubElements[nInd], xSubStorage );
                    }

                    uno::Reference< beans::XPropertySet > xProps( xSubStorage, uno::UNO_QUERY_THROW );
                    bGotMediaType = ( xProps->getPropertyValue( aMediaTypePropName ) >>= aMediaType );
                }

                // TODO/LATER: there should be a way to detect whether an object with such a MediaType can exist
                //             probably it should be placed in the MimeType-ClassID table or in standalone table
                if ( aMediaType.getLength()
                  && aMediaType.compareToAscii( "application/vnd.sun.star.oleobject" ) != COMPARE_EQUAL )
                {
                    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
                    aDataFlavor.MimeType = aMediaType;
                    sal_uInt32 nFormat = SotExchange::GetFormat( aDataFlavor );

                    switch ( nFormat )
                    {
                        case SOT_FORMATSTR_ID_STARWRITER_60 :
                        case SOT_FORMATSTR_ID_STARWRITERWEB_60 :
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_60 :
                        case SOT_FORMATSTR_ID_STARDRAW_60 :
                        case SOT_FORMATSTR_ID_STARIMPRESS_60 :
                        case SOT_FORMATSTR_ID_STARCALC_60 :
                        case SOT_FORMATSTR_ID_STARCHART_60 :
                        case SOT_FORMATSTR_ID_STARMATH_60 :
                        case SOT_FORMATSTR_ID_STARWRITER_8:
                        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
                        case SOT_FORMATSTR_ID_STARDRAW_8:
                        case SOT_FORMATSTR_ID_STARIMPRESS_8:
                        case SOT_FORMATSTR_ID_STARCALC_8:
                        case SOT_FORMATSTR_ID_STARCHART_8:
                        case SOT_FORMATSTR_ID_STARMATH_8:
                            break;

                        default:
                        {
                            if ( !xTarget->hasByName( aSubElements[nInd] ) )
                                return sal_False;
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Cant check storage consistency!\n" );
    }

    return sal_True;
}


sal_Bool SfxObjectShell::SwitchPersistance( const uno::Reference< embed::XStorage >& xStorage )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SwitchPersistance" );

    sal_Bool bResult = sal_False;
#ifdef DBG_UTIL
    // check for wrong creation of object container
    BOOL bHasContainer = ( pImp->mpObjectContainer != 0 );
#endif
    if ( xStorage.is() )
    {
        if ( pImp->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( xStorage );
        bResult = SwitchChildrenPersistance( xStorage );

        // TODO/LATER: substorages that have unknown mimetypes probably should be copied to the target storage here
        OSL_ENSURE( StoragesOfUnknownMediaTypeAreCopied_Impl( pImp->m_xDocStorage, xStorage ),
                    "Some of substorages with unknown mimetypes is lost!" );
    }

    if ( bResult )
    {
        // make sure that until the storage is assigned the object container is not created by accident!
        DBG_ASSERT( bHasContainer == (pImp->mpObjectContainer != 0), "Wrong storage in object container!" );
        if ( pImp->m_xDocStorage != xStorage )
            DoSaveCompleted( new SfxMedium( xStorage, GetMedium()->GetBaseURL() ) );

        if ( IsEnableSetModified() )
            SetModified( sal_True ); // ???
    }

    return bResult;
}

sal_Bool SfxObjectShell::CopyStoragesOfUnknownMediaType( const uno::Reference< embed::XStorage >& xSource,
                                                         const uno::Reference< embed::XStorage >& xTarget )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::CopyStoragesOfUnknownMediaType" );

    // This method does not commit the target storage and should not do it
    sal_Bool bResult = sal_True;

    try
    {
        uno::Sequence< ::rtl::OUString > aSubElements = xSource->getElementNames();
        for ( sal_Int32 nInd = 0; nInd < aSubElements.getLength(); nInd++ )
        {
            if ( aSubElements[nInd].equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Configurations" ) ) ) )
            {
                // The workaround for compatibility with SO7, "Configurations" substorage must be preserved
                if ( xSource->isStorageElement( aSubElements[nInd] ) )
                {
                    OSL_ENSURE( !xTarget->hasByName( aSubElements[nInd] ),
                                "The target storage is an output storage, the element should not exist in the target!\n" );

                    xSource->copyElementTo( aSubElements[nInd], xTarget, aSubElements[nInd] );
                }
            }
            else if ( xSource->isStorageElement( aSubElements[nInd] ) )
            {
                ::rtl::OUString aMediaType;
                ::rtl::OUString aMediaTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
                sal_Bool bGotMediaType = sal_False;

                try
                {
                    uno::Reference< embed::XOptimizedStorage > xOptStorage( xSource, uno::UNO_QUERY_THROW );
                    bGotMediaType =
                        ( xOptStorage->getElementPropertyValue( aSubElements[nInd], aMediaTypePropName ) >>= aMediaType );
                }
                catch( uno::Exception& )
                {}

                if ( !bGotMediaType )
                {
                    uno::Reference< embed::XStorage > xSubStorage;
                    try {
                        xSubStorage = xSource->openStorageElement( aSubElements[nInd], embed::ElementModes::READ );
                    } catch( uno::Exception& )
                    {}

                    if ( !xSubStorage.is() )
                    {
                        // TODO/LATER: as optimization in future a substorage of target storage could be used
                        //             instead of the temporary storage; this substorage should be removed later
                        //             if the MimeType is wrong
                        xSubStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        xSource->copyStorageElementLastCommitTo( aSubElements[nInd], xSubStorage );
                    }

                    uno::Reference< beans::XPropertySet > xProps( xSubStorage, uno::UNO_QUERY_THROW );
                    bGotMediaType = ( xProps->getPropertyValue( aMediaTypePropName ) >>= aMediaType );
                }

                // TODO/LATER: there should be a way to detect whether an object with such a MediaType can exist
                //             probably it should be placed in the MimeType-ClassID table or in standalone table
                if ( aMediaType.getLength()
                  && aMediaType.compareToAscii( "application/vnd.sun.star.oleobject" ) != COMPARE_EQUAL )
                {
                    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
                    aDataFlavor.MimeType = aMediaType;
                    sal_uInt32 nFormat = SotExchange::GetFormat( aDataFlavor );

                    switch ( nFormat )
                    {
                        case SOT_FORMATSTR_ID_STARWRITER_60 :
                        case SOT_FORMATSTR_ID_STARWRITERWEB_60 :
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_60 :
                        case SOT_FORMATSTR_ID_STARDRAW_60 :
                        case SOT_FORMATSTR_ID_STARIMPRESS_60 :
                        case SOT_FORMATSTR_ID_STARCALC_60 :
                        case SOT_FORMATSTR_ID_STARCHART_60 :
                        case SOT_FORMATSTR_ID_STARMATH_60 :
                        case SOT_FORMATSTR_ID_STARWRITER_8:
                        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
                        case SOT_FORMATSTR_ID_STARDRAW_8:
                        case SOT_FORMATSTR_ID_STARIMPRESS_8:
                        case SOT_FORMATSTR_ID_STARCALC_8:
                        case SOT_FORMATSTR_ID_STARCHART_8:
                        case SOT_FORMATSTR_ID_STARMATH_8:
                            break;

                        default:
                        {
                            OSL_ENSURE(
                                aSubElements[nInd].equalsAscii( "Configurations2" ) || !xTarget->hasByName( aSubElements[nInd] ),
                                "The target storage is an output storage, the element should not exist in the target!\n" );

                            if ( !xTarget->hasByName( aSubElements[nInd] ) )
                            {
                                xSource->copyElementTo( aSubElements[nInd], xTarget, aSubElements[nInd] );
                            }
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {
        bResult = sal_False;
        // TODO/LATER: a specific error could be provided
    }

    return bResult;
}

sal_Bool SfxObjectShell::GenerateAndStoreThumbnail( sal_Bool bEncrypted,
                                                    sal_Bool bSigned,
                                                    sal_Bool bIsTemplate,
                                                    const uno::Reference< embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::GenerateAndStoreThumbnail" );

    sal_Bool bResult = sal_False;

    try {
        uno::Reference< embed::XStorage > xThumbnailStor =
                                        xStor->openStorageElement( ::rtl::OUString::createFromAscii( "Thumbnails" ),
                                                                    embed::ElementModes::READWRITE );
        if ( xThumbnailStor.is() )
        {
            uno::Reference< io::XStream > xStream = xThumbnailStor->openStreamElement(
                                                        ::rtl::OUString::createFromAscii( "thumbnail.png" ),
                                                        embed::ElementModes::READWRITE );

            if ( xStream.is() && WriteThumbnail( bEncrypted, bSigned, bIsTemplate, xStream ) )
            {
                uno::Reference< embed::XTransactedObject > xTransact( xThumbnailStor, uno::UNO_QUERY_THROW );
                xTransact->commit();
                bResult = sal_True;
            }
        }
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}

sal_Bool SfxObjectShell::WriteThumbnail( sal_Bool bEncrypted,
                                         sal_Bool bSigned,
                                         sal_Bool bIsTemplate,
                                         const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;

    if ( xStream.is() )
    {
        try {
            uno::Reference< io::XTruncate > xTruncate( xStream->getOutputStream(), uno::UNO_QUERY_THROW );
            xTruncate->truncate();

            if ( bEncrypted )
            {
                sal_uInt16 nResID = GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl(
                                        ::rtl::OUString::createFromAscii( GetFactory().GetShortName() ),
                                        bIsTemplate );
                if ( nResID )
                {
                    if ( !bSigned )
                    {
                        bResult = GraphicHelper::getThumbnailReplacement_Impl( nResID, xStream );
                    }
                    else
                    {
                        // retrieve the bitmap and write a signature bitmap over it
                        SfxResId aResId( nResID );
                        BitmapEx aThumbBitmap( aResId );
                        bResult = GraphicHelper::getSignedThumbnailFormatFromBitmap_Impl( aThumbBitmap, xStream );
                    }
                }
            }
            else
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    GetPreviewMetaFile( sal_False );
                if ( pMetaFile )
                {
                    bResult = GraphicHelper::getThumbnailFormatFromGDI_Impl(
                                pMetaFile.get(), bSigned, xStream );
                }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return bResult;
}

void SfxObjectShell::UpdateLinks()
{
}

sal_Bool SfxObjectShell::QuerySaveSizeExceededModules_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    if ( !HasBasic() )
        return sal_True;

    if ( !pImp->pBasicManager->isValid() )
        GetBasicManager();
    uno::Sequence< rtl::OUString > sModules;
    if ( xHandler.is() )
    {
        if( pImp->pBasicManager->LegacyPsswdBinaryLimitExceeded( sModules ) )
        {
            ModuleSizeExceeded* pReq =  new ModuleSizeExceeded( sModules );
            uno::Reference< task::XInteractionRequest > xReq( pReq );
            xHandler->handle( xReq );
            return pReq->isApprove();
        }
    }
    // No interaction handler, default is to continue to save
    return sal_True;
}
// -----------------------------------------------------------------------------
uno::Reference< task::XInteractionHandler > SfxObjectShell::getInteractionHandler() const
{
    uno::Reference< task::XInteractionHandler > xRet;
    if ( GetMedium() )
        xRet = GetMedium()->GetInteractionHandler();
    return xRet;
}
