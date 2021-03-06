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

#pragma once

#include "xeroot.hxx"
#include "xerecord.hxx"
#include "excrecds.hxx"
#include <memory>

// Forwards -

class SvStream;
class XclExpNote;
class XclExpStream;
class XclExpXmlStream;
class XclExpChangeTrack;


class XclExpCellTable;

class ExcTable : public XclExpRecordBase, public XclExpRoot
{
private:
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef rtl::Reference< XclExpCellTable >       XclExpCellTableRef;
    typedef XclExpRecordList< XclExpNote >          XclExpNoteList;
    typedef rtl::Reference< XclExpNoteList >        XclExpNoteListRef;

    XclExpRecordList<>          aRecList;
    XclExpCellTableRef          mxCellTable;

    SCTAB                       mnScTab;    // table number SC document
    sal_uInt16                  nExcTab;    // table number Excel document

    XclExpNoteListRef           mxNoteList;

    // re-create and forget pRec; delete is done by ExcTable itself!
    void                        Add( XclExpRecordBase* pRec );

public:
                                ExcTable( const XclExpRoot& rRoot );
                                ExcTable( const XclExpRoot& rRoot, SCTAB nScTab );
                                virtual ~ExcTable() override;

    void FillAsHeaderBinary( ExcBoundsheetList& rBoundsheetList );
    void FillAsHeaderXml( ExcBoundsheetList& rBoundsheetList );

    void FillAsTableBinary( SCTAB nCodeNameIdx );
    void FillAsTableXml();

    void                        FillAsEmptyTable( SCTAB nCodeNameIdx );

    void                        Write( XclExpStream& );
    void                        WriteXml( XclExpXmlStream& );
};

class ExcDocument : protected XclExpRoot
{
friend class ExcTable;

private:
    typedef XclExpRecordList< ExcTable >            ExcTableList;
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef ExcBoundsheetList::RecordRefType        ExcBoundsheetRef;

    ExcTable            aHeader;

    ExcTableList        maTableList;
    ExcBoundsheetList   maBoundsheetList;

    std::unique_ptr<XclExpChangeTrack> m_xExpChangeTrack;

public:
    explicit                    ExcDocument( const XclExpRoot& rRoot );
    virtual                     ~ExcDocument() override;

    void                ReadDoc();
    void                Write( SvStream& rSvStrm );
    void                WriteXml( XclExpXmlStream& );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
