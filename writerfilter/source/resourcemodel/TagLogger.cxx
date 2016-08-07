/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <fstream>
#include <string.h>
#include <resourcemodel/TagLogger.hxx>
#include <resourcemodel/util.hxx>
#include <resourcemodel/QNameToString.hxx>

namespace writerfilter
{
XMLTag::Pointer_t XMLTag::NIL(new XMLTag("NIL"));

void XMLTag::addAttr(string sName, string sValue)
{
    XMLAttribute aAttr(sName, sValue);

    mAttrs.push_back(aAttr);
}

void XMLTag::addAttr(string sName, const ::rtl::OUString & sValue)
{
    addAttr(sName,
            OUStringToOString
            (sValue, RTL_TEXTENCODING_ASCII_US).getStr());
}

void XMLTag::addAttr(string sName, sal_uInt32 nValue)
{
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%" SAL_PRIdINT32, nValue);
    addAttr(sName, buffer);
}

void XMLTag::addAttr(string sName, uno::Any aAny)
{
    string aTmpStrInt;
    string aTmpStrFloat;
    string aTmpStrString;

    static char buffer[256];

    try
    {
        sal_Int32 nInt = 0;
        aAny >>= nInt;

        snprintf(buffer, sizeof(buffer), "%" SAL_PRIdINT32,
                 nInt);

        aTmpStrInt = buffer;
    }
    catch (uno::Exception aExcept)
    {
        aTmpStrInt = "exception";
    }

    try
    {
        float nFloat = 0.0;
        aAny >>= nFloat;

        snprintf(buffer, sizeof(buffer), "%f",
                 nFloat);

        aTmpStrFloat = buffer;
    }
    catch (uno::Exception aExcept)
    {
        aTmpStrFloat = "exception";
    }

    try
    {
        ::rtl::OUString aStr;
        aAny >>= aStr;

        aTmpStrString = OUStringToOString(aStr, RTL_TEXTENCODING_ASCII_US).getStr();
    }
    catch (uno::Exception aExcept)
    {
        aTmpStrString = "exception";
    }

    addAttr(sName, "i:" + aTmpStrInt + " f:" + aTmpStrFloat + " s:" +
            aTmpStrString);
}

void XMLTag::addTag(XMLTag::Pointer_t pTag)
{
    if (pTag != XMLTag::Pointer_t())
        mTags.push_back(pTag);
}

void XMLTag::chars(const string & rChars)
{
    mChars += rChars;
}

void XMLTag::chars(const ::rtl::OUString & rChars)
{
    chars(OUStringToOString(rChars, RTL_TEXTENCODING_ASCII_US).getStr());
}

const string & XMLTag::getTag() const
{
    return mTag;
}

string XMLTag::toString() const
{
    if (mChars.length() > 0)
        return mChars;

    string sResult;

    if (mMode == START || mMode == COMPLETE)
    {
        sResult += "<" + mTag;

        XMLAttributes_t::const_iterator aIt = mAttrs.begin();
        while (aIt != mAttrs.end())
        {
            sResult += " ";
            sResult += aIt->mName;
            sResult += "=\"";
            sResult += xmlify(aIt->mValue);
            sResult += "\"";

            aIt++;
        }

        sResult +=">";

        if (mTags.size() > 0)
        {
            XMLTags_t::const_iterator aItTags = mTags.begin();
            while (aItTags != mTags.end())
            {
                if ((*aItTags).get() != NULL)
                    sResult += (*aItTags)->toString();

                aItTags++;
            }
        }
    }

    if (mMode == END || mMode == COMPLETE)
        sResult += "</" + mTag + ">";

    return sResult;
}

string XMLTag::toTree(const string & sIndent) const
{
    if (mChars.length() > 0)
        return sIndent + mChars;

    string sResult;

    {
        size_t nSize = sIndent.size();
        if (nSize > 1)
        {
            sResult += sIndent.substr(0, nSize - 2) + "+-\\" + mTag;
        }
        else
        {
            sResult += "\\" + mTag;
        }
    }

    XMLAttributes_t::const_iterator aIt = mAttrs.begin();
    while (aIt != mAttrs.end())
    {
        if (aIt == mAttrs.begin())
        {
            sResult += "(";
        }
        else
        {
            sResult += sIndent + ", ";
        }

        sResult += aIt->mName;
        sResult += "=";
        sResult += aIt->mValue;

        aIt++;

        if (aIt == mAttrs.end())
        {
            sResult += ")";
        }
    }

    sResult += "\n";

    if (mTags.size() > 0)
    {
        XMLTags_t::const_iterator aItTags = mTags.begin();
        size_t nSize = mTags.size();
        while (aItTags != mTags.end())
        {
            if ((*aItTags).get() != NULL)
            {
                if (nSize == 1)
                {
                    sResult += (*aItTags)->toTree(sIndent + "  ");
                }
                else
                {
                    sResult += (*aItTags)->toTree(sIndent + "| ");
                }
            }

            aItTags++;
            nSize--;
        }
    }

    return sResult;
}

ostream & XMLTag::output(ostream & o, const string & sIndent) const
{
    bool bHasContent = mChars.size() > 0 || mTags.size() > 0;

    if (mMode == START || mMode == COMPLETE)
    {
        o << sIndent << "<" << mTag;

        XMLAttributes_t::const_iterator aItAttrs(mAttrs.begin());
        while (aItAttrs != mAttrs.end())
        {
            o << " " << aItAttrs->mName << "=\""
              << xmlify(aItAttrs->mValue)
              << "\"";

            aItAttrs++;
        }

        if (bHasContent)
        {
            o << ">";

            string sNewIndent = sIndent + "  ";
            XMLTags_t::const_iterator aItTags(mTags.begin());
            while (aItTags != mTags.end())
            {
                if (aItTags == mTags.begin())
                    o << endl;

                (*aItTags)->output(o, sNewIndent);
                aItTags++;
            }

            o << mChars;
        }
    }

    if (mMode == END || mMode == COMPLETE)
    {
        if (bHasContent)
        {
            if (mTags.size() > 0)
                o << sIndent;

            o << "</" << mTag << ">" << endl;
        }
        else
            o << "/>" << endl;
    }

    return o;
}

typedef hash_map< const char*, TagLogger::Pointer_t, rtl::CStringHash, rtl::CStringEqual> TagLoggerHashMap_t;
static TagLoggerHashMap_t * tagLoggers = NULL;

TagLogger::TagLogger()
: mFileName("writerfilter")
{
}

TagLogger::~TagLogger()
{
}

void TagLogger::setFileName(const string & rName)
{
    mFileName = rName;
}

TagLogger::Pointer_t TagLogger::getInstance(const char * name)
{
    if (tagLoggers == NULL)
        tagLoggers = new TagLoggerHashMap_t();

    TagLoggerHashMap_t::iterator aIt = tagLoggers->end();

    if (! tagLoggers->empty())
        aIt = tagLoggers->find(name);

    if (aIt == tagLoggers->end())
    {
        TagLogger::Pointer_t pTagLogger(new TagLogger());
        pair<const char *, TagLogger::Pointer_t> entry(name, pTagLogger);
        aIt = tagLoggers->insert(entry).first;
    }

    return aIt->second;
}

XMLTag::Pointer_t TagLogger::currentTag() const
{
    bool bEmpty=mTags.empty();
    if (!bEmpty)
        return mTags.top();

    return XMLTag::NIL;
}

void TagLogger::startDocument()
{
    XMLTag::Pointer_t pTag(new XMLTag("root"));
    mTags.push(pTag);
    mpRoot = pTag;
}

void TagLogger::element(const string & name)
{
    startElement(name);
    endElement(name);
}

void TagLogger::startElement(const string & name)
{
    XMLTag::Pointer_t pTag(new XMLTag(name));
    currentTag()->addTag(pTag);
    mTags.push(pTag);
}

void TagLogger::attribute(const string & name, const string & value)
{
    currentTag()->addAttr(name, value);
}

void TagLogger::attribute(const string & name, const ::rtl::OUString & value)
{
    currentTag()->addAttr(name, value);
}

void TagLogger::attribute(const string & name, sal_uInt32 value)
{
    currentTag()->addAttr(name, value);
}

void TagLogger::attribute(const string & name, const uno::Any aAny)
{
    currentTag()->addAttr(name, aAny);
}

void TagLogger::addTag(XMLTag::Pointer_t pTag)
{
    currentTag()->addTag(pTag);
}

void TagLogger::chars(const string & rChars)
{
    currentTag()->chars(xmlify(rChars));
}

void TagLogger::chars(const ::rtl::OUString & rChars)
{
    chars(OUStringToOString(rChars, RTL_TEXTENCODING_ASCII_US).getStr());
}

void TagLogger::endElement(const string & name)
{
    string nameRemoved = currentTag()->getTag();

    if (name == nameRemoved)
        mTags.pop();
    else {
        XMLTag::Pointer_t pTag(new XMLTag("end.mismatch"));
        pTag->addAttr("name", name);
        pTag->addAttr("top", nameRemoved);

        currentTag()->addTag(pTag);
    }

}

void TagLogger::endDocument()
{
    mTags.pop();
}

ostream & TagLogger::output(ostream & o) const
{
    return mpRoot->output(o);
}

void TagLogger::dump(const char * name)
{
    TagLoggerHashMap_t::iterator aIt(tagLoggers->find(name));
    if (aIt != tagLoggers->end())
    {
        string fileName;
        char * temp = getenv("TAGLOGGERTMP");

        if (temp != NULL)
            fileName += temp;
        else
            fileName += "/tmp";

        string sPrefix = aIt->second->mFileName;
        size_t nLastSlash = sPrefix.find_last_of('/');
        size_t nLastBackslash = sPrefix.find_last_of('\\');
        size_t nCutPos = nLastSlash;
        if (nLastBackslash < nCutPos)
            nCutPos = nLastBackslash;
        if (nCutPos < sPrefix.size())
            sPrefix = sPrefix.substr(nCutPos + 1);

        fileName += "/";
        fileName += sPrefix;
        fileName +=".";
        fileName += name;
        fileName += ".xml";

        ofstream dumpStream(fileName.c_str());
        aIt->second->output(dumpStream);
    }
}

PropertySetToTagHandler::PropertySetToTagHandler(IdToString::Pointer_t pIdToString)
  : mpTag(new XMLTag("propertyset")), mpIdToString(pIdToString)
{
}

PropertySetToTagHandler::~PropertySetToTagHandler()
{
}

void PropertySetToTagHandler::resolve
(XMLTag & rTag, writerfilter::Reference<Properties>::Pointer_t pProps)
{
    if (pProps.get() != NULL)
    {
        PropertySetToTagHandler aHandler(mpIdToString);
        pProps->resolve(aHandler);
        rTag.addTag(aHandler.getTag());
    }
}

void PropertySetToTagHandler::attribute(Id name, Value & val)
{
    XMLTag::Pointer_t pTag(new XMLTag("attribute"));

    pTag->addAttr("name", (*QNameToString::Instance())(name));
    pTag->addAttr("value", val.toString());

    resolve(*pTag, val.getProperties());

    mpTag->addTag(pTag);
}

void PropertySetToTagHandler::sprm(Sprm & rSprm)
{
    XMLTag::Pointer_t pTag(new XMLTag("sprm"));

    string sName;

    if (mpIdToString != IdToString::Pointer_t())
        sName = mpIdToString->toString(rSprm.getId());

    pTag->addAttr("name", sName);

    static char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer),
             "0x%" SAL_PRIxUINT32 ", %" SAL_PRIuUINT32, rSprm.getId(),
             rSprm.getId());
    pTag->addAttr("id", sBuffer);
    pTag->addAttr("value", rSprm.getValue()->toString());

    resolve(*pTag, rSprm.getProps());

    mpTag->addTag(pTag);
}


XMLTag::Pointer_t unoPropertySetToTag(uno::Reference<beans::XPropertySet> rPropSet)
{
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());
    uno::Sequence<beans::Property> aProps(xPropSetInfo->getProperties());

    XMLTag::Pointer_t pResult(new XMLTag("unoPropertySet"));

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        XMLTag::Pointer_t pPropTag(new XMLTag("property"));

        ::rtl::OUString sName(aProps[i].Name);

        pPropTag->addAttr("name", sName);
        try
        {
            pPropTag->addAttr("value", rPropSet->getPropertyValue(sName));
        }
        catch (uno::Exception aException)
        {
            XMLTag::Pointer_t pException(new XMLTag("exception"));

            pException->chars("getPropertyValue(\"");
            pException->chars(sName);
            pException->chars("\")");
            pPropTag->addTag(pException);
        }

        pResult->addTag(pPropTag);
    }

    return pResult;
}

}
