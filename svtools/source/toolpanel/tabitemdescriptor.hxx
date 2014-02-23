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

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TABITEMDESCRIPTOR_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TABITEMDESCRIPTOR_HXX

#include "svtools/toolpanel/toolpanel.hxx"
#include "svtools/toolpanel/tabitemcontent.hxx"

#include <tools/gen.hxx>
#include <osl/diagnose.h>

#include <vector>


namespace svt
{



    //= ItemDescriptor

    struct ItemDescriptor
    {
        PToolPanel      pPanel;
        Rectangle       aCompleteArea;  // bounding area if the both text and icon are to be rendererd
        Rectangle       aIconOnlyArea;  // bounding area if the icon is to be rendererd
        Rectangle       aTextOnlyArea;  // bounding area if the text is to be rendererd
        TabItemContent  eContent;
            // content to be used for this particular item. Might differ from item content which has been set
            // up for the complete control, in case not the complete content fits into the available space.

        ItemDescriptor()
            :pPanel()
            ,aCompleteArea()
            ,aIconOnlyArea()
            ,aTextOnlyArea()
            ,eContent( TABITEM_IMAGE_AND_TEXT )
        {
        }

        const Rectangle& GetRect( const TabItemContent i_eItemContent ) const
        {
            OSL_ENSURE( i_eItemContent != TABITEM_AUTO, "ItemDescriptor::GetRect: illegal value!" );

            return  ( i_eItemContent == TABITEM_IMAGE_AND_TEXT )
                ?   aCompleteArea
                :   (   ( i_eItemContent == TABITEM_TEXT_ONLY )
                    ?   aTextOnlyArea
                    :   aIconOnlyArea
                    );
        }

        const Rectangle& GetCurrentRect() const
        {
            return GetRect( eContent );
        }
    };

    typedef ::std::vector< ItemDescriptor > ItemDescriptors;



} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TABITEMDESCRIPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
