#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <localsession.hxx>
#include <treecache.hxx>
#include <options.hxx>
#include <rtl/ustring.hxx>
#include "treeload.hxx"

// -----------------------------------------------------------------------------
namespace configmgr
{
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;

#define ASCII(x) ::rtl::OUString::createFromAscii(x)

// -----------------------------------------------------------------------------
// ------------------------- requestSubtree without API -------------------------
// -----------------------------------------------------------------------------

    OTreeLoad::OTreeLoad(uno::Reference<lang::XMultiServiceFactory> const& _xServiceProvider,
                         rtl::OUString const& _sSourceDirectory, rtl::OUString const& _sUpdateDirectory) throw (uno::Exception)
            :m_xServiceProvider(_xServiceProvider)
{
    // Create a TypeConverter
    uno::Reference<script::XTypeConverter>      xConverter;
    xConverter = xConverter.query(m_xServiceProvider->createInstance(ASCII( "com.sun.star.script.Converter" )) );

    m_xDefaultOptions = new OOptions(xConverter);
    m_xDefaultOptions->setNoCache(true);

    // create it .. and connect
    std::auto_ptr<LocalSession> pLocal( new LocalSession(m_xServiceProvider) );
    sal_Bool bOpen = pLocal->open(_sSourceDirectory, _sUpdateDirectory);

    IConfigSession* pConfigSession = pLocal.release();

    m_pTreeMgr = new TreeManager(pConfigSession, m_xDefaultOptions);
}
// -----------------------------------------------------------------------------
ISubtree* OTreeLoad::requestSubtree( OUString const& aSubtreePath) throw (uno::Exception)
{
    return m_pTreeMgr->requestSubtree(aSubtreePath, m_xDefaultOptions, /* MinLevel */ -1);
}
// -----------------------------------------------------------------------------
void OTreeLoad::releaseSubtree( OUString const& aSubtreePath) throw (uno::Exception)
{
    m_pTreeMgr->releaseSubtree(aSubtreePath, m_xDefaultOptions);
}

// -----------------------------------------------------------------------------
} // namespace
