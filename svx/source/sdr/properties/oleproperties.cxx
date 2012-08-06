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


#include <svx/sdr/properties/oleproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        OleProperties::OleProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        OleProperties::OleProperties(const OleProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        OleProperties::~OleProperties()
        {
        }

        BaseProperties& OleProperties::Clone(SdrObject& rObj) const
        {
            return *(new OleProperties(*this, rObj));
        }

        void OleProperties::ForceDefaultAttributes()
        {
            // call parent
            RectangleProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            // #i108221#
            mpItemSet->Put( XFillStyleItem(XFILL_NONE) );
            mpItemSet->Put( XLineStyleItem(XLINE_NONE) );
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
