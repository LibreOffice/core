#include "ftpcontentidentifier.hxx"



using namespace ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;


FtpContentIdentifier::FtpContentIdentifier(const rtl::OUString& aIdent)
    : m_aIdent(aIdent)
{
}


Any SAL_CALL
FtpContentIdentifier::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException
    )
{
    Any aRet = ::cppu::queryInterface(rType,
                                      SAL_STATIC_CAST(XContentIdentifier*,this));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



void SAL_CALL FtpContentIdentifier::acquire( void ) throw() {
    OWeakObject::acquire();
}



void SAL_CALL FtpContentIdentifier::release( void ) throw() {
    OWeakObject::release();
}


::rtl::OUString SAL_CALL
FtpContentIdentifier::getContentIdentifier(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    return m_aIdent;
}


::rtl::OUString SAL_CALL
FtpContentIdentifier::getContentProviderScheme(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    return rtl::OUString::createFromAscii("ftp");
}

