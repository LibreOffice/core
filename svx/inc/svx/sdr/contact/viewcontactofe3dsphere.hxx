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

#ifndef _SDR_CONTACT_VIEWCONTACTOFE3DSPHERE_HXX
#define _SDR_CONTACT_VIEWCONTACTOFE3DSPHERE_HXX

#include <svx/sdr/contact/viewcontactofe3d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class E3dSphereObj;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfE3dSphere : public ViewContactOfE3d
        {
        public:
            // basic constructor, used from SdrObject.
            ViewContactOfE3dSphere(E3dSphereObj& rSphere);
            virtual ~ViewContactOfE3dSphere();

            // access to SdrObject
            E3dSphereObj& GetE3dSphereObj() const
            {
                return (E3dSphereObj&)GetE3dObject();
            }

        protected:
            // This method is responsible for creating the graphical visualisation data which is
            // stored in the local primitive list. Default creates a yellow replacement rectangle.
            virtual drawinglayer::primitive3d::Primitive3DSequence createViewIndependentPrimitive3DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFE3DSPHERE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
