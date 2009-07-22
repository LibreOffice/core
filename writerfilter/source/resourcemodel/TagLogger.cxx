/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TagLogger.cxx,v $
 * $Revision: 1.2 $
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

    void XMLTag::addTag(XMLTag::Pointer_t pTag)
    {
        if (pTag != XMLTag::Pointer_t())
            mTags.push_back(pTag);
    }

    void XMLTag::chars(const string & rChars)
    {
        mChars = rChars;
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

    ostream & XMLTag::output(ostream & o) const
    {
        if (mMode == START || mMode == COMPLETE)
        {
            o << "<" << mTag;

            XMLAttributes_t::const_iterator aItAttrs(mAttrs.begin());
            while (aItAttrs != mAttrs.end())
            {
                o << " " << aItAttrs->mName << "=\""
                << xmlify(aItAttrs->mValue)
                << "\"";

                aItAttrs++;
            }

            o << ">";

            XMLTags_t::const_iterator aItTags(mTags.begin());
            while (aItTags != mTags.end())
            {
                (*aItTags)->output(o);
                aItTags++;
            }

            o << mChars;
        }

        if (mMode == END || mMode == COMPLETE)
            o << "</" << mTag << ">";

        return o;
    }

    struct eqstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) == 0;
        }
    };

    typedef hash_map<const char *, TagLogger::Pointer_t, hash<const char *>, eqstr> TagLoggerHashMap_t;
    static TagLoggerHashMap_t * tagLoggers = NULL;

    TagLogger::TagLogger()
    {
    }

    TagLogger::~TagLogger()
    {
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
        attribute(name, OUStringToOString(value, RTL_TEXTENCODING_ASCII_US).getStr());
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
        string nameRemoved;
        bool found = false;
        do
        {
            nameRemoved = currentTag()->getTag();
            mTags.pop();

            if (name == nameRemoved)
                found = true;
            else
                found = false; // for debugging
        }
        while (! found && ! mTags.empty());
    }

    void TagLogger::endDocument()
    {
        mTags.pop();
    }

    ostream & TagLogger::output(ostream & o) const
    {
        return currentTag()->output(o);
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

            fileName += "/writerfilter.";
            fileName += name;
            fileName += ".xml";

            ofstream dumpStream(fileName.c_str());
            aIt->second->output(dumpStream);
        }
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
                 "0x%" SAL_PRIxUINT32 "x, %" SAL_PRIxUINT32 "d", rSprm.getId(),
                 rSprm.getId());
        pTag->addAttr("id", sBuffer);
        pTag->addAttr("value", rSprm.getValue()->toString());

        resolve(*pTag, rSprm.getProps());

        mpTag->addTag(pTag);
    }


}
