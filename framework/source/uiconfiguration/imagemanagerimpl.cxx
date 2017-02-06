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

#include "imagemanagerimpl.hxx"
#include <xml/imagesconfiguration.hxx>
#include <uiconfiguration/graphicnameaccess.hxx>
#include <services.h>

#include "properties.h"

#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/InvalidStorageException.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/ui/ImageType.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <o3tl/enumrange.hxx>
#include <osl/mutex.hxx>
#include <comphelper/sequence.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <rtl/instance.hxx>
#include <svtools/miscopt.hxx>
#include <memory>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::graphic::XGraphic;
using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui;
using namespace ::cppu;

const sal_Int16 MAX_IMAGETYPE_VALUE       = css::ui::ImageType::SIZE_32;

static const char   IMAGE_FOLDER[]        = "images";
static const char   BITMAPS_FOLDER[]      = "Bitmaps";

static const o3tl::enumarray<vcl::ImageType, const char*> IMAGELIST_XML_FILE =
{
    "sc_imagelist.xml",
    "lc_imagelist.xml",
    "xc_imagelist.xml"
};

static const o3tl::enumarray<vcl::ImageType, const char*> BITMAP_FILE_NAMES =
{
    "sc_userimages.png",
    "lc_userimages.png",
    "xc_userimages.png"
};

namespace framework
{

static GlobalImageList*     pGlobalImageList = nullptr;

typedef GraphicNameAccess CmdToXGraphicNameAccess;

namespace
{
    class theGlobalImageListMutex
        : public rtl::Static<osl::Mutex, theGlobalImageListMutex> {};
}

static osl::Mutex& getGlobalImageListMutex()
{
    return theGlobalImageListMutex::get();
}

static GlobalImageList* getGlobalImageList( const uno::Reference< uno::XComponentContext >& rxContext )
{
    osl::MutexGuard guard( getGlobalImageListMutex() );

    if ( pGlobalImageList == nullptr )
        pGlobalImageList = new GlobalImageList( rxContext );

    return pGlobalImageList;
}

CmdImageList::CmdImageList( const uno::Reference< uno::XComponentContext >& rxContext, const OUString& aModuleIdentifier ) :
    m_bInitialized(false),
    m_aModuleIdentifier( aModuleIdentifier ),
    m_xContext( rxContext )
{
}

CmdImageList::~CmdImageList()
{
}

void CmdImageList::initialize()
{
    if (!m_bInitialized)
    {
        const OUString aCommandImageList(UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDIMAGELIST);

        Sequence<OUString> aCommandImageSeq;
        uno::Reference<XNameAccess> xCommandDesc = frame::theUICommandDescription::get(m_xContext);

        if (!m_aModuleIdentifier.isEmpty())
        {
            // If we have a module identifier - use to retrieve the command image name list from it.
            // Otherwise we will use the global command image list
            try
            {
                xCommandDesc->getByName(m_aModuleIdentifier) >>= xCommandDesc;
                if (xCommandDesc.is())
                    xCommandDesc->getByName(aCommandImageList) >>= aCommandImageSeq;
            }
            catch (const NoSuchElementException&)
            {
                // Module unknown we will work with an empty command image list!
                return;
            }
        }

        if (xCommandDesc.is())
        {
            try
            {
                xCommandDesc->getByName(aCommandImageList) >>= aCommandImageSeq;
            }
            catch (const NoSuchElementException&)
            {
            }
            catch (const WrappedTargetException&)
            {
            }
        }

        m_aResolver.registerCommands(aCommandImageSeq);

        m_bInitialized = true;
    }
}


Image CmdImageList::getImageFromCommandURL(vcl::ImageType nImageType, const OUString& rCommandURL)
{
    initialize();
    return m_aResolver.getImageFromCommandURL(nImageType, rCommandURL);
}

bool CmdImageList::hasImage(vcl::ImageType /*nImageType*/, const OUString& rCommandURL)
{
    initialize();
    return m_aResolver.hasImage(rCommandURL);
}

std::vector<OUString>& CmdImageList::getImageCommandNames()
{
    return m_aResolver.getCommandNames();
}

GlobalImageList::GlobalImageList( const uno::Reference< uno::XComponentContext >& rxContext ) :
    CmdImageList( rxContext, OUString() )
{
}

GlobalImageList::~GlobalImageList()
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    // remove global pointer as we destroy the object now
    pGlobalImageList = nullptr;
}

Image GlobalImageList::getImageFromCommandURL( vcl::ImageType nImageType, const OUString& rCommandURL )
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return CmdImageList::getImageFromCommandURL( nImageType, rCommandURL );
}

bool GlobalImageList::hasImage( vcl::ImageType nImageType, const OUString& rCommandURL )
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return CmdImageList::hasImage( nImageType, rCommandURL );
}

::std::vector< OUString >& GlobalImageList::getImageCommandNames()
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return CmdImageList::getImageCommandNames();
}

