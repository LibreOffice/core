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

Stream::Pointer_t createStreamHandler()
{
    return Stream::Pointer_t(new WW8StreamHandler());
}

void dump(OutputWithDepth<string> & /*o*/, const char * /*name*/,
          writerfilter::Reference<Properties>::Pointer_t /*props*/)
{
}

void dump(OutputWithDepth<string> & o, const char * name, sal_uInt32 n)
{
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIuUINT32, n);
    string tmpStr = name;
    tmpStr += "=";
    tmpStr += sBuffer;

    o.addItem(tmpStr);
}

void dump(OutputWithDepth<string> & /*o*/, const char * /*name*/,
          const OUString & /*str*/)
{
}

void dump(OutputWithDepth<string> & /*o*/, const char * /*name*/,
          writerfilter::Reference<BinaryObj>::Pointer_t /*binary*/)
{
}

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
    snprintf(sBuffer, sizeof(sBuffer), "%d", nRows);
    tmpStr += sBuffer;
    tmpStr += "\" depth=\"";
    snprintf(sBuffer, sizeof(sBuffer), "%d", nDepth);
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

    snprintf(sBuffer, sizeof(sBuffer), "%d", nCols);
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

class WW8TableManager :
    public TableManager<string, TablePropsRef_t>
{
    typedef TableDataHandler<string, TablePropsRef_t>
    TableDataHandlerPointer_t;

public:
    WW8TableManager();
    virtual ~WW8TableManager() {}
    virtual void endParagraphGroup();
    virtual bool sprm(Sprm & rSprm);
};

WW8TableManager::WW8TableManager()
{
    TableDataHandler<string, TablePropsRef_t>::Pointer_t pHandler(new WW8TableDataHandler());
    setHandler(pHandler);
}

bool WW8TableManager::sprm(Sprm & rSprm)
{
    TableManager<string, TablePropsRef_t>::sprm(rSprm);
    output.setDepth(getTableDepthNew());
    return true;
}

void WW8TableManager::endParagraphGroup()
{
    string tmpStr = "<tabledepth depth=\"";
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIuUINT32, getTableDepthNew());
    tmpStr += sBuffer;
    tmpStr += "\"/>";
    output.addItem(tmpStr);
    TableManager<string, TablePropsRef_t>::endParagraphGroup();
}

/* WW8StreamHandler */

WW8StreamHandler::WW8StreamHandler()
: mnUTextCount(0)
{
    output.closeGroup();
    output.addItem("<stream>");
    mpTableManager = new WW8TableManager;
    mpTableManager->startLevel();
}

WW8StreamHandler::~WW8StreamHandler()
{
    mpTableManager->endLevel();
    delete mpTableManager;

    output.closeGroup();
    output.addItem("</stream>");
}

void WW8StreamHandler::startSectionGroup()
{
    output.addItem("<section-group>");
}

void WW8StreamHandler::endSectionGroup()
{
    output.addItem("</section-group>");
}

void WW8StreamHandler::startParagraphGroup()
{
    output.openGroup();
    output.addItem("<paragraph-group>");

    mpTableManager->startParagraphGroup();
    mpTableManager->handle(gInfo);
}

void WW8StreamHandler::endParagraphGroup()
{
    mpTableManager->endParagraphGroup();

    output.addItem("</paragraph-group>");
    output.closeGroup();

}

void WW8StreamHandler::startCharacterGroup()
{
    output.addItem("<character-group>");
}

void WW8StreamHandler::endCharacterGroup()
{
    output.addItem("</character-group>");
}

void WW8StreamHandler::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > /*xShape*/ )
{
    output.addItem("<shape>");
}

void WW8StreamHandler::endShape( )
{
    output.addItem( "</shape>" );
}

void WW8StreamHandler::text(const sal_uInt8 * data, size_t len)
{
    string tmpStr = "<text>";

    for (unsigned int n = 0; n < len; ++n)
    {
        switch (static_cast<unsigned char>(data[n]))
        {
        case '<':
            tmpStr += "&lt;";

            break;
        case '>':
            tmpStr += "&gt;";

            break;

        case '&':
            tmpStr += "&amp;";

            break;
        default:
            if (isprint(data[n]))
                tmpStr += static_cast<char>(data[n]);
            else
            {
                char sBuffer[256];

                snprintf(sBuffer, sizeof(sBuffer), "\\0x%02x", data[n]);

                tmpStr += sBuffer;
            }
        }
    }

    tmpStr += "</text>";

    output.addItem(tmpStr);

    mpTableManager->text(data, len);
}

void WW8StreamHandler::utext(const sal_uInt8 * data, size_t len)
{
    static char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "<utext count=\"%d\">", mnUTextCount);
    string tmpStr(sBuffer);

    for (unsigned int n = 0; n < len; ++n)
    {
        sal_Unicode nChar = data[n * 2] + (data[n * 2 + 1] << 8);
        if (nChar < 0xff && isprint(nChar))
        {
            switch (nChar)
            {
            case '&':
                tmpStr += "&amp;";
                break;
            case '<':
                tmpStr += "&lt;";
                break;
            case '>':
                tmpStr += "&gt;";
                break;
            default:
                tmpStr += static_cast<char>(nChar);
            }
        }
        else
        {
            snprintf(sBuffer, sizeof(sBuffer), "\\0x%04x", nChar);

            tmpStr += sBuffer;
        }
    }

    tmpStr += "</utext>";

    output.addItem(tmpStr);

    mpTableManager->utext(data, len);

    mnUTextCount++;
}

