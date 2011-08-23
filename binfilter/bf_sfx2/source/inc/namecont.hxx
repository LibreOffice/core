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

#ifndef _SFX_NAMECONT_HXX
#define _SFX_NAMECONT_HXX

#include <hash_map>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include <com/sun/star/util/XMacroExpander.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <bf_so3/svstor.hxx>
#ifndef _XMLSCRIPT_XMLLIB_IMEXP_HXX_
#include <xmlscript/xmllib_imexp.hxx>
#endif

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase4.hxx>
namespace binfilter {

class BasicManager;

typedef ::cppu::WeakImplHelper4<
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::script::XLibraryContainer2,
    ::com::sun::star::script::XLibraryContainerPassword,
    ::com::sun::star::container::XContainer > LibraryContainerHelper;


namespace SfxContainer_Impl
{

typedef ::cppu::WeakImplHelper2< ::com::sun::star::container::XNameContainer,
    ::com::sun::star::container::XContainer > NameContainerHelper;


struct hashName_Impl
{
    size_t operator()(const ::rtl::OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqName_Impl
{
    sal_Bool operator()(const ::rtl::OUString Str1, const ::rtl::OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef std::hash_map
<
    ::rtl::OUString,
    sal_Int32,
    hashName_Impl,
    eqName_Impl
>
NameContainerNameMap;



//============================================================================

struct MutexHolder
{
    ::osl::Mutex mMutex;
};

class NameContainer_Impl : public MutexHolder, public NameContainerHelper
{
    NameContainerNameMap mHashMap;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > mNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > mValues;
    sal_Int32 mnElementCount;

    ::com::sun::star::uno::Type mType;
    ::com::sun::star::uno::XInterface* mpxEventSource;

    ::cppu::OInterfaceContainerHelper maListenerContainer;

public:
    NameContainer_Impl( const ::com::sun::star::uno::Type& rType )
        : mType( rType )
        , mpxEventSource( NULL )
        , mnElementCount( 0 )
        , maListenerContainer( mMutex )
    {}

    void setEventSource( ::com::sun::star::uno::XInterface* pxEventSource )
        { mpxEventSource = pxEventSource; }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);

};

}	// namespace SfxContainer_Impl


//============================================================================

class SfxLibrary_Impl;


enum InitMode
{
    DEFAULT,
    CONTAINER_INIT_FILE,
    LIBRARY_INIT_FILE,
    OFFICE_DOCUMENT,
    OLD_BASIC_STORAGE
};

class SfxLibraryContainer_Impl : public LibraryContainerHelper
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >   mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >       mxSFI;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringSubstitution >    mxStringSubstitution;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >         mxMacroExpander;

    SfxContainer_Impl::NameContainer_Impl maNameContainer;
    sal_Bool mbModified;
    sal_Bool mbOldInfoFormat;

    ::rtl::OUString maInitialisationParam;
    ::rtl::OUString maInfoFileName;
    ::rtl::OUString maOldInfoFileName;
    ::rtl::OUString maLibElementFileExtension;
    ::rtl::OUString maLibraryPath;
    ::rtl::OUString maLibrariesDir;

    SotStorageRef   mxStorage;
    BasicManager*   mpBasMgr;
    sal_Bool        mbOwnBasMgr;

    InitMode meInitMode;

    void implStoreLibraryIndexFile( SfxLibrary_Impl* pLib,
        const ::xmlscript::LibDescriptor& rLib, SotStorageRef xStorage );
    sal_Bool implLoadLibraryIndexFile( SfxLibrary_Impl* pLib, ::xmlscript::LibDescriptor& rLib,
                                       SotStorageRef xStorage, const ::rtl::OUString& aIndexFileName );
    void implImportLibDescriptor( SfxLibrary_Impl* pLib, ::xmlscript::LibDescriptor& rLib );

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary_Impl* SAL_CALL implCreateLibrary( void ) = 0;
    virtual SfxLibrary_Impl* SAL_CALL implCreateLibraryLink
        ( const ::rtl::OUString& aLibInfoFileURL,
          const ::rtl::OUString& StorageURL, sal_Bool ReadOnly ) = 0;
    virtual ::com::sun::star::uno::Any SAL_CALL createEmptyLibraryElement( void ) = 0;
    virtual sal_Bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) = 0;
    virtual void SAL_CALL writeLibraryElement
    (
        ::com::sun::star::uno::Any aElement,
        const ::rtl::OUString& aElementName,
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutput
    )
        throw(::com::sun::star::uno::Exception) = 0;

    virtual ::com::sun::star::uno::Any SAL_CALL importLibraryElement
        ( const ::rtl::OUString& aFile, SotStorageStreamRef xElementStream ) = 0;
    virtual void SAL_CALL importFromOldStorage( const ::rtl::OUString& aFile ) = 0;

    void implStoreLibrary( SfxLibrary_Impl* pLib,
        const ::rtl::OUString& aName, SotStorageRef xStorage );

