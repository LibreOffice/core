/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <imagemanagerimpl.hxx>
#include <threadhelp/resetableguard.hxx>
#include <xml/imagesconfiguration.hxx>
#include <uiconfiguration/graphicnameaccess.hxx>
#include <services.h>

#include "properties.h"

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/ui/ImageType.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/sequence.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include <svtools/miscopt.hxx>

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
using namespace ::cppu;

// Image sizes for our toolbars/menus
const sal_Int32 IMAGE_SIZE_NORMAL         = 16;
const sal_Int32 IMAGE_SIZE_LARGE          = 26;
const sal_Int16 MAX_IMAGETYPE_VALUE       = ::com::sun::star::ui::ImageType::SIZE_LARGE;

static const char   IMAGE_FOLDER[]        = "images";
static const char   BITMAPS_FOLDER[]      = "Bitmaps";

static const char   ModuleImageList[]     = "private:resource/images/moduleimages";

static const char*  IMAGELIST_XML_FILE[]  =
{
    "sc_imagelist.xml",
    "lc_imagelist.xml"
};

static const char*  BITMAP_FILE_NAMES[]   =
{
    "sc_userimages.png",
    "lc_userimages.png"
};

namespace framework
{
    static GlobalImageList*     pGlobalImageList = 0;
    static const char* ImageType_Prefixes[ImageType_COUNT] =
    {
        "cmd/sc_",
        "cmd/lc_"
    };

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

static GlobalImageList* getGlobalImageList( const uno::Reference< XMultiServiceFactory >& rServiceManager )
{
    osl::MutexGuard guard( getGlobalImageListMutex() );

    if ( pGlobalImageList == 0 )
        pGlobalImageList = new GlobalImageList( rServiceManager );

    return pGlobalImageList;
}

static rtl::OUString getCanonicalName( const rtl::OUString& rFileName )
{
    bool               bRemoveSlash( true );
    sal_Int32          nLength = rFileName.getLength();
    const sal_Unicode* pString = rFileName.getStr();

    rtl::OUStringBuffer aBuf( nLength );
    for ( sal_Int32 i = 0; i < nLength; i++ )
    {
        const sal_Unicode c = pString[i];
        switch ( c )
        {
            // map forbidden characters to escape
            case '/' : if ( !bRemoveSlash )
                         aBuf.appendAscii( "%2f" );
                       break;
            case '\\': aBuf.appendAscii( "%5c" ); bRemoveSlash = false; break;
            case ':' : aBuf.appendAscii( "%3a" ); bRemoveSlash = false; break;
            case '*' : aBuf.appendAscii( "%2a" ); bRemoveSlash = false; break;
            case '?' : aBuf.appendAscii( "%3f" ); bRemoveSlash = false; break;
            case '<' : aBuf.appendAscii( "%3c" ); bRemoveSlash = false; break;
            case '>' : aBuf.appendAscii( "%3e" ); bRemoveSlash = false; break;
            case '|' : aBuf.appendAscii( "%7c" ); bRemoveSlash = false; break;
            default: aBuf.append( c ); bRemoveSlash = false;
        }
    }
    return aBuf.makeStringAndClear();
}

//_________________________________________________________________________________________________________________

CmdImageList::CmdImageList( const uno::Reference< XMultiServiceFactory >& rServiceManager, const rtl::OUString& aModuleIdentifier ) :
    m_bVectorInit( sal_False ),
    m_aModuleIdentifier( aModuleIdentifier ),
    m_xServiceManager( rServiceManager ),
    m_nSymbolsStyle( SvtMiscOptions().GetCurrentSymbolsStyle() )
{
    for ( sal_Int32 n=0; n < ImageType_COUNT; n++ )
        m_pImageList[n] = 0;
}

CmdImageList::~CmdImageList()
{
    for ( sal_Int32 n=0; n < ImageType_COUNT; n++ )
        delete m_pImageList[n];
}

void CmdImageList::impl_fillCommandToImageNameMap()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework: CmdImageList::impl_fillCommandToImageNameMap" );

