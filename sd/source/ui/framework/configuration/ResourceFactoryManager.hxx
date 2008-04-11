/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResourceFactoryManager.hxx,v $
 *
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
