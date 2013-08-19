/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_XEDBDATA_HXX
#define SC_ZEDBDATA_HXX

#include "dbdata.hxx"
#include "xeroot.hxx"
#include "xerecord.hxx"
#include "address.hxx"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

const sal_uInt16 EXC_ID_DBDATATABLES = 0x11A01;

class XclExpXmlDBDataStyleInfo : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpXmlDBDataStyleInfo( const XclExpRoot& rRoot, ScDBDataFormatting& rDBDataFormatting );
    virtual void SaveXml( XclExpXmlStream& rStrm );
private:
    ScDBDataFormatting maDBDataFormatting;
};
// ===========================================================================
class XclExpXmlDBDataColumn : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpXmlDBDataColumn( const XclExpRoot& rRoot, int iID, OUString& rName );
    virtual void SaveXml( XclExpXmlStream& rStrm );
private:
    OUString maName;
    int miID;
};

// ===========================================================================
class XclExpXmlDBDataColumns : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpXmlDBDataColumns( const XclExpRoot& rRoot, ScDBData& rDBData );
    virtual void SaveXml( XclExpXmlStream& rStrm );
private:
    typedef boost::ptr_vector< XclExpXmlDBDataColumn > DBDataColumnContainer;
    DBDataColumnContainer maDBDataColumnContainer;
};

// ============================================================================
class XclExpXmlDBDataAutoFilter : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpXmlDBDataAutoFilter( const XclExpRoot& rRoot, ScRange& pRange );
    virtual void SaveXml( XclExpXmlStream& rStrm );
private:
    ScRange maRange;
};

// ============================================================================
class XclExpXmlDBDataTable : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpXmlDBDataTable( const XclExpRoot& rRoot, ScDBData& rDBData, int nTableId );
    virtual void SaveXml( XclExpXmlStream& rStrm );
private:
    typedef boost::scoped_ptr < XclExpXmlDBDataColumns > DBDataTableColumns;
    typedef boost::scoped_ptr < XclExpXmlDBDataStyleInfo > DBDataStyleInfo;
    typedef boost::scoped_ptr < XclExpXmlDBDataAutoFilter > DBDataAutoFilter;
    DBDataTableColumns maTableColumns;
    DBDataStyleInfo maStyleInfo;
    DBDataAutoFilter maAutoFilter;
    int mnTableId;
    OUString maName;
    OUString maDisplayName;
    ScRange maRange;
    bool mbTotalsRowShown;
};

// ============================================================================
class XclExpXmlDBDataTables : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpXmlDBDataTables( const XclExpRoot& rRoot );
    virtual void SaveXml( XclExpXmlStream& rStrm );
private:
    typedef boost::ptr_vector< XclExpXmlDBDataTable > DBDataTableContainer;
    DBDataTableContainer maDBDataTableContainer;
};

#endif
