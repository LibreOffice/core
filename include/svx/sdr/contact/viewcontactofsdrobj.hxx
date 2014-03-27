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

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/svxdllapi.h>


// predeclarations

class SdrObject;
class GeoStat;
class Bitmap;



namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewContactOfSdrObj : public ViewContact
        {
        protected:
            // the owner of this ViewContact. Set from constructor and not
            // to be changed in any way.
            SdrObject&                                      mrObject;

            // Remember AnimationKind of object. Used to find out if that kind
            // has changed in ActionChanged()
            SdrTextAniKind                                  meRememberedAnimationKind;

            // Create a Object-Specific ViewObjectContact, set ViewContact and
            // ObjectContact. Always needs to return something.
            virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) SAL_OVERRIDE;

        public:
            // access to SdrObject
            SdrObject& GetSdrObject() const
            {
                return mrObject;
            }

            // basic constructor, used from SdrObject.
            explicit ViewContactOfSdrObj(SdrObject& rObj);
            virtual ~ViewContactOfSdrObj();

            // Access to possible sub-hierarchy
            virtual sal_uInt32 GetObjectCount() const SAL_OVERRIDE;
            virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const SAL_OVERRIDE;
            virtual ViewContact* GetParentContact() const SAL_OVERRIDE;

            // React on changes of the object of this ViewContact
            virtual void ActionChanged() SAL_OVERRIDE;

            // overload for acessing the SdrObject
            virtual SdrObject* TryToGetSdrObject() const SAL_OVERRIDE;


            // primitive stuff

            // add Gluepoints (if available)
            virtual drawinglayer::primitive2d::Primitive2DSequence createGluePointPrimitive2DSequence() const SAL_OVERRIDE;

            // allow embedding if needed (e.g. for SdrObjects, evtl. Name, Title and description get added). This
            // is a helper normally used from getViewIndependentPrimitive2DSequence(), but there is one exception
            // for 3D scenes
            virtual drawinglayer::primitive2d::Primitive2DSequence embedToObjectSpecificInformation(const drawinglayer::primitive2d::Primitive2DSequence& rSource) const SAL_OVERRIDE;
        };
    } // end of namespace contact
} // end of namespace sdr



#endif // INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
