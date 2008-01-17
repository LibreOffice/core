/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Analyzer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-17 14:28:52 $
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

#include <stdlib.h>
#include <fstream>
#include <WW8Analyzer.hxx>
#include <doctok/resourceids.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

namespace writerfilter
{

using text::TextContentAnchorType;

bool eqSalUInt32::operator () (sal_uInt32 n1, sal_uInt32 n2) const
{
    return n1 == n2;
}

WW8Analyzer::WW8Analyzer()
{
}

WW8Analyzer::~WW8Analyzer()
{
    dumpStats(cout);
}

void WW8Analyzer::attribute(Id name, Value & val)
{
    string aAttrName = (*QNameToString::Instance())(name);
    string aStr;

    if (aAttrName.length() > 6)
        aStr = aAttrName.substr(4, 2);
    else
        logger("DEBUG", "WW8Analyzer::attribute:" + aAttrName);

    bool bAdd = false;
    if (aStr.compare("LC") == 0 || aStr.compare("FC") == 0)
    {
        if (val.getInt() != 0)
        {
            bAdd = true;
        }
    }
    else
    {
        bAdd = true;
    }

    if (bAdd)
    {
        if (mAttributeMap.count(name) > 0)
        {
            sal_uInt32 nCount = mAttributeMap[name] + 1;
            mAttributeMap[name] = nCount;
        }
        else
            mAttributeMap[name] = 1;

        mAttributeIdSet.insert(name);
    }
}

void WW8Analyzer::sprm(Sprm & sprm_)
{
    if (mSprmMap.count(sprm_.getId()) > 0)
    {
        sal_uInt32 nCount = mSprmMap[sprm_.getId()] + 1;
        mSprmMap[sprm_.getId()] = nCount;
    }
    else
        mSprmMap[sprm_.getId()] = 1;

    mSprmIdSet.insert(sprm_.getId());

    writerfilter::Reference<Properties>::Pointer_t pProps = sprm_.getProps();

    if (pProps.get() != NULL)
    {
        pProps->resolve(*this);
    }

}

void WW8Analyzer::entry(int /*pos*/, ::writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::data(const sal_uInt8 * /*buf*/, size_t /*len*/,
                       ::writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}

void WW8Analyzer::startSectionGroup()
{
}

void WW8Analyzer::endSectionGroup()
{
}

void WW8Analyzer::startParagraphGroup()
{
}

void WW8Analyzer::endParagraphGroup()
{
}

void WW8Analyzer::startCharacterGroup()
{
}

void WW8Analyzer::endCharacterGroup()
{
}

void WW8Analyzer::text(const sal_uInt8 * /*data*/, size_t /*len*/)
{
}

void WW8Analyzer::utext(const sal_uInt8 * /*data*/, size_t /*len*/)
{
}

void WW8Analyzer::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::table(Id /*id*/, writerfilter::Reference<Table>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::substream(Id /*name*/,
                            writerfilter::Reference<Stream>::Pointer_t ref)
{
    ref->resolve(*this);
}

void WW8Analyzer::info(const string & /*info*/)
{
}

void WW8Analyzer::dumpStats(ostream & o) const
{
    {
        for (IdSet::const_iterator aIt = mSprmIdSet.begin();
             aIt != mSprmIdSet.end(); aIt++)
        {
            sal_uInt32 aId = *aIt;

            o << "<sprm>" << endl
              << "<id>" << hex << aId << "</id>" << endl
              << "<name>" << (*SprmIdToString::Instance())(aId)
              << "</name>"  << endl
              << "<count>" << dec << mSprmMap[aId] << "</count>"
              << endl
              << "</sprm>" << endl;
        }
    }

    {
        for (IdSet::const_iterator aIt = mAttributeIdSet.begin();
             aIt != mAttributeIdSet.end(); aIt++)
        {
            sal_uInt32 aId = *aIt;

            o << "<attribute>" << endl
              << "<name>" << (*QNameToString::Instance())(aId) << "</name>"
              << endl
              << "<count>" << dec << mAttributeMap[aId] << "</count>"
              << endl
              << "</attribute>" << endl;
        }
    }

}

Stream::Pointer_t createAnalyzer()
{
    return Stream::Pointer_t(new WW8Analyzer());
}

static string & logger_file()
{
    static string _logger_file = string(getenv("TEMP")?getenv("TEMP"):"/tmp") + "/writerfilter.ooxml.tmp";
    return _logger_file;
}

static ofstream & logger_stream()
{
    static ofstream _logger_stream(logger_file().c_str());
    return _logger_stream;
}


void logger(string prefix, string message)
{
    logger_stream() << prefix <<  ":" << message << endl;
}

string propertysetToString(uno::Reference<beans::XPropertySet> const & xPropSet)
{
    string sResult;

    static int nAttribNames = 9;
    static string sPropertyAttribNames[9] =
        {
            "MAYBEVOID",
            "BOUND",
            "CONSTRAINED",
            "TRANSIENT",
            "READONLY",
            "MAYBEAMBIGUOUS",
            "MAYBEDEFAULT",
            "REMOVEABLE",
            "OPTIONAL"
        };

    uno::Reference<beans::XPropertySetInfo> xPropSetInfo
        (xPropSet->getPropertySetInfo());

    if (xPropSetInfo.is())
    {
        uno::Sequence<beans::Property> aProps(xPropSetInfo->getProperties());

        sResult +="<propertyset>";

        for (sal_Int32 n = 0; n < aProps.getLength(); n++)
        {
            ::rtl::OUString sPropName(aProps[n].Name);

            if (xPropSetInfo->hasPropertyByName(sPropName))
            {
                uno::Any aAny;
                try
                {
                    xPropSet->getPropertyValue(sPropName) >>= aAny;
                }
                catch (beans::UnknownPropertyException)
                {
                    sResult += "<unknown-property>";
                    sResult += OUStringToOString
                        (sPropName, RTL_TEXTENCODING_ASCII_US).getStr();
                    sResult += "</unknown-property>";
                }

                if (aAny.hasValue())
                {
                    sResult += "<property name=\"";
                    sResult += OUStringToOString
                        (sPropName, RTL_TEXTENCODING_ASCII_US).getStr();
                    sResult +="\" type=\"";

                    ::rtl::OUString sPropType(aProps[n].Type.getTypeName());
                    sResult += OUStringToOString
                        (sPropType, RTL_TEXTENCODING_ASCII_US).getStr();

                    sResult += "\" attribs=\"";

                    sal_uInt16 nMask = 1;
                    bool bFirstAttrib = true;
                    sal_uInt16 nAttribs = aProps[n].Attributes;
                    for (int i = 0; i < nAttribNames; i++)
                    {
                        if ((nAttribs & nMask) != 0)
                        {
                            if (bFirstAttrib)
                                bFirstAttrib = false;
                            else
                                sResult += "|";

                            sResult += sPropertyAttribNames[i];
                        }

                        nMask <<= 1;
                    }

                    sResult += "\">";

                    char buffer[256];
                    if (sPropType ==
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                        ("byte")))
                    {
                        sal_Int8 nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%d", nValue);
                        sResult += buffer;
                    }
                    if (sPropType ==
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                        ("short")))
                    {
                        sal_Int16 nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%d", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("long")))
                    {
                        sal_Int32 nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%ld", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("float")))
                    {
                        float nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%f", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("double")))
                    {
                        double nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%lf", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("boolean")))
                    {
                        sal_Bool nValue = sal_Bool();
                        aAny >>= nValue;

                        if (nValue)
                            sResult += "true";
                        else
                            sResult += "false";
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("string")))
                    {
                        ::rtl::OUString sValue;
                        aAny >>= sValue;

                        sResult += OUStringToOString
                            (sValue, RTL_TEXTENCODING_ASCII_US).getStr();
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.text.TextContentAnchorType")))
                    {
                        text::TextContentAnchorType nValue;
                        aAny >>= nValue;

                        switch (nValue)
                        {
                        case text::TextContentAnchorType_AT_PARAGRAPH:
                            sResult += "AT_PARAGRAPH";
                            break;
                        case text::TextContentAnchorType_AS_CHARACTER:
                            sResult += "AS_CHARACTER";
                            break;
                        case text::TextContentAnchorType_AT_PAGE:
                            sResult += "AT_PAGE";
                            break;
                        case text::TextContentAnchorType_AT_FRAME:
                            sResult += "AT_FRAME";
                            break;
                        case text::TextContentAnchorType_AT_CHARACTER:
                            sResult += "AT_CHARACTER";
                            break;
                        case text::TextContentAnchorType_MAKE_FIXED_SIZE:
                            sResult += "MAKE_FIXED_SIZE";
                            break;
                        default:
                            break;
                        }
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.awt.Point")))
                    {
                        awt::Point aPoint;
                        aAny >>= aPoint;

                        snprintf(buffer, sizeof(buffer), "(%ld, %ld)", aPoint.X,
                                 aPoint.Y);

                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.awt.Rectangle")))
                    {
                        awt::Rectangle aRect;
                        aAny >>= aRect;

                        snprintf(buffer, sizeof(buffer), "(%ld, %ld, %ld, %ld)",
                                 aRect.X, aRect.Y, aRect.Width, aRect.Height);
                        sResult += buffer;
                    }

                    sResult += "</property>";
                }
            }
        }
        sResult += "</propertyset>";
    }

    return sResult;
}

}