void WW8StreamHandler::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    WW8PropertiesHandler aHandler(mpTableManager);

    output.addItem("<properties type=\"" + ref->getType() + "\">");
    ref->resolve(aHandler);

    //mpTableManager->props(ref);

    output.addItem("</properties>");
}

void WW8StreamHandler::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
    WW8TableHandler aHandler(mpTableManager);

    output.addItem("<table id=\"" + (*QNameToString::Instance())(name)
                   + "\">");

    try
    {
        ref->resolve(aHandler);
    }
    catch (const Exception &e)
    {
        output.addItem("<exception>" + e.getText() + "</exception>");
    }

    output.addItem("</table>");
}

void WW8StreamHandler::substream(Id name,
                                 writerfilter::Reference<Stream>::Pointer_t ref)
{
    output.addItem("<substream name=\"" + (*QNameToString::Instance())(name)
                   + "\">");

    mpTableManager->startLevel();

    ref->resolve(*this);

    mpTableManager->endLevel();

    output.addItem("</substream>");
}

void WW8StreamHandler::info(const string & info_)
{
    gInfo = info_;
    output.addItem("<info>" + info_ + "</info>");
}

void WW8PropertiesHandler::attribute(Id name, Value & val)
{
    boost::shared_ptr<OString> pStr(new OString());
    OUString aStr = val.getString();
    aStr.convertToString(pStr.get(), RTL_TEXTENCODING_ASCII_US,
                         OUSTRING_TO_OSTRING_CVTFLAGS);
    string sXMLValue = xmlify(pStr->getStr());

    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "0x%x", val.getInt());

    output.addItem("<attribute name=\"" +
                   (*QNameToString::Instance())(name) +
                   "\" value=\"" +
                   sXMLValue +
                   + "\" hexvalue=\""
                   + sBuffer + "\">");

    writerfilter::Reference<Properties>::Pointer_t pProps = val.getProperties();

    if (pProps.get() != NULL)
    {
        output.addItem("<properties name=\"" +
                       (*QNameToString::Instance())(name)
                       + "\" type=\"" + pProps->getType() + "\">");

        try
        {
            pProps->resolve(*this);
        }
        catch (const ExceptionOutOfBounds &)
        {
        }

        output.addItem("</properties>");
    }

    writerfilter::Reference<Stream>::Pointer_t pStream = val.getStream();

    if (pStream.get() != NULL)
    {
        try
        {
            WW8StreamHandler aHandler;

            pStream->resolve(aHandler);
        }
        catch (const ExceptionOutOfBounds &)
        {
        }
    }

    writerfilter::Reference<BinaryObj>::Pointer_t pBinObj = val.getBinary();

    if (pBinObj.get() != NULL)
    {
        try
        {
            WW8BinaryObjHandler aHandler;

            pBinObj->resolve(aHandler);
        }
        catch (const ExceptionOutOfBounds &)
        {
        }
    }

    output.addItem("</attribute>");
}

void WW8PropertiesHandler::sprm(Sprm & sprm_)
{
    string tmpStr = "<sprm id=\"";
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "0x%" SAL_PRIxUINT32, sprm_.getId());
    tmpStr += buffer;
    tmpStr += "\" name=\"";
    tmpStr += sprm_.getName();
    tmpStr += "\">";
    output.addItem(tmpStr);
    output.addItem(sprm_.toString());

    writerfilter::Reference<Properties>::Pointer_t pProps = sprm_.getProps();

    if (pProps.get() != NULL)
    {
        output.addItem("<properties type=\"" + pProps->getType() + "\">");
        pProps->resolve(*this);
        output.addItem("</properties>");
    }

    writerfilter::Reference<BinaryObj>::Pointer_t pBinObj = sprm_.getBinary();

    if (pBinObj.get() != NULL)
    {
        output.addItem("<binary>");
        WW8BinaryObjHandler aHandler;
        pBinObj->resolve(aHandler);
        output.addItem("</binary>");
    }

    writerfilter::Reference<Stream>::Pointer_t pStream = sprm_.getStream();

    if (pStream.get() != NULL)
    {
        output.addItem("<stream>");
        WW8StreamHandler aHandler;
        pStream->resolve(aHandler);
        output.addItem("</stream>");
    }

    mpTableManager->sprm(sprm_);

    output.addItem("</sprm>");
}

void WW8TableHandler::entry(int /*pos*/,
                            writerfilter::Reference<Properties>::Pointer_t ref)
{
    output.addItem("<tableentry>");

    WW8PropertiesHandler aHandler(mpTableManager);

    try
    {
        ref->resolve(aHandler);
    }
    catch (const Exception &e)
    {
        output.addItem("<exception>" + e.getText() + "</exception>");
        output.addItem("</tableentry>");

        throw;
    }

    output.addItem("</tableentry>");
}

void WW8BinaryObjHandler::data
(const sal_uInt8 * buf, size_t length,
 writerfilter::Reference<Properties>::Pointer_t /*pRef*/)
{
    SubSequence<sal_uInt8> aSeq(buf, length);

    aSeq.dump(output);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
