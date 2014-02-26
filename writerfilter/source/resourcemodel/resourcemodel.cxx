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

#include <stdio.h>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <resourcemodel/TableManager.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/exceptions.hxx>
#include <resourcemodel/SubSequence.hxx>
#include <resourcemodel/util.hxx>
#include <resourcemodel.hxx>

namespace writerfilter {

class ResourceModelOutputWithDepth : public OutputWithDepth<string>
{
public:
    ResourceModelOutputWithDepth()
    : OutputWithDepth<string>("<tablegroup>", "</tablegroup>") {}

    ~ResourceModelOutputWithDepth() {outputGroup();}

    void output(const string & str) const { cout << str << endl; }
};

ResourceModelOutputWithDepth output;

string gInfo = "";
// ------- WW8TableDataHandler ---------

class TablePropsRef : public writerfilter::Reference<Properties>
{
public:
    typedef boost::shared_ptr<TablePropsRef> Pointer_t;

    TablePropsRef() {}
    virtual ~TablePropsRef() {}

    virtual void resolve(Properties & /*rHandler*/) {}

    virtual string getType() const { return "TableProps"; }
    void reset() {}
    void InsertProps(Pointer_t /* pTablePropsRef */) {}
};

typedef TablePropsRef::Pointer_t TablePropsRef_t;

class WW8TableDataHandler : public TableDataHandler<string,
                            TablePropsRef_t>
{
public:
    virtual ~WW8TableDataHandler() {}

    typedef boost::shared_ptr<WW8TableDataHandler> Pointer_t;
    virtual void startTable(unsigned int nRows, unsigned int nDepth,
                            TablePropsRef_t pProps);
    virtual void endTable(unsigned int nestedTableLevel);
    virtual void startRow(unsigned int nCols,
                          TablePropsRef_t pProps);
    virtual void endRow();
    virtual void startCell(const string & start, TablePropsRef_t pProps);
    virtual void endCell(const string & end);
};

void WW8TableDataHandler::startTable(unsigned int nRows, unsigned int nDepth,
                                     TablePropsRef_t /*pProps*/)
{
    char sBuffer[256];

    string tmpStr = "<tabledata.table rows=\"";
    snprintf(sBuffer, sizeof(sBuffer), "%u", nRows);
    tmpStr += sBuffer;
    tmpStr += "\" depth=\"";
    snprintf(sBuffer, sizeof(sBuffer), "%u", nDepth);
    tmpStr += sBuffer;
    tmpStr += "\">";

    output.addItem(tmpStr);
}

void WW8TableDataHandler::endTable(unsigned int /*nestedTableLevel*/)
{
    output.addItem("</tabledata.table>");
}

void WW8TableDataHandler::startRow
(unsigned int nCols, TablePropsRef_t /*pProps*/)
{
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "%u", nCols);
    string tmpStr = "<tabledata.row cells=\"";
    tmpStr += sBuffer;
    tmpStr += "\">";
    output.addItem(tmpStr);
}

void WW8TableDataHandler::endRow()
{
    output.addItem("</tabledata.row>");
}

void WW8TableDataHandler::startCell(const string & start,
                                    TablePropsRef_t /*pProps*/)
{
    output.addItem("<tabledata.cell>");
    output.addItem(start);
    output.addItem(", ");
}

void WW8TableDataHandler::endCell(const string & end)
{
    output.addItem(end);
    output.addItem("</tabledata.cell>");
}

// ----- WW8TableDataManager -------------------------------

void WW8BinaryObjHandler::data
(const sal_uInt8 * buf, size_t length,
 writerfilter::Reference<Properties>::Pointer_t /*pRef*/)
{
    SubSequence<sal_uInt8> aSeq(buf, length);

    aSeq.dump(output);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
