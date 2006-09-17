/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mailmodel.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:33:06 $
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
// includes --------------------------------------------------------------

#ifndef  _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef  _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef  _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef  _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XSIMPLEMAILCLIENTSUPPLIER_HPP_
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SIMPLEMAILCLIENTFLAGS_HPP_
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_CERTIFICATEVALIDITY_HPP_
#include <com/sun/star/security/CertificateValidity.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_DOCUMENTSIGNATURESINFORMATION_HPP_
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_XDOCUMENTDIGITALSIGNATURES_HPP_
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif

#ifndef _RTL_TEXTENC_H
#include <rtl/textench.h>
#endif
#ifndef _RTL_URI_H_
#include <rtl/uri.h>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include <mailmodelapi.hxx>
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "sfxsids.hrc"
#include "mailwindow.hrc"
#include "dialog.hrc"

#include <unotools/tempfile.hxx>
#include <unotools/configitem.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>
#include <svtools/useroptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

// --------------------------------------------------------------
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::rtl;

namespace css = ::com::sun::star;

// class AddressList_Impl ------------------------------------------------

typedef String* AddressItemPtr_Impl;
DECLARE_LIST( AddressList_Impl, AddressItemPtr_Impl )

// class SfxMailModel -----------------------------------------------

void SfxMailModel::ClearList( AddressList_Impl* pList )
{
    if ( pList )
    {
        ULONG i, nCount = pList->Count();
        for ( i = 0; i < nCount; ++i )
            delete pList->GetObject(i);
        pList->Clear();
    }
}

void SfxMailModel::MakeValueList( AddressList_Impl* pList, String& rValueList )
{
    rValueList.Erase();
    if ( pList )
    {
        ULONG i, nCount = pList->Count();
        for ( i = 0; i < nCount; ++i )
        {
            if ( rValueList.Len() > 0 )
                rValueList += ',';
            rValueList += *pList->GetObject(i);
        }
    }
}

sal_Bool HasDocumentValidSignature( const css::uno::Reference< css::frame::XModel >& xModel )
{
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet( xModel, css::uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            Any a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasValidSignatures" )));
            sal_Bool bReturn = sal_Bool();
            if ( a >>= bReturn )
                return bReturn;
        }
    }
    catch ( css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }

    return sal_False;
}

sal_Int32 SfxMailModel::GetCount() const
{
    return maAttachedDocuments.size();
}

sal_Bool SfxMailModel::IsEmpty() const
{
    return maAttachedDocuments.empty();
}

