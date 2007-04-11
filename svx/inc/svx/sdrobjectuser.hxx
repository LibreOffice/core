/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrobjectuser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:11:10 $
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

#ifndef _SDR_OBJECTUSER_HXX
#define _SDR_OBJECTUSER_HXX

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

////////////////////////////////////////////////////////////////////////////////////////////////////
// To make things more safe, allow users of the page to register at it. The users need to be derived
// from sdr::PageUser to get a call. The users do not need to call RemovePageUser() at the page
// when they get called from PageInDestruction().

namespace sdr
{
    class ObjectUser
    {
    public:
        // this method is called form the destructor of the referenced page.
        // do all necessary action to forget the page. It is not necessary to call
        // RemovePageUser(), that is done form the destructor.
        virtual void ObjectInDestruction(const SdrObject& rObject) = 0;
    };

    // typedef for ObjectUserVector
    typedef ::std::vector< ObjectUser* > ObjectUserVector;
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OBJECTUSER_HXX
// eof
