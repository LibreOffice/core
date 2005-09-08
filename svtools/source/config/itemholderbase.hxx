/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemholderbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:41:25 $
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

#ifndef INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_

//-----------------------------------------------
// includes

#include <vector>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//-----------------------------------------------
// definitions

struct ItemHolderMutexBase
{
    ::osl::Mutex m_aLock;
};

enum EItem
{
    E_MODULEOPTIONS,
    E_ADDXMLTOSTORAGEOPTIONS,
    E_EXTENDEDSECURITYOPTIONS,
    E_FONTOPTIONS,
    E_HELPOPTIONS,
    E_HISTORYOPTIONS,
    E_INETOPTIONS,
    E_INTERNALOPTIONS,
    E_LOCALISATIONOPTIONS,
    E_MENUOPTIONS,
    E_MISCOPTIONS,
    E_SAVEOPTIONS,
    E_SECURITYOPTIONS,
    E_STARTOPTIONS,
    E_SYSLOCALEOPTIONS,
    E_UNDOOPTIONS,
    E_USEROPTIONS
};

struct TItemInfo
{
    TItemInfo()
        : pItem(0)
    {}

    void* pItem;
    EItem eItem;
};

typedef ::std::vector< TItemInfo > TItems;

#endif // INCLUDED_SVTOOLS_ITEMHOLDERBASE_HXX_
