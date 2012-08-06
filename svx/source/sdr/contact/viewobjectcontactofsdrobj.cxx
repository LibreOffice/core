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


#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>

#include "fmobj.hxx"

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        const SdrObject& ViewObjectContactOfSdrObj::getSdrObject() const
        {
            return static_cast< ViewContactOfSdrObj& >(GetViewContact()).GetSdrObject();
        }

        ViewObjectContactOfSdrObj::ViewObjectContactOfSdrObj(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContact(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfSdrObj::~ViewObjectContactOfSdrObj()
        {
        }

        bool ViewObjectContactOfSdrObj::isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const
        {
            const SdrObject& rObject = getSdrObject();

            // Test layer visibility
            if(!rDisplayInfo.GetProcessLayers().IsSet(rObject.GetLayer()))
            {
                return false;
            }

            if(GetObjectContact().isOutputToPrinter() )
            {
                // Test if print output but not printable
                if( !rObject.IsPrintable())
                    return false;
            }
            else
            {
                // test is object is not visible on screen
                if( !rObject.IsVisible() )
                    return false;
            }

            // Test for hidden object on MasterPage
            if(rDisplayInfo.GetSubContentActive() && rObject.IsNotVisibleAsMaster())
            {
                return false;
            }

            // Test for Calc object hiding (for OLE and Graphic it's extra, see there)
            const SdrPageView* pSdrPageView = GetObjectContact().TryToGetSdrPageView();

            if(pSdrPageView)
            {
                const SdrView& rSdrView = pSdrPageView->GetView();
                const bool bHideOle(rSdrView.getHideOle());
                const bool bHideChart(rSdrView.getHideChart());
                const bool bHideDraw(rSdrView.getHideDraw());
                const bool bHideFormControl(rSdrView.getHideFormControl());

                if(bHideOle || bHideChart || bHideDraw || bHideFormControl)
                {
                    if(OBJ_OLE2 == rObject.GetObjIdentifier())
                    {
                        if(((SdrOle2Obj&)rObject).IsChart())
                        {
                            // chart
                            if(bHideChart)
                            {
                                return false;
                            }
                        }
                        else
                        {
                            // OLE
                            if(bHideOle)
                            {
                                return false;
                            }
                        }
                    }
                    else if(OBJ_GRAF == rObject.GetObjIdentifier())
                    {
                        // graphic handled like OLE
                        if(bHideOle)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        const bool bIsFormControl = dynamic_cast< const FmFormObj * >( &rObject ) != 0;
                        if(bIsFormControl && bHideFormControl)
                        {
                            return false;
                        }
                        // any other draw object
                        if(!bIsFormControl && bHideDraw)
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
