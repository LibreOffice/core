#ifndef _ZIP_PACKAGE_HXX
#define _ZIP_PACKAGE_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx> // helper for implementations
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _ZIP_FILE_HXX
#include "ZipFile.hxx"
#endif

#ifndef _ZIP_OUTPUT_STREAM_HXX
#include "ZipOutputStream.hxx"
#endif

#include <hash_map>
class ZipPackageFolder;
#include "ZipPackageFolder.hxx"
#include "ZipPackageStream.hxx"
class ZipPackage : public cppu::WeakImplHelper4<
                        com::sun::star::lang::XInitialization,
                        com::sun::star::container::XHierarchicalNameAccess,
                        com::sun::star::lang::XSingleServiceFactory,
                        com::sun::star::util::XChangesBatch
                        >
{
private:
    ZipPackageFolder aRootFolder;
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
public:
    ZipFile          aZipFile;
    ZipOutputStream  aZipOut;
    ZipPackage (com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xInput, com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > xOutput  );
    virtual ~ZipPackage( void );
    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XHierarchicalNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByHierarchicalName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);
    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XChangesBatch
    virtual void SAL_CALL commitChanges(  )
        throw(::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPendingChanges(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::ElementChange > SAL_CALL getPendingChanges(  )
        throw(::com::sun::star::uno::RuntimeException);
};
#endif
