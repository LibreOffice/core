
#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#include "ZipPackageFolderEnumeration.hxx"
#endif

using namespace com::sun::star;

ZipPackageFolderEnumeration::ZipPackageFolderEnumeration ( TunnelHash &rInput)
//std::hash_map < rtl::OUString, com::sun::star::uno::Reference < com::sun::star::container::XNamed >, hashFunc, eqFunc > &rContents);
: rContents (rInput)
, aIterator (rInput.begin())
{
}

ZipPackageFolderEnumeration::~ZipPackageFolderEnumeration( void )
{
}

sal_Bool SAL_CALL ZipPackageFolderEnumeration::hasMoreElements(  )
        throw(uno::RuntimeException)
{
    return  (aIterator != rContents.end() );
}
uno::Any SAL_CALL ZipPackageFolderEnumeration::nextElement(  )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    aIterator++;
    aAny <<= (*aIterator).second;
    return aAny;
}
