#ifndef _ZIP_PACKAGE_FOLDER_HXX
#define _ZIP_PACKAGE_FOLDER_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef __com_sun_star_package_ZipEntry_hpp__
#include <com/sun/star/package/ZipEntry.hpp>
#endif

#ifndef __com_sun_star_beans_XPropertySet_hpp__
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef __com_sun_star_lang_XUnoTunnel_hpp__
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#include <hash_map>

struct eqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};

struct hashFunc
{
    sal_Int32 operator()(const rtl::OUString &r1) const
    {
        return r1.hashCode();
    }
};
//#include "ZipPackageStream.hxx"

/* This include must be after the above struct and typedef declarations.
 * Ugly...but true :)
 */

#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#include "ZipPackageFolderEnumeration.hxx"
#endif

#ifndef _ZIP_PACKAGE_ENTRY_HXX
#include "ZipPackageEntry.hxx"
#endif

#ifndef _ZIP_OUTPUT_STREAM_HXX
#include "ZipOutputStream.hxx"
#endif

typedef std::hash_map < rtl::OUString, com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > , hashFunc, eqFunc > PropertyHash;

class ZipPackageFolder : public ZipPackageEntry,
                         public ::com::sun::star::container::XNameContainer,
                         public ::com::sun::star::container::XEnumerationAccess,
                         public ::com::sun::star::beans::XPropertySet,
                         public ::com::sun::star::lang::XUnoTunnel
{
private:
    //std::hash_map < rtl::OUString, ZipPackageFolder, hashFunc, eqFunc > aFolders;
    //StreamHash aStreams;
    ZipOutputStream &rZipOut;
    ::rtl::OUString sMediaType;
    ::rtl::OUString sPath;
    PropertyHash aContents;
    com::sun::star::uno::Reference < com::sun::star::uno::XInterface > xParent;
public:
    ZipPackageFolder (ZipOutputStream &rStream );//ZipPackage &rInPackage);
    void  saveContents(rtl::OUString &rPath);
    inline sal_Bool isFolder( void ) {return sal_True;}
    inline sal_Bool isStream( void ) {return sal_False;}

    virtual ~ZipPackageFolder( void );
    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw();
    virtual void SAL_CALL release(  )
        throw();
    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  )
        throw(::com::sun::star::uno::RuntimeException);
    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);
    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);
    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);
};
#endif
