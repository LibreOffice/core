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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTREDIRECTOR_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTREDIRECTOR_HXX

#include "svx/svxdllapi.h"
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr { namespace contact {
    class DisplayInfo;
    class ViewObjectContact;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // This class provides a mechanism to redirect the paint mechanism for all or
        // single ViewObjectContacts. An own derivation may be set at single ViewContacts
        // or at the ObjectContact for redirecting all. If both is used, the one at single
        // objects will have priority.
        class SVX_DLLPUBLIC ViewObjectContactRedirector
        {
        public:
            // basic constructor.
            ViewObjectContactRedirector();

            // The destructor.
            virtual ~ViewObjectContactRedirector();

            // all default implementations just call the same methods at the original. To do something
            // different, overload the method and at least do what the method does.
            virtual drawinglayer::primitive2d::Primitive2DSequence createRedirectedPrimitive2DSequence(
                const sdr::contact::ViewObjectContact& rOriginal,
                const sdr::contact::DisplayInfo& rDisplayInfo);
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTREDIRECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
