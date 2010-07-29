/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXMETRICITEM_HXX
#define _SFXMETRICITEM_HXX

#include "svl/svldllapi.h"
#include <svl/intitem.hxx>

DBG_NAMEEX_VISIBILITY(SfxMetricItem, SVL_DLLPUBLIC)

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxMetricItem: public SfxInt32Item
{
public:
                             TYPEINFO();
                             SfxMetricItem( sal_uInt16 nWhich = 0, sal_uInt32 nValue = 0 );
                             SfxMetricItem( sal_uInt16 nWhich, SvStream & );
                             SfxMetricItem( const SfxMetricItem& );
                             ~SfxMetricItem() {
                                 DBG_DTOR(SfxMetricItem, 0); }

    virtual int              ScaleMetrics( long lMult, long lDiv );
    virtual int              HasMetrics() const;

};

#endif