    if ( !m_bVectorInit )
    {
        const rtl::OUString aCommandImageList( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDIMAGELIST );
        Sequence< OUString > aCmdImageSeq;
        uno::Reference< XNameAccess > xCmdDesc( m_xServiceManager->createInstance(
                                                SERVICENAME_UICOMMANDDESCRIPTION ),
                                            UNO_QUERY );

        if ( !m_aModuleIdentifier.isEmpty() )
        {
            // If we have a module identifier - use to retrieve the command image name list from it.
            // Otherwise we will use the global command image list
            try
            {
                xCmdDesc->getByName( m_aModuleIdentifier ) >>= xCmdDesc;
                if ( xCmdDesc.is() )
                    xCmdDesc->getByName( aCommandImageList ) >>= aCmdImageSeq;
            }
            catch ( const NoSuchElementException& )
            {
                // Module unknown we will work with an empty command image list!
                return;
            }
        }

        if ( xCmdDesc.is() )
        {
            try
            {
                xCmdDesc->getByName( aCommandImageList ) >>= aCmdImageSeq;
            }
            catch ( const NoSuchElementException& )
            {
            }
            catch ( const WrappedTargetException& )
            {
            }
        }

        // We have to map commands which uses special characters like '/',':','?','\','<'.'>','|'
        String aExt = rtl::OUString(".png");
        m_aImageCommandNameVector.resize(aCmdImageSeq.getLength() );
        m_aImageNameVector.resize( aCmdImageSeq.getLength() );

        ::std::copy( aCmdImageSeq.getConstArray(),
                     aCmdImageSeq.getConstArray()+aCmdImageSeq.getLength(),
                     m_aImageCommandNameVector.begin() );

        // Create a image name vector that must be provided to the vcl imagelist. We also need
        // a command to image name map to speed up access time for image retrieval.
        OUString aUNOString( ".uno:" );
        String   aEmptyString;
        const sal_uInt32 nCount = m_aImageCommandNameVector.size();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            OUString aCommandName( m_aImageCommandNameVector[i] );
            String   aImageName;

            if ( aCommandName.indexOf( aUNOString ) != 0 )
            {
                INetURLObject aUrlObject( aCommandName, INetURLObject::ENCODE_ALL );
                aImageName = aUrlObject.GetURLPath();
                aImageName = getCanonicalName( aImageName ); // convert to valid filename
            }
            else
            {
                // just remove the schema
                if ( aCommandName.getLength() > 5 )
                    aImageName = aCommandName.copy( 5 );
                else
                    aImageName = aEmptyString;

                // Search for query part.
                sal_Int32 nIndex = aImageName.Search( '?' );
                if ( nIndex != STRING_NOTFOUND )
                    aImageName = getCanonicalName( aImageName ); // convert to valid filename
            }
            // Image names are not case-dependent. Always use lower case characters to
            // reflect this.
            aImageName += aExt;
            aImageName.ToLowerAscii();

            m_aImageNameVector[i] = aImageName;
            m_aCommandToImageNameMap.insert( CommandToImageNameMap::value_type( aCommandName, aImageName ));
        }

        m_bVectorInit = sal_True;
    }
}

ImageList* CmdImageList::impl_getImageList( sal_Int16 nImageType )
{
    SvtMiscOptions aMiscOptions;

    sal_Int16 nSymbolsStyle = aMiscOptions.GetCurrentSymbolsStyle();
    if ( nSymbolsStyle != m_nSymbolsStyle )
    {
        m_nSymbolsStyle = nSymbolsStyle;
        for ( sal_Int32 n=0; n < ImageType_COUNT; n++ )
            delete m_pImageList[n], m_pImageList[n] = NULL;
    }

    if ( !m_pImageList[nImageType] )
    {
        m_pImageList[nImageType] = new ImageList( m_aImageNameVector,
                                                  OUString::createFromAscii( ImageType_Prefixes[nImageType] ) );
    }

    return m_pImageList[nImageType];
}

std::vector< ::rtl::OUString >& CmdImageList::impl_getImageNameVector()
{
    return m_aImageNameVector;
}

