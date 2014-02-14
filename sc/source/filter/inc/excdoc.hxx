/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_EXCDOC_HXX
#define SC_EXCDOC_HXX

#include "excrecds.hxx"
#include "xeroot.hxx"
#include "root.hxx"
#include "xeescher.hxx"
#include <boost/shared_ptr.hpp>

// Forwards -

class SvStream;

class NameBuffer;

class XclExpChangeTrack;

// class ExcTable -

class XclExpCellTable;

class ExcTable : public XclExpRecordBase, public XclExpRoot
{
private:
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef boost::shared_ptr< XclExpCellTable >    XclExpCellTableRef;
    typedef XclExpRecordList< XclExpNote >      XclExpNoteList;
    typedef boost::shared_ptr< XclExpNoteList >     XclExpNoteListRef;

    XclExpRecordList<>          aRecList;
    XclExpCellTableRef          mxCellTable;

    SCTAB                       mnScTab;    // table number SC document
    sal_uInt16                      nExcTab;    // table number Excel document

    NameBuffer*                 pTabNames;

    XclExpNoteListRef   mxNoteList;

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
    void                WriteXml( XclExpXmlStream& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
