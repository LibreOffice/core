/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: agitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:08:01 $
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

#ifndef _AGITEM_HXX
#define _AGITEM_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class String;
class INetURLObject;
class AgentApi;
class ChApi;

class AgentItem
{
public:
    virtual const String&           GetChAgentName() const = 0;
    virtual const INetURLObject&    GetLocation() const = 0;
    virtual AgentApi*               GetApi() const = 0;
    virtual ChApi*                  GetChApi() const = 0;

    virtual void                    SetIsActive(BOOL bNew) = 0;
    virtual BOOL                    IsActive() const = 0;
};

#endif //_AGITEM_HXX