static bool implts_checkAndScaleGraphic( uno::Reference< XGraphic >& rOutGraphic, const uno::Reference< XGraphic >& rInGraphic, vcl::ImageType nImageType )
{
    static Size aNormSize(16, 16);
    static Size aLargeSize(26, 26);
    static Size aSize32(32, 32);

    if ( !rInGraphic.is() )
    {
        rOutGraphic = uno::Reference<graphic::XGraphic>();
        return false;
    }

    // Check size and scale it
    Graphic aImage(rInGraphic);
    Size   aSize = aImage.GetSizePixel();
    bool   bMustScale( false );

    if (nImageType == vcl::ImageType::Size26)
        bMustScale = (aSize != aLargeSize);
    else if (nImageType == vcl::ImageType::Size32)
        bMustScale = (aSize != aSize32);
    else
        bMustScale = (aSize != aNormSize);

    if (bMustScale)
    {
        BitmapEx aBitmap = aImage.GetBitmapEx();
        aBitmap.Scale( aNormSize );
        aImage = Graphic(aBitmap);
        rOutGraphic = aImage.GetXGraphic();
    }
    else
        rOutGraphic = rInGraphic;

    return true;
}

static vcl::ImageType implts_convertImageTypeToIndex( sal_Int16 nImageType )
{
    if (nImageType & css::ui::ImageType::SIZE_LARGE)
        return vcl::ImageType::Size26;
    else if (nImageType & css::ui::ImageType::SIZE_32)
        return vcl::ImageType::Size32;
    else
        return vcl::ImageType::Size16;
}

ImageList* ImageManagerImpl::implts_getUserImageList( vcl::ImageType nImageType )
{
    SolarMutexGuard g;
    if ( !m_pUserImageList[nImageType] )
        implts_loadUserImages( nImageType, m_xUserImageStorage, m_xUserBitmapsStorage );

    return m_pUserImageList[nImageType];
}

void ImageManagerImpl::implts_initialize()
{
    // Initialize the top-level structures with the storage data
    if ( m_xUserConfigStorage.is() )
    {
        long nModes = m_bReadOnly ? ElementModes::READ : ElementModes::READWRITE;

        try
        {
            m_xUserImageStorage = m_xUserConfigStorage->openStorageElement( IMAGE_FOLDER,
                                                                            nModes );
            if ( m_xUserImageStorage.is() )
            {
                m_xUserBitmapsStorage = m_xUserImageStorage->openStorageElement( BITMAPS_FOLDER,
                                                                                 nModes );
            }
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::embed::InvalidStorageException& )
        {
        }
        catch ( const css::lang::IllegalArgumentException& )
        {
        }
        catch ( const css::io::IOException& )
        {
        }
        catch ( const css::embed::StorageWrappedTargetException& )
        {
        }
    }
}

bool ImageManagerImpl::implts_loadUserImages(
    vcl::ImageType nImageType,
    const uno::Reference< XStorage >& xUserImageStorage,
    const uno::Reference< XStorage >& xUserBitmapsStorage )
{
    SolarMutexGuard g;

    if ( xUserImageStorage.is() && xUserBitmapsStorage.is() )
    {
        try
        {
            uno::Reference< XStream > xStream = xUserImageStorage->openStreamElement( OUString::createFromAscii( IMAGELIST_XML_FILE[nImageType] ),
                                                                                      ElementModes::READ );
            uno::Reference< XInputStream > xInputStream = xStream->getInputStream();

            ImageListsDescriptor aUserImageListInfo;
            ImagesConfiguration::LoadImages( m_xContext,
                                             xInputStream,
                                             aUserImageListInfo );
            if (( aUserImageListInfo.pImageList != nullptr ) &&
                ( !aUserImageListInfo.pImageList->empty() ))
            {
                ImageListItemDescriptor* pList = aUserImageListInfo.pImageList->front().get();
                sal_Int32 nCount = pList->pImageItemList->size();
                std::vector< OUString > aUserImagesVector;
                aUserImagesVector.reserve(nCount);
                for ( sal_Int32 i=0; i < nCount; i++ )
                {
                    const ImageItemDescriptor* pItem = (*pList->pImageItemList)[i].get();
                    aUserImagesVector.push_back( pItem->aCommandURL );
                }

                uno::Reference< XStream > xBitmapStream = xUserBitmapsStorage->openStreamElement(
                                                        OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ),
                                                        ElementModes::READ );

                if ( xBitmapStream.is() )
                {
                    BitmapEx aUserBitmap;
                    {
                        std::unique_ptr<SvStream> pSvStream(utl::UcbStreamHelper::CreateStream( xBitmapStream ));
                        vcl::PNGReader aPngReader( *pSvStream );
                        aUserBitmap = aPngReader.Read();
                    }

                    // Delete old image list and create a new one from the read bitmap
                    delete m_pUserImageList[nImageType];
                    m_pUserImageList[nImageType] = new ImageList();
                    m_pUserImageList[nImageType]->InsertFromHorizontalStrip
                        ( aUserBitmap, aUserImagesVector );
                    return true;
                }
            }
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::embed::InvalidStorageException& )
        {
        }
        catch ( const css::lang::IllegalArgumentException& )
        {
        }
        catch ( const css::io::IOException& )
        {
        }
        catch ( const css::embed::StorageWrappedTargetException& )
        {
        }
    }

    // Destroy old image list - create a new empty one
    delete m_pUserImageList[nImageType];
    m_pUserImageList[nImageType] = new ImageList;

    return true;
}

