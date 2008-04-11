/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofvirtobj.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <svx/svdovirt.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        SdrVirtObj& ViewContactOfVirtObj::GetVirtObj() const
        {
            return (SdrVirtObj&)mrObject;
        }

        ViewContactOfVirtObj::ViewContactOfVirtObj(SdrVirtObj& rObj)
        :   ViewContactOfSdrObj(rObj)
        {
        }

        ViewContactOfVirtObj::~ViewContactOfVirtObj()
        {
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfVirtObj::GetObjectCount() const
        {
            // Here, SdrVirtObj's need to return 0L to show that they have no
            // sub-hierarchy, even when they are group objects. This is necessary
            // to avoid that the same VOCs will be added to the draw hierarchy
            // twice which leads to problems.
            // This solution is only a first solution to get things running. Later
            // this needs to be replaced with creating real VOCs for the objects
            // referenced by virtual objects to avoid the 'trick' of setting the
            // offset for painting at the destination OutputDevive.
            return 0L;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
