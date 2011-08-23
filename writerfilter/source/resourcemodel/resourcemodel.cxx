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
          const rtl::OUString & /*str*/)
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
    void insert(Pointer_t /* pTablePropsRef */) {}
};

typedef TablePropsRef::Pointer_t TablePropsRef_t;

class WW8TableDataHandler : public TableDataHandler<string,
                            TablePropsRef_t>
{
public:
    typedef boost::shared_ptr<WW8TableDataHandler> Pointer_t;
    virtual void startTable(unsigned int nRows, unsigned int nDepth,
                            TablePropsRef_t pProps);
    virtual void endTable();
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

void WW8TableDataHandler::endTable()
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

WW8TableManager gTableManager;

/* WW8StreamHandler */

WW8StreamHandler::WW8StreamHandler()
: mnUTextCount(0)
{
    output.closeGroup();
    output.addItem("<stream>");
    gTableManager.startLevel();
}

WW8StreamHandler::~WW8StreamHandler()
{
    gTableManager.endLevel();

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

    gTableManager.startParagraphGroup();
    gTableManager.handle(gInfo);
}

void WW8StreamHandler::endParagraphGroup()
{
    gTableManager.endParagraphGroup();

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

    gTableManager.text(data, len);
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

    gTableManager.utext(data, len);
    
    mnUTextCount++;
}

void WW8StreamHandler::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    WW8PropertiesHandler aHandler;

    output.addItem("<properties type=\"" + ref->getType() + "\">");
    ref->resolve(aHandler);

    //gTableManager.props(ref);

    output.addItem("</properties>");
}

void WW8StreamHandler::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
    WW8TableHandler aHandler;

    output.addItem("<table id=\"" + (*QNameToString::Instance())(name)
                   + "\">");

    try
    {
        ref->resolve(aHandler);
    }
    catch (Exception e)
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

    gTableManager.startLevel();

    ref->resolve(*this);

    gTableManager.endLevel();

    output.addItem("</substream>");
}

void WW8StreamHandler::info(const string & info_)
{
    gInfo = info_;
    output.addItem("<info>" + info_ + "</info>");
}

void WW8PropertiesHandler::attribute(Id name, Value & val)
{
    boost::shared_ptr<rtl::OString> pStr(new ::rtl::OString());
    ::rtl::OUString aStr = val.getString();
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
        catch (ExceptionOutOfBounds e)
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
        catch (ExceptionOutOfBounds e)
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
        catch (ExceptionOutOfBounds e)
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

    gTableManager.sprm(sprm_);

    output.addItem("</sprm>");
}

void WW8TableHandler::entry(int /*pos*/, 
                            writerfilter::Reference<Properties>::Pointer_t ref)
{
    output.addItem("<tableentry>");

    WW8PropertiesHandler aHandler;

    try
    {
        ref->resolve(aHandler);
    }
    catch (Exception e)
    {
        output.addItem("<exception>" + e.getText() + "</exception>");
        output.addItem("</tableentry>");

        throw e;
    }

    output.addItem("</tableentry>");
}

void WW8BinaryObjHandler::data
(const sal_uInt8 * buf, size_t length,
 writerfilter::Reference<Properties>::Pointer_t /*pRef*/)
{
#if 1
    SubSequence<sal_uInt8> aSeq(buf, length);

    aSeq.dump(output);
#endif
}

}