bool ImageManagerImpl::implts_storeUserImages(
    vcl::ImageType nImageType,
    const uno::Reference< XStorage >& xUserImageStorage,
    const uno::Reference< XStorage >& xUserBitmapsStorage )
{
    SolarMutexGuard g;

    if ( m_bModified )
    {
        ImageList* pImageList = implts_getUserImageList( nImageType );
        if ( pImageList->GetImageCount() > 0 )
        {
            ImageListsDescriptor aUserImageListInfo;
            aUserImageListInfo.pImageList = new ImageListDescriptor;

            ImageListItemDescriptor* pList = new ImageListItemDescriptor;
            aUserImageListInfo.pImageList->push_back( std::unique_ptr<ImageListItemDescriptor>(pList) );

            pList->pImageItemList.reset( new ImageItemListDescriptor );
            for ( sal_uInt16 i=0; i < pImageList->GetImageCount(); i++ )
            {
                ImageItemDescriptor* pItem = new ImageItemDescriptor;
                pItem->nIndex = i;
                pItem->aCommandURL = pImageList->GetImageName( i );
                pList->pImageItemList->push_back( std::unique_ptr<ImageItemDescriptor>(pItem) );
            }

            pList->aURL = "Bitmaps/" + OUString::createFromAscii(BITMAP_FILE_NAMES[nImageType]);

            uno::Reference< XTransactedObject > xTransaction;
            uno::Reference< XOutputStream >     xOutputStream;
            uno::Reference< XStream > xStream = xUserImageStorage->openStreamElement( OUString::createFromAscii( IMAGELIST_XML_FILE[nImageType] ),
                                                                                      ElementModes::WRITE|ElementModes::TRUNCATE );
            if ( xStream.is() )
            {
                uno::Reference< XStream > xBitmapStream =
                    xUserBitmapsStorage->openStreamElement( OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ),
                                                            ElementModes::WRITE|ElementModes::TRUNCATE );
                if ( xBitmapStream.is() )
                {
                    {
                        std::unique_ptr<SvStream> pSvStream(utl::UcbStreamHelper::CreateStream( xBitmapStream ));
                        vcl::PNGWriter aPngWriter( pImageList->GetAsHorizontalStrip() );
                        aPngWriter.Write( *pSvStream );
                    }

                    // Commit user bitmaps storage
                    xTransaction.set( xUserBitmapsStorage, UNO_QUERY );
                    if ( xTransaction.is() )
                        xTransaction->commit();
                }

                xOutputStream = xStream->getOutputStream();
                if ( xOutputStream.is() )
                    ImagesConfiguration::StoreImages( m_xContext, xOutputStream, aUserImageListInfo );

                // Commit user image storage
                xTransaction.set( xUserImageStorage, UNO_QUERY );
                if ( xTransaction.is() )
                    xTransaction->commit();
            }

            return true;
        }
        else
        {
            // Remove the streams from the storage, if we have no data. We have to catch
            // the NoSuchElementException as it can be possible that there is no stream at all!
            try
            {
                xUserImageStorage->removeElement( OUString::createFromAscii( IMAGELIST_XML_FILE[nImageType] ));
            }
            catch ( const css::container::NoSuchElementException& )
            {
            }

            try
            {
                xUserBitmapsStorage->removeElement( OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ));
            }
            catch ( const css::container::NoSuchElementException& )
            {
            }

            uno::Reference< XTransactedObject > xTransaction;

            // Commit user image storage
            xTransaction.set( xUserImageStorage, UNO_QUERY );
            if ( xTransaction.is() )
                xTransaction->commit();

            // Commit user bitmaps storage
            xTransaction.set( xUserBitmapsStorage, UNO_QUERY );
            if ( xTransaction.is() )
                xTransaction->commit();

            return true;
        }
    }

    return false;
}

const rtl::Reference< GlobalImageList >& ImageManagerImpl::implts_getGlobalImageList()
{
    SolarMutexGuard g;

    if ( !m_pGlobalImageList.is() )
        m_pGlobalImageList = getGlobalImageList( m_xContext );
    return m_pGlobalImageList;
}

