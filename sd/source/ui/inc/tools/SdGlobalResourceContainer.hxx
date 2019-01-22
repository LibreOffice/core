/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_TOOLS_SDGLOBALRESOURCECONTAINER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TOOLS_SDGLOBALRESOURCECONTAINER_HXX

#include <memory>
#include <sal/types.h>
#include <o3tl/deleter.hxx>

namespace com { namespace sun { namespace star { namespace uno { template <class interface_type> class Reference; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XInterface; } } } }

namespace sd {

class SdGlobalResource
{
public:
    virtual ~SdGlobalResource() COVERITY_NOEXCEPT_FALSE {};
};

/** The purpose of this container is to hold references to resources that
    are globally available to all interested objects and to destroy them
    when the sd module is destroyed.  Examples for resources can be
    containers of bitmaps or the container of master pages used by the
    MasterPagesSelector objects in the task panel.

    It works like a singleton in that there is one instance per sd module.
    Resources can be added (by themselves or their owners) to the
    container.  The main task of the container is the destruction of all
    resources that have been added to it.

    As you may note, there is no method to get a resource from the
    container.  It is the task of the resource to provide other means of
    access to it.

    The reason for this design is not to have to change the SdModule
    destructor every time when there is a new resource.  This is done by
    reversing the dependency between module and resource: the resource knows
    about the module--this container class to be more precisely--and tells
    it to destroy the resource when the sd module is at the end of its
    lifetime.
*/
class SdGlobalResourceContainer final
{
public:
    static SdGlobalResourceContainer& Instance();

    /** Add a resource to the container.  The ownership of the resource is
        transferred to the container.  The resource is destroyed when the
        container is destroyed, i.e. when the sd module is destroyed.

        When in doubt, use the shared_ptr variant of this method.
    */
    void AddResource (::std::unique_ptr<SdGlobalResource> pResource);

    /** Add a resource to the container.  By using a shared_ptr and
        releasing it only when the SgGlobalResourceContainer is destroyed
        the given resource is kept alive at least that long.  When at the
        time of the destruction of SgGlobalResourceContainer no other
        references exist the resource is destroyed as well.
    */
    void AddResource (const std::shared_ptr<SdGlobalResource>& pResource);

    /** Add a resource that is implemented as UNO object.  Destruction
        (when the sd modules is unloaded) is done by a) calling dispose()
        when the XComponent is supported and by b) releasing the reference.
    */
    void AddResource (const css::uno::Reference<css::uno::XInterface>& rxResource);

private:
    friend class SdGlobalResourceContainerInstance;
    friend struct o3tl::default_delete<SdGlobalResourceContainer>;

    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    SdGlobalResourceContainer();
    ~SdGlobalResourceContainer();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
