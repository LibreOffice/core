/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swuiexp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 07:15:56 $
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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#include "swdlgfact.hxx"

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

namespace swui
{
    static SwAbstractDialogFactory_Impl* pFactory=NULL;
    SwAbstractDialogFactory * GetFactory()
    {
        if ( !pFactory )
            pFactory = new SwAbstractDialogFactory_Impl;
        if ( !pSwResMgr)
            SwDialogsResMgr::GetResMgr();
        return pFactory;
    }
};

extern "C"
{
    SAL_DLLPUBLIC_EXPORT SwAbstractDialogFactory* CreateDialogFactory()
    {
        return ::swui::GetFactory();
    }
}
