/*************************************************************************
 *
 *  $RCSfile: elementparser.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:43:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "elementparser.hxx"

#ifndef CONFIGMGR_XML_STRINGS_HXX_
#include "xmlstrings.hxx"
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif

#include <drafts/com/sun/star/configuration/backend/SchemaAttribute.hpp>
#include <drafts/com/sun/star/configuration/backend/NodeAttribute.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace sax       = ::com::sun::star::xml::sax;
// -----------------------------------------------------------------------------

static
inline
sal_Int16 impl_getIndexByName(uno::Reference< sax::XAttributeList > const& xAttribs, OUString const& aAttributeName)
{
    OSL_PRECOND( xAttribs.is(), "ERROR: NULL Attribute list");

    sal_Int16 nIndex = xAttribs->getLength();

    while (--nIndex >= 0)
    {
        if (xAttribs->getNameByIndex(nIndex).equals(aAttributeName))
            break;
    }
    // nIndex == -1 if not found

    return nIndex;
}
// -----------------------------------------------------------------------------
static
inline
bool impl_maybeGetAttribute(uno::Reference< sax::XAttributeList > const& xAttribs, OUString const& aAttributeName, /* OUT */ OUString& rAttributeValue)
{
    OSL_PRECOND( xAttribs.is(), "ERROR: NULL Attribute list");

    OUString aValue = xAttribs->getValueByName(aAttributeName);
    if( aValue.getLength()!=0)
    {
        rAttributeValue = aValue;
        return true;
    }
    return false;
}
// -----------------------------------------------------------------------------

/// retrieve the (almost) complete information for an element
ElementInfo ElementParser::parseElementInfo(OUString const& _sTag, SaxAttributeList const& _xAttribs) const
{
    ElementType::Enum aType = this->getNodeType(_sTag,_xAttribs);

    ElementInfo aInfo( this->getName(_sTag,_xAttribs,aType), aType );

    aInfo.op    = this->getOperation(_xAttribs,aType);
    aInfo.flags = this->getNodeFlags(_xAttribs,aType);

    return aInfo;
}
// -----------------------------------------------------------------------------

ElementType::Enum ElementParser::getNodeType(OUString const& _sElementName, SaxAttributeList const& _xAttribs) const
{
    OSL_PRECOND( _xAttribs.is(), "ERROR: NULL Attribute list");

  // todo: make this use a table, if necessary
    ElementType::Enum eResult = ElementType::unknown;
    if (_sElementName.equals(TAG_VALUE))
        eResult = ElementType::value;

    else if (_sElementName.equals(TAG_PROP))
        eResult = ElementType::property;

    else if (_sElementName.equals(TAG_NODE))
        eResult = ElementType::node;

    else if (_sElementName.equals(TAG_GROUP))
        eResult = ElementType::group;

    else if (_sElementName.equals(TAG_SET))
        eResult = ElementType::set;

    else if (_sElementName.equals(TAG_INSTANCE))
        eResult = ElementType::instance;

    else if (_sElementName.equals(TAG_ITEMTYPE))
        eResult = ElementType::item_type;

    else if (_sElementName.equals(TAG_IMPORT))
        eResult = ElementType::import;

    else if (_sElementName.equals(TAG_LAYER))
        eResult = ElementType::layer;

    else if (_sElementName.equals(TAG_SCHEMA))
        eResult = ElementType::schema;

    else if (_sElementName.equals(TAG_COMPONENT))
        eResult = ElementType::component;

    else if (_sElementName.equals(TAG_TEMPLATES))
        eResult = ElementType::templates;

    else if (_sElementName.equals(TAG_USES))
        eResult = ElementType::uses;

    else
        eResult = ElementType::other;

    return eResult;
}
// -----------------------------------------------------------------------------

