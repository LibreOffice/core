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

#include <svx/sdr/properties/e3dextrudeproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/extrud3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        E3dExtrudeProperties::E3dExtrudeProperties(SdrObject& rObj)
        :   E3dCompoundProperties(rObj)
        {
        }

        E3dExtrudeProperties::E3dExtrudeProperties(const E3dExtrudeProperties& rProps, SdrObject& rObj)
        :   E3dCompoundProperties(rProps, rObj)
        {
        }

        E3dExtrudeProperties::~E3dExtrudeProperties()
        {
        }

        BaseProperties& E3dExtrudeProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dExtrudeProperties(*this, rObj));
        }

        void E3dExtrudeProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            // call parent
            E3dCompoundProperties::PostItemChange(nWhich);

            // handle value change
            E3dExtrudeObj& rObj = (E3dExtrudeObj&)GetSdrObject();

            switch(nWhich)
            {
                case SDRATTR_3DOBJ_PERCENT_DIAGONAL:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_BACKSCALE:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_DEPTH:
                {
                    rObj.ActionChanged();
                    break;
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
