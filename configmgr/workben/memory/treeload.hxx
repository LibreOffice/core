#ifndef CONFIGMGR_TREELOAD_HXX
#define CONFIGMGR_TREELOAD_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _CONFIGMGR_LOCAL_LOCAL_HXX_
#include <localsession.hxx>
#endif

#ifndef _CONFIGMGR_TREECACHE_HXX_
#include <treecache.hxx>
#endif

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include <options.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
