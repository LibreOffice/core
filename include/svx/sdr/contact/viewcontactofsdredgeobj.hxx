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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDREDGEOBJ_HXX
#define _SDR_CONTACT_VIEWCONTACTOFSDREDGEOBJ_HXX

#include <svx/sdr/contact/viewcontactoftextobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrEdgeObj;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ViewContactOfSdrEdgeObj : public ViewContactOfTextObj
        {
        protected:
            // internal access to SdrEdgeObj
            SdrEdgeObj& GetEdgeObj() const
            {
                return (SdrEdgeObj&)GetSdrObject();
            }

        public:
            // basic constructor, used from SdrObject.
            explicit ViewContactOfSdrEdgeObj(SdrEdgeObj& rEdgeObj);
            virtual ~ViewContactOfSdrEdgeObj();

        protected:
            // This method is responsible for creating the graphical visualisation data
            // ONLY based on model data
            virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWCONTACTOFSDREDGEOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
