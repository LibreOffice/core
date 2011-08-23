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

#ifndef SC_DBFUNC_HXX
#define SC_DBFUNC_HXX

#include "viewfunc.hxx"
namespace binfilter {

struct ScSortParam;
struct ScQueryParam;
class ScDBData;
class ScDBCollection;
class ScDPObject;
class ScDPSaveData;

// ---------------------------------------------------------------------------

class ScDBFunc : public ScViewFunc
{
public:
                    ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                     ScDBFunc( Window* pParent, const ScDBFunc& rDBFunc, ScTabViewShell* pViewShell );
                    ~ScDBFunc();

                    //	nur UISort wiederholt bei Bedarf die Teilergebnisse







                    // DB-Bereich vom Cursor
    ScDBData* 		GetDBData( BOOL bMarkArea = TRUE, ScGetDBMode eMode = SC_DB_MAKE );




    void			TestRemoveOutline( BOOL& rCol, BOOL& rRow );




    void			UpdateCharts(BOOL bAllCharts = FALSE);		// Default: am Cursor
};



} //namespace binfilter
#endif

