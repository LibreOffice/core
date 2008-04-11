/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imagemanager.cxx,v $
 * $Revision: 1.16 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uiconfiguration/imagemanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <xml/imagesconfiguration.hxx>
#include <uiconfiguration/graphicnameaccess.hxx>
#include <services.h>

#include "properties.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/ui/ImageType.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <comphelper/sequence.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using ::rtl::OUString;
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

// Image sizes for our toolbars/menus
const sal_Int32 IMAGE_SIZE_NORMAL         = 16;
const sal_Int32 IMAGE_SIZE_LARGE          = 26;
const sal_Int16 MAX_IMAGETYPE_VALUE       = ::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST|
                                            ::com::sun::star::ui::ImageType::SIZE_LARGE;

static const char   IMAGE_FOLDER[]        = "images";
static const char   BITMAPS_FOLDER[]      = "Bitmaps";
static const char   IMAGE_EXTENSION[]     = ".png";

static const char*  IMAGELIST_XML_FILE[]  =
{
    "sc_imagelist.xml",
    "lc_imagelist.xml",
    "sch_imagelist.xml",
    "lch_imagelist.xml"
};

static const char*  BITMAP_FILE_NAMES[]   =
{
    "sc_userimages.png",
    "lc_userimages.png",
    "sch_userimages.png",
    "lch_userimages.png"
};

namespace framework
{

static char ModuleImageList[] = "private:resource/images/moduleimages";

typedef GraphicNameAccess CmdToXGraphicNameAccess;

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_6                    (    ImageManager                                                    ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                         ),
                                            DIRECT_INTERFACE( css::lang::XInitialization                                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XImageManager                     ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfiguration                  ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationPersistence       )
                                        )

DEFINE_XTYPEPROVIDER_6                  (   ImageManager                                          ,
                                            css::lang::XTypeProvider                                    ,
                                            css::lang::XComponent                                       ,
                                            css::lang::XInitialization                                  ,
                                            ::com::sun::star::ui::XImageManager                   ,
                                            ::com::sun::star::ui::XUIConfiguration                ,
                                            ::com::sun::star::ui::XUIConfigurationPersistence
                                        )

DEFINE_XSERVICEINFO_MULTISERVICE        (   ImageManager                        ,
                                            ::cppu::OWeakObject                 ,
                                            SERVICENAME_IMAGEMANAGER            ,
                                            IMPLEMENTATIONNAME_IMAGEMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   ImageManager, {} )

static sal_Bool implts_checkAndScaleGraphic( uno::Reference< XGraphic >& rOutGraphic, const uno::Reference< XGraphic >& rInGraphic, sal_Int16 nImageType )
{
    static Size   aNormSize( IMAGE_SIZE_NORMAL, IMAGE_SIZE_NORMAL );
    static Size   aLargeSize( IMAGE_SIZE_LARGE, IMAGE_SIZE_LARGE );

    if ( !rInGraphic.is() )
    {
        rOutGraphic = Image().GetXGraphic();
        return sal_False;
    }

    // Check size and scale it
    Image  aImage( rInGraphic );
    Size   aSize = aImage.GetSizePixel();
    bool   bMustScale( false );

    if (( nImageType == ImageType_Color_Large ) ||
        ( nImageType == ImageType_HC_Large ))
        bMustScale = ( aSize != aLargeSize );
    else
        bMustScale = ( aSize != aNormSize );

    if ( bMustScale )
    {
        BitmapEx aBitmap = aImage.GetBitmapEx();
        aBitmap.Scale( aNormSize );
        aImage = Image( aBitmap );
        rOutGraphic = aImage.GetXGraphic();
    }
    else
        rOutGraphic = rInGraphic;
    return sal_True;
}

static sal_Int16 implts_convertImageTypeToIndex( sal_Int16 nImageType )
{
    sal_Int16 nIndex( 0 );
    if ( nImageType & ::com::sun::star::ui::ImageType::SIZE_LARGE )
        nIndex += 1;
    if ( nImageType & ::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST )
        nIndex += 2;
    return nIndex;
}

