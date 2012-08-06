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

#ifndef _SDR_OBJECTUSER_HXX
#define _SDR_OBJECTUSER_HXX

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

////////////////////////////////////////////////////////////////////////////////////////////////////
// To make things more safe, allow users of the page to register at it. The users need to be derived
// from sdr::PageUser to get a call. The users do not need to call RemovePageUser() at the page
// when they get called from PageInDestruction().

namespace sdr
{
    class ObjectUser
    {
    public:
        // this method is called form the destructor of the referenced page.
        // do all necessary action to forget the page. It is not necessary to call
        // RemovePageUser(), that is done form the destructor.
        virtual void ObjectInDestruction(const SdrObject& rObject) = 0;

    protected:
        ~ObjectUser() {}
    };

    // typedef for ObjectUserVector
    typedef ::std::vector< ObjectUser* > ObjectUserVector;
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OBJECTUSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
