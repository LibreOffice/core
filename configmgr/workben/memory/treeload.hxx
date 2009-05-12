#ifndef CONFIGMGR_TREELOAD_HXX
#define CONFIGMGR_TREELOAD_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _CONFIGMGR_LOCAL_LOCAL_HXX_
#include <localsession.hxx>
#endif

#ifndef _CONFIGMGR_TREECACHE_HXX_
#include <treecache.hxx>
#endif
#include <options.hxx>
#include <rtl/ustring.hxx>

// -----------------------------------------------------------------------------
namespace configmgr
{
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;

// -----------------------------------------------------------------------------
class OTreeLoad
{
    // LocalSession* m_pLocalSession; // in TreeMgr
    TreeManager* m_pTreeMgr;
    ::vos::ORef<OOptions>       m_xDefaultOptions;
    uno::Reference<lang::XMultiServiceFactory> m_xServiceProvider;

public:

    OTreeLoad(uno::Reference<lang::XMultiServiceFactory> const& _xServiceProvider,
             rtl::OUString const& _sSourceDirectory, rtl::OUString const& _sUpdateDirectory);

    ISubtree* requestSubtree(rtl::OUString const& aSubtreePath);
    void releaseSubtree(rtl::OUString const& aSubtreePath);
};

} // namespace

#endif