ImageList* ImageManager::implts_getUserImageList( ImageType nImageType )
{
    ResetableGuard aGuard( m_aLock );
    if ( !m_pUserImageList[nImageType] )
        implts_loadUserImages( nImageType, m_xUserImageStorage, m_xUserBitmapsStorage );

    return m_pUserImageList[nImageType];
}

void ImageManager::implts_initialize()
{
    // Initialize the top-level structures with the storage data
    if ( m_xUserConfigStorage.is() )
    {
        long nModes = m_bReadOnly ? ElementModes::READ : ElementModes::READWRITE;

        try
        {
            m_xUserImageStorage = m_xUserConfigStorage->openStorageElement( OUString::createFromAscii( IMAGE_FOLDER ),
                                                                            nModes );
            if ( m_xUserImageStorage.is() )
            {
                m_xUserBitmapsStorage = m_xUserImageStorage->openStorageElement( OUString::createFromAscii( BITMAPS_FOLDER ),
                                                                                 nModes );
            }
        }
        catch ( com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( ::com::sun::star::embed::InvalidStorageException& )
        {
        }
        catch ( ::com::sun::star::lang::IllegalArgumentException& )
        {
        }
        catch ( ::com::sun::star::io::IOException& )
        {
        }
        catch ( ::com::sun::star::embed::StorageWrappedTargetException& )
        {
        }
    }
}

sal_Bool ImageManager::implts_loadUserImages(
    ImageType nImageType,
    const uno::Reference< XStorage >& xUserImageStorage,
    const uno::Reference< XStorage >& xUserBitmapsStorage )
{
    ResetableGuard aGuard( m_aLock );

    if ( xUserImageStorage.is() && xUserBitmapsStorage.is() )
    {
        try
        {
            uno::Reference< XStream > xStream = xUserImageStorage->openStreamElement( rtl::OUString::createFromAscii( IMAGELIST_XML_FILE[nImageType] ),
                                                                                      ElementModes::READ );
            uno::Reference< XInputStream > xInputStream = xStream->getInputStream();

            ImageListsDescriptor aUserImageListInfo;
            ImagesConfiguration::LoadImages( m_xServiceManager,
                                             xInputStream,
                                             aUserImageListInfo );
            if (( aUserImageListInfo.pImageList != 0 ) &&
                ( aUserImageListInfo.pImageList->Count() > 0 ))
            {
                ImageListItemDescriptor* pList = aUserImageListInfo.pImageList->GetObject(0);
                sal_Int32 nCount = pList->pImageItemList->Count();

                std::vector< OUString > aUserImagesVector;
                for ( USHORT i=0; i < nCount; i++ )
                {
                    const ImageItemDescriptor* pItem = pList->pImageItemList->GetObject(i);
                    aUserImagesVector.push_back( pItem->aCommandURL );
                }

                uno::Reference< XStream > xBitmapStream = xUserBitmapsStorage->openStreamElement(
                                                        rtl::OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ),
                                                        ElementModes::READ );

                if ( xBitmapStream.is() )
                {
                    SvStream* pSvStream( 0 );
                    BitmapEx aUserBitmap;
                    {
                        pSvStream = utl::UcbStreamHelper::CreateStream( xBitmapStream );
                        vcl::PNGReader aPngReader( *pSvStream );
                        aUserBitmap = aPngReader.Read();
                    }
                    delete pSvStream;

                    // Delete old image list and create a new one from the read bitmap
                    delete m_pUserImageList[nImageType];
                    m_pUserImageList[nImageType] = new ImageList();
                    m_pUserImageList[nImageType]->InsertFromHorizontalStrip
                        ( aUserBitmap, aUserImagesVector );
                    return sal_True;
                }
            }
        }
        catch ( com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( ::com::sun::star::embed::InvalidStorageException& )
        {
        }
        catch ( ::com::sun::star::lang::IllegalArgumentException& )
        {
        }
        catch ( ::com::sun::star::io::IOException& )
        {
        }
        catch ( ::com::sun::star::embed::StorageWrappedTargetException& )
        {
        }
    }

    // Destroy old image list - create a new empty one
    delete m_pUserImageList[nImageType];
    m_pUserImageList[nImageType] = new ImageList;

    return sal_True;
}

sal_Bool ImageManager::implts_storeUserImages(
    ImageType nImageType,
    const uno::Reference< XStorage >& xUserImageStorage,
    const uno::Reference< XStorage >& xUserBitmapsStorage )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bModified )
    {
        ImageList* pImageList = implts_getUserImageList( nImageType );
        if ( pImageList->GetImageCount() > 0 )
        {
            ImageListsDescriptor aUserImageListInfo;
            aUserImageListInfo.pImageList = new ImageListDescriptor;

            ImageListItemDescriptor* pList = new ImageListItemDescriptor;
            aUserImageListInfo.pImageList->Insert( pList, 0 );

            pList->pImageItemList = new ImageItemListDescriptor;
            for ( USHORT i=0; i < pImageList->GetImageCount(); i++ )
            {
                ImageItemDescriptor* pItem = new ::framework::ImageItemDescriptor;

                pItem->nIndex = i;
                pItem->aCommandURL = pImageList->GetImageName( i );
                pList->pImageItemList->Insert( pItem, pList->pImageItemList->Count() );
            }

            pList->aURL = String::CreateFromAscii("Bitmaps/");
            pList->aURL += String::CreateFromAscii( BITMAP_FILE_NAMES[nImageType] );

            uno::Reference< XTransactedObject > xTransaction;
            uno::Reference< XOutputStream >     xOutputStream;
            uno::Reference< XStream > xStream = xUserImageStorage->openStreamElement( rtl::OUString::createFromAscii( IMAGELIST_XML_FILE[nImageType] ),
                                                                                      ElementModes::WRITE|ElementModes::TRUNCATE );
            if ( xStream.is() )
            {
                uno::Reference< XStream > xBitmapStream =
                    xUserBitmapsStorage->openStreamElement( rtl::OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ),
                                                            ElementModes::WRITE|ElementModes::TRUNCATE );
                if ( xBitmapStream.is() )
                {
                    SvStream* pSvStream = utl::UcbStreamHelper::CreateStream( xBitmapStream );
                    {
                        vcl::PNGWriter aPngWriter( pImageList->GetAsHorizontalStrip() );
                        aPngWriter.Write( *pSvStream );
                    }
                    delete pSvStream;

                    // Commit user bitmaps storage
                    xTransaction = uno::Reference< XTransactedObject >( xUserBitmapsStorage, UNO_QUERY );
                    if ( xTransaction.is() )
                        xTransaction->commit();
                }

                xOutputStream = xStream->getOutputStream();
                if ( xOutputStream.is() )
                    ImagesConfiguration::StoreImages( m_xServiceManager, xOutputStream, aUserImageListInfo );

                // Commit user image storage
                xTransaction = uno::Reference< XTransactedObject >( xUserImageStorage, UNO_QUERY );
                if ( xTransaction.is() )
                    xTransaction->commit();
            }

            return sal_True;
        }
        else
        {
            // Remove the streams from the storage, if we have no data. We have to catch
            // the NoSuchElementException as it can be possible that there is no stream at all!
            try
            {
                xUserImageStorage->removeElement( rtl::OUString::createFromAscii( IMAGELIST_XML_FILE[nImageType] ));
            }
            catch ( ::com::sun::star::container::NoSuchElementException& )
            {
            }

            try
            {
                xUserBitmapsStorage->removeElement( rtl::OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ));
            }
            catch ( ::com::sun::star::container::NoSuchElementException& )
            {
            }

            uno::Reference< XTransactedObject > xTransaction;

            // Commit user image storage
            xTransaction = uno::Reference< XTransactedObject >( xUserImageStorage, UNO_QUERY );
            if ( xTransaction.is() )
                xTransaction->commit();

            // Commit user bitmaps storage
            xTransaction = uno::Reference< XTransactedObject >( xUserBitmapsStorage, UNO_QUERY );
            if ( xTransaction.is() )
                xTransaction->commit();

            return sal_True;
        }
    }

    return sal_False;
}

