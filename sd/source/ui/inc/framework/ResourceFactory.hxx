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

#pragma once

#include <com/sun/star/drawing/framework/XResource.hpp>
#include <comphelper/compbase.hxx>
#include <sddllapi.h>

namespace sd::framework
{
/** Factory and possibly cache for creating and releasing resources.
    <p>A resource factory is created and used by the
    XConfigurationController object.</p>
    <p>A factory may want to implement a cache to reuse previously released
    resources.</p>
*/
class SD_DLLPUBLIC ResourceFactory : public comphelper::WeakComponentImplHelper<>
{
public:
    /** Create a resource for the given XResourceId object.
        @param xResourceId
            The resource URL of this id specifies the type of resource to
            create.  The anchor can be used to obtain the associated object
            from the configuration controller.  For example, when a view is
            created, then the new object can be initialized with the pane
            that is its anchor.
            <p>Valid values are those for which the factory has previously been
            registered at the XConfigurationController</p>.
        @return
            Returns a resource object that has been just created or was
            taken from a cache.  When the requested resource can not be
            created then an empty reference is returned.  A later call may
            be successful.  This, for example, can be the case with SFX
            based side panes.  They are not available right after the
            creation of a new application frame.
        @throws InvalidArgumentException
            when the given URL is not supported by the factory.
    */
    virtual ::css::uno::Reference<::css::drawing::framework::XResource>
    createResource(const ::css::uno::Reference<::css::drawing::framework::XResourceId>& xResourceId)
        = 0;

    /** Call this method to tell a factory that the given resource is no
        longer in use.  The factory can decide whether to destroy the
        resource or to keep it in a cache in order to reuse it later.
        @param xResource
            The given resource has to be one created by the same factory.
        @throws InvalidArgumentException
            when the given pane was not created by the same factory.
    */
    virtual void
    releaseResource(const ::css::uno::Reference<::css::drawing::framework::XResource>& xResource)
        = 0;

protected:
    virtual ~ResourceFactory() override;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
