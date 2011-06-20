/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SD_GLOBAL_RESOURCE_CONTAINER_HXX
#define SD_GLOBAL_RESOURCE_CONTAINER_HXX

#include "sdmod.hxx"
#include <memory>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/XInterface.hpp>

namespace css = ::com::sun::star;

namespace sd {

class SdGlobalResource
{
public:
    virtual ~SdGlobalResource (void) {};
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
class SdGlobalResourceContainer
{
public:
    static SdGlobalResourceContainer& Instance (void);

    /** Add a resource to the container.  The ownership of the resource is
        transferred to the container.  The resource is destroyed when the
        container is destroyed, i.e. when the sd module is destroyed.

        When in doubt, use the shared_ptr variant of this method.
    */
    void AddResource (::std::auto_ptr<SdGlobalResource> pResource);

    /** Add a resource to the container.  By using a shared_ptr and
        releasing it only when the SgGlobalResourceContainer is destroyed
        the given resource is kept alive at least that long.  When at the
        time of the destruction of SgGlobalResourceContainer no other
        references exist the resource is destroyed as well.
    */
    void AddResource (::boost::shared_ptr<SdGlobalResource> pResource);

    /** Add a resource that is implemented as UNO object.  Destruction
        (when the sd modules is unloaded) is done by a) calling dispose()
        when the XComponent is supported and by b) releasing the reference.
    */
    void AddResource (const ::css::uno::Reference<css::uno::XInterface>& rxResource);

protected:
    friend class ::SdModule;
    friend class ::std::auto_ptr<SdGlobalResourceContainer>;

    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    SdGlobalResourceContainer (void);
    ~SdGlobalResourceContainer (void);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