    // Password encryption
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary_Impl* pLib, const ::rtl::OUString& aName,
        SotStorageRef xStorage );
    virtual sal_Bool implLoadPasswordLibrary( SfxLibrary_Impl* pLib, const ::rtl::OUString& Name,
        sal_Bool bVerifyPasswordOnly=false )
            throw(::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    // #56666, Creates another library container
    //         instance of the same derived class
    virtual SfxLibraryContainer_Impl* createInstanceImpl( void ) = 0;


    // Interface to get the BasicManager (Hack for password implementation)
    BasicManager* getBasicManager( void ) { return mpBasMgr; }
    ::rtl::OUString createAppLibraryFolder( SfxLibrary_Impl* pLib, const ::rtl::OUString& aName );

    sal_Bool init( const ::rtl::OUString& aInitialisationParam,
                   const ::rtl::OUString& aInfoFileName,
                   const ::rtl::OUString& aOldInfoFileName,
                   const ::rtl::OUString& aLibElementFileExtension,
                   const ::rtl::OUString& aLibrariesDir,
                   SotStorageRef xStorage );

    // Handle maLibInfoFileURL and maStorageURL correctly
    void checkStorageURL
    (
        const ::rtl::OUString& aSourceURL,
        ::rtl::OUString& aLibInfoFileURL,
        ::rtl::OUString& aStorageURL,
        ::rtl::OUString& aUnexpandedStorageURL
    );
    ::rtl::OUString expand_url( const ::rtl::OUString& url )
        throw(::com::sun::star::uno::RuntimeException);

    SfxLibrary_Impl* getImplLib( const String& rLibraryName );

public:
    SfxLibraryContainer_Impl( void );
    ~SfxLibraryContainer_Impl();

    // Interface to release and reset storage for saving
    void setStorage( SotStorageRef xStorage )
    {
        mxStorage = xStorage;
    }

    // Interface to set the BasicManager (Hack for password implementation)
    void setBasicManager( BasicManager* pBasMgr )
    {
        mpBasMgr = pBasMgr;
    }

    // TODO: Methods of new XLibraryStorage interface?
    virtual void SAL_CALL storeLibraries( sal_Bool bComplete );
    virtual void SAL_CALL storeLibrariesToStorage( SotStorageRef xStorage );
    virtual void SAL_CALL storeLibraries_Impl( SotStorageRef xStorage, sal_Bool bComplete );

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL
        createLibrary( const ::rtl::OUString& Name )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL createLibraryLink
        ( const ::rtl::OUString& Name, const ::rtl::OUString& StorageURL, sal_Bool ReadOnly )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeLibrary( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryLoaded( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL loadLibrary( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException) = 0;

    // Methods XLibraryContainer2
    virtual sal_Bool SAL_CALL isLibraryLink( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLibraryLinkURL( const ::rtl::OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryReadOnly( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLibraryReadOnly( const ::rtl::OUString& Name, sal_Bool bReadOnly )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL renameLibrary( const ::rtl::OUString& Name, const ::rtl::OUString& NewName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const ::rtl::OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const ::rtl::OUString& Name, const ::rtl::OUString& Password )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeLibraryPassword( const ::rtl::OUString& Name,
        const ::rtl::OUString& OldPassword, const ::rtl::OUString& NewPassword )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
};


//============================================================================

struct OLibraryMutex
{
    ::osl::Mutex m_mutex;
};

class SfxLibrary_Impl
    : public ::com::sun::star::container::XNameContainer
    , public ::com::sun::star::container::XContainer
    , public OLibraryMutex
    , public ::cppu::OComponentHelper
{
    friend class SfxLibraryContainer_Impl;
    friend class SfxScriptLibraryContainer;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >   mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >       mxSFI;

    SfxContainer_Impl::NameContainer_Impl maNameContainer;

    sal_Bool mbLoaded;
    sal_Bool mbModified;
    sal_Bool mbInitialised;

    ::rtl::OUString maLibElementFileExtension;
    ::rtl::OUString maLibInfoFileURL;
    ::rtl::OUString maStorageURL;
    ::rtl::OUString maUnexpandedStorageURL;
    sal_Bool mbLink;
    sal_Bool mbReadOnly;
    sal_Bool mbReadOnlyLink;

    sal_Bool mbPasswordProtected;
    sal_Bool mbPasswordVerified;
    sal_Bool mbDoc50Password;
    ::rtl::OUString maPassword;

    sal_Bool mbSharedIndexFile;


public:
    SfxLibrary_Impl( ::com::sun::star::uno::Type aType,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF,
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSFI );
    SfxLibrary_Impl( ::com::sun::star::uno::Type aType,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF,
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSFI,
        const ::rtl::OUString& aLibInfoFileURL, const ::rtl::OUString& aStorageURL, sal_Bool ReadOnly );


    // Methods XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw() { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() { OComponentHelper::release(); }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
        throw( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  )
        throw( ::com::sun::star::uno::RuntimeException );

    // Methods XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XContainerListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException);
};


}//end of namespace binfilter
#endif

