
#ifndef _ZIP_PACKAGE_HXX
#include "ZipPackage.hxx"
#endif

using namespace com::sun::star::package::ZipConstants;
using namespace com::sun::star;
using namespace rtl;

ZipPackage::ZipPackage (uno::Reference < io::XInputStream > xInput, uno::Reference < io::XOutputStream > xOutput)
: xStream(xInput)
, aZipFile(xInput)
, aZipOut(xOutput, 65535)
, aRootFolder(aZipOut)//*this)
{
    uno::Reference< container::XEnumeration > xEnum = aZipFile.entries();
    uno::Reference< container::XNameContainer > xCurrent  = uno::Reference < container::XNameContainer > (&aRootFolder);
    ZipPackageStream *pPkgStream;
    ZipPackageFolder *pPkgFolder;//, *pCurrentFolder =&aRootFolder;
    package::ZipEntry aEntry;
    aZipOut.setMethod(DEFLATED);
    aZipOut.setLevel(DEFAULT_COMPRESSION);

    //for (sal_uInt16 i=0;i<aZipFile.nTotal;i++)
    while (xEnum->hasMoreElements())
    {
        xCurrent  = uno::Reference < container::XNameContainer > (&aRootFolder);
        sal_Int32 nOldIndex =0,nIndex = 0;
        uno::Any aAny = xEnum->nextElement();
        aAny >>= aEntry;
        OUString &rName = aEntry.sName;

        if (rName.lastIndexOf('/') == rName.getLength()-1)
        {
            while ((nIndex = rName.indexOf('/', nOldIndex)) != -1)
            {
                OUString sTemp = rName.copy (nOldIndex, nIndex - nOldIndex);
                if (nIndex == nOldIndex) //sTemp.getLength() == 1)
                    break;
                if (!xCurrent->hasByName(sTemp))
                {
                    pPkgFolder = new ZipPackageFolder(aZipOut);//*this);
                    pPkgFolder->setName(sTemp);
                    pPkgFolder->setParent( uno::Reference < uno::XInterface >(xCurrent, uno::UNO_QUERY));
                    aAny <<= uno::Reference < lang::XUnoTunnel > (pPkgFolder);
                    xCurrent->insertByName(sTemp, aAny);
                    xCurrent = uno::Reference < container::XNameContainer > (pPkgFolder);
                }
                else
                {
                    aAny = xCurrent->getByName(sTemp);
                    uno::Reference < lang::XUnoTunnel> xRef;
                    aAny >>= xRef;
                    xCurrent = uno::Reference < container::XNameContainer > (xRef, uno::UNO_QUERY);
                }

                nOldIndex = nIndex+1;
            }
        }
        else
        {
            while ((nIndex = rName.indexOf('/', nOldIndex)) != -1)
            {
                OUString sTemp = rName.copy (nOldIndex, nIndex - nOldIndex);
                if (nIndex == nOldIndex) //sTemp.getLength() == 1)
                    break;
                if (xCurrent->hasByName(sTemp))
                {
                    uno::Reference < lang::XUnoTunnel > xChildRef;
                    aAny = xCurrent->getByName(sTemp);
                    aAny >>= xChildRef;
                    xCurrent = uno::Reference < container::XNameContainer > (xChildRef, uno::UNO_QUERY);
                }
                nOldIndex = nIndex+1;
            }
            OUString sStreamName = rName.copy( nOldIndex, rName.getLength() - nOldIndex);
            pPkgStream = new ZipPackageStream( aZipFile );
            pPkgStream->bPackageMember = sal_True;
            pPkgStream->setParent( uno::Reference < uno::XInterface > (xCurrent, uno::UNO_QUERY));
            pPkgStream->setZipEntry( aEntry );
            pPkgStream->setName( sStreamName );
            aAny <<= uno::Reference < lang::XUnoTunnel > (pPkgStream);
            xCurrent->insertByName(sStreamName, aAny);
        }
    }
}

ZipPackage::~ZipPackage( void )
{
}

