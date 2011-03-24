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

#ifndef SC_DBDOCFUN_HXX
#define SC_DBDOCFUN_HXX

#include "address.hxx"
#include <tools/solar.h>
#include <com/sun/star/uno/Sequence.hxx>

class String;

struct ScImportParam;
struct ScQueryParam;
struct ScSortParam;
struct ScSubTotalParam;

class SfxViewFrame;
class ScDBData;
class ScDocShell;
class ScAddress;
class ScRange;
class ScDPObject;

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
} } }

namespace svx {
    class ODataAccessDescriptor;
}

// ---------------------------------------------------------------------------
// -----------------------------------------------------------------


class ScDBDocFunc
{
friend class ScDBFunc;

private:
    ScDocShell&     rDocShell;

public:
                    ScDBDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDBDocFunc() {}

    void            UpdateImport( const String& rTarget, const svx::ODataAccessDescriptor& rDescriptor );

    sal_Bool        DoImport( SCTAB nTab, const ScImportParam& rParam,
                        const svx::ODataAccessDescriptor* pDescriptor,      // used for selection and existing ResultSet
                        sal_Bool bRecord,
                        sal_Bool bAddrInsert = sal_False );

    sal_Bool        DoImportUno( const ScAddress& rPos,
                                const com::sun::star::uno::Sequence<
                                    com::sun::star::beans::PropertyValue>& aArgs );

    static void     ShowInBeamer( const ScImportParam& rParam, SfxViewFrame* pFrame );

    sal_Bool        Sort( SCTAB nTab, const ScSortParam& rSortParam,
                            sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    SC_DLLPUBLIC sal_Bool Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                            const ScRange* pAdvSource, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                                    const ScSortParam* pForceNewSort,
                                    sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            AddDBRange( const String& rName, const ScRange& rRange, sal_Bool bApi );
    sal_Bool            DeleteDBRange( const String& rName, sal_Bool bApi );
    sal_Bool            RenameDBRange( const String& rOld, const String& rNew, sal_Bool bApi );
    sal_Bool            ModifyDBData( const ScDBData& rNewData, sal_Bool bApi );    // Name unveraendert

    sal_Bool            RepeatDB( const String& rDBName, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                        sal_Bool bRecord, sal_Bool bApi, sal_Bool bAllowMove = sal_False );
};



#endif