ImageManager::ImageManager( uno::Reference< XMultiServiceFactory > xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xUserConfigStorage( 0 )
    , m_xUserImageStorage( 0 )
    , m_xUserBitmapsStorage( 0 )
    , m_bReadOnly( true )
    , m_bInitialized( false )
    , m_bModified( false )
    , m_bConfigRead( false )
    , m_bDisposed( false )
    , m_aXMLPostfix( RTL_CONSTASCII_USTRINGPARAM( ".xml" ))
    , m_aResourceString( RTL_CONSTASCII_USTRINGPARAM( ModuleImageList ))
    , m_xServiceManager( xServiceManager )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    for ( sal_Int32 n=0; n < ImageType_COUNT; n++ )
    {
        m_pUserImageList[n] = 0;
        m_bUserImageListModified[n] = false;
    }
}

ImageManager::~ImageManager()
{
    ResetableGuard aGuard( m_aLock );
    for ( sal_Int32 n = 0; n < ImageType_COUNT; n++ )
    {
        delete m_pUserImageList[n];
        m_pUserImageList[n] = 0;
    }
}

// XComponent
void SAL_CALL ImageManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        m_xUserConfigStorage.clear();
        m_xUserImageStorage.clear();
        m_bConfigRead = false;
        m_bModified = false;
        m_bDisposed = true;
    }
}

