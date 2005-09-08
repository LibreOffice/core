/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listenerbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:31:34 $
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
#ifndef _SVT_LISTENERBASE_HXX
#define _SVT_LISTENERBASE_HXX

class SvtBroadcaster;
class SvtListener;

class SvtListenerBase
{
    SvtListenerBase *pNext;
    SvtListenerBase *pLeft, *pRight;
    SvtBroadcaster *pBroadcaster;
    SvtListener *pListener;

public:

    SvtListenerBase( SvtListener& rLst, SvtBroadcaster& rBroadcaster );
    ~SvtListenerBase();

    SvtListenerBase* GetNext() const        { return pNext; }
    void SetNext( SvtListenerBase* p )      { pNext = p; }

    SvtBroadcaster* GetBroadcaster() const  { return pBroadcaster; }
    SvtListener* GetListener() const        { return pListener; }

    SvtListenerBase* GetLeft() const        { return pLeft; }
    SvtListenerBase* GetRight() const       { return pRight; }
};


#endif

