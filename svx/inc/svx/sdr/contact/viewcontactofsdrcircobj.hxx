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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRCIRCOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDRCIRCOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrCircObj;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrCircObj : public ViewContactOfSdrRectObj
        {
        protected:
            // internal access to SdrCircObj
            SdrCircObj& GetCircObj() const
            {
                return (SdrCircObj&)GetSdrObject();
            }

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfSdrCircObj(SdrCircObj& rCircObj);
            virtual ~ViewContactOfSdrCircObj();

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDRCIRCOBJ_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
