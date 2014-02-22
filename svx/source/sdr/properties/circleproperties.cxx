/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/properties/circleproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdocirc.hxx>
#include <svx/sxcikitm.hxx>
#include <svx/sxciaitm.hxx>



namespace sdr
{
    namespace properties
    {
        
        SfxItemSet& CircleProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                
                SDRATTR_CIRC_FIRST, SDRATTR_CIRC_LAST,

                
                EE_ITEMS_START, EE_ITEMS_END,

                
                0, 0));
        }

        CircleProperties::CircleProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        CircleProperties::CircleProperties(const CircleProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        CircleProperties::~CircleProperties()
        {
        }

        BaseProperties& CircleProperties::Clone(SdrObject& rObj) const
        {
            return *(new CircleProperties(*this, rObj));
        }

        void CircleProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrCircObj& rObj = (SdrCircObj&)GetSdrObject();

            
            RectangleProperties::ItemSetChanged(rSet);

            
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrCircObj& rObj = (SdrCircObj&)GetSdrObject();

            
            rObj.SetXPolyDirty();

            
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::ForceDefaultAttributes()
        {
            SdrCircObj& rObj = (SdrCircObj&)GetSdrObject();
            SdrCircKind eKindA = SDRCIRC_FULL;
            SdrObjKind eKind = rObj.GetCircleKind();

            if(eKind == OBJ_SECT)
            {
                eKindA = SDRCIRC_SECT;
            }
            else if(eKind == OBJ_CARC)
            {
                eKindA = SDRCIRC_ARC;
            }
            else if(eKind == OBJ_CCUT)
            {
                eKindA = SDRCIRC_CUT;
            }

            if(eKindA != SDRCIRC_FULL)
            {
                
                GetObjectItemSet();

                mpItemSet->Put(SdrCircKindItem(eKindA));

                if(rObj.GetStartWink())
                {
                    mpItemSet->Put(SdrCircStartAngleItem(rObj.GetStartWink()));
                }

                if(rObj.GetEndWink() != 36000)
                {
                    mpItemSet->Put(SdrCircEndAngleItem(rObj.GetEndWink()));
                }
            }

            
            
            
            
            RectangleProperties::ForceDefaultAttributes();
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
