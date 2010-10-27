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

#ifndef _SDR_CONTACT_VIEWCONTACTOFTABLEOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFTABLEOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace table
    {
        class SdrTableObj;
    }

    namespace contact
    {
        class ViewContactOfTableObj : public ViewContactOfSdrObj
        {
        protected:
            // internal access to SdrTextObj
            ::sdr::table::SdrTableObj& GetTableObj() const
            {
                return (::sdr::table::SdrTableObj&)GetSdrObject();
            }

            // This method is responsible for creating the graphical visualisation data derived ONLY from
            // the model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            // basic constructor, used from SdrObject.
            ViewContactOfTableObj(::sdr::table::SdrTableObj& rTextObj);
            virtual ~ViewContactOfTableObj();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
