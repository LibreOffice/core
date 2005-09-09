/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: condedit.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:06:30 $
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
#ifndef _CONDEDTI_HXX
#define _CONDEDTI_HXX

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC ConditionEdit : public Edit, public DropTargetHelper
{
    BOOL bBrackets, bEnableDrop;

    SW_DLLPRIVATE virtual sal_Int8  AcceptDrop( const AcceptDropEvent& rEvt );
    SW_DLLPRIVATE virtual sal_Int8  ExecuteDrop( const ExecuteDropEvent& rEvt );

public:
    ConditionEdit( Window* pParent, const ResId& rResId );

    inline void ShowBrackets(BOOL bShow)        { bBrackets = bShow; }

    inline void SetDropEnable( BOOL bFlag )     { bEnableDrop = bFlag; }
    inline BOOL IsDropEnable() const            { return bEnableDrop; }
};

#endif
