std::vector< rtl::OUString >& CmdImageList::impl_getImageCommandNameVector()
{
    return m_aImageCommandNameVector;
}

Image CmdImageList::getImageFromCommandURL( sal_Int16 nImageType, const rtl::OUString& rCommandURL )
{
    impl_fillCommandToImageNameMap();
    CommandToImageNameMap::const_iterator pIter = m_aCommandToImageNameMap.find( rCommandURL );
    if ( pIter != m_aCommandToImageNameMap.end() )
    {
        ImageList* pImageList = impl_getImageList( nImageType );
        return pImageList->GetImage( pIter->second );
    }

    return Image();
}

bool CmdImageList::hasImage( sal_Int16 /*nImageType*/, const rtl::OUString& rCommandURL )
{
    impl_fillCommandToImageNameMap();
    CommandToImageNameMap::const_iterator pIter = m_aCommandToImageNameMap.find( rCommandURL );
    if ( pIter != m_aCommandToImageNameMap.end() )
        return true;
    else
        return false;
}

::std::vector< rtl::OUString >& CmdImageList::getImageNames()
{
    return impl_getImageNameVector();
}

::std::vector< rtl::OUString >& CmdImageList::getImageCommandNames()
{
    return impl_getImageCommandNameVector();
}

//_________________________________________________________________________________________________________________

GlobalImageList::GlobalImageList( const uno::Reference< XMultiServiceFactory >& rServiceManager ) :
    CmdImageList( rServiceManager, rtl::OUString() ),
    m_nRefCount( 0 )
{
}

GlobalImageList::~GlobalImageList()
{
}

Image GlobalImageList::getImageFromCommandURL( sal_Int16 nImageType, const rtl::OUString& rCommandURL )
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return CmdImageList::getImageFromCommandURL( nImageType, rCommandURL );
}

bool GlobalImageList::hasImage( sal_Int16 nImageType, const rtl::OUString& rCommandURL )
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return CmdImageList::hasImage( nImageType, rCommandURL );
}

::std::vector< rtl::OUString >& GlobalImageList::getImageNames()
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return impl_getImageNameVector();
}

::std::vector< rtl::OUString >& GlobalImageList::getImageCommandNames()
{
    osl::MutexGuard guard( getGlobalImageListMutex() );
    return impl_getImageCommandNameVector();
}

oslInterlockedCount GlobalImageList::acquire()
{
    osl_atomic_increment( &m_nRefCount );
    return m_nRefCount;
}

oslInterlockedCount GlobalImageList::release()
{
    osl::MutexGuard guard( getGlobalImageListMutex() );

    if ( !osl_atomic_decrement( &m_nRefCount ))
    {
        oslInterlockedCount nCount( m_nRefCount );
        // remove global pointer as we destroy the object now
        pGlobalImageList = 0;
        delete this;
        return nCount;
    }

    return m_nRefCount;
}

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

    if ( nImageType == ImageType_Color_Large )
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
    return nIndex;
}

ImageList* ImageManagerImpl::implts_getUserImageList( ImageType nImageType )
{
    ResetableGuard aGuard( m_aLock );
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
            m_xUserImageStorage = m_xUserConfigStorage->openStorageElement( OUString(IMAGE_FOLDER ),
                                                                            nModes );
            if ( m_xUserImageStorage.is() )
            {
                m_xUserBitmapsStorage = m_xUserImageStorage->openStorageElement( OUString(BITMAPS_FOLDER ),
                                                                                 nModes );
            }
        }
        catch ( const ::com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( const ::com::sun::star::embed::InvalidStorageException& )
        {
        }
        catch ( const ::com::sun::star::lang::IllegalArgumentException& )
        {
        }
        catch ( const ::com::sun::star::io::IOException& )
        {
        }
        catch ( const ::com::sun::star::embed::StorageWrappedTargetException& )
        {
        }
    }
}

