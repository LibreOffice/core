#include "ftpcontentidentifier.hxx"


using namespace ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;


FTPContentIdentifier::FTPContentIdentifier(
    const rtl::OUString& aIdent,
    FTPContentProvider* pFCP
)
    : m_pURL(new FTPURL(aIdent,pFCP))
{
}

FTPContentIdentifier::FTPContentIdentifier(FTPURL* pURL)
    : m_pURL(pURL)
{
}


FTPContentIdentifier::~FTPContentIdentifier()
{
    delete m_pURL;
}


Any SAL_CALL
FTPContentIdentifier::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException
    )
{
    Any aRet =
        ::cppu::queryInterface(rType,
                               SAL_STATIC_CAST(XContentIdentifier*,this));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



void SAL_CALL FTPContentIdentifier::acquire( void ) throw() {
    OWeakObject::acquire();
}



void SAL_CALL FTPContentIdentifier::release( void ) throw() {
    OWeakObject::release();
}


::rtl::OUString SAL_CALL
FTPContentIdentifier::getContentIdentifier(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    return m_pURL->ident();
}


::rtl::OUString SAL_CALL
FTPContentIdentifier::getContentProviderScheme(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    return rtl::OUString::createFromAscii("ftp");
}

