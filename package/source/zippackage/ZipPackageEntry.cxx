
#ifndef _ZIP_PACKAGE_ENTRY_HXX
#include "ZipPackageEntry.hxx"
#endif

using namespace com::sun::star;
ZipPackageEntry::ZipPackageEntry (void)
{
}

ZipPackageEntry::~ZipPackageEntry( void )
{
}

uno::Any SAL_CALL ZipPackageEntry::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    // Ask for my own supported interfaces ...
    uno::Any aReturn    ( ::cppu::queryInterface    (   rType                                       ,
                                                static_cast< container::XNamed*     > ( this )  ,
                                                static_cast< container::XChild*     > ( this )  ) );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue () == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface ( rType ) ;
    }
}
void SAL_CALL ZipPackageEntry::acquire(  )
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL ZipPackageEntry::release(  )
    throw()
{
    OWeakObject::release();
}
    // XChild
::rtl::OUString SAL_CALL ZipPackageEntry::getName(  )
    throw(uno::RuntimeException)
{
    return aEntry.sName;
}
void SAL_CALL ZipPackageEntry::setName( const ::rtl::OUString& aName )
    throw(uno::RuntimeException)
{
    aEntry.sName = aName;
}
uno::Reference< uno::XInterface > SAL_CALL ZipPackageEntry::getParent(  )
        throw(uno::RuntimeException)
{
    return xParent;
}
void SAL_CALL ZipPackageEntry::setParent( const uno::Reference< uno::XInterface >& Parent )
        throw(lang::NoSupportException, uno::RuntimeException)
{
    xParent = Parent;
}