SfxMailModel::SaveResult SfxMailModel::SaveDocumentAsFormat(
    const rtl::OUString& aSaveFileName,
    const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
    const rtl::OUString& rType,
    rtl::OUString& rFileNamePath )
{
    SaveResult  eRet( SAVE_ERROR );

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = ::comphelper::getProcessServiceFactory();
    if (!xSMGR.is())
        return eRet;

    const rtl::OUString aModuleManager( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ModuleManager" ));
    css::uno::Reference< css::frame::XModuleManager > xModuleManager( xSMGR->createInstance( aModuleManager ), css::uno::UNO_QUERY_THROW );
    if ( !xModuleManager.is() )
        return eRet;

    rtl::OUString aModule;
    try
    {
         aModule = xModuleManager->identify( xFrameOrModel );
    }
    catch ( css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }

    css::uno::Reference< css::frame::XFrame > xFrame( xFrameOrModel, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XModel > xModel( xFrameOrModel, css::uno::UNO_QUERY );
    if ( xFrame.is() )
    {
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }

    // We need at least a valid module name and model reference
    if (( aModule.getLength() > 0 ) && xModel.is() )
    {
        bool bModified( false );
        bool bHasLocation( false );
        bool bStoreTo( false );

        css::uno::Reference< css::util::XModifiable > xModifiable( xModel, css::uno::UNO_QUERY );
        css::uno::Reference< css::frame::XStorable > xStorable( xModel, css::uno::UNO_QUERY );

        if ( xModifiable.is() )
            bModified = xModifiable->isModified();
        if ( xStorable.is() )
        {
            rtl::OUString aLocation = xStorable->getLocation();
            INetURLObject aFileObj( aLocation );

            bool bPrivateProtocol = ( aFileObj.GetProtocol() == INET_PROT_PRIV_SOFFICE );

            bHasLocation = ( aLocation.getLength() > 0 ) && !bPrivateProtocol;
            OSL_ASSERT( !bPrivateProtocol );
        }
        if ( rType.getLength() > 0 )
            bStoreTo = true;

        if ( xStorable.is() )
        {
            rtl::OUString aFilterName;
            rtl::OUString aTypeName( rType );
            rtl::OUString aFileName;
            rtl::OUString aExtension;

            css::uno::Reference< css::container::XContainerQuery > xContainerQuery(
                xSMGR->createInstance( rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ))),
                    css::uno::UNO_QUERY );

            if ( bStoreTo )
            {
                // Retrieve filter from type
                css::uno::Sequence< css::beans::NamedValue > aQuery( 2 );
                aQuery[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Type" ));
                aQuery[0].Value = css::uno::makeAny( aTypeName );
                aQuery[1].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentService" ));
                aQuery[1].Value = css::uno::makeAny( aModule );

                css::uno::Reference< css::container::XEnumeration > xEnumeration =
                    xContainerQuery->createSubSetEnumerationByProperties( aQuery );

                if ( xEnumeration->hasMoreElements() )
                {
                    ::comphelper::SequenceAsHashMap aFilterPropsHM( xEnumeration->nextElement() );
                    aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                ::rtl::OUString::createFromAscii( "Name" ),
                                                ::rtl::OUString() );
                }

                if ( bHasLocation )
                {
                    // Retrieve filter from media descriptor
                    ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
                    rtl::OUString aOrgFilterName = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" )),
                                    ::rtl::OUString() );
                    if ( aOrgFilterName == aFilterName )
                    {
                        // We should save the document in the original format. Therefore this
                        // is not a storeTo operation. To support signing in this case, reset
                        // bStoreTo flag.
                        bStoreTo = false;
                    }
                }
            }
            else
            {
                if ( bHasLocation )
                {
                    // Retrieve filter from media descriptor
                    ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
                    aFilterName = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" )),
                                    ::rtl::OUString() );
                }

                if ( !bHasLocation || ( aFilterName.getLength() == 0 ))
                {
                    // Retrieve the user defined default filter
                    css::uno::Reference< css::container::XNameAccess > xNameAccess( xModuleManager, css::uno::UNO_QUERY );
                    try
                    {
                        ::comphelper::SequenceAsHashMap aFilterPropsHM( xNameAccess->getByName( aModule ) );
                        aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                    ::rtl::OUString::createFromAscii( "ooSetupFactoryDefaultFilter" ),
                                                    ::rtl::OUString() );
                        css::uno::Reference< css::container::XNameAccess > xNameAccess2(
                            xContainerQuery, css::uno::UNO_QUERY );
                        if ( xNameAccess2.is() )
                        {
                            ::comphelper::SequenceAsHashMap aFilterPropsHM2( xNameAccess2->getByName( aFilterName ) );
                            aTypeName = aFilterPropsHM2.getUnpackedValueOrDefault(
                                                        ::rtl::OUString::createFromAscii( "Type" ),
                                                        ::rtl::OUString() );
                        }
                    }
                    catch ( css::container::NoSuchElementException& )
                    {
                    }
                    catch ( css::beans::UnknownPropertyException& )
                    {
                    }
                }
            }

            // No filter found => error
            // No type and no location => error
            if (( aFilterName.getLength() == 0 ) ||
                (( aTypeName.getLength() == 0 ) && !bHasLocation ))
                return eRet;

            // Determine filen name and extension
            if ( bHasLocation && !bStoreTo )
            {
                INetURLObject aFileObj( xStorable->getLocation() );
                aExtension = (rtl::OUString)aFileObj.getExtension();
            }
            else
            {
                css::uno::Reference< container::XNameAccess > xTypeDetection(
                    xSMGR->createInstance( ::rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ))),
                    css::uno::UNO_QUERY );


                if ( xTypeDetection.is() )
                {
                    try
                    {
                        ::comphelper::SequenceAsHashMap aTypeNamePropsHM( xTypeDetection->getByName( aTypeName ) );
                        uno::Sequence< ::rtl::OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                                        ::rtl::OUString::createFromAscii( "Extensions" ),
                                                        ::uno::Sequence< ::rtl::OUString >() );
                        if ( aExtensions.getLength() )
                            aExtension = aExtensions[0];
                    }
                    catch ( css::container::NoSuchElementException& )
                    {
                    }
                }
            }

            // Use provided save file name. If empty determine file name
            aFileName = aSaveFileName;
            if ( aFileName.getLength() == 0 )
            {
                if ( !bHasLocation )
                {
                    // Create a noname file name with the correct extension
                    const rtl::OUString aNoNameFileName( RTL_CONSTASCII_USTRINGPARAM( "noname" ));
                    aFileName = aNoNameFileName;
                }
                else
                {
                    // Determine file name from model
                    INetURLObject aFileObj( xStorable->getLocation() );
                    aFileObj.removeExtension();
                    aFileName = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::NO_DECODE );
                }
            }

            // No file name => error
            if ( aFileName.getLength() == 0 )
                return eRet;

            OSL_ASSERT( aFilterName.getLength() > 0 );
            OSL_ASSERT( aFileName.getLength() > 0 );

            // Creates a temporary directory to store a predefined file into it.
            // This makes it possible to store the file for "send document as e-mail"
            // with the original file name. We cannot use the original file as
            // some mail programs need exclusive access.
            ::utl::TempFile aTempDir( NULL, sal_True );

            INetURLObject aFilePathObj( aTempDir.GetURL() );
            aFilePathObj.insertName( aFileName );
            aFilePathObj.setExtension( aExtension );

            rtl::OUString aFileURL = aFilePathObj.GetMainURL( INetURLObject::NO_DECODE );

            sal_Int32 nNumArgs(0);
            const rtl::OUString aPasswordPropName( RTL_CONSTASCII_USTRINGPARAM( "Password" ));
            css::uno::Sequence< css::beans::PropertyValue > aArgs( ++nNumArgs );
            aArgs[nNumArgs-1].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
            aArgs[nNumArgs-1].Value = css::uno::makeAny( aFilterName );

            ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
            rtl::OUString aPassword = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                            aPasswordPropName,
                                            ::rtl::OUString() );
            if ( aPassword.getLength() > 0 )
            {
                aArgs.realloc( ++nNumArgs );
                aArgs[nNumArgs-1].Name = aPasswordPropName;
                aArgs[nNumArgs-1].Value = css::uno::makeAny( aPassword );
            }

            if ( bModified || !bHasLocation || bStoreTo )
            {
                // Document is modified, is newly created or should be stored in a special format
                try
                {
                    css::uno::Reference< css::util::XURLTransformer > xURLTransformer(
                        xSMGR->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                        css::uno::UNO_QUERY );

                    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( xFrame, css::uno::UNO_QUERY );
                    css::uno::Reference< css::frame::XDispatch > xDispatch;

                    css::util::URL aURL;
                    css::uno::Sequence< css::beans::PropertyValue > aDispatchArgs;

                    if ( xURLTransformer.is() )
                    {
                        aURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PrepareMailExport" ));
                        xURLTransformer->parseStrict( aURL );
                    }

                    if ( xDispatchProvider.is() )
                    {
                        xDispatch = css::uno::Reference< css::frame::XDispatch >(
                            xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 ));
                        if ( xDispatch.is() )
                        {
                            try
                            {
                                xDispatch->dispatch( aURL, aDispatchArgs );
                            }
                            catch ( css::uno::RuntimeException& )
                            {
                                throw;
                            }
                            catch ( css::uno::Exception& )
                            {
                            }
                        }
                    }

                    xStorable->storeToURL( aFileURL, aArgs );
                    rFileNamePath = aFileURL;
                    eRet = SAVE_SUCCESSFULL;

                    // #i30432# notify that export is finished - the Writer may want to restore removed content
                    if ( xURLTransformer.is() )
                    {
                        aURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:MailExportFinished" ));
                        xURLTransformer->parseStrict( aURL );
                    }

                    if ( xDispatchProvider.is() )
                    {
                        xDispatch = css::uno::Reference< css::frame::XDispatch >(
                            xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 ));
                        if ( xDispatch.is() )
                        {
                            try
                            {
                                xDispatch->dispatch( aURL, aDispatchArgs );
                            }
                            catch ( css::uno::RuntimeException& )
                            {
                                throw;
                            }
                            catch ( css::uno::Exception& )
                            {
                            }
                        }
                    }

                    // If the model is not modified, it could be modified by the dispatch calls.
                    // Therefore set back to modified = false. This should not hurt if we call
                    // on a non-modified model.
                    if ( !bModified )
                    {
                        try
                        {
                            xModifiable->setModified( sal_False );
                        }
                        catch( com::sun::star::beans::PropertyVetoException& )
                        {
                        }
                    }
                }
                catch ( com::sun::star::io::IOException& )
                {
                    eRet = SAVE_ERROR;
                }
            }
            else
            {
                // We need 1:1 copy of the document to preserve an added signature.
                aArgs.realloc( ++nNumArgs );
                aArgs[nNumArgs-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CopyStreamIfPossible" ) );
                aArgs[nNumArgs-1].Value = css::uno::makeAny( (sal_Bool)sal_True );

                try
                {
                    xStorable->storeToURL( aFileURL, aArgs );
                    rFileNamePath = aFileURL;
                    eRet = SAVE_SUCCESSFULL;
                }
                catch ( com::sun::star::io::IOException& )
                {
                    eRet = SAVE_ERROR;
                }
            }
        }
    }

    return eRet;
}

