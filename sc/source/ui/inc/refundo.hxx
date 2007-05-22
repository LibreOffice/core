/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refundo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 20:08:07 $
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

#ifndef SC_REFUNDO_HXX
#define SC_REFUNDO_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class ScDocument;
class ScDBCollection;
class ScRangeName;
class ScPrintRangeSaver;
class ScPivotCollection;
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
    ScPivotCollection*          pPivotCollection;
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
    void        DoUndo( ScDocument* pDoc, BOOL bUndoRefFirst );
};



#endif