sal_Bool ImageManagerImpl::implts_loadUserImages(
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
            ImagesConfiguration::LoadImages( comphelper::getComponentContext(m_xServiceManager),
                                             xInputStream,
                                             aUserImageListInfo );
            if (( aUserImageListInfo.pImageList != 0 ) &&
                ( !aUserImageListInfo.pImageList->empty() ))
            {
                ImageListItemDescriptor* pList = &aUserImageListInfo.pImageList->front();
                sal_Int32 nCount = pList->pImageItemList->size();
                std::vector< OUString > aUserImagesVector;
                aUserImagesVector.reserve(nCount);
                for ( sal_uInt16 i=0; i < nCount; i++ )
                {
                    const ImageItemDescriptor* pItem = &(*pList->pImageItemList)[i];
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
        catch ( const ::com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( const ::com::sun::star::embed::InvalidStorageException& )
        {
        }
        catch ( const ::com::sun::star::lang::IllegalArgumentException& )
        {
        }
        catch ( const ::com::sun::star::io::IOException& )
        {
        }
        catch ( const ::com::sun::star::embed::StorageWrappedTargetException& )
        {
        }
    }

    // Destroy old image list - create a new empty one
    delete m_pUserImageList[nImageType];
    m_pUserImageList[nImageType] = new ImageList;

    return sal_True;
}

sal_Bool ImageManagerImpl::implts_storeUserImages(
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
            aUserImageListInfo.pImageList->push_back( pList );

            pList->pImageItemList = new ImageItemListDescriptor;
            for ( sal_uInt16 i=0; i < pImageList->GetImageCount(); i++ )
            {
                ImageItemDescriptor* pItem = new ::framework::ImageItemDescriptor;

                pItem->nIndex = i;
                pItem->aCommandURL = pImageList->GetImageName( i );
                pList->pImageItemList->push_back( pItem );
            }

            pList->aURL = rtl::OUString("Bitmaps/");
            pList->aURL += rtl::OUString::createFromAscii(BITMAP_FILE_NAMES[nImageType]);

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
            catch ( const ::com::sun::star::container::NoSuchElementException& )
            {
            }

            try
            {
                xUserBitmapsStorage->removeElement( rtl::OUString::createFromAscii( BITMAP_FILE_NAMES[nImageType] ));
            }
            catch ( const ::com::sun::star::container::NoSuchElementException& )
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
const rtl::Reference< GlobalImageList >& ImageManagerImpl::implts_getGlobalImageList()
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_pGlobalImageList.is() )
        m_pGlobalImageList = getGlobalImageList( m_xServiceManager );
    return m_pGlobalImageList;
}

CmdImageList* ImageManagerImpl::implts_getDefaultImageList()
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_pDefaultImageList )
        m_pDefaultImageList = new CmdImageList( m_xServiceManager, m_aModuleIdentifier );

    return m_pDefaultImageList;
}

ImageManagerImpl::ImageManagerImpl( const uno::Reference< XMultiServiceFactory >& xServiceManager,bool _bUseGlobal ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xServiceManager( xServiceManager )
    , m_pDefaultImageList( 0 )
    , m_aXMLPostfix( ".xml" )
    , m_aResourceString( ModuleImageList )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , m_bUseGlobal(_bUseGlobal)
    , m_bReadOnly( true )
    , m_bInitialized( false )
    , m_bModified( false )
    , m_bConfigRead( false )
    , m_bDisposed( false )
{
    for ( sal_Int32 n=0; n < ImageType_COUNT; n++ )
    {
        m_pUserImageList[n] = 0;
        m_bUserImageListModified[n] = false;
    }
}

ImageManagerImpl::~ImageManagerImpl()
{
    clear();
}

void ImageManagerImpl::dispose( const uno::Reference< XInterface >& xOwner )
{
    css::lang::EventObject aEvent( xOwner );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        m_xUserConfigStorage.clear();
        m_xUserImageStorage.clear();
        m_xUserRootCommit.clear();
        m_bConfigRead = false;
        m_bModified = false;
        m_bDisposed = true;

        // delete user and default image list on dispose
        for ( sal_Int32 n=0; n < ImageType_COUNT; n++ )
        {
            delete m_pUserImageList[n];
            m_pUserImageList[n] = 0;
        }
        delete m_pDefaultImageList;
        m_pDefaultImageList = 0;
    }

}
void ImageManagerImpl::addEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const uno::Reference< XEventListener >* ) NULL ), xListener );
}