/// takes the node name from either an attribute or the element name
OUString ElementParser::getName(OUString const& _sElementName, SaxAttributeList const& _xAttribs, ElementType::Enum _eType) const
{
    OUString aName;
    OUString aPackage;

    bool bNameFound = this->maybeGetAttribute(_xAttribs, ATTR_NAME, aName);
    bool bPackage = false;

    switch (_eType)
    {
    case ElementType::schema:
        bPackage = this->maybeGetAttribute(_xAttribs,ATTR_PACKAGE,aPackage);
        OSL_ENSURE(bPackage, "configmgr::xml::ElementParser: Found schema without package.");
        break;

    case ElementType::layer:
        bPackage =  this->maybeGetAttribute(_xAttribs,ATTR_PACKAGE,aPackage);

        if (!bPackage) // for compatibility we still support 'oor:context'
        {
            bPackage =  this->maybeGetAttribute(_xAttribs,ATTR_CONTEXT,aPackage);

            if (bPackage)
            {
                OSL_TRACE("configmgr::xml::ElementParser: Found obsolete layer attribute "
                          "oor:context=\"%s\" in component \"%s\".\n",
                          rtl::OUStringToOString(aPackage,RTL_TEXTENCODING_ASCII_US).getStr(),
                          rtl::OUStringToOString(aName,RTL_TEXTENCODING_ASCII_US).getStr());
            }
        }

        OSL_ENSURE(bPackage, "configmgr::xml::ElementParser: Found layer without package.");
        break;

    case ElementType::node:
    case ElementType::set:
    case ElementType::group:
    case ElementType::instance:
    case ElementType::property:
        break;

    // these have no name to speak of
    case ElementType::value:
    case ElementType::item_type:
    case ElementType::import:
    case ElementType::uses:
    case ElementType::templates:
    case ElementType::component:
        OSL_ENSURE(!bNameFound, "Configuration Parser: Unexpected name attribute is ignored\n");
        return _sElementName;

    // for unknown prefer name to
    case ElementType::unknown:
        if (!bNameFound) return _sElementName;

        bPackage = this->maybeGetAttribute(_xAttribs,ATTR_PACKAGE,aPackage);
        break;

    default:
        if (!bNameFound) return _sElementName;
        break;
    }

    OSL_ENSURE(aName.getLength(),"Found empty name tag on element");

    if (bPackage)
    {
        static const sal_Unicode chPackageSep = '.';

        aName = aPackage.concat(OUString(&chPackageSep,1)).concat(aName);
    }
    else
    {
        OSL_ENSURE(!this->maybeGetAttribute(_xAttribs,ATTR_PACKAGE,aPackage),
                   "configmgr::xml::ElementParser: Found unexpected 'oor:package' on inner or unknown node." );
    }

    return aName;
}
// -----------------------------------------------------------------------------

Operation::Enum ElementParser::getOperation(SaxAttributeList const& xAttribs,ElementType::Enum _eType) const
{
    OUString sOpName;
    if ((_eType != ElementType::property) && (_eType !=ElementType::node))
    {
        return Operation::none;
    }

    if ( !this->maybeGetAttribute(xAttribs,ATTR_OPERATION, sOpName) )
        return Operation::none;

    if (sOpName.equals(OPERATION_MODIFY))
        return Operation::modify;

    else if (sOpName.equals(OPERATION_REPLACE))
        return Operation::replace;

    else if (sOpName.equals(OPERATION_REMOVE))
        return Operation::remove;

    else
        return Operation::unknown;
}
// -----------------------------------------------------------------------------


/// retrieve the locale stored in the attribute list
bool ElementParser::getLanguage(SaxAttributeList const& xAttribs, OUString& _rsLanguage) const
{
    return this->maybeGetAttribute(xAttribs, EXT_ATTR_LANGUAGE, _rsLanguage);
}
// -----------------------------------------------------------------------------

