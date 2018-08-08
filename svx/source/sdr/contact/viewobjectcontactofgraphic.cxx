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

#include <sdr/contact/viewobjectcontactofgraphic.hxx>
#include <sdr/contact/viewcontactofgraphic.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>

#include <eventhandler.hxx>

namespace sdr
{
    namespace contact
    {
        drawinglayer::primitive2d::Primitive2DContainer ViewObjectContactOfGraphic::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            // get return value by calling parent
            drawinglayer::primitive2d::Primitive2DContainer xRetval = ViewObjectContactOfSdrObj::createPrimitive2DSequence(rDisplayInfo);

            if(!xRetval.empty())
            {
                // #i103255# suppress when graphic needs draft visualisation and output
                // is for PDF export/Printer
                const ViewContactOfGraphic& rVCOfGraphic = static_cast< const ViewContactOfGraphic& >(GetViewContact());

                if(rVCOfGraphic.visualisationUsesDraft())
                {
                    const ObjectContact& rObjectContact = GetObjectContact();

                    if(rObjectContact.isOutputToPDFFile() || rObjectContact.isOutputToPrinter())
                    {
                        xRetval = drawinglayer::primitive2d::Primitive2DContainer();
                    }
                }
            }

            return xRetval;
        }

        ViewObjectContactOfGraphic::ViewObjectContactOfGraphic(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfGraphic::~ViewObjectContactOfGraphic()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