void ImageManagerImpl::removeEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const uno::Reference< XEventListener >* ) NULL ), xListener );
}

// XInitialization
void ImageManagerImpl::initialize( const Sequence< Any >& aArguments )
{
    ResetableGuard aLock( m_aLock );

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
                if ( xPropSet->getPropertyValue( rtl::OUString( "OpenMode" )) >>= nOpenMode )
                    m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
            }
        }

        implts_initialize();

        m_bInitialized = true;
    }
}

// XImageManagerImpl
void ImageManagerImpl::reset( const uno::Reference< XInterface >& xOwner )
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
        const sal_uInt32 nCount = aUserImageNames.size();
        for ( sal_uInt32 j = 0; j < nCount; j++ )
            aRemoveList[j] = aUserImageNames[j];

        // Remove images
        removeImages( xOwner, sal_Int16( i ), aRemoveList );
        m_bUserImageListModified[i] = true;
    }

    m_bModified = sal_True;
}

Sequence< ::rtl::OUString > ImageManagerImpl::getAllImageNames( ::sal_Int16 nImageType )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    ImageNameMap aImageCmdNameMap;

    sal_Int16 nIndex = implts_convertImageTypeToIndex( nImageType );

    sal_uInt32 i( 0 );
    if ( m_bUseGlobal )
    {
        rtl::Reference< GlobalImageList > rGlobalImageList = implts_getGlobalImageList();

        const std::vector< OUString >& rGlobalImageNameVector = rGlobalImageList->getImageCommandNames();
        const sal_uInt32 nGlobalCount = rGlobalImageNameVector.size();
        for ( i = 0; i < nGlobalCount; i++ )
            aImageCmdNameMap.insert( ImageNameMap::value_type( rGlobalImageNameVector[i], sal_True ));

        const std::vector< OUString >& rModuleImageNameVector = implts_getDefaultImageList()->getImageCommandNames();
        const sal_uInt32 nModuleCount = rModuleImageNameVector.size();
        for ( i = 0; i < nModuleCount; i++ )
            aImageCmdNameMap.insert( ImageNameMap::value_type( rModuleImageNameVector[i], sal_True ));
    }

    ImageList* pImageList = implts_getUserImageList( ImageType( nIndex ));
    std::vector< OUString > rUserImageNames;
    pImageList->GetImageNames( rUserImageNames );
    const sal_uInt32 nUserCount = rUserImageNames.size();
    for ( i = 0; i < nUserCount; i++ )
        aImageCmdNameMap.insert( ImageNameMap::value_type( rUserImageNames[i], sal_True ));

    Sequence< OUString > aImageNameSeq( aImageCmdNameMap.size() );
    ImageNameMap::const_iterator pIter;
    i = 0;
    for ( pIter = aImageCmdNameMap.begin(); pIter != aImageCmdNameMap.end(); ++pIter )
        aImageNameSeq[i++] = pIter->first;

    return aImageNameSeq;
}

::sal_Bool ImageManagerImpl::hasImage( ::sal_Int16 nImageType, const ::rtl::OUString& aCommandURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
        throw IllegalArgumentException();

    sal_Int16 nIndex = implts_convertImageTypeToIndex( nImageType );
    if ( m_bUseGlobal && implts_getGlobalImageList()->hasImage( nIndex, aCommandURL ))
        return sal_True;
    else
    {
        if ( m_bUseGlobal && implts_getDefaultImageList()->hasImage( nIndex, aCommandURL ))
            return sal_True;
        else
        {
            // User layer
            ImageList* pImageList = implts_getUserImageList( ImageType( nIndex ));
            if ( pImageList )
                return ( pImageList->GetImagePos( aCommandURL ) != IMAGELIST_IMAGE_NOTFOUND );
        }
    }

    return sal_False;
}

