/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:42:54 $
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

#ifndef _SC_INDEXMAP_HXX
#define _SC_INDEXMAP_HXX


#ifndef _SOLAR_H //autogen wg. USHORT
#include <tools/solar.h>
#endif


class ScIndexMap
{
                                // not implemented
                                ScIndexMap( const ScIndexMap& );
            ScIndexMap&         operator=( const ScIndexMap& );

private:
            USHORT*             pMap;
            USHORT              nCount;

public:
                                ScIndexMap( USHORT nEntries );
                                ~ScIndexMap();

            void                SetPair( USHORT nEntry, USHORT nIndex1, USHORT nIndex2 );
                                /// returns nIndex2 if found, else nIndex1
            USHORT              Find( USHORT nIndex1 ) const;
};


#endif // _SC_INDEXMAP_HXX

