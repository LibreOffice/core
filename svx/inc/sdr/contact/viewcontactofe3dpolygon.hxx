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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFE3DPOLYGON_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFE3DPOLYGON_HXX

#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <svx/polygn3d.hxx>

namespace sdr
{
    namespace contact
    {
        class ViewContactOfE3dPolygon : public ViewContactOfE3d
        {
        public:
            // basic constructor, used from SdrObject.
            explicit ViewContactOfE3dPolygon(E3dPolygonObj& rPolygon);
            virtual ~ViewContactOfE3dPolygon();

            // access to SdrObject
            const E3dPolygonObj& GetE3dPolygonObj() const
            {
                return static_cast<const E3dPolygonObj&>(GetE3dObject());
            }

        protected:
            // This method is responsible for creating the graphical visualisation data which is
            // stored in the local primitive list. Default creates a yellow replacement rectangle.
            virtual drawinglayer::primitive3d::Primitive3DContainer createViewIndependentPrimitive3DContainer() const override;
        };
    } // end of namespace contact
} // end of namespace sdr


#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFE3DPOLYGON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
