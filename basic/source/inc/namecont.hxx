/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namecont.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:52:55 $
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

#ifndef BASIC_NAMECONTAINER_HXX
#define BASIC_NAMECONTAINER_HXX

#include <hash_map>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSTORAGEBASEDLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINEREXPORT_HPP_
#include <com/sun/star/script/XLibraryContainerExport.hpp>
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
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include <com/sun/star/util/XMacroExpander.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTORAGEBASEDDOCUMENT_HPP_
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include <cppuhelper/basemutex.hxx>
#endif

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _XMLSCRIPT_XMLLIB_IMEXP_HXX_
#include <xmlscript/xmllib_imexp.hxx>
#endif

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/interfacecontainer.hxx>

class BasicManager;

namespace basic
{

typedef ::cppu::WeakComponentImplHelper6<
    ::com::sun::star::lang::XInitialization,
    ::com::sun::star::script::XStorageBasedLibraryContainer,
    ::com::sun::star::script::XLibraryContainerPassword,
    ::com::sun::star::script::XLibraryContainerExport,
    ::com::sun::star::container::XContainer,
    ::com::sun::star::lang::XServiceInfo > LibraryContainerHelper;

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

class NameContainer : public ::cppu::BaseMutex, public NameContainerHelper
{
    NameContainerNameMap mHashMap;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > mNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > mValues;
    sal_Int32 mnElementCount;

    ::com::sun::star::uno::Type mType;
    ::com::sun::star::uno::XInterface* mpxEventSource;

    ::cppu::OInterfaceContainerHelper maListenerContainer;

public:
    NameContainer( const ::com::sun::star::uno::Type& rType )
        : mnElementCount( 0 )
        , mType( rType )
        , mpxEventSource( NULL )
        , maListenerContainer( m_aMutex )
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

//============================================================================

class SfxLibrary;

enum InitMode
{
    DEFAULT,
    CONTAINER_INIT_FILE,
    LIBRARY_INIT_FILE,
    OFFICE_DOCUMENT,
    OLD_BASIC_STORAGE
};

class ModifiableHelper
{
private:
    ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
    ::cppu::OWeakObject&                m_rEventSource;
    sal_Bool                            mbModified;

public:
    ModifiableHelper( ::cppu::OWeakObject& _rEventSource, ::osl::Mutex& _rMutex )
        :m_aModifyListeners( _rMutex )
        ,m_rEventSource( _rEventSource )
        ,mbModified( sal_False )
    {
    }

    inline  sal_Bool    isModified() const  { return mbModified; }
            void        setModified( sal_Bool _bModified );

    inline  void    addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.addInterface( _rxListener );
    }

    inline  void    removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener )
    {
        m_aModifyListeners.removeInterface( _rxListener );
    }
};

class SfxLibraryContainer   :public LibraryContainerHelper
                            ,public ::utl::OEventListenerAdapter
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >   mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >       mxSFI;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >         mxMacroExpander;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringSubstitution >    mxStringSubstitution;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >            mxOwnerDocument;

    ::osl::Mutex        maMutex;
    ModifiableHelper    maModifiable;

    NameContainer maNameContainer;
    sal_Bool    mbOldInfoFormat;
    sal_Bool    mbOasis2OOoFormat;

    ::rtl::OUString maInitialDocumentURL;
    ::rtl::OUString maInfoFileName;
    ::rtl::OUString maOldInfoFileName;
    ::rtl::OUString maLibElementFileExtension;
    ::rtl::OUString maLibraryPath;
    ::rtl::OUString maLibrariesDir;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > mxStorage;
    BasicManager*   mpBasMgr;
    sal_Bool        mbOwnBasMgr;

    InitMode meInitMode;

    void implStoreLibrary( SfxLibrary* pLib,
                            const ::rtl::OUString& aName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibrary( SfxLibrary* pLib,
                            const ::rtl::OUString& aName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            const ::rtl::OUString& aTargetURL,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );

    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // New variant for library export
    void implStoreLibraryIndexFile( SfxLibrary* pLib, const ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const ::rtl::OUString& aTargetURL,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI );

    sal_Bool implLoadLibraryIndexFile( SfxLibrary* pLib,
                                    ::xmlscript::LibDescriptor& rLib,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const ::rtl::OUString& aIndexFileName );

    void implImportLibDescriptor( SfxLibrary* pLib, ::xmlscript::LibDescriptor& rLib );

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const ::rtl::OUString& aName ) = 0;
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const ::rtl::OUString& aName, const ::rtl::OUString& aLibInfoFileURL,
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
        ( const ::rtl::OUString& aFile,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xElementStream ) = 0;
    virtual void SAL_CALL importFromOldStorage( const ::rtl::OUString& aFile ) = 0;

    // Password encryption
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    // New variant for library export
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        const ::rtl::OUString& aTargetURL,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    virtual sal_Bool implLoadPasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& Name,
        sal_Bool bVerifyPasswordOnly=false )
            throw(::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    virtual void onNewRootStorage() = 0;


    // #56666, Creates another library container
    //         instance of the same derived class
    virtual SfxLibraryContainer* createInstanceImpl( void ) = 0;


    // Interface to get the BasicManager (Hack for password implementation)
    BasicManager* getBasicManager( void );
    ::rtl::OUString createAppLibraryFolder( SfxLibrary* pLib, const ::rtl::OUString& aName );

    sal_Bool init( const ::rtl::OUString& rInitialDocumentURL,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxInitialStorage );

    virtual const sal_Char* SAL_CALL    getInfoFileName() const = 0;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const = 0;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const = 0;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const = 0;

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

    SfxLibrary* getImplLib( const String& rLibraryName );

    void storeLibraries_Impl(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                            sal_Bool bComplete );

    void SAL_CALL initializeFromDocumentURL( const ::rtl::OUString& _rInitialDocumentURL );
    void SAL_CALL initializeFromDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageBasedDocument >& _rxDocument );

    // OEventListenerAdapter
    virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    // OComponentHelper
    virtual void SAL_CALL disposing();

