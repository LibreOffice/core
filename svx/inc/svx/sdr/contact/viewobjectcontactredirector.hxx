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

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
