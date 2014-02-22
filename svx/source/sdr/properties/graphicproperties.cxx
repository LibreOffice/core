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

#include <svx/sdr/properties/graphicproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>



namespace sdr
{
    namespace properties
    {
        
        SfxItemSet& GraphicProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                
                SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,

                
                EE_ITEMS_START, EE_ITEMS_END,

                
                0, 0));
        }

        GraphicProperties::GraphicProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        GraphicProperties::GraphicProperties(const GraphicProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        GraphicProperties::~GraphicProperties()
        {
        }

        BaseProperties& GraphicProperties::Clone(SdrObject& rObj) const
        {
            return *(new GraphicProperties(*this, rObj));
        }

        void GraphicProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrGrafObj& rObj = (SdrGrafObj&)GetSdrObject();

            
            rObj.SetXPolyDirty();

            
            
            
            
            
            
            rObj.ImpSetAttrToGrafInfo();

            
            RectangleProperties::ItemSetChanged(rSet);
        }

        void GraphicProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrGrafObj& rObj = (SdrGrafObj&)GetSdrObject();

            
            rObj.SetXPolyDirty();

            
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            
            rObj.ImpSetAttrToGrafInfo();
        }

        void GraphicProperties::ForceDefaultAttributes()
        {
            
            RectangleProperties::ForceDefaultAttributes();

            
            GetObjectItemSet();

            mpItemSet->Put( SdrGrafLuminanceItem( 0 ) );
            mpItemSet->Put( SdrGrafContrastItem( 0 ) );
            mpItemSet->Put( SdrGrafRedItem( 0 ) );
            mpItemSet->Put( SdrGrafGreenItem( 0 ) );
            mpItemSet->Put( SdrGrafBlueItem( 0 ) );
            mpItemSet->Put( SdrGrafGamma100Item( 100 ) );
            mpItemSet->Put( SdrGrafTransparenceItem( 0 ) );
            mpItemSet->Put( SdrGrafInvertItem( sal_False ) );
            mpItemSet->Put( SdrGrafModeItem( GRAPHICDRAWMODE_STANDARD ) );
            mpItemSet->Put( SdrGrafCropItem( 0, 0, 0, 0 ) );
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
