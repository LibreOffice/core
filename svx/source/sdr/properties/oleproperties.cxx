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

#include <sdr/properties/oleproperties.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>

namespace sdr::properties
{
        void OleProperties::applyDefaultStyleSheetFromSdrModel()
        {
            SfxStyleSheet* pStyleSheet(GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj());

            if(pStyleSheet)
            {
                // do not delete hard attributes when setting dsefault Style
                SetStyleSheet(pStyleSheet, true);
            }
            else
            {
                SetMergedItem(XFillStyleItem(com::sun::star::drawing::FillStyle_NONE));
                SetMergedItem(XLineStyleItem(com::sun::star::drawing::LineStyle_NONE));
            }
        }

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

        std::unique_ptr<BaseProperties> OleProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new OleProperties(*this, rObj));
        }

        void OleProperties::ForceDefaultAttributes()
        {
            // call parent
            RectangleProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