CmdImageList* ImageManagerImpl::implts_getDefaultImageList()
{
    SolarMutexGuard g;

    if ( !m_pDefaultImageList )
        m_pDefaultImageList = new CmdImageList( m_xContext, m_aModuleIdentifier );

    return m_pDefaultImageList;
}

ImageManagerImpl::ImageManagerImpl( const uno::Reference< uno::XComponentContext >& rxContext,::cppu::OWeakObject* pOwner,bool _bUseGlobal ) :
    m_xContext( rxContext )
    , m_pOwner(pOwner)
    , m_pDefaultImageList( nullptr )
    , m_aResourceString( "private:resource/images/moduleimages" )
    , m_aListenerContainer( m_mutex )
    , m_bUseGlobal(_bUseGlobal)
    , m_bReadOnly( true )
    , m_bInitialized( false )
    , m_bModified( false )
    , m_bDisposed( false )
{
    for ( vcl::ImageType n : o3tl::enumrange<vcl::ImageType>() )
    {
        m_pUserImageList[n] = nullptr;
        m_bUserImageListModified[n] = false;
    }
}

ImageManagerImpl::~ImageManagerImpl()
{
    clear();
}

void ImageManagerImpl::dispose()
{
    uno::Reference< uno::XInterface > xOwner(m_pOwner);
    css::lang::EventObject aEvent( xOwner );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        SolarMutexGuard g;
        m_xUserConfigStorage.clear();
        m_xUserImageStorage.clear();
        m_xUserRootCommit.clear();
        m_bModified = false;
        m_bDisposed = true;

        // delete user and default image list on dispose
        for (ImageList*& n : m_pUserImageList)
        {
            delete n;
            n = nullptr;
        }
        delete m_pDefaultImageList;
        m_pDefaultImageList = nullptr;
    }

}
void ImageManagerImpl::addEventListener( const uno::Reference< XEventListener >& xListener )
{
    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void ImageManagerImpl::removeEventListener( const uno::Reference< XEventListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

// XInitialization
void ImageManagerImpl::initialize( const Sequence< Any >& aArguments )
{
    SolarMutexGuard g;

    if ( !m_bInitialized )
    {
        for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[n] >>= aPropValue )
            {
                if ( aPropValue.Name == "UserConfigStorage" )
                {
                    aPropValue.Value >>= m_xUserConfigStorage;
                }
                else if ( aPropValue.Name == "ModuleIdentifier" )
                {
                    aPropValue.Value >>= m_aModuleIdentifier;
                }
                else if ( aPropValue.Name == "UserRootCommit" )
                {
                    aPropValue.Value >>= m_xUserRootCommit;
                }
            }
        }

        if ( m_xUserConfigStorage.is() )
        {
            uno::Reference< XPropertySet > xPropSet( m_xUserConfigStorage, UNO_QUERY );
            if ( xPropSet.is() )
            {
                long nOpenMode = 0;
                if ( xPropSet->getPropertyValue("OpenMode") >>= nOpenMode )
                    m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
            }
        }

        implts_initialize();

        m_bInitialized = true;
    }
}

// XImageManagerImpl
void ImageManagerImpl::reset()
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    std::vector< OUString > aUserImageNames;

    for ( vcl::ImageType i : o3tl::enumrange<vcl::ImageType>() )
    {
        aUserImageNames.clear();
        ImageList* pImageList = implts_getUserImageList(i);
        pImageList->GetImageNames( aUserImageNames );

        Sequence< OUString > aRemoveList( comphelper::containerToSequence(aUserImageNames) );

        // Remove images
        removeImages( sal_Int16( i ), aRemoveList );
        m_bUserImageListModified[i] = true;
    }

    m_bModified = true;
}

Sequence< OUString > ImageManagerImpl::getAllImageNames( ::sal_Int16 nImageType )
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    ImageNameMap aImageCmdNameMap;

    vcl::ImageType nIndex = implts_convertImageTypeToIndex( nImageType );

    sal_uInt32 i( 0 );
    if ( m_bUseGlobal )
    {
        rtl::Reference< GlobalImageList > rGlobalImageList = implts_getGlobalImageList();

        const std::vector< OUString >& rGlobalImageNameVector = rGlobalImageList->getImageCommandNames();
        const sal_uInt32 nGlobalCount = rGlobalImageNameVector.size();
        for ( i = 0; i < nGlobalCount; i++ )
            aImageCmdNameMap.insert( ImageNameMap::value_type( rGlobalImageNameVector[i], true ));

        const std::vector< OUString >& rModuleImageNameVector = implts_getDefaultImageList()->getImageCommandNames();
        const sal_uInt32 nModuleCount = rModuleImageNameVector.size();
        for ( i = 0; i < nModuleCount; i++ )
            aImageCmdNameMap.insert( ImageNameMap::value_type( rModuleImageNameVector[i], true ));
    }

    ImageList* pImageList = implts_getUserImageList(nIndex);
    std::vector< OUString > rUserImageNames;
    pImageList->GetImageNames( rUserImageNames );
    const sal_uInt32 nUserCount = rUserImageNames.size();
    for ( i = 0; i < nUserCount; i++ )
        aImageCmdNameMap.insert( ImageNameMap::value_type( rUserImageNames[i], true ));

    return comphelper::mapKeysToSequence( aImageCmdNameMap );
}

