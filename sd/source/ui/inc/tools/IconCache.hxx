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

#ifndef SD_ICON_CACHE_HXX
#define SD_ICON_CACHE_HXX

#include "SdGlobalResourceContainer.hxx"
#include <vcl/image.hxx>

namespace sd {

/** This simple class stores frequently used icons so that the classes that
    use the icons do not have to store them in every one of their
    instances.

    Icons are adressed over their resource id and are loaded on demand.

    This cache acts like a singleton with a lifetime equal to that of the sd
    module.
*/
class IconCache
    : public SdGlobalResource
{
public:
    /** The lifetime of the returned reference is limited to that of the sd
        module.
    */
    static IconCache& Instance (void);

    /** Return the icon with the given resource id.
        @return
            The returned Image may be empty when there is no icon for the
            given id or an error occurred.  Should not happen under normal
            circumstances.
    */
    Image GetIcon (sal_uInt16 nResourceId);

private:
    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    /** The constructor creates the one instance of the cache and registers
        it at the SdGlobalResourceContainer to limit is lifetime to that of
        the sd module.
    */
    IconCache (void);

    /** This destructor is called by SdGlobalResourceContainer.
    */
    virtual ~IconCache (void);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
