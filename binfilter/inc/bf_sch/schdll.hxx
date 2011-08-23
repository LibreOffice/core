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

#ifndef _SCH_DLL_HXX
#define _SCH_DLL_HXX

#include <bf_sfx2/sfxdefs.hxx>

// header for class SfxModule
#ifndef _SFXMODULE_HXX
#include <bf_sfx2/module.hxx>
#endif
// header for class SvInPlaceObject
#ifndef _IPOBJ_HXX
#include <bf_so3/ipobj.hxx>
#endif

#include <bf_sch/chattr.hxx>	// to do: remove this line and replace most of the schdll.hxx includes to chattr.hxx
class OutputDevice;
class Window;
class SvStream;

namespace binfilter {
class SvFactory;
class SvInPlaceObjectRef;
class SfxItemSet;

#define extern_c extern "C"

/*-------------------------------------------------------------------------

  The class SchDLL initializes the data, interfaces, controls, etc.
  that are needed by the shared library.

  It also maintains the ressource manager.

  to use the shared chart library SchDLL must be instanciated.

-------------------------------------------------------------------------*/

class SchDragServer;
class SchMemChart;
class SfxMedium;
class SfxFilter;
class XOutdevItemPool;
class ChartModel;

class SchDLL
{
public:

    SchDLL();
    ~SchDLL();

    static void     Init();
    static void     Exit();
    static void     LibInit();
    static void     LibExit();

    static void     Update( SvInPlaceObjectRef aIPObj, SchMemChart* pData, OutputDevice* pOut=NULL);

    static SchMemChart*  GetChartData( SvInPlaceObjectRef aIPObj );

    static SchMemChart* NewMemChart( short nCols, short nRows );

    static void GetDefaultForColumnText( const SchMemChart& rMemChart, sal_Int32 nCol, String& aResult );
    static void GetDefaultForRowText( const SchMemChart& rMemChart, sal_Int32 nRow, String& aResult );

    static void ConvertChartRangeForWriter( SchMemChart& rMemChart,
                                            BOOL bOldToNew );

    static void MemChartInsertCols( SchMemChart& rMemChart,
                                    short nAtCol, short nCount);
    static void MemChartRemoveCols( SchMemChart& rMemChart,
                                    short nAtCol, short nCount);

    static void MemChartInsertRows( SchMemChart& rMemChart,
                                    short nAtRow, short nCount);
    static void MemChartRemoveRows( SchMemChart& rMemChart,
                                    short nAtRow, short nCount);
};

/*-------------------------------------------------------------------------

  The following functions are called from the static interface (above)
  after the shared library has been loaded

-------------------------------------------------------------------------*/

extern_c void __LOADONCALLAPI SchUpdate( SvInPlaceObjectRef aIPObj, SchMemChart* pData, OutputDevice* pOut = NULL );
extern_c void __LOADONCALLAPI SchUpdateAttr( SvInPlaceObjectRef aIPObj, SchMemChart* pData,
                                             const SfxItemSet& rAttr, OutputDevice* pOut = NULL );
extern_c SchMemChart* __LOADONCALLAPI SchGetChartData( SvInPlaceObjectRef aIPObj );
extern_c SchMemChart* __LOADONCALLAPI SchNewMemChartNone();
extern_c SchMemChart* __LOADONCALLAPI SchNewMemChartXY( short nCols, short nRows );
extern_c SchMemChart* __LOADONCALLAPI SchNewMemChartCopy( const SchMemChart &rMemChart );
extern_c void __LOADONCALLAPI SetTransparentBackground( SvInPlaceObjectRef aIPObj, BOOL bTransp = TRUE );
extern_c void __LOADONCALLAPI SchGetDefaultForColumnText( const SchMemChart& rMemChart, sal_Int32 nCol, String& aResult );
extern_c void __LOADONCALLAPI SchGetDefaultForRowText( const SchMemChart& rMemChart, sal_Int32 nRow, String& aResult );

} //namespace binfilter
#endif          // _SCH_DLL_HXX