void SAL_CALL ImageManager::addEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const uno::Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL ImageManager::removeEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const uno::Reference< XEventListener >* ) NULL ), xListener );
}

// Non-UNO methods
void ImageManager::setStorage( const uno::Reference< XStorage >& Storage )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    m_xUserConfigStorage = Storage;
    implts_initialize();
}

sal_Bool ImageManager::hasStorage()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    return ( m_xUserConfigStorage.is() );
}

// XInitialization
void SAL_CALL ImageManager::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bInitialized )
    {
        for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[n] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "UserConfigStorage" ))
                {
                    aPropValue.Value >>= m_xUserConfigStorage;
                }
                else if ( aPropValue.Name.equalsAscii( "ModuleIdentifier" ))
                {
                    aPropValue.Value >>= m_aModuleIdentifier;
                }
            }
        }

        if ( m_xUserConfigStorage.is() )
        {
            uno::Reference< XPropertySet > xPropSet( m_xUserConfigStorage, UNO_QUERY );
            if ( xPropSet.is() )
            {
                long nOpenMode = 0;
                Any a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenMode" )));
                if ( a >>= nOpenMode )
                    m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
            }
        }

        implts_initialize();

        m_bInitialized = true;
    }
}

// XImageManager
void SAL_CALL ImageManager::reset()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    std::vector< OUString > aUserImageNames;

    for ( sal_Int32 i = 0; i < ImageType_COUNT; i++ )
    {
        aUserImageNames.clear();
        ImageList* pImageList = implts_getUserImageList( ImageType(i));
        pImageList->GetImageNames( aUserImageNames );

        Sequence< rtl::OUString > aRemoveList( aUserImageNames.size() );
        for ( sal_uInt32 j = 0; j < aUserImageNames.size(); j++ )
            aRemoveList[j] = aUserImageNames[j];

        // Remove images
        removeImages( sal_Int16( i ), aRemoveList );
        m_bUserImageListModified[i] = true;
    }

    m_bModified = sal_True;
}

Sequence< ::rtl::OUString > SAL_CALL ImageManager::getAllImageNames( ::sal_Int16 nImageType )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    sal_uInt32 i( 0 );
    sal_Int16  nIndex     = implts_convertImageTypeToIndex( nImageType );
    ImageList* pImageList = implts_getUserImageList( ImageType( nIndex ));

    std::vector< OUString > aUserImageNames;
    pImageList->GetImageNames( aUserImageNames );

    Sequence< OUString > aImageNameSeq( aUserImageNames.size() );
    for ( i = 0; i < aUserImageNames.size(); i++ )
        aImageNameSeq[i] = aUserImageNames[i];

    return aImageNameSeq;
}

::sal_Bool SAL_CALL ImageManager::hasImage( ::sal_Int16 nImageType, const ::rtl::OUString& aCommandURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
        throw IllegalArgumentException();

    sal_Int16 nIndex = implts_convertImageTypeToIndex( nImageType );
    ImageList* pImageList = implts_getUserImageList( ImageType( nIndex ));
    if ( pImageList )
        return ( pImageList->GetImagePos( aCommandURL ) != IMAGELIST_IMAGE_NOTFOUND );

    return sal_False;
}

