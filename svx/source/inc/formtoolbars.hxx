/*************************************************************************
 *
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

#ifndef SVX_SOURCE_INC_FORMTOOLBARS_HXX
#define SVX_SOURCE_INC_FORMTOOLBARS_HXX

#include "fmdocumentclassification.hxx"
#include <com/sun/star/frame/XLayoutManager.hpp>

#include <tools/solar.h>

#include <svx/svxdllapi.h>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= FormToolboxes
    //====================================================================
    class FormToolboxes
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >
                        m_xLayouter;

    public:
        /** constructs an instance
            @param _rxFrame
                the frame to analyze
        */
        SVX_DLLPUBLIC FormToolboxes(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
        );

    public:
        /** retrieves the URI for the toolbox associated with the given slot, depending
            on the type of our document
        */
        SVX_DLLPUBLIC ::rtl::OUString
                getToolboxResourceName( sal_uInt16 _nSlotId ) const;

        /** toggles the toolbox associated with the given slot
        */
        SVX_DLLPUBLIC void    toggleToolbox( sal_uInt16 _nSlotId ) const;

        /** determines whether the toolbox associated with the given slot is currently visible
        */
        SVX_DLLPUBLIC bool    isToolboxVisible( sal_uInt16 _nSlotId ) const;

        /** ensures that a given toolbox is visible
        */
        inline void showToolbox( sal_uInt16 _SlotId ) const
        {
            if ( !isToolboxVisible( _SlotId ) )
                toggleToolbox( _SlotId );
        }
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FORMTOOLBARS_HXX

