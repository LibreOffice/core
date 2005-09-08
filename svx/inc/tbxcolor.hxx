/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxcolor.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:18:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_TBXCOLOR_HXX
#define _SVX_TBXCOLOR_HXX

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