/// reads attributes for nodes from the attribute list
ElementInfo::FlagsType ElementParser::getNodeFlags(SaxAttributeList const& xAttribs,ElementType::Enum _eType) const
{
    namespace NodeAttribute   = drafts::com::sun::star::configuration::backend::NodeAttribute;
    namespace SchemaAttribute = drafts::com::sun::star::configuration::backend::SchemaAttribute;

    bool bValue;

    ElementInfo::FlagsType aResult = 0;

    switch(_eType)
    {
        case ElementType::property :
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_NULLABLE, bValue) && ! bValue)
                aResult |= SchemaAttribute::REQUIRED;
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_LOCALIZED, bValue) && bValue)
                aResult |= SchemaAttribute::LOCALIZED;
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_READONLY, bValue) && bValue)
                aResult |= NodeAttribute::READONLY;
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_FINALIZED, bValue) && bValue)
                aResult |= NodeAttribute::FINALIZED;
            break;

        case ElementType::node:
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_FINALIZED, bValue) && bValue)
                aResult |= NodeAttribute::FINALIZED;
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_MANDATORY, bValue) && bValue)
                aResult |= NodeAttribute::MANDATORY;
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_READONLY, bValue) && bValue)
                aResult |= NodeAttribute::READONLY;
            break;

        case ElementType::group:
        case ElementType::set:
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_EXTENSIBLE, bValue) && bValue)
                aResult |= SchemaAttribute::EXTENSIBLE;
            break;

        case ElementType::layer:
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_READONLY, bValue) && bValue)
                aResult |= NodeAttribute::READONLY;
            if (this->maybeGetAttribute(xAttribs, ATTR_FLAG_FINALIZED, bValue) && bValue)
                aResult |= NodeAttribute::FINALIZED;
            break;

        default:
            break;

    }
    return aResult;
}
// -----------------------------------------------------------------------------
static
void badValueType(sal_Char const * _pMsg, OUString const & _sExtra)
{
    rtl::OString sMessage( "Configuration XML parser: Bad value type attribute:" );
    sMessage += _pMsg;
    sMessage += rtl::OUStringToOString(_sExtra,RTL_TEXTENCODING_ASCII_US);

    OSL_ENSURE(false, sMessage.getStr());
//    throw sax::SAXException(sMessage, NULL, uno::Any());
}
// -----------------------------------------------------------------------------
static
inline
sal_Bool matchNsPrefix(OUString const & _sString, OUString const & _sPrefix)
{
    return _sString.match(_sPrefix) &&
            _sString.getStr()[_sPrefix.getLength()] == k_NS_SEPARATOR;
}
// -----------------------------------------------------------------------------
static
inline
sal_Bool matchSuffix(OUString const & _sString, OUString const & _sSuffix)
{
    sal_Int32 nSuffixStart = _sString.getLength() - _sSuffix.getLength();
    if (nSuffixStart < 0)
        return false;

    return _sString.match(_sSuffix,nSuffixStart);
}
// -----------------------------------------------------------------------------
static
inline
OUString stripNsPrefix(OUString const & _sString, OUString const & _sPrefix)
{
    OSL_ASSERT( matchNsPrefix(_sString,_sPrefix) );

    return _sString.copy(_sPrefix.getLength() + 1);
}
// -----------------------------------------------------------------------------
static
inline
OUString stripSuffix(OUString const & _sString, OUString const & _sSuffix)
{
    OSL_ASSERT( matchSuffix(_sString,_sSuffix) );

    sal_Int32 nSuffixStart = _sString.getLength() - _sSuffix.getLength();

    return _sString.copy(0,nSuffixStart);
}
// -----------------------------------------------------------------------------
static
inline
OUString stripTypeName(OUString const & _sString, OUString const & _sPrefix)
{
    if ( matchNsPrefix(_sString,_sPrefix))
        return stripNsPrefix(_sString, _sPrefix);

    badValueType("Missing expected namespace prefix: ", _sPrefix);

    return _sString;
}
// -----------------------------------------------------------------------------
static
uno::Type xmlToScalarType(const OUString& _rType)
{
    uno::Type aRet;

    if     (_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_BOOLEAN))
        aRet = ::getBooleanCppuType();

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_SHORT))
        aRet = ::getCppuType(static_cast<sal_Int16 const*>(0));

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_INT))
        aRet = ::getCppuType(static_cast<sal_Int32 const*>(0));

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_LONG))
        aRet = ::getCppuType(static_cast<sal_Int64 const*>(0));

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_DOUBLE))
        aRet = ::getCppuType(static_cast< double  const*>(0));

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_STRING))
        aRet = ::getCppuType(static_cast<OUString const*>(0));

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_BINARY))
        aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int8> const*>(0));

    else if(_rType.equalsIgnoreAsciiCaseAscii(VALUETYPE_ANY))
        aRet = ::getCppuType(static_cast<uno::Any const*>(0));

    else
        OSL_ENSURE(false,"Cannot parse: Unknown value type");

    return aRet;
}
// -----------------------------------------------------------------------------
uno::Type xmlToListType(const OUString& _aElementType)
{
    uno::Type aRet;

    if     (_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_BOOLEAN))
        aRet = ::getCppuType(static_cast<uno::Sequence<sal_Bool> const*>(0));

    else if(_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_SHORT))
        aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int16> const*>(0));

    else if(_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_INT))
        aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));

    else if(_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_LONG))
        aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int64> const*>(0));

    else if(_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_DOUBLE))
        aRet = ::getCppuType(static_cast<uno::Sequence<double> const*>(0));

    else if(_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_STRING))
        aRet = ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const*>(0));

    else if(_aElementType.equalsIgnoreAsciiCaseAscii(VALUETYPE_BINARY))
        aRet = ::getCppuType(static_cast<uno::Sequence<uno::Sequence<sal_Int8> > const*>(0));

    else
        OSL_ENSURE(false,"Cannot parse: Unknown list value type");

    return aRet;
}
// -----------------------------------------------------------------------------
/// retrieve data type of a property,
uno::Type ElementParser::getPropertyValueType(SaxAttributeList const& xAttribs) const
{
    OUString sTypeName;
    if (!this->maybeGetAttribute(xAttribs, ATTR_VALUETYPE, sTypeName))
        return uno::Type(); // => VOID

    uno::Type aType;

    // valuetype names are either 'xs:<type>' or 'oor:<type>' or 'oor:<type>-list'
    if (matchSuffix(sTypeName,VALUETYPE_LIST_SUFFIX))
    {
        OUString sBasicName = stripTypeName( stripSuffix(sTypeName,VALUETYPE_LIST_SUFFIX), NS_PREFIX_OOR );

        aType = xmlToListType(sBasicName);
    }
    else
    {
        OUString sPrefix = matchNsPrefix(sTypeName,NS_PREFIX_OOR) ? NS_PREFIX_OOR : NS_PREFIX_XS;

        OUString sBasicName = stripTypeName( sTypeName, sPrefix );

        aType = xmlToScalarType(sBasicName);
    }

    if (aType == uno::Type())
        badValueType("Unknown type name: ", sTypeName);

    return aType;
}
// -----------------------------------------------------------------------------