// XInitialization
void SAL_CALL ZipPackage::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw(uno::Exception, uno::RuntimeException)
{
}
// XHierarchicalNameAccess
uno::Any SAL_CALL ZipPackage::getByHierarchicalName( const ::rtl::OUString& aName )
        throw(container::NoSuchElementException, uno::RuntimeException)
{
    sal_Int32 nOldIndex =0,nIndex = 0;
    uno::Any aAny;
    uno::Reference < container::XNameContainer > xCurrent  = uno::Reference < container::XNameContainer > (&aRootFolder);
    if (aName.lastIndexOf('/') == aName.getLength()-1)
    {
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            OUString sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            if (nIndex == nOldIndex) //sTemp.getLength() == 1)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                uno::Reference < lang::XUnoTunnel > xRef;
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xRef;
                xCurrent = uno::Reference < container::XNameContainer > (xRef, uno::UNO_QUERY);
            }
            nOldIndex = nIndex+1;
        }
        OUString sStreamName = aName.copy( nOldIndex, aName.getLength() - nOldIndex);
        aAny <<= xCurrent->getByName(sStreamName);
    }
    else
    {
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            OUString sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            if (nIndex == nOldIndex) //sTemp.getLength() == 1)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                uno::Reference < lang::XUnoTunnel > xChildRef;
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xChildRef;
                xCurrent = uno::Reference < container::XNameContainer > (xChildRef, uno::UNO_QUERY);
            }
            nOldIndex = nIndex+1;
        }
        OUString sStreamName = aName.copy( nOldIndex, aName.getLength() - nOldIndex);

        aAny <<= xCurrent->getByName(sStreamName);
    }
    return aAny;
}
sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const ::rtl::OUString& aName )
        throw(uno::RuntimeException)
{
    sal_Int32 nOldIndex =0,nIndex = 0;
    uno::Any aAny;
    uno::Reference < container::XNameContainer > xCurrent  = uno::Reference < container::XNameContainer > (&aRootFolder);
    if (aName.lastIndexOf('/') == aName.getLength()-1)
    {
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            OUString sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            if (nIndex == nOldIndex) //sTemp.getLength() == 1)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                uno::Reference < lang::XUnoTunnel > xRef;
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xRef;
                xCurrent = uno::Reference < container::XNameContainer > (xRef, uno::UNO_QUERY);
            }
            nOldIndex = nIndex+1;
        }
        OUString sStreamName = aName.copy( nOldIndex, aName.getLength() - nOldIndex);
        return xCurrent->hasByName(sStreamName);
    }
    else
    {
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            OUString sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            if (nIndex == nOldIndex) //sTemp.getLength() == 1)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                uno::Reference < lang::XUnoTunnel > xChildRef;
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xChildRef;
                xCurrent = uno::Reference < container::XNameContainer > (xChildRef, uno::UNO_QUERY);
            }
            nOldIndex = nIndex+1;
        }
        OUString sStreamName = aName.copy( nOldIndex, aName.getLength() - nOldIndex);
        return xCurrent->hasByName(sStreamName);
    }
}
// XSingleServiceFactory
uno::Reference< uno::XInterface > SAL_CALL ZipPackage::createInstance(  )
        throw(uno::Exception, uno::RuntimeException)
{
    return uno::Reference < uno::XInterface > (*this);
}
uno::Reference< uno::XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const uno::Sequence< uno::Any >& aArguments )
        throw(uno::Exception, uno::RuntimeException)
{
    return uno::Reference < uno::XInterface > (*this);
}
// XChangesBatch
void SAL_CALL ZipPackage::commitChanges(  )
        throw(lang::WrappedTargetException, uno::RuntimeException)
{
    aRootFolder.saveContents(OUString::createFromAscii(""));
    aZipOut.finish();
}
sal_Bool SAL_CALL ZipPackage::hasPendingChanges(  )
        throw(uno::RuntimeException)
{
    return sal_False;
}
uno::Sequence< util::ElementChange > SAL_CALL ZipPackage::getPendingChanges(  )
        throw(uno::RuntimeException)
{
    return uno::Sequence < util::ElementChange > ( NULL, 0 );
}
