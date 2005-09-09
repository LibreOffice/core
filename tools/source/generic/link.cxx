/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: link.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:21:20 $
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

#ifndef _LINK_HXX
#include <link.hxx>
#endif

/*************************************************************************
|*
|*    Link::operator==()
|*
|*    Beschreibung      LINK.SDW
|*    Ersterstellung    AM 14.02.91
|*    Letzte Aenderung  TH 07.11.95
|*
*************************************************************************/

BOOL Link::operator==( const Link& rLink ) const
{
    if ( pFunc == rLink.pFunc )
    {
        if ( pFunc )
        {
            if ( pInst == rLink.pInst )
                return TRUE;
            else
                return FALSE;
        }
        else
            return TRUE;
    }
    else
        return FALSE;
}
