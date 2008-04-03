/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceFactoryManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:31:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_FRAMEWORK_RESOURCE_FACTORY_MANAGER_HXX
#define SD_FRAMEWORK_RESOURCE_FACTORY_MANAGER_HXX

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XModuleController.hpp>
#include <com/sun/star/drawing/framework/XResourceFactoryManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>
#include <hash_map>

namespace css = ::com::sun::star;

namespace sd { namespace framework {

/** Container of resource factories of the drawing framework.
*/
class ResourceFactoryManager
{
public:
    ResourceFactoryManager (
        const css::uno::Reference<css::drawing::framework::XControllerManager>& rxManager);

    ~ResourceFactoryManager (void);

    /** Register a resource factory for one type of resource.
        @param rsURL
            The type of the resource that will be created by the factory.
        @param rxFactory
            The factory that will create resource objects of the specfied type.
    */
    void AddFactory (
        const ::rtl::OUString& rsURL,
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxFactory)
        throw (css::uno::RuntimeException);

    /** Unregister the specifed factory.
        @param rsURL
            Unregister only the factory for this URL.  When the same factory
            is registered for other URLs then these remain registered.
    */
    void RemoveFactoryForURL(
        const ::rtl::OUString& rsURL)
        throw (css::uno::RuntimeException);

    /** Unregister the specified factory.
        @param rxFactory
            Unregister the this factory for all URLs that it has been
            registered for.
    */
    void RemoveFactoryForReference(
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxFactory)
        throw (css::uno::RuntimeException);

    /** Return a factory that can create resources specified by the given URL.
        @param rsCompleteURL
            This URL specifies the type of the resource.  It may contain arguments.
        @return
            When a factory for the specified URL has been registered by a
            previous call to AddFactory() then a reference to that factory
            is returned.  Otherwise an empty reference is returned.
    */
    css::uno::Reference<css::drawing::framework::XResourceFactory> GetFactory (
        const ::rtl::OUString& rsURL)
        throw (css::uno::RuntimeException);

private:
    ::osl::Mutex maMutex;
    typedef ::std::hash_map<
        ::rtl::OUString,
        css::uno::Reference<css::drawing::framework::XResourceFactory>,
        ::comphelper::UStringHash,
        ::comphelper::UStringEqual> FactoryMap;
    FactoryMap maFactoryMap;

    typedef ::std::vector<
        ::std::pair<
            rtl::OUString,
            css::uno::Reference<css::drawing::framework::XResourceFactory> > >
        FactoryPatternList;
    FactoryPatternList maFactoryPatternList;

    css::uno::Reference<css::drawing::framework::XControllerManager> mxControllerManager;
    css::uno::Reference<css::util::XURLTransformer> mxURLTransformer;

    /** Look up the factory for the given URL.
        @param rsURLBase
            The css::tools::URL.Main part of a URL. Arguments have to be
            stripped off by the caller.
        @return
            When the factory has not yet been added then return NULL.
    */
    css::uno::Reference<css::drawing::framework::XResourceFactory> FindFactory (
        const ::rtl::OUString& rsURLBase)
        throw (css::uno::RuntimeException);
};


} } // end of namespace sd::framework

#endif
