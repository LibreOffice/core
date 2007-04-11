/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: metitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:28:13 $
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
#ifndef _SFXMETRICITEM_HXX
#define _SFXMETRICITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

DBG_NAMEEX_VISIBILITY(SfxMetricItem, SVT_DLLPUBLIC)

// -----------------------------------------------------------------------

class SVT_DLLPUBLIC SfxMetricItem: public SfxInt32Item
{
public:
                             TYPEINFO();
                             SfxMetricItem( USHORT nWhich = 0, UINT32 nValue = 0 );
                             SfxMetricItem( USHORT nWhich, SvStream & );
                             SfxMetricItem( const SfxMetricItem& );
                             ~SfxMetricItem() {
                                 DBG_DTOR(SfxMetricItem, 0); }

    virtual int              ScaleMetrics( long lMult, long lDiv );
    virtual int              HasMetrics() const;

};

#endif

