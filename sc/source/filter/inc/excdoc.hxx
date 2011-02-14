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

#ifndef SC_EXCDOC_HXX
#define SC_EXCDOC_HXX

#include <tools/solar.h>
#include "excrecds.hxx"
#include "xeroot.hxx"
#include "root.hxx"

//------------------------------------------------------------------ Forwards -

class SvStream;
class ScBaseCell;
class ScHorizontalCellIterator;
class ScDocument;
class ScProgress;

class NameBuffer;

class XclExpChangeTrack;


//------------------------------------------------------------ class ExcTable -

class XclExpCellTable;

class ExcTable : public XclExpRecordBase, public XclExpRoot
{
private:
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef ScfRef< XclExpCellTable >               XclExpCellTableRef;

    XclExpRecordList<>          aRecList;
    XclExpCellTableRef          mxCellTable;

    SCTAB                       mnScTab;    // table number SC document
    sal_uInt16                      nExcTab;    // table number Excel document
    sal_uInt16                      nAktRow;    // fuer'n Iterator
    sal_uInt16                      nAktCol;

    NameBuffer*                 pTabNames;

    // pRec mit new anlegen und vergessen, delete macht ExcTable selber!
    void                        Add( XclExpRecordBase* pRec );

    void                        FillAsXmlTable( SCTAB nCodeNameIdx );

public:
                                ExcTable( const XclExpRoot& rRoot );
                                ExcTable( const XclExpRoot& rRoot, SCTAB nScTab );
                                ~ExcTable();

    void                        FillAsHeader( ExcBoundsheetList& rBoundsheetList );
    void                        FillAsTable( SCTAB nCodeNameIdx );
    void                        FillAsEmptyTable( SCTAB nCodeNameIdx );

    void                        Write( XclExpStream& );
    void                        WriteXml( XclExpXmlStream& );
};


//--------------------------------------------------------- class ExcDocument -

class ExcDocument : protected XclExpRoot
{
friend class ExcTable;

private:
    typedef XclExpRecordList< ExcTable >            ExcTableList;
    typedef ExcTableList::RecordRefType             ExcTableRef;
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef ExcBoundsheetList::RecordRefType        ExcBoundsheetRef;

    ExcTable            aHeader;

    ExcTableList        maTableList;
    ExcBoundsheetList   maBoundsheetList;

    XclExpChangeTrack*  pExpChangeTrack;

public:
    explicit                    ExcDocument( const XclExpRoot& rRoot );
    virtual                     ~ExcDocument();

    void                ReadDoc( void );
    void                Write( SvStream& rSvStrm );
    void                WriteXml( SvStream& rSvStrm );
};




#endif