SfxMailModel::SfxMailModel() :
    mpToList    ( NULL ),
    mpCcList    ( NULL ),
    mpBccList   ( NULL ),
    mePriority  ( PRIO_NORMAL ),
    mbLoadDone  ( sal_True )
{
}

SfxMailModel::~SfxMailModel()
{
    ClearList( mpToList );
    delete mpToList;
    ClearList( mpCcList );
    delete mpCcList;
    ClearList( mpBccList );
    delete mpBccList;
}

void SfxMailModel::AddAddress( const String& rAddress, AddressRole eRole )
{
    // don't add a empty address
    if ( rAddress.Len() > 0 )
    {
        AddressList_Impl* pList = NULL;
        if ( ROLE_TO == eRole )
        {
            if ( !mpToList )
                // create the list
                mpToList = new AddressList_Impl;
            pList = mpToList;
        }
        else if ( ROLE_CC == eRole )
        {
            if ( !mpCcList )
                // create the list
                mpCcList = new AddressList_Impl;
            pList = mpCcList;
        }
        else if ( ROLE_BCC == eRole )
        {
            if ( !mpBccList )
                // create the list
                mpBccList = new AddressList_Impl;
            pList = mpBccList;
        }
        else
        {
            DBG_ERRORFILE( "invalid address role" );
        }

        if ( pList )
        {
            // add address to list
            AddressItemPtr_Impl pAddress = new String( rAddress );
            pList->Insert( pAddress, LIST_APPEND );
        }
    }
}