private:
    sal_Bool init_Impl( const ::rtl::OUString& rInitialDocumentURL,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxInitialStorage );
public:
    SfxLibraryContainer( void );
    ~SfxLibraryContainer();


    // Interface to set the BasicManager (Hack for password implementation)
    void setBasicManager( BasicManager* pBasMgr )
    {
        mpBasMgr = pBasMgr;
    }

    void    enterMethod();
    void    leaveMethod();
    bool    isDisposed() const { return rBHelper.bInDispose || rBHelper.bDisposed; }
    void    checkDisposed() const;

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

    // Members XStorageBasedLibraryContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getRootStorage() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRootStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rootstorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeLibrariesToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& RootStorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Methods XModifiable (base of XPersistentLibraryContainer)
    virtual ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( ::sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // Methods XPersistentLibraryContainer (base of XStorageBasedLibraryContainer)
    virtual ::com::sun::star::uno::Any SAL_CALL getRootLocation() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getContainerLocationName() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeLibraries(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Methods XLibraryContainer2 (base of XPersistentLibraryContainer)
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

    // Methods XLibraryContainer (base of XLibraryContainer2)
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

    // Methods XLibraryContainerExport
    virtual void SAL_CALL exportLibrary( const ::rtl::OUString& Name, const ::rtl::OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::container::NoSuchElementException,
                   ::com::sun::star::uno::RuntimeException);

    // Methods XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw (::com::sun::star::uno::RuntimeException) = 0;
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException) = 0;
};

class LibraryContainerMethodGuard
{
private:
    SfxLibraryContainer&    m_rContainer;
public:
    LibraryContainerMethodGuard( SfxLibraryContainer& _rContainer )
        :m_rContainer( _rContainer )
    {
        m_rContainer.enterMethod();
    }

    ~LibraryContainerMethodGuard()
    {
        m_rContainer.leaveMethod();
    }
};


//============================================================================

class SfxLibrary
    : public ::com::sun::star::container::XNameContainer
    , public ::com::sun::star::container::XContainer
    , public ::cppu::BaseMutex
    , public ::cppu::OComponentHelper
{
    friend class SfxLibraryContainer;
    friend class SfxDialogLibraryContainer;
    friend class SfxScriptLibraryContainer;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >   mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >       mxSFI;

    ModifiableHelper&   mrModifiable;
    NameContainer       maNameContainer;

    sal_Bool mbLoaded;
    sal_Bool mbIsModified;
    sal_Bool mbInitialised;

private:

    ::rtl::OUString maLibElementFileExtension;
    ::rtl::OUString maLibInfoFileURL;
    ::rtl::OUString maStorageURL;
    ::rtl::OUString maUnexpandedStorageURL;
    sal_Bool mbLink;
    sal_Bool mbReadOnly;
    sal_Bool mbReadOnlyLink;
    sal_Bool mbPreload;

    sal_Bool mbPasswordProtected;
    sal_Bool mbPasswordVerified;
    sal_Bool mbDoc50Password;
    ::rtl::OUString maPassword;

    sal_Bool mbSharedIndexFile;

    // Additional functionality for localisation
    // Provide modify state including resources
    virtual sal_Bool isModified( void ) = 0;
    virtual void storeResources( void ) = 0;
    virtual void storeResourcesAsURL( const ::rtl::OUString& URL, const ::rtl::OUString& NewName ) = 0;
    virtual void storeResourcesToURL( const ::rtl::OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) = 0;
    virtual void storeResourcesToStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage ) = 0;

protected:
    inline  sal_Bool    implIsModified() const  { return mbIsModified; }
            void        implSetModified( sal_Bool _bIsModified );

private:
    /** checks whether the lib is readonly, or a readonly link, throws an IllegalArgumentException if so
    */
    void impl_checkReadOnly();

public:
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Type& aType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI
    );
    SfxLibrary(
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Type& aType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI,
        const ::rtl::OUString& aLibInfoFileURL,
        const ::rtl::OUString&
        aStorageURL,
        sal_Bool ReadOnly
    );

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


}   // namespace basic

#endif

