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

#ifndef SC_REFUNDO_HXX
#define SC_REFUNDO_HXX

#include <tools/solar.h>

class ScDocument;
class ScDBCollection;
class ScRangeName;
class ScPrintRangeSaver;
class ScDPCollection;
class ScChartCollection;
class ScConditionalFormatList;
class ScDetOpList;
class ScChartListenerCollection;
class ScAreaLinkSaveCollection;
class ScUnoRefList;

class ScRefUndoData
{
private:
    ScDBCollection*             pDBCollection;
    ScRangeName*                pRangeName;
    ScPrintRangeSaver*          pPrintRanges;
    ScDPCollection*             pDPCollection;
    ScConditionalFormatList*    pCondFormList;
    ScDetOpList*                pDetOpList;
    ScChartListenerCollection*  pChartListenerCollection;
    ScAreaLinkSaveCollection*   pAreaLinks;
    ScUnoRefList*               pUnoRefs;

public:
                ScRefUndoData( const ScDocument* pDoc );
                ~ScRefUndoData();

    void        DeleteUnchanged( const ScDocument* pDoc );
    void        DoUndo( ScDocument* pDoc, sal_Bool bUndoRefFirst );
};



#endif