Sequence< uno::Reference< XGraphic > > SAL_CALL ImageManager::getImages(
    ::sal_Int16 nImageType,
    const Sequence< ::rtl::OUString >& aCommandURLSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
        throw IllegalArgumentException();

    Sequence< uno::Reference< XGraphic > > aGraphSeq( aCommandURLSequence.getLength() );

    const rtl::OUString* aStrArray      = aCommandURLSequence.getConstArray();
    sal_Int16            nIndex         = implts_convertImageTypeToIndex( nImageType );
    ImageList*           pUserImageList = implts_getUserImageList( ImageType( nIndex ));

    for ( sal_Int32 n = 0; n < aCommandURLSequence.getLength(); n++ )
    {
        Image aImage = pUserImageList->GetImage( aStrArray[n] );
        aGraphSeq[n] = aImage.GetXGraphic();
    }

    return aGraphSeq;
}

void SAL_CALL ImageManager::replaceImages(
    ::sal_Int16 nImageType,
    const Sequence< ::rtl::OUString >& aCommandURLSequence,
    const Sequence< uno::Reference< XGraphic > >& aGraphicsSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    CmdToXGraphicNameAccess* pInsertedImages( 0 );
    CmdToXGraphicNameAccess* pReplacedImages( 0 );

    {
        ResetableGuard aLock( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();

        if (( aCommandURLSequence.getLength() != aGraphicsSequence.getLength() ) ||
            (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE )))
            throw IllegalArgumentException();

        if ( m_bReadOnly )
            throw IllegalAccessException();

        sal_Int16 nIndex = implts_convertImageTypeToIndex( nImageType );
        ImageList* pImageList = implts_getUserImageList( ImageType( nIndex ));

        uno::Reference< XGraphic > xGraphic;
        for ( sal_Int32 i = 0; i < aCommandURLSequence.getLength(); i++ )
        {
            // Check size and scale. If we don't have any graphics ignore it
            if ( !implts_checkAndScaleGraphic( xGraphic, aGraphicsSequence[i], nIndex ))
                continue;

            USHORT nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
            if ( nPos == IMAGELIST_IMAGE_NOTFOUND )
            {
                pImageList->AddImage( aCommandURLSequence[i], xGraphic );
                if ( !pInsertedImages )
                    pInsertedImages = new CmdToXGraphicNameAccess();
                pInsertedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
            else
            {
                pImageList->ReplaceImage( aCommandURLSequence[i], xGraphic );
                if ( !pReplacedImages )
                    pReplacedImages = new CmdToXGraphicNameAccess();
                pReplacedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
        }

        if (( pInsertedImages != 0 ) || (  pReplacedImages != 0 ))
        {
            m_bModified = sal_True;
            m_bUserImageListModified[nIndex] = true;
        }
    }

    // Notify listeners
    uno::Reference< XImageManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    uno::Reference< XInterface > xIfac( xThis, UNO_QUERY );

    if ( pInsertedImages != 0 )
    {
        ConfigurationEvent aInsertEvent;
        aInsertEvent.aInfo           = uno::makeAny( nImageType );
        aInsertEvent.Accessor        = uno::makeAny( xThis );
        aInsertEvent.Source          = xIfac;
        aInsertEvent.ResourceURL     = m_aResourceString;
        aInsertEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                        static_cast< OWeakObject *>( pInsertedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
    }
    if ( pReplacedImages != 0 )
    {
        ConfigurationEvent aReplaceEvent;
        aReplaceEvent.aInfo           = uno::makeAny( nImageType );
        aReplaceEvent.Accessor        = uno::makeAny( xThis );
        aReplaceEvent.Source          = xIfac;
        aReplaceEvent.ResourceURL     = m_aResourceString;
        aReplaceEvent.ReplacedElement = Any();
        aReplaceEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                            static_cast< OWeakObject *>( pReplacedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
    }
}

void SAL_CALL ImageManager::removeImages( ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    CmdToXGraphicNameAccess* pRemovedImages( 0 );

    {
        ResetableGuard aLock( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();

        if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
            throw IllegalArgumentException();

        if ( m_bReadOnly )
            throw IllegalAccessException();

        sal_Int16                   nIndex      = implts_convertImageTypeToIndex( nImageType );
        ImageList*                  pImageList  = implts_getUserImageList( ImageType( nIndex ));
        uno::Reference< XGraphic >  xEmptyGraphic( Image().GetXGraphic() );

        for ( sal_Int32 i = 0; i < aCommandURLSequence.getLength(); i++ )
        {
            USHORT nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
            if ( nPos != IMAGELIST_IMAGE_NOTFOUND )
            {
                Image aImage = pImageList->GetImage( nPos );
                USHORT nId   = pImageList->GetImageId( nPos );
                pImageList->RemoveImage( nId );

                if ( !pRemovedImages )
                    pRemovedImages = new CmdToXGraphicNameAccess();
                pRemovedImages->addElement( aCommandURLSequence[i], xEmptyGraphic );
            }
        }

        if ( pRemovedImages != 0 )
        {
            m_bModified = sal_True;
            m_bUserImageListModified[nIndex] = true;
        }
    }

    // Notify listeners
    uno::Reference< XImageManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    uno::Reference< XInterface > xIfac( xThis, UNO_QUERY );

    if ( pRemovedImages != 0 )
    {
        ConfigurationEvent aRemoveEvent;
        aRemoveEvent.aInfo           = uno::makeAny( nImageType );
        aRemoveEvent.Accessor        = uno::makeAny( xThis );
        aRemoveEvent.Source          = xIfac;
        aRemoveEvent.ResourceURL     = m_aResourceString;
        aRemoveEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                            static_cast< OWeakObject *>( pRemovedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aRemoveEvent, NotifyOp_Remove );
    }
}

void SAL_CALL ImageManager::insertImages( ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence, const Sequence< uno::Reference< XGraphic > >& aGraphicSequence )
throw ( ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    CmdToXGraphicNameAccess* pInsertedImages( 0 );
    CmdToXGraphicNameAccess* pReplacedImages( 0 );

    {
        ResetableGuard aLock( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();

        if (( aCommandURLSequence.getLength() != aGraphicSequence.getLength() ) ||
            (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE )))
            throw IllegalArgumentException();

        if ( m_bReadOnly )
            throw IllegalAccessException();

        sal_Int16 nIndex = implts_convertImageTypeToIndex( nImageType );
        ImageList* pImageList = implts_getUserImageList( ImageType( nIndex ));

        uno::Reference< XGraphic > xGraphic;
        for ( sal_Int32 i = 0; i < aCommandURLSequence.getLength(); i++ )
        {
            if ( !implts_checkAndScaleGraphic( xGraphic, aGraphicSequence[i], nIndex ))
                continue;

            USHORT nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
            if ( nPos == IMAGELIST_IMAGE_NOTFOUND )
            {
                pImageList->AddImage( aCommandURLSequence[i], xGraphic );
                if ( !pInsertedImages )
                    pInsertedImages = new CmdToXGraphicNameAccess();
                pInsertedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
            else
            {
                pImageList->ReplaceImage( aCommandURLSequence[i], xGraphic );
                if ( !pReplacedImages )
                    pReplacedImages = new CmdToXGraphicNameAccess();
                pReplacedImages->addElement( aCommandURLSequence[i], xGraphic );
            }
        }

        if (( pInsertedImages != 0 ) || ( pReplacedImages != 0 ))
        {
            m_bModified = sal_True;
            m_bUserImageListModified[nIndex] = true;
        }
    }

    // Notify listeners
    uno::Reference< XImageManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    uno::Reference< XInterface > xIfac( xThis, UNO_QUERY );

    if ( pInsertedImages != 0 )
    {
        ConfigurationEvent aInsertEvent;
        aInsertEvent.aInfo           = uno::makeAny( nImageType );
        aInsertEvent.Accessor        = uno::makeAny( xThis );
        aInsertEvent.Source          = xIfac;
        aInsertEvent.ResourceURL     = m_aResourceString;
        aInsertEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                        static_cast< OWeakObject *>( pInsertedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
    }
    if ( pReplacedImages != 0 )
    {
        ConfigurationEvent aReplaceEvent;
        aReplaceEvent.aInfo           = uno::makeAny( nImageType );
        aReplaceEvent.Accessor        = uno::makeAny( xThis );
        aReplaceEvent.Source          = xIfac;
        aReplaceEvent.ResourceURL     = m_aResourceString;
        aReplaceEvent.ReplacedElement = Any();
        aReplaceEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                         static_cast< OWeakObject *>( pReplacedImages ), UNO_QUERY ));
        implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
    }
}

// XUIConfiguration
void SAL_CALL ImageManager::addConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const uno::Reference< XUIConfigurationListener >* ) NULL ), xListener );
}

void SAL_CALL ImageManager::removeConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const uno::Reference< XUIConfigurationListener >* ) NULL ), xListener );
}

