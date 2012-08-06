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

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#define _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SfxItemSet;
class Bitmap;
class MapMode;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfMasterPageDescriptor : public ViewContact
        {
        protected:
            // the owner of this ViewContact. Set from constructor and not
            // to be changed in any way.
            sdr::MasterPageDescriptor&                      mrMasterPageDescriptor;

            // Create a Object-Specific ViewObjectContact
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact);

            // This method is responsible for creating the graphical visualisation data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;

        public:
            // basic constructor
            explicit ViewContactOfMasterPageDescriptor(sdr::MasterPageDescriptor& rDescriptor);

            // The destructor.
            virtual ~ViewContactOfMasterPageDescriptor();

            // access to MasterPageDescriptor
            sdr::MasterPageDescriptor& GetMasterPageDescriptor() const
            {
                return mrMasterPageDescriptor;
            }

            // Access to possible sub-hierarchy and parent
            virtual sal_uInt32 GetObjectCount() const;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const;
            virtual ViewContact* GetParentContact() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