bool ImageManagerImpl::hasImage( ::sal_Int16 nImageType, const OUString& aCommandURL )
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
        throw IllegalArgumentException();

    vcl::ImageType nIndex = implts_convertImageTypeToIndex( nImageType );
    if ( m_bUseGlobal && implts_getGlobalImageList()->hasImage( nIndex, aCommandURL ))
        return true;
    else
    {
        if ( m_bUseGlobal && implts_getDefaultImageList()->hasImage( nIndex, aCommandURL ))
            return true;
        else
        {
            // User layer
            ImageList* pImageList = implts_getUserImageList(nIndex);
            if ( pImageList )
                return ( pImageList->GetImagePos( aCommandURL ) != IMAGELIST_IMAGE_NOTFOUND );
        }
    }

    return false;
}

namespace
{
    css::uno::Reference< css::graphic::XGraphic > GetXGraphic(const Image &rImage)
    {
        return Graphic(rImage.GetBitmapEx()).GetXGraphic();
    }
}

Sequence< uno::Reference< XGraphic > > ImageManagerImpl::getImages(
    ::sal_Int16 nImageType,
    const Sequence< OUString >& aCommandURLSequence )
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
        throw IllegalArgumentException();

    Sequence< uno::Reference< XGraphic > > aGraphSeq( aCommandURLSequence.getLength() );

    const OUString* aStrArray = aCommandURLSequence.getConstArray();

    vcl::ImageType                    nIndex            = implts_convertImageTypeToIndex( nImageType );
    rtl::Reference< GlobalImageList > rGlobalImageList;
    CmdImageList*                     pDefaultImageList = nullptr;
    if ( m_bUseGlobal )
    {
        rGlobalImageList  = implts_getGlobalImageList();
        pDefaultImageList = implts_getDefaultImageList();
    }
    ImageList*                        pUserImageList    = implts_getUserImageList(nIndex);

    // We have to search our image list in the following order:
    // 1. user image list (read/write)
    // 2. module image list (read)
    // 3. global image list (read)
    for ( sal_Int32 n = 0; n < aCommandURLSequence.getLength(); n++ )
    {
        Image aImage = pUserImageList->GetImage( aStrArray[n] );
        if ( !aImage && m_bUseGlobal )
        {
            aImage = pDefaultImageList->getImageFromCommandURL( nIndex, aStrArray[n] );
            if ( !aImage )
                aImage = rGlobalImageList->getImageFromCommandURL( nIndex, aStrArray[n] );
        }

        aGraphSeq[n] = GetXGraphic(aImage);
    }

    return aGraphSeq;
}

