
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include "ZipPackageFolder.hxx"
#endif

using namespace com::sun::star;
using namespace rtl;

ZipPackageFolder::ZipPackageFolder (ZipOutputStream &rStream)//ZipPackage &rInPackage)
: rZipOut(rStream)
{
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nFlag        |= 1 << 10;
    aEntry.nMethod      = -1;
    aEntry.nTime        = -1;
    aEntry.nCrc         = -1;
    aEntry.nCompressedSize  = -1;
    aEntry.nSize        = -1;
    aEntry.nOffset      = -1;
}

ZipPackageFolder::~ZipPackageFolder( void )
{

}
uno::Any SAL_CALL ZipPackageFolder::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    // Ask for my own supported interfaces ...
    uno::Any aReturn    ( ::cppu::queryInterface    (   rType                                       ,
                                                static_cast< container::XNamed*     > ( this )  ,
                                                static_cast< container::XChild*     > ( this )  ,
                                                static_cast< container::XNameContainer*     > ( this )  ,
                                                static_cast< container::XEnumerationAccess*     > ( this )  ,
                                                static_cast< beans::XPropertySet*       > ( this ) ) ) ;

    // If searched interface supported by this class ...
    if ( aReturn.hasValue () == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return ZipPackageEntry::queryInterface ( rType ) ;
    }
}
void SAL_CALL ZipPackageFolder::acquire(  )
    throw()
{
    ZipPackageEntry::acquire();
}
void SAL_CALL ZipPackageFolder::release(  )
    throw()
{
    ZipPackageEntry::release();
}
    // XNameContainer
void SAL_CALL ZipPackageFolder::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (hasByName(aName))
        throw container::ElementExistException();
    else
    {
        uno::Reference < beans::XPropertySet > xRef;
        aElement >>= xRef;
        aContents[aName] = xRef;
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const ::rtl::OUString& Name )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    aContents.erase(Name);
}
    // XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < container::XEnumeration> (new ZipPackageFolderEnumeration(aContents));
}
    // XElementAccess
uno::Type SAL_CALL ZipPackageFolder::getElementType(  )
        throw(uno::RuntimeException)
{
    return ::getCppuType ((const uno::Reference< container::XNamed > *) 0);
}
sal_Bool SAL_CALL ZipPackageFolder::hasElements(  )
        throw(uno::RuntimeException)
{
    return aContents.size() > 0;
}
    // XNameAccess
uno::Any SAL_CALL ZipPackageFolder::getByName( const ::rtl::OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    PropertyHash::const_iterator aCI = aContents.find(aName);
    if (aCI == aContents.end())
        throw container::NoSuchElementException();
//  rtl::OUString sTemp = aCI->first;
//  uno::Reference < container::XNamed > xRef (aCI->second);
    aAny <<= (*aCI).second;
    return aAny;
}
uno::Sequence< ::rtl::OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(uno::RuntimeException)
{
    sal_uInt32 i=0, nSize = aContents.size();
    OUString *pNames = new OUString[nSize];
    for (PropertyHash::const_iterator aIterator = aContents.begin() ; aIterator != aContents.end(); i++,aIterator++)
        pNames[i] = (*aIterator).first;
    return uno::Sequence < OUString > (pNames, nSize);
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const ::rtl::OUString& aName )
        throw(uno::RuntimeException)
{
    return aContents.find(aName) != aContents.end();
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    removeByName(aName);
    insertByName(aName, aElement);
}
    //XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageFolder::getPropertySetInfo(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < beans::XPropertySetInfo > (NULL);
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (aPropertyName == L"MediaType")
        aValue >>= sMediaType;
    else if (aPropertyName == L"ZipEntry")
        aValue >>= aEntry;
    else if (aPropertyName == L"Path")
        aValue >>= sPath;
    else
        throw beans::UnknownPropertyException();
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (PropertyName == L"MediaType")
    {
        uno::Any aAny;
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName == L"ZipEntry")
    {
        uno::Any aAny;
        aAny <<= aEntry;
        return aAny;
    }
    else if (PropertyName == L"Path")
    {
        uno::Any aAny;
        aAny <<= sPath;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}
void SAL_CALL ZipPackageFolder::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageFolder::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageFolder::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageFolder::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void ZipPackageFolder::saveContents(rtl::OUString &rPath)
{
    sPath = sPath + getName();
    PropertyHash::const_iterator aCI = aContents.begin();
    uno::Reference < beans::XPropertySet > xProp;
    package::ZipEntry aEntry;
    uno::Any aAny;

    for (;aCI!=aContents.end();aCI++)
    {
        //xProp = uno::Reference < beans::XPropertySet> (aCI->second, uno::UNO_QUERY);
        aAny = aCI->second->getPropertyValue(L"ZipEntry");
        aAny >>= aEntry;
        aEntry.nCrc = -1;
        aEntry.nSize = -1;
        aEntry.nCompressedSize = -1;

        rZipOut.putNextEntry(aEntry);
        if (aEntry.nFlag & 1 << 10)
        {
            aEntry.sName = aEntry.sName + L"/";
            rZipOut.closeEntry();
        }
        else
        {
            sal_Int64 nLength;
            uno::Sequence < sal_Int8 > aSeq (65535);
            uno::Reference < io::XActiveDataSink > xSink = uno::Reference < io::XActiveDataSink > (aCI->second, uno::UNO_QUERY);
            uno::Reference < io::XInputStream > xStream = xSink->getInputStream();

            while (1)
            {
                nLength = xStream->readBytes(aSeq, 65535);
                if (nLength < 65535)
                    aSeq.realloc(nLength);
                rZipOut.write(aSeq, 0, nLength);
                if (nLength < 65535) // EOF
                    break;
            }
            rZipOut.closeEntry();
        }
    }
}
sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
    throw(::com::sun::star::uno::RuntimeException)
{
    return (sal_Int64)0;
}
