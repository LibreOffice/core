/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#ifndef TABITEMDESCRIPTOR_HXX
#define TABITEMDESCRIPTOR_HXX

#include "svtools/toolpanel/toolpanel.hxx"
#include "svtools/toolpanel/tabitemcontent.hxx"

#include <tools/gen.hxx>
#include <osl/diagnose.h>

#include <vector>

//........................................................................
namespace svt
{
//........................................................................

    //==================================================================================================================
    //= ItemDescriptor
    //==================================================================================================================
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


//........................................................................
} // namespace svt
//........................................................................

#endif // TABITEMDESCRIPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