void ImageManagerImpl::replaceImages(
    ::sal_Int16 nImageType,
    const Sequence< OUString >& aCommandURLSequence,
    const Sequence< uno::Reference< XGraphic > >& aGraphicsSequence )
{
    CmdToXGraphicNameAccess* pInsertedImages( nullptr );
    CmdToXGraphicNameAccess* pReplacedImages( nullptr );

    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();

        if (( aCommandURLSequence.getLength() != aGraphicsSequence.getLength() ) ||
            (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE )))
            throw IllegalArgumentException();

        if ( m_bReadOnly )
            throw IllegalAccessException();

        vcl::ImageType nIndex = implts_convertImageTypeToIndex( nImageType );
        ImageList* pImageList = implts_getUserImageList(nIndex);

        uno::Reference< XGraphic > xGraphic;
        for ( sal_Int32 i = 0; i < aCommandURLSequence.getLength(); i++ )
        {
            // Check size and scale. If we don't have any graphics ignore it
            if ( !implts_checkAndScaleGraphic( xGraphic, aGraphicsSequence[i], nIndex ))
                continue;

            sal_uInt16 nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
            if ( nPos == IMAGELIST_IMAGE_NOTFOUND )
            {
                pImageList->AddImage(aCommandURLSequence[i], Image(xGraphic));
                if ( !pInsertedImages )
                    pInsertedImages = new CmdToXGraphicNameAccess();
                pInsertedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
            else
            {
                pImageList->ReplaceImage(aCommandURLSequence[i], Image(xGraphic));
                if ( !pReplacedImages )
                    pReplacedImages = new CmdToXGraphicNameAccess();
                pReplacedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
        }

        if (( pInsertedImages != nullptr ) || (  pReplacedImages != nullptr ))
        {
            m_bModified = true;
            m_bUserImageListModified[nIndex] = true;
        }
    }

    uno::Reference< uno::XInterface > xOwner(m_pOwner);
    // Notify listeners
    if ( pInsertedImages != nullptr )
    {
        ConfigurationEvent aInsertEvent;
        aInsertEvent.aInfo           <<= nImageType;
        aInsertEvent.Accessor        <<= xOwner;
        aInsertEvent.Source          = xOwner;
        aInsertEvent.ResourceURL     = m_aResourceString;
        aInsertEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                        static_cast< OWeakObject *>( pInsertedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
    }
    if ( pReplacedImages != nullptr )
    {
        ConfigurationEvent aReplaceEvent;
        aReplaceEvent.aInfo           <<= nImageType;
        aReplaceEvent.Accessor        <<= xOwner;
        aReplaceEvent.Source          = xOwner;
        aReplaceEvent.ResourceURL     = m_aResourceString;
        aReplaceEvent.ReplacedElement = Any();
        aReplaceEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                            static_cast< OWeakObject *>( pReplacedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
    }
}

void ImageManagerImpl::removeImages( ::sal_Int16 nImageType, const Sequence< OUString >& aCommandURLSequence )
{
    CmdToXGraphicNameAccess* pRemovedImages( nullptr );
    CmdToXGraphicNameAccess* pReplacedImages( nullptr );

    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();

        if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
            throw IllegalArgumentException();

        if ( m_bReadOnly )
            throw IllegalAccessException();

        vcl::ImageType nIndex = implts_convertImageTypeToIndex( nImageType );
        rtl::Reference< GlobalImageList > rGlobalImageList;
        CmdImageList*                     pDefaultImageList = nullptr;
        if ( m_bUseGlobal )
        {
            rGlobalImageList  = implts_getGlobalImageList();
            pDefaultImageList = implts_getDefaultImageList();
        }
        ImageList*                        pImageList        = implts_getUserImageList(nIndex);
        uno::Reference<XGraphic> xEmptyGraphic;

        for ( sal_Int32 i = 0; i < aCommandURLSequence.getLength(); i++ )
        {
            sal_uInt16 nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
            if ( nPos != IMAGELIST_IMAGE_NOTFOUND )
            {
                sal_uInt16 nId   = pImageList->GetImageId( nPos );
                pImageList->RemoveImage( nId );

                if ( m_bUseGlobal )
                {
                    // Check, if we have a image in our module/global image list. If we find one =>
                    // this is a replace instead of a remove operation!
                    Image aNewImage = pDefaultImageList->getImageFromCommandURL( nIndex, aCommandURLSequence[i] );
                    if ( !aNewImage )
                        aNewImage = rGlobalImageList->getImageFromCommandURL( nIndex, aCommandURLSequence[i] );
                    if ( !aNewImage )
                    {
                        if ( !pRemovedImages )
                            pRemovedImages = new CmdToXGraphicNameAccess();
                        pRemovedImages->addElement( aCommandURLSequence[i], xEmptyGraphic );
                    }
                    else
                    {
                        if ( !pReplacedImages )
                            pReplacedImages = new CmdToXGraphicNameAccess();
                        pReplacedImages->addElement(aCommandURLSequence[i], GetXGraphic(aNewImage));
                    }
                } // if ( m_bUseGlobal )
                else
                {
                    if ( !pRemovedImages )
                        pRemovedImages = new CmdToXGraphicNameAccess();
                    pRemovedImages->addElement( aCommandURLSequence[i], xEmptyGraphic );
                }
            }
        }

        if (( pReplacedImages != nullptr ) || ( pRemovedImages != nullptr ))
        {
            m_bModified = true;
            m_bUserImageListModified[nIndex] = true;
        }
    }

    // Notify listeners
    uno::Reference< uno::XInterface > xOwner(m_pOwner);
    if ( pRemovedImages != nullptr )
    {
        ConfigurationEvent aRemoveEvent;
        aRemoveEvent.aInfo           = uno::makeAny( nImageType );
        aRemoveEvent.Accessor        = uno::makeAny( xOwner );
        aRemoveEvent.Source          = xOwner;
        aRemoveEvent.ResourceURL     = m_aResourceString;
        aRemoveEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                            static_cast< OWeakObject *>( pRemovedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aRemoveEvent, NotifyOp_Remove );
    }
    if ( pReplacedImages != nullptr )
    {
        ConfigurationEvent aReplaceEvent;
        aReplaceEvent.aInfo           = uno::makeAny( nImageType );
        aReplaceEvent.Accessor        = uno::makeAny( xOwner );
        aReplaceEvent.Source          = xOwner;
        aReplaceEvent.ResourceURL     = m_aResourceString;
        aReplaceEvent.ReplacedElement = Any();
        aReplaceEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                            static_cast< OWeakObject *>( pReplacedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
    }
}

void ImageManagerImpl::insertImages( ::sal_Int16 nImageType, const Sequence< OUString >& aCommandURLSequence, const Sequence< uno::Reference< XGraphic > >& aGraphicSequence )
{
    replaceImages(nImageType,aCommandURLSequence,aGraphicSequence);
}

// XUIConfigurationPersistence
void ImageManagerImpl::reload()
{
    SolarMutexClearableGuard aGuard;

    if ( m_bDisposed )
        throw DisposedException();

    CommandMap                   aOldUserCmdImageSet;
    std::vector< OUString > aNewUserCmdImageSet;

    if ( m_bModified )
    {
        for ( vcl::ImageType i : o3tl::enumrange<vcl::ImageType>() )
        {
            if ( !m_bDisposed && m_bUserImageListModified[i] )
            {
                std::vector< OUString > aOldUserCmdImageVector;
                ImageList* pImageList = implts_getUserImageList(i);
                pImageList->GetImageNames( aOldUserCmdImageVector );

                // Fill hash map to speed up search afterwards
                sal_uInt32 j( 0 );
                const sal_uInt32 nOldCount = aOldUserCmdImageVector.size();
                for ( j = 0; j < nOldCount; j++ )
                    aOldUserCmdImageSet.insert( CommandMap::value_type( aOldUserCmdImageVector[j], false ));

                // Attention: This can make the old image list pointer invalid!
                implts_loadUserImages( i, m_xUserImageStorage, m_xUserBitmapsStorage );
                pImageList = implts_getUserImageList(i);
                pImageList->GetImageNames( aNewUserCmdImageSet );

                CmdToXGraphicNameAccess* pInsertedImages( nullptr );
                CmdToXGraphicNameAccess* pReplacedImages( nullptr );
                CmdToXGraphicNameAccess* pRemovedImages( nullptr );

                const sal_uInt32 nNewCount = aNewUserCmdImageSet.size();
                for ( j = 0; j < nNewCount; j++ )
                {
                    CommandMap::iterator pIter = aOldUserCmdImageSet.find( aNewUserCmdImageSet[j] );
                    if ( pIter != aOldUserCmdImageSet.end() )
                    {
                        pIter->second = true; // mark entry as replaced
                        if ( !pReplacedImages )
                            pReplacedImages = new CmdToXGraphicNameAccess();
                        pReplacedImages->addElement( aNewUserCmdImageSet[j],
                                                     GetXGraphic(pImageList->GetImage(aNewUserCmdImageSet[j])) );
                    }
                    else
                    {
                        if ( !pInsertedImages )
                            pInsertedImages = new CmdToXGraphicNameAccess();
                        pInsertedImages->addElement( aNewUserCmdImageSet[j],
                                                     GetXGraphic(pImageList->GetImage(aNewUserCmdImageSet[j])) );
                    }
                }

                // Search map for unmarked entries => they have been removed from the user list
                // through this reload operation.
                // We have to search the module and global image list!
                rtl::Reference< GlobalImageList > rGlobalImageList;
                CmdImageList*                     pDefaultImageList = nullptr;
                if ( m_bUseGlobal )
                {
                    rGlobalImageList  = implts_getGlobalImageList();
                    pDefaultImageList = implts_getDefaultImageList();
                }
                uno::Reference<XGraphic> xEmptyGraphic;
                CommandMap::const_iterator pIter = aOldUserCmdImageSet.begin();
                while ( pIter != aOldUserCmdImageSet.end() )
                {
                    if ( !pIter->second )
                    {
                        if ( m_bUseGlobal )
                        {
                            Image aImage = pDefaultImageList->getImageFromCommandURL( i, pIter->first );
                            if ( !aImage )
                                aImage = rGlobalImageList->getImageFromCommandURL( i, pIter->first );

                            if ( !aImage )
                            {
                                // No image in the module/global image list => remove user image
                                if ( !pRemovedImages )
                                    pRemovedImages = new CmdToXGraphicNameAccess();
                                pRemovedImages->addElement( pIter->first, xEmptyGraphic );
                            }
                            else
                            {
                                // Image has been found in the module/global image list => replace user image
                                if ( !pReplacedImages )
                                    pReplacedImages = new CmdToXGraphicNameAccess();
                                pReplacedImages->addElement(pIter->first, GetXGraphic(aImage));
                            }
                        } // if ( m_bUseGlobal )
                        else
                        {
                            // No image in the user image list => remove user image
                            if ( !pRemovedImages )
                                pRemovedImages = new CmdToXGraphicNameAccess();
                            pRemovedImages->addElement( pIter->first, xEmptyGraphic );
                        }
                    }
                    ++pIter;
                }

                aGuard.clear();

                // Now notify our listeners. Unlock mutex to prevent deadlocks
                uno::Reference< uno::XInterface > xOwner(m_pOwner);
                if ( pInsertedImages != nullptr )
                {
                    ConfigurationEvent aInsertEvent;
                    aInsertEvent.aInfo           = uno::makeAny( (sal_uInt16)i );
                    aInsertEvent.Accessor        = uno::makeAny( xOwner );
                    aInsertEvent.Source          = xOwner;
                    aInsertEvent.ResourceURL     = m_aResourceString;
                    aInsertEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                    static_cast< OWeakObject *>( pInsertedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
                }
                if ( pReplacedImages != nullptr )
                {
                    ConfigurationEvent aReplaceEvent;
                    aReplaceEvent.aInfo           = uno::makeAny( (sal_uInt16)i );
                    aReplaceEvent.Accessor        = uno::makeAny( xOwner );
                    aReplaceEvent.Source          = xOwner;
                    aReplaceEvent.ResourceURL     = m_aResourceString;
                    aReplaceEvent.ReplacedElement = Any();
                    aReplaceEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                    static_cast< OWeakObject *>( pReplacedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
                }
                if ( pRemovedImages != nullptr )
                {
                    ConfigurationEvent aRemoveEvent;
                    aRemoveEvent.aInfo           = uno::makeAny( (sal_uInt16)i );
                    aRemoveEvent.Accessor        = uno::makeAny( xOwner );
                    aRemoveEvent.Source          = xOwner;
                    aRemoveEvent.ResourceURL     = m_aResourceString;
                    aRemoveEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                        static_cast< OWeakObject *>( pRemovedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aRemoveEvent, NotifyOp_Remove );
                }

                aGuard.clear();
            }
        }
    }
}

void ImageManagerImpl::store()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bModified )
    {
        bool bWritten( false );
        for ( vcl::ImageType i : o3tl::enumrange<vcl::ImageType>() )
        {
            bool bSuccess = implts_storeUserImages(i, m_xUserImageStorage, m_xUserBitmapsStorage );
            if ( bSuccess )
                bWritten = true;
            m_bUserImageListModified[i] = false;
        }

        if ( bWritten &&
             m_xUserConfigStorage.is() )
        {
            uno::Reference< XTransactedObject > xUserConfigStorageCommit( m_xUserConfigStorage, UNO_QUERY );
            if ( xUserConfigStorageCommit.is() )
                xUserConfigStorageCommit->commit();
            if ( m_xUserRootCommit.is() )
                m_xUserRootCommit->commit();
        }

        m_bModified = false;
    }
}

void ImageManagerImpl::storeToStorage( const uno::Reference< XStorage >& Storage )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bModified && Storage.is() )
    {
        long nModes = ElementModes::READWRITE;

        uno::Reference< XStorage > xUserImageStorage = Storage->openStorageElement( IMAGE_FOLDER,
                                                                                    nModes );
        if ( xUserImageStorage.is() )
        {
            uno::Reference< XStorage > xUserBitmapsStorage = xUserImageStorage->openStorageElement( BITMAPS_FOLDER,
                                                                                                    nModes );
            for ( vcl::ImageType i : o3tl::enumrange<vcl::ImageType>() )
            {
                implts_getUserImageList(i);
                implts_storeUserImages( i, xUserImageStorage, xUserBitmapsStorage );
            }

            uno::Reference< XTransactedObject > xTransaction( Storage, UNO_QUERY );
            if ( xTransaction.is() )
                xTransaction->commit();
        }
    }
}

