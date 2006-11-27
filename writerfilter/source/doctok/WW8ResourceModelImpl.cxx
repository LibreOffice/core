/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8ResourceModelImpl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2006-11-27 09:03:47 $
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

#include <WW8ResourceModelImpl.hxx>
#include <WW8Analyzer.hxx>
#include <resources.hxx>
#include <util.hxx>
#include <WW8OutputWithDepth.hxx>
#include <doctok/TableManager.hxx>
#include <rtl/string.hxx>

namespace doctok
{
using namespace ::std;

string gInfo = "";

typedef WW8PropertySet::Pointer_t TablePropsPointer_t;

string xmlify(string str)
{
    string result = "";
    char sBuffer[16];

    for (string::iterator aIt = str.begin(); aIt != str.end(); ++aIt)
    {
        char c = *aIt;

        if (isprint(c))
            result += c;
        else
        {
            snprintf(sBuffer, sizeof(sBuffer), "\\%03d", c);
            result += sBuffer;
        }
    }

    return result;
}

// ------- WW8TableDataHandler ---------

class WW8TableDataHandler : public TableDataHandler<string,
                            TablePropsPointer_t>
{
public:
    typedef boost::shared_ptr<WW8TableDataHandler> Pointer_t;
    virtual void startTable(unsigned int nRows, unsigned int nDepth,
                            TablePropsPointer_t pProps);
    virtual void endTable();
    virtual void startRow(unsigned int nCols,
                          TablePropsPointer_t pProps);
    virtual void endRow();
    virtual void startCell(const string & start, TablePropsPointer_t pProps);
    virtual void endCell(const string & end);
};

void WW8TableDataHandler::startTable(unsigned int nRows, unsigned int nDepth,
                                     TablePropsPointer_t /*pProps*/)
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
(unsigned int nCols, TablePropsPointer_t /*pProps*/)
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
                                    TablePropsPointer_t /*pProps*/)
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
    public TableManager<string, TablePropsPointer_t>
{
    typedef TableDataHandler<string, TablePropsPointer_t>
    TableDataHandlerPointer_t;

public:
    WW8TableManager();
    virtual ~WW8TableManager() {}
    virtual void endParagraphGroup();
    virtual void sprm(Sprm & rSprm);
};

WW8TableManager::WW8TableManager()
{
    TableDataHandler<string, TablePropsPointer_t>::Pointer_t pHandler(new WW8TableDataHandler());
    setHandler(pHandler);
}

void WW8TableManager::sprm(Sprm & rSprm)
{
    TableManager<string, TablePropsPointer_t>::sprm(rSprm);
    output.setDepth(getTableDepthNew());
}

void WW8TableManager::endParagraphGroup()
{
    string tmpStr = "<tabledepth depth=\"";
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%ld", getTableDepthNew());
    tmpStr += sBuffer;
    tmpStr += "\"/>";
    output.addItem(tmpStr);
    TableManager<string, TablePropsPointer_t>::endParagraphGroup();
}

WW8TableManager gTableManager;

//-------- WW8TableReference -----------------------------------

void WW8TableReference::resolve(Table & /*rHandler*/)
{
    output.addItem("<table/>");
}

string WW8TableReference::getType() const
{
    return "WW8TableReference";
}

void WW8PropertiesReference::resolve(Properties & rHandler)
{
    if (mpPropSet != NULL)
    {
        //mpPropSet->dump(clog);

        if (mpPropSet->isPap())
        {
            WW8IntValue aValue(mpPropSet->get_istd());

            rHandler.attribute(NS_rtf::LN_ISTD, aValue);
        }

        WW8PropertySetIterator::Pointer_t pIt = mpPropSet->begin();
        WW8PropertySetIterator::Pointer_t pItEnd = mpPropSet->end();

        try
        {
            while (! pIt->equal(*pItEnd))
            {
                WW8Sprm aSprm(pIt->get());
                rHandler.sprm(aSprm);

                ++(*pIt);
            }
        }
        catch (ExceptionOutOfBounds e)
        {
        }
    }
}

string WW8PropertiesReference::getType() const
{
    return "WW8PropertiesReference";
}

