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
#ifndef _SVX_TBXCOLOR_HXX
#define _SVX_TBXCOLOR_HXX

#include <com/sun/star/frame/XLayoutManager.hpp>
#include "svx/svxdllapi.h"

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= ToolboxAccess
    //====================================================================
    class SVX_DLLPUBLIC ToolboxAccess
    {
    private:
        bool                                                    m_bDocking;
        ::rtl::OUString                                         m_sToolboxResName;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XLayoutManager >   m_xLayouter;

    public:
        ToolboxAccess( const ::rtl::OUString& rToolboxName );

    public:
        /** toggles the toolbox
        */
        void        toggleToolbox() const;

        /** determines whether the toolbox is currently visible
        */
        bool        isToolboxVisible() const;

        /** forces that the toolbox is docked
        */
        inline void forceDocking() { m_bDocking = true; }
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // #ifndef _SVX_TBXCOLOR_HXX