// XUIConfigurationPersistence
void SAL_CALL ImageManager::reload()
throw ( ::com::sun::star::uno::Exception,
        ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    uno::Reference< uno::XInterface > xRefThis( static_cast< OWeakObject* >( this ));

    if ( m_bDisposed )
        throw DisposedException();

    CommandMap                   aOldUserCmdImageSet;
    std::vector< rtl::OUString > aNewUserCmdImageSet;

    if ( m_bModified )
    {
        uno::Reference< XImageManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
        uno::Reference< XInterface > xIfac( xThis, UNO_QUERY );

        for ( sal_Int16 i = 0; i < sal_Int16( ImageType_COUNT ); i++ )
        {
            if ( !m_bDisposed && m_bUserImageListModified[i] )
            {
                std::vector< rtl::OUString > aOldUserCmdImageVector;
                ImageList* pImageList = implts_getUserImageList( (ImageType)i );
                pImageList->GetImageNames( aOldUserCmdImageVector );

                // Fill hash map to speed up search afterwards
                sal_uInt32 j( 0 );
                for ( j = 0; j < aOldUserCmdImageVector.size(); j++ )
                    aOldUserCmdImageSet.insert( CommandMap::value_type( aOldUserCmdImageVector[j], false ));

                // Attention: This can make the old image list pointer invalid!
                implts_loadUserImages( (ImageType)i, m_xUserImageStorage, m_xUserBitmapsStorage );
                pImageList = implts_getUserImageList( (ImageType)i );
                pImageList->GetImageNames( aNewUserCmdImageSet );

                CmdToXGraphicNameAccess* pInsertedImages( 0 );
                CmdToXGraphicNameAccess* pReplacedImages( 0 );
                CmdToXGraphicNameAccess* pRemovedImages( 0 );

                for ( j = 0; j < aNewUserCmdImageSet.size(); j++ )
                {
                    CommandMap::iterator pIter = aOldUserCmdImageSet.find( aNewUserCmdImageSet[j] );
                    if ( pIter != aOldUserCmdImageSet.end() )
                    {
                        pIter->second = true; // mark entry as replaced
                        if ( !pReplacedImages )
                            pReplacedImages = new CmdToXGraphicNameAccess();
                        pReplacedImages->addElement( aNewUserCmdImageSet[j],
                                                     pImageList->GetImage( aNewUserCmdImageSet[j] ).GetXGraphic() );
                    }
                    else
                    {
                        if ( !pInsertedImages )
                            pInsertedImages = new CmdToXGraphicNameAccess();
                        pInsertedImages->addElement( aNewUserCmdImageSet[j],
                                                     pImageList->GetImage( aNewUserCmdImageSet[j] ).GetXGraphic() );
                    }
                }

                // Search map for unmarked entries => they have been removed from the user list
                // through this reload operation.
                uno::Reference< XGraphic >        xEmptyGraphic( Image().GetXGraphic() );
                CommandMap::const_iterator        pIter = aOldUserCmdImageSet.begin();
                while ( pIter != aOldUserCmdImageSet.end() )
                {
                    if ( !pIter->second )
                    {
                        // No image in the user image list => remove user image
                        if ( !pRemovedImages )
                            pRemovedImages = new CmdToXGraphicNameAccess();
                        pRemovedImages->addElement( pIter->first, xEmptyGraphic );
                    }
                    ++pIter;
                }

                aGuard.unlock();

                // Now notify our listeners. Unlock mutex to prevent deadlocks
                if ( pInsertedImages != 0 )
                {
                    ConfigurationEvent aInsertEvent;
                    aInsertEvent.aInfo           = uno::makeAny( i );
                    aInsertEvent.Accessor        = uno::makeAny( xThis );
                    aInsertEvent.Source          = xIfac;
                    aInsertEvent.ResourceURL     = m_aResourceString;
                    aInsertEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                    static_cast< OWeakObject *>( pInsertedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
                }
                if ( pReplacedImages != 0 )
                {
                    ConfigurationEvent aReplaceEvent;
                    aReplaceEvent.aInfo           = uno::makeAny( i );
                    aReplaceEvent.Accessor        = uno::makeAny( xThis );
                    aReplaceEvent.Source          = xIfac;
                    aReplaceEvent.ResourceURL     = m_aResourceString;
                    aReplaceEvent.ReplacedElement = Any();
                    aReplaceEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                    static_cast< OWeakObject *>( pReplacedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aReplaceEvent, NotifyOp_Replace );
                }
                if ( pRemovedImages != 0 )
                {
                    ConfigurationEvent aRemoveEvent;
                    aRemoveEvent.aInfo           = uno::makeAny( i );
                    aRemoveEvent.Accessor        = uno::makeAny( xThis );
                    aRemoveEvent.Source          = xIfac;
                    aRemoveEvent.ResourceURL     = m_aResourceString;
                    aRemoveEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                        static_cast< OWeakObject *>( pRemovedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aRemoveEvent, NotifyOp_Remove );
                }

                aGuard.lock();
            }
        }
    }
}

void SAL_CALL ImageManager::store()
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bModified )
    {
        sal_Bool bWritten( sal_False );
        for ( sal_Int32 i = 0; i < ImageType_COUNT; i++ )
        {
            sal_Bool bSuccess = implts_storeUserImages( ImageType(i), m_xUserImageStorage, m_xUserBitmapsStorage );
            if ( bSuccess )
                bWritten = sal_True;
            m_bUserImageListModified[i] = false;
        }

        if ( bWritten &&
             m_xUserConfigStorage.is())
        {
            uno::Reference< XTransactedObject > xUserConfigStorageCommit( m_xUserConfigStorage, UNO_QUERY );
            if ( xUserConfigStorageCommit.is() )
                xUserConfigStorageCommit->commit();
        }

        m_bModified = sal_False;
    }
}

