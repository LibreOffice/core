#ifndef _FILCMD_HXX_
#include "filcmd.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


XCommandInfo_impl::XCommandInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath )
    : m_pMyShell( pMyShell ),
      m_xProvider( pMyShell->m_pProvider )
{
}

XCommandInfo_impl::~XCommandInfo_impl()
{
}



void SAL_CALL
XCommandInfo_impl::acquire(
                 void )
  throw( uno::RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
XCommandInfo_impl::release(
    void )
  throw( uno::RuntimeException )
{
    OWeakObject::release();
}


uno::Any SAL_CALL
XCommandInfo_impl::queryInterface(
                    const uno::Type& rType )
  throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( XCommandInfo*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< CommandInfo > SAL_CALL
XCommandInfo_impl::getCommands(
    void )
    throw( uno::RuntimeException )
{
    return m_pMyShell->m_sCommandInfo;
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByName(
    const rtl::OUString& aName )
    throw( UnsupportedCommandException,
           uno::RuntimeException)
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); i++ )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException();
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByHandle(
    sal_Int32 Handle )
    throw( UnsupportedCommandException,
           uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException();
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByName(
    const rtl::OUString& aName )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return true;

    return false;
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByHandle(
    sal_Int32 Handle )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return true;

    return false;
}