WW8BinaryObjReference::WW8BinaryObjReference
(WW8StructBase & rParent, sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(rParent, nOffset, nCount)
{
}

WW8BinaryObjReference::WW8BinaryObjReference
(WW8StructBase * pParent, sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(pParent, nOffset, nCount)
{
}

WW8BinaryObjReference::WW8BinaryObjReference
(WW8StructBase * pParent)
: WW8StructBase(pParent, 0x0, pParent->getCount())
{
}

WW8BinaryObjReference::WW8BinaryObjReference
(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount)
{
}

doctok::Reference<BinaryObj>::Pointer_t
WW8BinaryObjReference::getBinary()
{
    return doctok::Reference<BinaryObj>::Pointer_t
        (new WW8BinaryObjReference(*this));
}

void WW8BinaryObjReference::resolve(BinaryObj & rHandler)
{
    doctok::Reference<Properties>::Pointer_t pRef =
        doctok::Reference<Properties>::Pointer_t();

    if (getCount() > 0)
        rHandler.data(get(0), getCount(), pRef);
}

string WW8BinaryObjReference::getType() const
{
    return "WW8BinaryObjReference";
}

void WW8BinaryObjHandler::data
(const sal_uInt8 * buf, size_t length,
 doctok::Reference<Properties>::Pointer_t /*pRef*/)
{
#if 1
    SubSequence<sal_uInt8> aSeq(buf, length);

    aSeq.dump(output);
#endif
}

sal_uInt32 WW8Sprm::getId() const
{
    sal_uInt32 nResult = 0;

    if (mpProperty.get() != NULL)
        nResult = mpProperty->getId();
    else if (mpBinary.get() != NULL)
        nResult = NS_rtf::LN_blob;

    return nResult;
}

string WW8Sprm::toString() const
{
    string sResult = "";

    if (mpProperty.get() != NULL)
        sResult = mpProperty->toString();

    return sResult;
}

Value::Pointer_t WW8Sprm::getValue()
{
    Value::Pointer_t pResult;

    if (mpProperty.get() != NULL)
        pResult = Value::Pointer_t(createValue(mpProperty->getParam()));

    return pResult;
}

doctok::Reference<BinaryObj>::Pointer_t WW8Sprm::getBinary()
{
    doctok::Reference<BinaryObj>::Pointer_t pResult;

    if (mpBinary.get() != NULL)
        pResult = doctok::Reference<BinaryObj>::Pointer_t(mpBinary->clone());
    else if (mpProperty.get() != NULL)
        pResult = createSprmBinary
            (dynamic_cast<WW8PropertyImpl &>(*(mpProperty.get())));

    return pResult;
}

doctok::Reference<Stream>::Pointer_t WW8Sprm::getStream()
{
    return doctok::Reference<Stream>::Pointer_t();
}

doctok::Reference<Properties>::Pointer_t WW8Sprm::getProps()
{
    doctok::Reference<Properties>::Pointer_t pResult;

    if (mpProperty.get() != NULL)
    {
        pResult = createSprmProps
            (dynamic_cast<WW8PropertyImpl &>(*(mpProperty.get())));
    }

    return pResult;
}

string WW8Sprm::getName() const
{
    return (*SprmIdToString::Instance())(getId());
}

int WW8Value::getInt() const
{
    return 0;
}

uno::Any WW8Value::getAny() const
{
    return uno::Any();
}

::rtl::OUString WW8Value::getString() const
{
    return ::rtl::OUString();
}

string WW8Value::toString() const
{
    return string();
}

doctok::Reference<Properties>::Pointer_t WW8Value::getProperties()
{
    return doctok::Reference<Properties>::Pointer_t();
}

doctok::Reference<Stream>::Pointer_t WW8Value::getStream()
{
    return doctok::Reference<Stream>::Pointer_t();
}

doctok::Reference<BinaryObj>::Pointer_t WW8Value::getBinary()
{
    return doctok::Reference<BinaryObj>::Pointer_t();
}

int WW8IntValue::getInt() const
{
    return mValue;
}

::rtl::OUString WW8IntValue::getString() const
{
    return ::rtl::OUString::valueOf(static_cast<sal_Int32>(mValue));
}

uno::Any WW8IntValue::getAny() const
{
    uno::Any aResult;

    aResult <<= static_cast<sal_uInt32>(mValue);

    return aResult;
}

string WW8IntValue::toString() const
{
    char sBuffer[255];

    snprintf(sBuffer, sizeof(sBuffer), "%x", mValue);

    return string(sBuffer);
}

WW8Value::Pointer_t createValue(int value)
{
    return WW8Value::Pointer_t(new WW8IntValue(value));
}

WW8Value::Pointer_t createValue(WW8Value::Pointer_t value)
{
    return value;
}

int WW8StringValue::getInt() const
{
    return 0;
}

::rtl::OUString WW8StringValue::getString() const
{
    return mString;
}

uno::Any WW8StringValue::getAny() const
{
    uno::Any aResult;

    aResult <<= mString;

    return aResult;
}

string WW8StringValue::toString() const
{
    string result;

    sal_uInt32 nCount = mString.getLength();
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        if (mString[n] <= 0xff && isprint(mString[n]))
        {
            sal_Unicode nC = mString[n];

            if (nC < 256)
                result += sal::static_int_cast<char>(nC);
            else
                result += ".";
        }
        else
        {
            char sBuffer[64];

            snprintf(sBuffer, sizeof(sBuffer), "\\u%04x", mString[n]);
            result += sBuffer;
        }
    }

    return result;
}

WW8Value::Pointer_t createValue(const rtl::OUString & rStr)
{
    return WW8Value::Pointer_t(new WW8StringValue(rStr));
}

doctok::Reference<Properties>::Pointer_t WW8PropertiesValue::getProperties()
{
    return mRef;
}

string WW8PropertiesValue::toString() const
{
    return "properties";
}

doctok::Reference<Stream>::Pointer_t WW8StreamValue::getStream()
{
    return mRef;
}

string WW8StreamValue::toString() const
{
    return "stream";
}

doctok::Reference<BinaryObj>::Pointer_t WW8BinaryObjValue::getBinary()
{
    return mRef;
}

string WW8BinaryObjValue::toString() const
{
    return "binaryObj";
}

WW8Value::Pointer_t createValue(doctok::Reference<Properties>::Pointer_t rRef)
{
    return WW8Value::Pointer_t(new WW8PropertiesValue(rRef));
}

WW8Value::Pointer_t createValue(doctok::Reference<Stream>::Pointer_t rRef)
{
    return WW8Value::Pointer_t(new WW8StreamValue(rRef));
}

WW8Value::Pointer_t createValue(doctok::Reference<BinaryObj>::Pointer_t rRef)
{
    return WW8Value::Pointer_t(new WW8BinaryObjValue(rRef));
}

WW8StreamHandler::WW8StreamHandler()
{
    output.addItem("<stream>");
    gTableManager.startLevel();
}

WW8StreamHandler::~WW8StreamHandler()
{
    gTableManager.endLevel();

    output.addItem("</stream>");
    output.closeGroup();
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
    string tmpStr = "<utext>";

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
            char sBuffer[256];

            snprintf(sBuffer, sizeof(sBuffer), "\\0x%04x", nChar);

            tmpStr += sBuffer;
        }
    }

    tmpStr += "</utext>";

    output.addItem(tmpStr);

    gTableManager.utext(data, len);
}