void SAL_CALL ImageManager::storeToStorage( const uno::Reference< XStorage >& Storage )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bModified && Storage.is() )
    {
        long nModes = ElementModes::READWRITE;

        uno::Reference< XStorage > xUserImageStorage = Storage->openStorageElement( OUString::createFromAscii( IMAGE_FOLDER ),
                                                                                    nModes );
        if ( xUserImageStorage.is() )
        {
            uno::Reference< XStorage > xUserBitmapsStorage = xUserImageStorage->openStorageElement( OUString::createFromAscii( BITMAPS_FOLDER ),
                                                                                                    nModes );
            for ( sal_Int32 i = 0; i < ImageType_COUNT; i++ )
            {
                implts_getUserImageList( (ImageType)i );
                implts_storeUserImages( (ImageType)i, xUserImageStorage, xUserBitmapsStorage );
            }

            uno::Reference< XTransactedObject > xTransaction( Storage, UNO_QUERY );
            if ( xTransaction.is() )
                xTransaction->commit();
        }
    }
}

sal_Bool SAL_CALL ImageManager::isModified()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );
    return m_bModified;
}

sal_Bool SAL_CALL ImageManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );
    return m_bReadOnly;
}

void ImageManager::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer(
                                        ::getCppuType( ( const css::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >*) NULL ) );
    if ( pContainer != NULL )
    {
        ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
        while ( pIterator.hasMoreElements() )
        {
            try
            {
                switch ( eOp )
                {
                    case NotifyOp_Replace:
                        ((::com::sun::star::ui::XUIConfigurationListener*)pIterator.next())->elementReplaced( aEvent );
                        break;
                    case NotifyOp_Insert:
                        ((::com::sun::star::ui::XUIConfigurationListener*)pIterator.next())->elementInserted( aEvent );
                        break;
                    case NotifyOp_Remove:
                        ((::com::sun::star::ui::XUIConfigurationListener*)pIterator.next())->elementRemoved( aEvent );
                        break;
                }
            }
            catch( css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

} // namespace framework
