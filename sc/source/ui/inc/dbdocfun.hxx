/*************************************************************************
 *
 *  $RCSfile: dbdocfun.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_DBDOCFUN_HXX
#define SC_DBDOCFUN_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class String;

struct ScImportParam;
struct ScQueryParam;
struct ScSortParam;
struct ScSubTotalParam;

class SbaSelectionList;
class ScDBData;
class ScDocShell;
class ScRange;
class ScDPObject;

// ---------------------------------------------------------------------------

class ScDBDocFunc
{
friend class ScDBFunc;

private:
    ScDocShell&     rDocShell;

public:
                    ScDBDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDBDocFunc() {}

    void            UpdateImport( const String& rTarget, const String& rDBName,
                                    const String& rTableName, const String& rStatement,
                                    BOOL bNative, BYTE nType,
                                    const SbaSelectionList* pSelection );
    BOOL            DoImport( USHORT nTab, const ScImportParam& rParam,
                                        const SbaSelectionList* pSelection,
                                        BOOL bRecord, BOOL bAddrInsert = FALSE );

    BOOL            Sort( USHORT nTab, const ScSortParam& rSortParam,
                            BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL            Query( USHORT nTab, const ScQueryParam& rQueryParam,
                            const ScRange* pAdvSource, BOOL bRecord, BOOL bApi );

    BOOL            DoSubTotals( USHORT nTab, const ScSubTotalParam& rParam,
                                    const ScSortParam* pForceNewSort,
                                    BOOL bRecord, BOOL bApi );

    BOOL            AddDBRange( const String& rName, const ScRange& rRange, BOOL bApi );
    BOOL            DeleteDBRange( const String& rName, BOOL bApi );
    BOOL            RenameDBRange( const String& rOld, const String& rNew, BOOL bApi );
    BOOL            ModifyDBData( const ScDBData& rNewData, BOOL bApi );    // Name unveraendert

    BOOL            RepeatDB( const String& rDBName, BOOL bRecord, BOOL bApi );

    BOOL            DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                        BOOL bRecord, BOOL bApi );
};



#endif
