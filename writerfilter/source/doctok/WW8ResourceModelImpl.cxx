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

#include <WW8ResourceModelImpl.hxx>
#include <resources.hxx>
#include <WW8OutputWithDepth.hxx>
#include <resourcemodel/TableManager.hxx>
#include <rtl/string.hxx>
#include <resourcemodel/QNameToString.hxx>

namespace writerfilter {

namespace doctok
{
using namespace ::std;


// ------- WW8TableDataHandler ---------

typedef WW8PropertySet::Pointer_t TablePropsPointer_t;

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
    virtual bool sprm(Sprm & rSprm);
};

WW8TableManager::WW8TableManager()
{
    TableDataHandler<string, TablePropsPointer_t>::Pointer_t pHandler(new WW8TableDataHandler());
    setHandler(pHandler);
}

bool WW8TableManager::sprm(Sprm & rSprm)
{
    TableManager<string, TablePropsPointer_t>::sprm(rSprm);
    output.setDepth(getTableDepthNew());
    return true;
}

void WW8TableManager::endParagraphGroup()
{
    string tmpStr = "<tabledepth depth=\"";
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIdINT32, getTableDepthNew());
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

writerfilter::Reference<BinaryObj>::Pointer_t
WW8BinaryObjReference::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t
        (new WW8BinaryObjReference(*this));
}

void WW8BinaryObjReference::resolve(BinaryObj & rHandler)
{
    writerfilter::Reference<Properties>::Pointer_t pRef =
        writerfilter::Reference<Properties>::Pointer_t();

    if (getCount() > 0)
        rHandler.data(get(0), getCount(), pRef);
}

string WW8BinaryObjReference::getType() const
{
    return "WW8BinaryObjReference";
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

writerfilter::Reference<BinaryObj>::Pointer_t WW8Sprm::getBinary()
{
    writerfilter::Reference<BinaryObj>::Pointer_t pResult;

    if (mpBinary.get() != NULL)
        pResult = writerfilter::Reference<BinaryObj>::Pointer_t
            (mpBinary->clone());
    else if (mpProperty.get() != NULL)
        pResult = createSprmBinary
            (dynamic_cast<WW8PropertyImpl &>(*(mpProperty.get())));

    return pResult;
}

writerfilter::Reference<Stream>::Pointer_t WW8Sprm::getStream()
{
    return writerfilter::Reference<Stream>::Pointer_t();
}

writerfilter::Reference<Properties>::Pointer_t WW8Sprm::getProps()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (mpProperty.get() != NULL)
    {
        pResult = createSprmProps
            (dynamic_cast<WW8PropertyImpl &>(*(mpProperty.get())));
    }

    return pResult;
}

Sprm::Kind WW8Sprm::getKind()
{
    return SprmKind(getId());
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

writerfilter::Reference<Properties>::Pointer_t WW8Value::getProperties()
{
    return writerfilter::Reference<Properties>::Pointer_t();
}

writerfilter::Reference<Stream>::Pointer_t WW8Value::getStream()
{
    return writerfilter::Reference<Stream>::Pointer_t();
}

writerfilter::Reference<BinaryObj>::Pointer_t WW8Value::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
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

writerfilter::Reference<Properties>::Pointer_t 
WW8PropertiesValue::getProperties()
{
    return mRef;
}

string WW8PropertiesValue::toString() const
{
    return "properties";
}

writerfilter::Reference<Stream>::Pointer_t WW8StreamValue::getStream()
{
    return mRef;
}

string WW8StreamValue::toString() const
{
    return "stream";
}

writerfilter::Reference<BinaryObj>::Pointer_t WW8BinaryObjValue::getBinary()
{
    return mRef;
}

string WW8BinaryObjValue::toString() const
{
    return "binaryObj";
}

WW8Value::Pointer_t createValue
(writerfilter::Reference<Properties>::Pointer_t rRef)
{
    return WW8Value::Pointer_t(new WW8PropertiesValue(rRef));
}

WW8Value::Pointer_t createValue(writerfilter::Reference<Stream>::Pointer_t rRef)
{
    return WW8Value::Pointer_t(new WW8StreamValue(rRef));
}

WW8Value::Pointer_t createValue
(writerfilter::Reference<BinaryObj>::Pointer_t rRef)
{
    return WW8Value::Pointer_t(new WW8BinaryObjValue(rRef));
}


}

}

