/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ActionDescriptionProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:14:22 $
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

#ifndef _SVX_ACTIONDESCRIPTIONPROVIDER_HXX
#define _SVX_ACTIONDESCRIPTIONPROVIDER_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// header for class OUString
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//-----------------------------------------------------------------------------
/** This class provides localized descriptions for some basic actions done with objects.
The strings are intended to be provided to the user e.g. as description for undo actions in the menu.
The name of the object acted on and the type of action needs to be given as input parameter.
*/

class SVX_DLLPUBLIC ActionDescriptionProvider
{
public:
    enum ActionType
    {
        INSERT
        , DELETE
        , CUT
        , MOVE
        , RESIZE
        , ROTATE
        , TRANSFORM
        , FORMAT
        , MOVE_TOTOP
        , MOVE_TOBOTTOM
        , POS_SIZE
    };

public:
    static ::rtl::OUString createDescription( ActionType eActionType
                        , const ::rtl::OUString& rObjectName );
};

#endif
