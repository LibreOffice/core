
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include "ZipPackageFolder.hxx"
#endif

using namespace com::sun::star::package::ZipConstants;
using namespace com::sun::star;
using namespace rtl;

ZipPackageFolder::ZipPackageFolder (ZipOutputStream &rStream)//ZipPackage &rInPackage)
: rZipOut(rStream)
{
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = STORED;
    aEntry.nTime        = -1;
    aEntry.nCrc         = 0;
    aEntry.nCompressedSize  = 0;
    aEntry.nSize        = 0;
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
                                                static_cast< lang::XUnoTunnel*      > ( this )  ,
                                                static_cast< beans::XPropertySet*   > ( this ) ) ) ;

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
        uno::Reference < lang::XUnoTunnel > xRef;
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
    return ::getCppuType ((const uno::Reference< lang::XUnoTunnel > *) 0);
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
    TunnelHash::const_iterator aCI = aContents.find(aName);
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
    for (TunnelHash::const_iterator aIterator = aContents.begin() ; aIterator != aContents.end(); i++,aIterator++)
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
    uno::Reference < lang::XUnoTunnel > xTunnel;
    package::ZipEntry *aEntry = NULL;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    sal_Bool bIsFolder;
    TunnelHash::const_iterator aCI = aContents.begin();

    //rPath = rPath + getName();

    for (;aCI!=aContents.end();aCI++)
    {
        xTunnel = uno::Reference < lang::XUnoTunnel> ((*aCI).second, uno::UNO_QUERY);
        try
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > (xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId()));
            bIsFolder = sal_True;
        }
        catch (uno::RuntimeException)
        {
            pStream = reinterpret_cast < ZipPackageStream* > (xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId()));
            bIsFolder = sal_False;
        }

        if (bIsFolder)
        {
            pFolder->aEntry.sName = rPath + pFolder->getName() + L"/";
            Time aTime;
            aTime = Time();
            pFolder->aEntry.nTime = aTime.GetTime();
            rZipOut.putNextEntry(pFolder->aEntry);
            rZipOut.closeEntry();
            pFolder->saveContents(pFolder->aEntry.sName);
        }
        else
        {
            pStream->aEntry.sName = rPath + pStream->getName();
            uno::Reference < io::XInputStream > xStream = pStream->getInputStream();
            rZipOut.putNextEntry(pStream->aEntry);
            while (1)
            {
                uno::Sequence < sal_Int8 > aSeq (65535);
                sal_Int64 nLength;
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

uno::Sequence< sal_Int8 > ZipPackageFolder::getUnoTunnelImplementationId( void )
    throw (uno::RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    if (aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    throw uno::RuntimeException();
}
