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

#include <memory>
#include <tools/SdGlobalResourceContainer.hxx>
#include <vcl/image.hxx>

namespace sd
{
/** This simple class stores frequently used icons so that the classes that
    use the icons do not have to store them in every one of their
    instances.

    Icons are addressed over their resource id and are loaded on demand.

    This cache acts like a singleton with a lifetime equal to that of the sd
    module.
*/
class IconCache : public SdGlobalResource
{
public:
    /** The lifetime of the returned reference is limited to that of the sd
        module.
    */
    static IconCache& Instance();

    /** Return the icon with the given resource id.
        @return
            The returned Image may be empty when there is no icon for the
            given id or an error occurred.  Should not happen under normal
            circumstances.
    */
    Image GetIcon(const OUString& rResourceId);

private:
    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    /** The constructor creates the one instance of the cache and registers
        it at the SdGlobalResourceContainer to limit is lifetime to that of
        the sd module.
    */
    IconCache();

    /** This destructor is called by SdGlobalResourceContainer.
    */
    virtual ~IconCache() override;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