SfxMailModel::SendMailResult SfxMailModel::AttachDocument(
    const ::rtl::OUString& sDocumentType,
    const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
    const ::rtl::OUString& sAttachmentTitle )
{
    rtl::OUString sFileName;

    SaveResult eSaveResult = SaveDocumentAsFormat( sAttachmentTitle, xFrameOrModel, sDocumentType, sFileName );
    if ( eSaveResult == SAVE_SUCCESSFULL && ( sFileName.getLength() > 0 ) )
        maAttachedDocuments.push_back(sFileName);
    return eSaveResult == SAVE_SUCCESSFULL ? SEND_MAIL_OK : SEND_MAIL_ERROR;
}

SfxMailModel::SendMailResult SfxMailModel::Send( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    OSL_ENSURE(!maAttachedDocuments.empty(),"No document added!");
    SendMailResult  eResult = SEND_MAIL_ERROR;
    if ( !maAttachedDocuments.empty() )
    {
        Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
        if ( xMgr.is() )
        {
            Reference< XSimpleMailClientSupplier >  xSimpleMailClientSupplier;

            // Prefer the SimpleSystemMail service if available
            xSimpleMailClientSupplier = Reference< XSimpleMailClientSupplier >(
                xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleSystemMail" ))),
                UNO_QUERY );

            if ( ! xSimpleMailClientSupplier.is() )
            {
                xSimpleMailClientSupplier = Reference< XSimpleMailClientSupplier >(
                    xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleCommandMail" ))),
                    UNO_QUERY );
            }

            if ( xSimpleMailClientSupplier.is() )
            {
                Reference< XSimpleMailClient > xSimpleMailClient = xSimpleMailClientSupplier->querySimpleMailClient();

                if ( !xSimpleMailClient.is() )
                {
                    // no mail client support => message box!
                    return SEND_MAIL_ERROR;
                }

                // we have a simple mail client
                Reference< XSimpleMailMessage > xSimpleMailMessage = xSimpleMailClient->createSimpleMailMessage();
                if ( xSimpleMailMessage.is() )
                {
                    sal_Int32 nSendFlags = SimpleMailClientFlags::DEFAULTS;
                    if ( maFromAddress.Len() == 0 )
                    {
                        // from address not set, try figure out users e-mail address
                        CreateFromAddress_Impl( maFromAddress );
                    }
                    xSimpleMailMessage->setOriginator( maFromAddress );

                    sal_Int32 nToCount      = mpToList ? mpToList->Count() : 0;
                    sal_Int32 nCcCount      = mpCcList ? mpCcList->Count() : 0;
                    sal_Int32 nCcSeqCount   = nCcCount;

                    // set recipient (only one) for this simple mail server!!
                    if ( nToCount > 1 )
                    {
                        nCcSeqCount = nToCount - 1 + nCcCount;
                        xSimpleMailMessage->setRecipient( *mpToList->GetObject( 0 ));
                        nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                    }
                    else if ( nToCount == 1 )
                    {
                        xSimpleMailMessage->setRecipient( *mpToList->GetObject( 0 ));
                        nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                    }

                    // all other recipient must be handled with CC recipients!
                    if ( nCcSeqCount > 0 )
                    {
                        sal_Int32               nIndex = 0;
                        Sequence< OUString >    aCcRecipientSeq;

                        aCcRecipientSeq.realloc( nCcSeqCount );
                        if ( nCcSeqCount > nCcCount )
                        {
                            for ( sal_Int32 i = 1; i < nToCount; ++i )
                            {
                                aCcRecipientSeq[nIndex++] = *mpToList->GetObject(i);
                            }
                        }

                        for ( sal_Int32 i = 0; i < nCcCount; i++ )
                        {
                            aCcRecipientSeq[nIndex++] = *mpCcList->GetObject(i);
                        }
                        xSimpleMailMessage->setCcRecipient( aCcRecipientSeq );
                    }

                    sal_Int32 nBccCount = mpBccList ? mpBccList->Count() : 0;
                    if ( nBccCount > 0 )
                    {
                        Sequence< OUString > aBccRecipientSeq( nBccCount );
                        for ( sal_Int32 i = 0; i < nBccCount; ++i )
                        {
                            aBccRecipientSeq[i] = *mpBccList->GetObject(i);
                        }
                        xSimpleMailMessage->setBccRecipient( aBccRecipientSeq );
                    }

                    Sequence< OUString > aAttachmentSeq(&(maAttachedDocuments[0]),maAttachedDocuments.size());

                    xSimpleMailMessage->setSubject( maSubject );
                    xSimpleMailMessage->setAttachement( aAttachmentSeq );

                    sal_Bool bSend( sal_False );
                    try
                    {
                        xSimpleMailClient->sendSimpleMailMessage( xSimpleMailMessage, nSendFlags );
                        bSend = sal_True;
                    }
                    catch ( IllegalArgumentException& )
                    {
                    }
                    catch ( Exception& )
                    {
                    }

                    if ( bSend == sal_False )
                    {
                        css::uno::Reference< css::awt::XWindow > xParentWindow = xFrame->getContainerWindow();

                        ::vos::OGuard aGuard( Application::GetSolarMutex() );
                        Window* pParentWindow = VCLUnoHelper::GetWindow( xParentWindow );

                        ErrorBox aBox( pParentWindow, SfxResId( RID_ERRBOX_MAIL_CONFIG ));
                        aBox.Execute();
                        eResult = SEND_MAIL_CANCELLED;
                    }
                    else
                        eResult = SEND_MAIL_OK;
                }
            }
        }
    }
    else
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

SfxMailModel::SendMailResult SfxMailModel::SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame, const rtl::OUString& rTypeName )
{
    SaveResult      eSaveResult;
    SendMailResult  eResult = SEND_MAIL_ERROR;
    rtl::OUString   aFileName;

    eSaveResult = SaveDocumentAsFormat( rtl::OUString(), xFrame, rTypeName, aFileName );

    if ( eSaveResult == SAVE_SUCCESSFULL )
    {
        maAttachedDocuments.push_back( aFileName );
        return Send( xFrame );
    }
    else if ( eSaveResult == SAVE_CANCELLED )
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

// functions -------------------------------------------------------------

BOOL CreateFromAddress_Impl( String& rFrom )

/*  [Beschreibung]

    Diese Funktion versucht mit Hilfe des IniManagers eine From-Adresse
    zu erzeugen. daf"ur werden die Felder 'Vorname', 'Name' und 'EMail'
    aus der Applikations-Ini-Datei ausgelesen. Sollten diese Felder
    nicht gesetzt sein, wird FALSE zur"uckgegeben.

    [R"uckgabewert]

    TRUE:   Adresse konnte erzeugt werden.
    FALSE:  Adresse konnte nicht erzeugt werden.
*/

{
    SvtUserOptions aUserCFG;
    String aName        = aUserCFG.GetLastName  ();
    String aFirstName   = aUserCFG.GetFirstName ();
    if ( aFirstName.Len() || aName.Len() )
    {
        if ( aFirstName.Len() )
        {
            rFrom = TRIM( aFirstName );

            if ( aName.Len() )
                rFrom += ' ';
        }
        rFrom += TRIM( aName );
        // unerlaubte Zeichen entfernen
        rFrom.EraseAllChars( '<' );
        rFrom.EraseAllChars( '>' );
        rFrom.EraseAllChars( '@' );
    }
    String aEmailName = aUserCFG.GetEmail();

    // unerlaubte Zeichen entfernen
    aEmailName.EraseAllChars( '<' );
    aEmailName.EraseAllChars( '>' );

    if ( aEmailName.Len() )
    {
        if ( rFrom.Len() )
            rFrom += ' ';
        ( ( rFrom += '<' ) += TRIM( aEmailName ) ) += '>';
    }
    else
        rFrom.Erase();
    return ( rFrom.Len() > 0 );
}