Sequence< uno::Reference< XGraphic > > ImageManagerImpl::getImages(
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

    const rtl::OUString* aStrArray = aCommandURLSequence.getConstArray();

    sal_Int16                         nIndex            = implts_convertImageTypeToIndex( nImageType );
    rtl::Reference< GlobalImageList > rGlobalImageList;
    CmdImageList*                     pDefaultImageList = NULL;
    if ( m_bUseGlobal )
    {
        rGlobalImageList  = implts_getGlobalImageList();
        pDefaultImageList = implts_getDefaultImageList();
    }
    ImageList*                        pUserImageList    = implts_getUserImageList( ImageType( nIndex ));

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

        aGraphSeq[n] = aImage.GetXGraphic();
    }

    return aGraphSeq;
}

void ImageManagerImpl::replaceImages(
    const uno::Reference< XInterface >& xOwner,
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

            sal_uInt16 nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
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
    if ( pInsertedImages != 0 )
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
    if ( pReplacedImages != 0 )
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

void ImageManagerImpl::removeImages( const uno::Reference< XInterface >& xOwner, ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence ) 
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    CmdToXGraphicNameAccess* pRemovedImages( 0 );
    CmdToXGraphicNameAccess* pReplacedImages( 0 );

    {
        ResetableGuard aLock( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();

        if (( nImageType < 0 ) || ( nImageType > MAX_IMAGETYPE_VALUE ))
            throw IllegalArgumentException();

        if ( m_bReadOnly )
            throw IllegalAccessException();

        sal_Int16 nIndex = implts_convertImageTypeToIndex( nImageType );
        rtl::Reference< GlobalImageList > rGlobalImageList;
        CmdImageList*                     pDefaultImageList = NULL;
        if ( m_bUseGlobal )
        {
            rGlobalImageList  = implts_getGlobalImageList();
            pDefaultImageList = implts_getDefaultImageList();
        }
        ImageList*                        pImageList        = implts_getUserImageList( ImageType( nIndex ));
        uno::Reference< XGraphic >        xEmptyGraphic( Image().GetXGraphic() );

        for ( sal_Int32 i = 0; i < aCommandURLSequence.getLength(); i++ )
        {
            sal_uInt16 nPos = pImageList->GetImagePos( aCommandURLSequence[i] );
            if ( nPos != IMAGELIST_IMAGE_NOTFOUND )
            {
                Image aImage = pImageList->GetImage( nPos );
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
                        pReplacedImages->addElement( aCommandURLSequence[i], aNewImage.GetXGraphic() );
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

        if (( pReplacedImages != 0 ) || ( pRemovedImages != 0 ))
        {
            m_bModified = sal_True;
            m_bUserImageListModified[nIndex] = true;
        }
    }

    // Notify listeners
    if ( pRemovedImages != 0 )
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
    if ( pReplacedImages != 0 )
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

void ImageManagerImpl::insertImages( const uno::Reference< XInterface >& xOwner, ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence, const Sequence< uno::Reference< XGraphic > >& aGraphicSequence ) 
throw ( ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    replaceImages(xOwner,nImageType,aCommandURLSequence,aGraphicSequence);
}


// XUIConfigurationPersistence
void ImageManagerImpl::reload( const uno::Reference< XInterface >& xOwner ) 
throw ( ::com::sun::star::uno::Exception,
        ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    CommandMap                   aOldUserCmdImageSet;
    std::vector< rtl::OUString > aNewUserCmdImageSet;

    if ( m_bModified )
    {
        for ( sal_Int16 i = 0; i < sal_Int16( ImageType_COUNT ); i++ )
        {
            if ( !m_bDisposed && m_bUserImageListModified[i] )
            {
                std::vector< rtl::OUString > aOldUserCmdImageVector;
                ImageList* pImageList = implts_getUserImageList( (ImageType)i );
                pImageList->GetImageNames( aOldUserCmdImageVector );

                // Fill hash map to speed up search afterwards
                sal_uInt32 j( 0 );
                const sal_uInt32 nOldCount = aOldUserCmdImageVector.size();
                for ( j = 0; j < nOldCount; j++ )
                    aOldUserCmdImageSet.insert( CommandMap::value_type( aOldUserCmdImageVector[j], false ));

                // Attention: This can make the old image list pointer invalid!
                implts_loadUserImages( (ImageType)i, m_xUserImageStorage, m_xUserBitmapsStorage );
                pImageList = implts_getUserImageList( (ImageType)i );
                pImageList->GetImageNames( aNewUserCmdImageSet );

                CmdToXGraphicNameAccess* pInsertedImages( 0 );
                CmdToXGraphicNameAccess* pReplacedImages( 0 );
                CmdToXGraphicNameAccess* pRemovedImages( 0 );

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
                // We have to search the module and global image list!
                rtl::Reference< GlobalImageList > rGlobalImageList;
                CmdImageList*                     pDefaultImageList = NULL;
                if ( m_bUseGlobal )
                {
                    rGlobalImageList  = implts_getGlobalImageList();
                    pDefaultImageList = implts_getDefaultImageList();
                }
                uno::Reference< XGraphic >        xEmptyGraphic( Image().GetXGraphic() );
                CommandMap::const_iterator        pIter = aOldUserCmdImageSet.begin();
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
                                pReplacedImages->addElement( pIter->first, aImage.GetXGraphic() );
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

                aGuard.unlock();

                // Now notify our listeners. Unlock mutex to prevent deadlocks
                if ( pInsertedImages != 0 )
                {
                    ConfigurationEvent aInsertEvent;
                    aInsertEvent.aInfo           = uno::makeAny( i );
                    aInsertEvent.Accessor        = uno::makeAny( xOwner );
                    aInsertEvent.Source          = xOwner;
                    aInsertEvent.ResourceURL     = m_aResourceString;
                    aInsertEvent.Element         = uno::makeAny( uno::Reference< XNameAccess >(
                                                    static_cast< OWeakObject *>( pInsertedImages ), UNO_QUERY ));
                    implts_notifyContainerListener( aInsertEvent, NotifyOp_Insert );
                }
                if ( pReplacedImages != 0 )
                {
                    ConfigurationEvent aReplaceEvent;
                    aReplaceEvent.aInfo           = uno::makeAny( i );
                    aReplaceEvent.Accessor        = uno::makeAny( xOwner );
                    aReplaceEvent.Source          = xOwner;
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
                    aRemoveEvent.Accessor        = uno::makeAny( xOwner );
                    aRemoveEvent.Source          = xOwner;
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

void ImageManagerImpl::store()
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
             m_xUserConfigStorage.is() )
        {
            uno::Reference< XTransactedObject > xUserConfigStorageCommit( m_xUserConfigStorage, UNO_QUERY );
            if ( xUserConfigStorageCommit.is() )
                xUserConfigStorageCommit->commit();
            if ( m_xUserRootCommit.is() )
                m_xUserRootCommit->commit();
        }

        m_bModified = sal_False;
    }
}

void ImageManagerImpl::storeToStorage( const uno::Reference< XStorage >& Storage )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bModified && Storage.is() )
    {
        long nModes = ElementModes::READWRITE;

        uno::Reference< XStorage > xUserImageStorage = Storage->openStorageElement( OUString(IMAGE_FOLDER ),
                                                                                    nModes );
        if ( xUserImageStorage.is() )
        {
            uno::Reference< XStorage > xUserBitmapsStorage = xUserImageStorage->openStorageElement( OUString(BITMAPS_FOLDER ),
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

sal_Bool ImageManagerImpl::isModified()
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );
    return m_bModified;
}

sal_Bool ImageManagerImpl::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );
    return m_bReadOnly;
}
// XUIConfiguration
void ImageManagerImpl::addConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
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

void ImageManagerImpl::removeConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const uno::Reference< XUIConfigurationListener >* ) NULL ), xListener );
}


void ImageManagerImpl::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
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
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}
void ImageManagerImpl::clear()
{
    ResetableGuard aGuard( m_aLock );

    if (!m_pUserImageList)
        return;

    for ( sal_Int32 n = 0; n < ImageType_COUNT; n++ )
    {
        delete m_pUserImageList[n];
        m_pUserImageList[n] = 0;
    }
}
} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
