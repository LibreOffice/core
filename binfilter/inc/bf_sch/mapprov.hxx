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

#ifndef SCH_MAPPROV_HXX
#define SCH_MAPPROV_HXX

// header for SfxItemPropertyMap
#ifndef _SFX_ITEMPROP_HXX
#include <bf_svtools/itemprop.hxx>
#endif

namespace binfilter {

// ----------------------------------------------
// 					 Map-Id's
// ----------------------------------------------

#define CHMAP_NONE					 0
#define CHMAP_CHART					 1
#define CHMAP_AREA					 2
#define CHMAP_LINE					 3
#define CHMAP_DOC					 4
#define CHMAP_AXIS					 5
#define CHMAP_LEGEND				 6
#define CHMAP_TITLE					 7
#define CHMAP_DATAROW				 8
#define CHMAP_DATAPOINT				 9
#define CHMAP_END					10		// last one used + 1

class ChartModel;

// ----------------------------------------------
// 		Map-Provider ( to sort the maps )
// ----------------------------------------------

class SchUnoPropertyMapProvider
{
private:
    SfxItemPropertyMap*	pMapArr[ CHMAP_END ];
    void Sort( short nId );

public:
    SchUnoPropertyMapProvider();
    ~SchUnoPropertyMapProvider();

    SfxItemPropertyMap* GetMap( short PropertyId, ChartModel* );
    SfxItemPropertyMap* CopyMap( const SfxItemPropertyMap* pMap1 );
};

} //namespace binfilter
#endif	// SCH_MAPPROV_HXX