bool ImageManagerImpl::isModified()
{
    SolarMutexGuard g;
    return m_bModified;
}

bool ImageManagerImpl::isReadOnly()
{
    SolarMutexGuard g;
    return m_bReadOnly;
}
// XUIConfiguration
void ImageManagerImpl::addConfigurationListener( const uno::Reference< css::ui::XUIConfigurationListener >& xListener )
{
    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( cppu::UnoType<XUIConfigurationListener>::get(), xListener );
}

void ImageManagerImpl::removeConfigurationListener( const uno::Reference< css::ui::XUIConfigurationListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<XUIConfigurationListener>::get(), xListener );
}

void ImageManagerImpl::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer(
                                        cppu::UnoType<css::ui::XUIConfigurationListener>::get());
    if ( pContainer != nullptr )
    {
        ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
        while ( pIterator.hasMoreElements() )
        {
            try
            {
                switch ( eOp )
                {
                    case NotifyOp_Replace:
                        static_cast< css::ui::XUIConfigurationListener*>(pIterator.next())->elementReplaced( aEvent );
                        break;
                    case NotifyOp_Insert:
                        static_cast< css::ui::XUIConfigurationListener*>(pIterator.next())->elementInserted( aEvent );
                        break;
                    case NotifyOp_Remove:
                        static_cast< css::ui::XUIConfigurationListener*>(pIterator.next())->elementRemoved( aEvent );
                        break;
                }
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}
void ImageManagerImpl::clear()
{
    SolarMutexGuard g;

    for (ImageList* & n : m_pUserImageList)
    {
        delete n;
        n = nullptr;
    }
}
} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