void WW8StreamHandler::props(doctok::Reference<Properties>::Pointer_t ref)
{
    WW8PropertiesHandler aHandler;

    output.addItem("<properties type=\"" + ref->getType() + "\">");
    ref->resolve(aHandler);

    //gTableManager.props(ref);

    output.addItem("</properties>");
}

void WW8StreamHandler::table(Id name, doctok::Reference<Table>::Pointer_t ref)
{
    WW8TableHandler aHandler;

    output.addItem("<table id=\"" + (*QNameToString::Instance())(name)
                   + "\">");

    try
    {
        ref->resolve(aHandler);
    }
    catch (doctok::Exception e)
    {
        output.addItem("<exception>" + e.getText() + "</exception>");
    }

    output.addItem("</table>");
}

void WW8StreamHandler::substream(Id name,
                                 doctok::Reference<Stream>::Pointer_t ref)
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

    doctok::Reference<Properties>::Pointer_t pProps = val.getProperties();

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

    doctok::Reference<Stream>::Pointer_t pStream = val.getStream();

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

    doctok::Reference<BinaryObj>::Pointer_t pBinObj = val.getBinary();

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
    snprintf(buffer, sizeof(buffer), "0x%lx", sprm_.getId());
    tmpStr += buffer;
    tmpStr += "\" name=\"";
    tmpStr += sprm_.getName();
    tmpStr += "\">";
    output.addItem(tmpStr);
    output.addItem(sprm_.toString());

    doctok::Reference<Properties>::Pointer_t pProps = sprm_.getProps();

    if (pProps.get() != NULL)
    {
        output.addItem("<properties type=\"" + pProps->getType() + "\">");
        pProps->resolve(*this);
        output.addItem("</properties>");
    }

    doctok::Reference<BinaryObj>::Pointer_t pBinObj = sprm_.getBinary();

    if (pBinObj.get() != NULL)
    {
        output.addItem("<binary>");
        WW8BinaryObjHandler aHandler;
        pBinObj->resolve(aHandler);
        output.addItem("</binary>");
    }

    doctok::Reference<Stream>::Pointer_t pStream = sprm_.getStream();

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

void WW8TableHandler::entry(int /*pos*/, doctok::Reference<Properties>::Pointer_t ref)
{
    output.addItem("<tableentry>");

    WW8PropertiesHandler aHandler;

    try
    {
        ref->resolve(aHandler);
    }
    catch (doctok::Exception e)
    {
        output.addItem("<exception>" + e.getText() + "</exception>");
        output.addItem("</tableentry>");

        throw e;
    }

    output.addItem("</tableentry>");
}

Stream::Pointer_t createStreamHandler()
{
    return Stream::Pointer_t(new WW8StreamHandler());
}

Stream::Pointer_t createAnalyzer()
{
    return Stream::Pointer_t(new WW8Analyzer());
}

QNameToString::Pointer_t QNameToString::pInstance;

QNameToString::Pointer_t QNameToString::Instance()
{
    if (pInstance.get() == NULL)
        pInstance = QNameToString::Pointer_t(new QNameToString());

    return pInstance;
}

string QNameToString::operator()(writerfilter::QName_t qName)
{
    return mMap[qName];
}

void dump(OutputWithDepth<string> & /*o*/, const char * /*name*/,
          doctok::Reference<Properties>::Pointer_t /*props*/)
{
}

void dump(OutputWithDepth<string> & o, const char * name, sal_uInt32 n)
{
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%ld", n);
    string tmpStr = name;
    tmpStr += "=";
    tmpStr += sBuffer;

    o.addItem(tmpStr);
}

}

