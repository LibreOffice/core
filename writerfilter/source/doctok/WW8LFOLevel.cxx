/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8LFOLevel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2006-11-01 09:14:31 $
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

/**
   $RCSfile: WW8LFOLevel.cxx,v $

   $Revision: 1.2 $

 */
#include "resources.hxx"

sal_uInt32 WW8LFOLevel::calcSize()
{
    sal_uInt32 nResult = WW8LFOLevel::getSize();

    if (get_fFormatting())
    {
        WW8ListLevel aLevel(parent, mnOffsetInParent + nResult)
            nResult += aLevel.calcSize();

        sal_uInt32 nXstSize = parent->getU16(nOffset);

        nResult += (nXstSize + 1) * 2;
    }

    return nResult;
}