/// retrieve element type and associated module name of a set,
bool ElementParser::getSetElementType(SaxAttributeList const& xAttribs, OUString& aElementType, OUString& aElementTypeModule) const
{
    if (!this->maybeGetAttribute(xAttribs, ATTR_ITEMTYPE, aElementType))
        return false;

    maybeGetAttribute(xAttribs, ATTR_ITEMTYPECOMPONENT, aElementTypeModule);

    return true;
}
// -----------------------------------------------------------------------------

/// retrieve instance type and associated module name of a set,
bool ElementParser::getInstanceType(SaxAttributeList const& xAttribs, OUString& aElementType, OUString& aElementTypeModule) const
{
    if (!this->maybeGetAttribute(xAttribs, ATTR_ITEMTYPE, aElementType))
        return false;

    maybeGetAttribute(xAttribs, ATTR_ITEMTYPECOMPONENT, aElementTypeModule);

    return true;
}
// -----------------------------------------------------------------------------

/// retrieve the component for an import or uses element,
bool ElementParser::getImportComponent(SaxAttributeList const& xAttribs, OUString& _rsComponent) const
{
    return this->maybeGetAttribute(xAttribs, ATTR_COMPONENT, _rsComponent);
}
// -----------------------------------------------------------------------------

/// reads attributes for values from the attribute list
bool ElementParser::isNull(SaxAttributeList const& _xAttribs) const
{
    bool bNull;
    return maybeGetAttribute(_xAttribs, EXT_ATTR_NULL, bNull) && bNull;
}
// -----------------------------------------------------------------------------

/// reads attributes for values from the attribute list
OUString ElementParser::getSeparator(SaxAttributeList const& _xAttribs) const
{
    OUString aSeparator;
    maybeGetAttribute(_xAttribs, ATTR_VALUESEPARATOR, aSeparator);
    return aSeparator;
}
// -----------------------------------------------------------------------------

// low-level internal methods
/// checks for presence of a boolean attribute and assigns its value if it exists (and is a bool)
bool ElementParser::maybeGetAttribute(SaxAttributeList const& xAttribs, OUString const& aAttributeName, bool& rAttributeValue) const
{
    OUString sAttribute;

    if ( !this->maybeGetAttribute(xAttribs, aAttributeName, sAttribute) )
    {
        return false;
    }

    else if (sAttribute.equals(ATTR_VALUE_TRUE))
        rAttributeValue = true; // will return true

    else if (sAttribute.equals(ATTR_VALUE_FALSE))
        rAttributeValue = false;  // will return true

    else
    {
        OSL_ENSURE(sAttribute.getLength() == 0, "Invalid text found in boolean attribute");
        return false;
    }

    return true;
}
// -----------------------------------------------------------------------------

/// checks for presence of an attribute and assigns its value if it exists
bool ElementParser::maybeGetAttribute(SaxAttributeList const& xAttribs, OUString const& aAttributeName, OUString& rAttributeValue) const
{
    return xAttribs.is() && impl_maybeGetAttribute(xAttribs, aAttributeName, rAttributeValue);
}
// -----------------------------------------------------------------------------

/// assigns an attribute value or an empty string if it doesn't exist
void ElementParser::alwaysGetAttribute(SaxAttributeList const& xAttribs, OUString const& aAttributeName, OUString& rAttributeValue) const
{
    if (xAttribs.is())
        rAttributeValue = xAttribs->getValueByName(aAttributeName);
    else
        rAttributeValue = OUString();
}
// -----------------------------------------------------------------------------
} // namespace
} // namespace

