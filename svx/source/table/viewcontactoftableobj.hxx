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

#ifndef INCLUDED_SVX_SOURCE_TABLE_VIEWCONTACTOFTABLEOBJ_HXX
#define INCLUDED_SVX_SOURCE_TABLE_VIEWCONTACTOFTABLEOBJ_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/svdotable.hxx>

namespace sdr
{
    namespace contact
    {
        class ViewContactOfTableObj : public ViewContactOfSdrObj
        {
        protected:
            // internal access to SdrTextObj
            const sdr::table::SdrTableObj& GetTableObj() const
            {
                return static_cast<const sdr::table::SdrTableObj&>(GetSdrObject());
            }

            // This method is responsible for creating the graphical visualisation data derived ONLY from
            // the model data
            virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

        public:
            // basic constructor, used from SdrObject.
            explicit ViewContactOfTableObj(sdr::table::SdrTableObj& rTextObj);
            virtual ~ViewContactOfTableObj();
        };
    } // end of namespace contact
} // end of namespace sdr

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
