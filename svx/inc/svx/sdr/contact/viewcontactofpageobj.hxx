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

#ifndef _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPageObj;
class SdrPage;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfPageObj : public ViewContactOfSdrObj
        {
        protected:
            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // create graphical visualisation data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        public:
            // basic constructor, used from SdrObject.
            explicit ViewContactOfPageObj(SdrPageObj& rPageObj);
            virtual ~ViewContactOfPageObj();

            // #WIP# React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // access to SdrObject
            SdrPageObj& GetPageObj() const
            {
                return (SdrPageObj&)GetSdrObject();
            }
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
