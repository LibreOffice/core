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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrOle2Obj;
class Graphic;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrOle2Obj : public ViewContactOfSdrRectObj
        {
        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

        public:
            // access to SdrOle2Obj
            SdrOle2Obj& GetOle2Obj() const
            {
                return (SdrOle2Obj&)GetSdrObject();
            }

            // basic constructor, used from SdrObject.
            explicit ViewContactOfSdrOle2Obj(SdrOle2Obj& rOle2Obj);
            virtual ~ViewContactOfSdrOle2Obj();

            // helper for creating a OLE sequence for this object. It takes care od attributes, needed
            // scaling (e.g. for EmptyPresObj's), the correct graphic and other stuff. It is used from
            // createViewIndependentPrimitive2DSequence with false, and with evtl. HighContrast true
            // from the VOC which knows that
            drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequenceWithParameters() const;

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data, just wraps to call createPrimitive2DSequenceWithParameters(false)
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDROLE2OBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
