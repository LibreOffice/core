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

#ifndef _SDR_OBJECTUSER_HXX
#define _SDR_OBJECTUSER_HXX

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

////////////////////////////////////////////////////////////////////////////////////////////////////
// To make things more safe, allow users of an object to register at it. The users need to be derived
// from sdr::ObjectUser to get a call. The users do not need to call RemoveObjectUser() at the page
// when they get called from ObjectInDestruction().

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
