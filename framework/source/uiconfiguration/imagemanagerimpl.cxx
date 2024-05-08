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
#include <utility>
#include <xml/imagesconfiguration.hxx>
#include <uiconfiguration/imagetype.hxx>
#include <uiconfiguration/graphicnameaccess.hxx>

#include <properties.h>

#include <com/sun/star/frame/theUICommandDescription.hpp>
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
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/enumrange.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <memory>
#include <unordered_set>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
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

constexpr OUString IMAGE_FOLDER = u"images"_ustr;
constexpr OUString BITMAPS_FOLDER = u"Bitmaps"_ustr;

constexpr o3tl::enumarray<vcl::ImageType, OUString> IMAGELIST_XML_FILE
{
    u"sc_imagelist.xml"_ustr,
    u"lc_imagelist.xml"_ustr,
    u"xc_imagelist.xml"_ustr
};

constexpr o3tl::enumarray<vcl::ImageType, OUString> BITMAP_FILE_NAMES
{
    u"sc_userimages.png"_ustr,
    u"lc_userimages.png"_ustr,
    u"xc_userimages.png"_ustr
};

namespace framework
{

static GlobalImageList*     pGlobalImageList = nullptr;

static std::mutex& getGlobalImageListMutex()
{
    static std::mutex mutex;
    return mutex;
}

static GlobalImageList* getGlobalImageList( const uno::Reference< uno::XComponentContext >& rxContext )
{
    std::unique_lock guard( getGlobalImageListMutex() );

    if ( pGlobalImageList == nullptr )
        pGlobalImageList = new GlobalImageList( rxContext );

    return pGlobalImageList;
}

CmdImageList::CmdImageList( uno::Reference< uno::XComponentContext >  rxContext, OUString aModuleIdentifier ) :
    m_bInitialized(false),
    m_aModuleIdentifier(std::move( aModuleIdentifier )),
    m_xContext(std::move( rxContext ))
{
}

CmdImageList::~CmdImageList()
{
}

void CmdImageList::initialize()
{
    if (m_bInitialized)
        return;

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
    std::unique_lock guard( getGlobalImageListMutex() );
    // remove global pointer as we destroy the object now
    pGlobalImageList = nullptr;
}

Image GlobalImageList::getImageFromCommandURL( vcl::ImageType nImageType, const OUString& rCommandURL )
{
    std::unique_lock guard( getGlobalImageListMutex() );
    return CmdImageList::getImageFromCommandURL( nImageType, rCommandURL );
}

bool GlobalImageList::hasImage( vcl::ImageType nImageType, const OUString& rCommandURL )
{
    std::unique_lock guard( getGlobalImageListMutex() );
    return CmdImageList::hasImage( nImageType, rCommandURL );
}

::std::vector< OUString >& GlobalImageList::getImageCommandNames()
{
    std::unique_lock guard( getGlobalImageListMutex() );
    return CmdImageList::getImageCommandNames();
}

static bool implts_checkAndScaleGraphic( uno::Reference< XGraphic >& rOutGraphic, const uno::Reference< XGraphic >& rInGraphic, vcl::ImageType nImageType )
{
    if ( !rInGraphic.is() )
    {
        rOutGraphic = uno::Reference<graphic::XGraphic>();
        return false;
    }

    static const o3tl::enumarray<vcl::ImageType, Size> BITMAP_SIZE =
    {
        Size(16, 16), Size(24, 24), Size(32, 32)
    };

    // Check size and scale it
    Graphic aImage(rInGraphic);
    if (BITMAP_SIZE[nImageType] != aImage.GetSizePixel())
    {
        BitmapEx aBitmap = aImage.GetBitmapEx();
        aBitmap.Scale(BITMAP_SIZE[nImageType]);
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

    return m_pUserImageList[nImageType].get();
}

void ImageManagerImpl::implts_initialize()
{
    // Initialize the top-level structures with the storage data
    if ( !m_xUserConfigStorage.is() )
        return;

    tools::Long nModes = m_bReadOnly ? ElementModes::READ : ElementModes::READWRITE;

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

void ImageManagerImpl::implts_loadUserImages(
    vcl::ImageType nImageType,
    const uno::Reference< XStorage >& xUserImageStorage,
    const uno::Reference< XStorage >& xUserBitmapsStorage )
{
    SolarMutexGuard g;

    if ( xUserImageStorage.is() && xUserBitmapsStorage.is() )
    {
        try
        {
            uno::Reference< XStream > xStream = xUserImageStorage->openStreamElement( IMAGELIST_XML_FILE[nImageType],
                                                                                      ElementModes::READ );
            uno::Reference< XInputStream > xInputStream = xStream->getInputStream();

            ImageItemDescriptorList aUserImageListInfo;
            ImagesConfiguration::LoadImages( m_xContext,
                                             xInputStream,
                                             aUserImageListInfo );
            if ( !aUserImageListInfo.empty() )
            {
                sal_Int32 nCount = aUserImageListInfo.size();
                std::vector< OUString > aUserImagesVector;
                aUserImagesVector.reserve(nCount);
                for ( sal_Int32 i=0; i < nCount; i++ )
                {
                    const ImageItemDescriptor& rItem = aUserImageListInfo[i];
                    aUserImagesVector.push_back( rItem.aCommandURL );
                }

                uno::Reference< XStream > xBitmapStream = xUserBitmapsStorage->openStreamElement(
                                                        BITMAP_FILE_NAMES[nImageType],
                                                        ElementModes::READ );

                if ( xBitmapStream.is() )
                {
                    BitmapEx aUserBitmap;
                    {
                        std::unique_ptr<SvStream> pSvStream(utl::UcbStreamHelper::CreateStream( xBitmapStream ));
                        vcl::PngImageReader aPngReader( *pSvStream );
                        aUserBitmap = aPngReader.read();
                    }

                    // Delete old image list and create a new one from the read bitmap
                    m_pUserImageList[nImageType].reset(new ImageList());
                    m_pUserImageList[nImageType]->InsertFromHorizontalStrip
                        ( aUserBitmap, aUserImagesVector );
                    return;
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
    m_pUserImageList[nImageType].reset(new ImageList);
}

bool ImageManagerImpl::implts_storeUserImages(
    vcl::ImageType nImageType,
    const uno::Reference< XStorage >& xUserImageStorage,
    const uno::Reference< XStorage >& xUserBitmapsStorage )
{
    SolarMutexGuard g;

    if ( !m_bModified )
        return false;

    ImageList* pImageList = implts_getUserImageList( nImageType );
    if ( pImageList->GetImageCount() > 0 )
    {
        ImageItemDescriptorList aUserImageListInfo;

        for ( sal_uInt16 i=0; i < pImageList->GetImageCount(); i++ )
        {
            ImageItemDescriptor aItem;
            aItem.aCommandURL = pImageList->GetImageName( i );
            aUserImageListInfo.push_back( aItem );
        }

        uno::Reference< XTransactedObject > xTransaction;
        uno::Reference< XOutputStream >     xOutputStream;
        uno::Reference< XStream > xStream = xUserImageStorage->openStreamElement( IMAGELIST_XML_FILE[nImageType],
                                                                                  ElementModes::WRITE|ElementModes::TRUNCATE );
        if ( xStream.is() )
        {
            uno::Reference< XStream > xBitmapStream =
                xUserBitmapsStorage->openStreamElement( BITMAP_FILE_NAMES[nImageType],
                                                        ElementModes::WRITE|ElementModes::TRUNCATE );
            if ( xBitmapStream.is() )
            {
                {
                    std::unique_ptr<SvStream> pSvStream(utl::UcbStreamHelper::CreateStream( xBitmapStream ));
                    vcl::PngImageWriter aPngWriter( *pSvStream );
                    auto rBitmap = pImageList->GetAsHorizontalStrip();
                    aPngWriter.write( rBitmap );
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
            xUserImageStorage->removeElement( IMAGELIST_XML_FILE[nImageType] );
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }

        try
        {
            xUserBitmapsStorage->removeElement( BITMAP_FILE_NAMES[nImageType] );
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
        m_pDefaultImageList.reset(new CmdImageList( m_xContext, m_aModuleIdentifier ));

    return m_pDefaultImageList.get();
}

ImageManagerImpl::ImageManagerImpl( uno::Reference< uno::XComponentContext > xContext, ::cppu::OWeakObject* pOwner, bool _bUseGlobal ) :
    m_xContext(std::move( xContext ))
    , m_pOwner(pOwner)
    , m_aResourceString( u"private:resource/images/moduleimages"_ustr )
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
    {
        std::unique_lock aGuard(m_mutex);
        m_aEventListeners.disposeAndClear( aGuard, aEvent );
    }
    {
        std::unique_lock aGuard(m_mutex);
        m_aConfigListeners.disposeAndClear( aGuard, aEvent );
    }

    {
        SolarMutexGuard g;
        m_xUserConfigStorage.clear();
        m_xUserImageStorage.clear();
        m_xUserRootCommit.clear();
        m_bModified = false;
        m_bDisposed = true;

        // delete user and default image list on dispose
        for (auto& n : m_pUserImageList)
        {
            n.reset();
        }
        m_pDefaultImageList.reset();
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

    std::unique_lock aGuard(m_mutex);
    m_aEventListeners.addInterface( aGuard, xListener );
}

void ImageManagerImpl::removeEventListener( const uno::Reference< XEventListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    std::unique_lock aGuard(m_mutex);
    m_aEventListeners.removeInterface( aGuard, xListener );
}

// XInitialization
void ImageManagerImpl::initialize( const Sequence< Any >& aArguments )
{
    SolarMutexGuard g;

    if ( m_bInitialized )
        return;

    for ( const Any& rArg : aArguments )
    {
        PropertyValue aPropValue;
        if ( rArg >>= aPropValue )
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
            tools::Long nOpenMode = 0;
            if ( xPropSet->getPropertyValue(u"OpenMode"_ustr) >>= nOpenMode )
                m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
        }
    }

    implts_initialize();

    m_bInitialized = true;
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

    std::unordered_set< OUString > aImageCmdNames;

    vcl::ImageType nIndex = implts_convertImageTypeToIndex( nImageType );

    sal_uInt32 i( 0 );
    if ( m_bUseGlobal )
    {
        rtl::Reference< GlobalImageList > rGlobalImageList = implts_getGlobalImageList();

        const std::vector< OUString >& rGlobalImageNameVector = rGlobalImageList->getImageCommandNames();
        const sal_uInt32 nGlobalCount = rGlobalImageNameVector.size();
        for ( i = 0; i < nGlobalCount; i++ )
            aImageCmdNames.insert( rGlobalImageNameVector[i] );

        const std::vector< OUString >& rModuleImageNameVector = implts_getDefaultImageList()->getImageCommandNames();
        const sal_uInt32 nModuleCount = rModuleImageNameVector.size();
        for ( i = 0; i < nModuleCount; i++ )
            aImageCmdNames.insert( rModuleImageNameVector[i] );
    }

    ImageList* pImageList = implts_getUserImageList(nIndex);
    std::vector< OUString > rUserImageNames;
    pImageList->GetImageNames( rUserImageNames );
    const sal_uInt32 nUserCount = rUserImageNames.size();
    for ( i = 0; i < nUserCount; i++ )
        aImageCmdNames.insert( rUserImageNames[i] );

    return comphelper::containerToSequence( aImageCmdNames );
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
        return Graphic(rImage).GetXGraphic();
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
    auto aGraphSeqRange = asNonConstRange(aGraphSeq);
    sal_Int32 n = 0;
    for ( const OUString& rURL : aCommandURLSequence )
    {
        Image aImage = pUserImageList->GetImage( rURL );
        if ( !aImage && m_bUseGlobal )
        {
            aImage = pDefaultImageList->getImageFromCommandURL( nIndex, rURL );
            if ( !aImage )
                aImage = rGlobalImageList->getImageFromCommandURL( nIndex, rURL );
        }

        aGraphSeqRange[n++] = GetXGraphic(aImage);
    }

    return aGraphSeq;
}

void ImageManagerImpl::replaceImages(
    ::sal_Int16 nImageType,
    const Sequence< OUString >& aCommandURLSequence,
    const Sequence< uno::Reference< XGraphic > >& aGraphicsSequence )
{
    rtl::Reference<GraphicNameAccess> pInsertedImages;
    rtl::Reference<GraphicNameAccess> pReplacedImages;

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
                    pInsertedImages = new GraphicNameAccess();
                pInsertedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
            else
            {
                pImageList->ReplaceImage(aCommandURLSequence[i], Image(xGraphic));
                if ( !pReplacedImages )
                    pReplacedImages = new GraphicNameAccess();
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
        aInsertEvent.Element         <<= uno::Reference< XNameAccess >(pInsertedImages);
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
        aReplaceEvent.Element         <<= uno::Reference< XNameAccess >(pReplacedImages);
        implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
    }
}

void ImageManagerImpl::removeImages( ::sal_Int16 nImageType, const Sequence< OUString >& aCommandURLSequence )
{
    rtl::Reference<GraphicNameAccess> pRemovedImages;
    rtl::Reference<GraphicNameAccess> pReplacedImages;

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

        for ( const OUString& rURL : aCommandURLSequence )
        {
            sal_uInt16 nPos = pImageList->GetImagePos( rURL );
            if ( nPos != IMAGELIST_IMAGE_NOTFOUND )
            {
                sal_uInt16 nId   = pImageList->GetImageId( nPos );
                pImageList->RemoveImage( nId );

                if ( m_bUseGlobal )
                {
                    // Check, if we have an image in our module/global image list. If we find one =>
                    // this is a replace instead of a remove operation!
                    Image aNewImage = pDefaultImageList->getImageFromCommandURL( nIndex, rURL );
                    if ( !aNewImage )
                        aNewImage = rGlobalImageList->getImageFromCommandURL( nIndex, rURL );
                    if ( !aNewImage )
                    {
                        if ( !pRemovedImages )
                            pRemovedImages = new GraphicNameAccess();
                        pRemovedImages->addElement( rURL, xEmptyGraphic );
                    }
                    else
                    {
                        if ( !pReplacedImages )
                            pReplacedImages = new GraphicNameAccess();
                        pReplacedImages->addElement(rURL, GetXGraphic(aNewImage));
                    }
                } // if ( m_bUseGlobal )
                else
                {
                    if ( !pRemovedImages )
                        pRemovedImages = new GraphicNameAccess();
                    pRemovedImages->addElement( rURL, xEmptyGraphic );
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
        aRemoveEvent.aInfo           <<= nImageType;
        aRemoveEvent.Accessor        <<= xOwner;
        aRemoveEvent.Source          = xOwner;
        aRemoveEvent.ResourceURL     = m_aResourceString;
        aRemoveEvent.Element         <<= uno::Reference< XNameAccess >(pRemovedImages);
        implts_notifyContainerListener( aRemoveEvent, NotifyOp_Remove );
    }
    if ( pReplacedImages != nullptr )
    {
        ConfigurationEvent aReplaceEvent;
        aReplaceEvent.aInfo           <<= nImageType;
        aReplaceEvent.Accessor        <<= xOwner;
        aReplaceEvent.Source          = xOwner;
        aReplaceEvent.ResourceURL     = m_aResourceString;
        aReplaceEvent.ReplacedElement = Any();
        aReplaceEvent.Element         <<= uno::Reference< XNameAccess >(pReplacedImages);
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
    SolarMutexResettableGuard aGuard;

    if ( m_bDisposed )
        throw DisposedException();

    CommandMap                   aOldUserCmdImageSet;
    std::vector< OUString > aNewUserCmdImageSet;

    if ( !m_bModified )
        return;

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
                aOldUserCmdImageSet.emplace( aOldUserCmdImageVector[j], false );

            // Attention: This can make the old image list pointer invalid!
            implts_loadUserImages( i, m_xUserImageStorage, m_xUserBitmapsStorage );
            pImageList = implts_getUserImageList(i);
            pImageList->GetImageNames( aNewUserCmdImageSet );

            rtl::Reference<GraphicNameAccess> pInsertedImages;
            rtl::Reference<GraphicNameAccess> pReplacedImages;
            rtl::Reference<GraphicNameAccess> pRemovedImages;

            for (auto const& newUserCmdImage : aNewUserCmdImageSet)
            {
                CommandMap::iterator pIter = aOldUserCmdImageSet.find(newUserCmdImage);
                if ( pIter != aOldUserCmdImageSet.end() )
                {
                    pIter->second = true; // mark entry as replaced
                    if ( !pReplacedImages )
                        pReplacedImages = new GraphicNameAccess();
                    pReplacedImages->addElement( newUserCmdImage,
                                                 GetXGraphic(pImageList->GetImage(newUserCmdImage)) );
                }
                else
                {
                    if ( !pInsertedImages )
                        pInsertedImages = new GraphicNameAccess();
                    pInsertedImages->addElement( newUserCmdImage,
                                                 GetXGraphic(pImageList->GetImage(newUserCmdImage)) );
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
            for (auto const& oldUserCmdImage : aOldUserCmdImageSet)
            {
                if ( !oldUserCmdImage.second )
                {
                    if ( m_bUseGlobal )
                    {
                        Image aImage = pDefaultImageList->getImageFromCommandURL( i, oldUserCmdImage.first );
                        if ( !aImage )
                            aImage = rGlobalImageList->getImageFromCommandURL( i, oldUserCmdImage.first );

                        if ( !aImage )
                        {
                            // No image in the module/global image list => remove user image
                            if ( !pRemovedImages )
                                pRemovedImages = new GraphicNameAccess();
                            pRemovedImages->addElement( oldUserCmdImage.first, xEmptyGraphic );
                        }
                        else
                        {
                            // Image has been found in the module/global image list => replace user image
                            if ( !pReplacedImages )
                                pReplacedImages = new GraphicNameAccess();
                            pReplacedImages->addElement(oldUserCmdImage.first, GetXGraphic(aImage));
                        }
                    } // if ( m_bUseGlobal )
                    else
                    {
                        // No image in the user image list => remove user image
                        if ( !pRemovedImages )
                            pRemovedImages = new GraphicNameAccess();
                        pRemovedImages->addElement( oldUserCmdImage.first, xEmptyGraphic );
                    }
                }
            }

            aGuard.clear();

            // Now notify our listeners. Unlock mutex to prevent deadlocks
            uno::Reference< uno::XInterface > xOwner(m_pOwner);
            if ( pInsertedImages != nullptr )
            {
                ConfigurationEvent aInsertEvent;
                aInsertEvent.aInfo           <<=static_cast<sal_uInt16>(i);
                aInsertEvent.Accessor        <<= xOwner;
                aInsertEvent.Source          = xOwner;
                aInsertEvent.ResourceURL     = m_aResourceString;
                aInsertEvent.Element         <<= uno::Reference< XNameAccess >( pInsertedImages );
                implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
            }
            if ( pReplacedImages != nullptr )
            {
                ConfigurationEvent aReplaceEvent;
                aReplaceEvent.aInfo           <<= static_cast<sal_uInt16>(i);
                aReplaceEvent.Accessor        <<= xOwner;
                aReplaceEvent.Source          = xOwner;
                aReplaceEvent.ResourceURL     = m_aResourceString;
                aReplaceEvent.ReplacedElement = Any();
                aReplaceEvent.Element         <<= uno::Reference< XNameAccess >( pReplacedImages );
                implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
            }
            if ( pRemovedImages != nullptr )
            {
                ConfigurationEvent aRemoveEvent;
                aRemoveEvent.aInfo           <<= static_cast<sal_uInt16>(i);
                aRemoveEvent.Accessor        <<= xOwner;
                aRemoveEvent.Source          = xOwner;
                aRemoveEvent.ResourceURL     = m_aResourceString;
                aRemoveEvent.Element         <<= uno::Reference< XNameAccess >( pRemovedImages );
                implts_notifyContainerListener( aRemoveEvent, NotifyOp_Remove );
            }

            aGuard.reset();
        }
    }
}

void ImageManagerImpl::store()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bModified )
        return;

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

void ImageManagerImpl::storeToStorage( const uno::Reference< XStorage >& Storage )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !(m_bModified && Storage.is()) )
        return;

    tools::Long nModes = ElementModes::READWRITE;

    uno::Reference< XStorage > xUserImageStorage = Storage->openStorageElement( IMAGE_FOLDER,
                                                                                nModes );
    if ( !xUserImageStorage.is() )
        return;

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

bool ImageManagerImpl::isModified() const
{
    SolarMutexGuard g;
    return m_bModified;
}

bool ImageManagerImpl::isReadOnly() const
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

    std::unique_lock aGuard(m_mutex);
    m_aConfigListeners.addInterface( aGuard, xListener );
}

void ImageManagerImpl::removeConfigurationListener( const uno::Reference< css::ui::XUIConfigurationListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    std::unique_lock aGuard(m_mutex);
    m_aConfigListeners.removeInterface( aGuard, xListener );
}

void ImageManagerImpl::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
{
    std::unique_lock aGuard(m_mutex);
    comphelper::OInterfaceIteratorHelper4 pIterator( aGuard, m_aConfigListeners );
    aGuard.unlock();
    while ( pIterator.hasMoreElements() )
    {
        try
        {
            switch ( eOp )
            {
                case NotifyOp_Replace:
                    pIterator.next()->elementReplaced( aEvent );
                    break;
                case NotifyOp_Insert:
                    pIterator.next()->elementInserted( aEvent );
                    break;
                case NotifyOp_Remove:
                    pIterator.next()->elementRemoved( aEvent );
                    break;
            }
        }
        catch( const css::uno::RuntimeException& )
        {
            aGuard.lock();
            pIterator.remove(aGuard);
            aGuard.unlock();
        }
    }
}
void ImageManagerImpl::clear()
{
    SolarMutexGuard g;

    for (auto & n : m_pUserImageList)
    {
        n.reset();
    }
}
} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
