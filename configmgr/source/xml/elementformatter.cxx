/*************************************************************************
 *
 *  $RCSfile: elementformatter.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:34:03 $
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

#include "elementformatter.hxx"

#ifndef CONFIGMGR_XML_STRINGS_HXX_
#include "xmlstrings.hxx"
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_MISC_ATTRIBUTELIST_HXX
#include "attributelist.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <com/sun/star/configuration/backend/SchemaAttribute.hpp>
#include <com/sun/star/configuration/backend/NodeAttribute.hpp>

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

ElementFormatter::ElementFormatter()
: m_aElementType(ElementType::unknown)
, m_xAttributes()
{
}
// -----------------------------------------------------------------------------

ElementFormatter::~ElementFormatter()
{
}
// -----------------------------------------------------------------------------

void ElementFormatter::reset()
{
    m_aElementType = ElementType::unknown;
    m_xAttributes.clear();
}
// -----------------------------------------------------------------------------

void ElementFormatter::addAttribute(OUString const & _anAttributeName, OUString const & _aValue)
{
    OSL_PRECOND(m_xAttributes.is(),"Trying to add an attribute to a non-existing list");

    m_xAttributes->addAttribute(_anAttributeName,
                                XML_ATTRTYPE_CDATA,
                                _aValue);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addAttribute(OUString const & _anAttributeName, bool _bValue)
{
    OSL_PRECOND(m_xAttributes.is(),"Trying to add an attribute to a non-existing list");

    m_xAttributes->addAttribute(_anAttributeName,
                                XML_ATTRTYPE_CDATA,
                                _bValue ? ATTR_VALUE_TRUE : ATTR_VALUE_FALSE);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addNamespaces()
{
    static OUString const sNamespaceDecl( RTL_CONSTASCII_USTRINGPARAM("xmlns:") );

    addAttribute( sNamespaceDecl.concat(NS_PREFIX_OOR), static_cast<OUString const &>(NS_URI_OOR));
    addAttribute( sNamespaceDecl.concat(NS_PREFIX_XS ), static_cast<OUString const &>(NS_URI_XS ));
}
// -----------------------------------------------------------------------------

void ElementFormatter::prepareElement(ElementInfo const& _aInfo)
{
    if (!m_xAttributes.is())
    {
        m_xAttributes.set( new AttributeListImpl() );
        addNamespaces();
    }
    else
        m_xAttributes->clear();

    m_aElementType = _aInfo.type;

    addName(_aInfo.name);
    addNodeFlags(_aInfo.flags);
    addOperation(_aInfo.op);
}
// -----------------------------------------------------------------------------

void ElementFormatter::prepareSimpleElement(ElementType::Enum _eType)
{
    if (!m_xAttributes.is())
    {
        m_xAttributes.set( new AttributeListImpl() );
        addNamespaces();
    }
    else
        m_xAttributes->clear();

    m_aElementType = _eType;
}
// -----------------------------------------------------------------------------

void ElementFormatter::addName(OUString const & _aName)
{
    if (_aName.getLength())
    {
        switch( m_aElementType )
        {
        case ElementType::schema:
        case ElementType::layer:
            {
                sal_Int32 nIndex = _aName.lastIndexOf('.');

                OUString aNodeName = _aName.copy(nIndex + 1);
                addAttribute(ATTR_NAME, aNodeName);

                OSL_ENSURE(nIndex > 0,"Found component root element without a package part in its name");
                if (nIndex > 0)
                {
                    OUString aPackage = _aName.copy(0, nIndex);
                    addAttribute(ATTR_PACKAGE, aPackage);
                }
            }
            break;

        default:
            addAttribute(ATTR_NAME, _aName);
            break;
        }
    }
}
// -----------------------------------------------------------------------------

inline
void ElementFormatter::maybeAddFlag(FlagsType _eFlags, FlagsType _eSelect, OUString const & _anAttributeName, bool _bValue)
{
    if (_eFlags & _eSelect) addAttribute(_anAttributeName,_bValue);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addNodeFlags(FlagsType _eFlags)
{
    using namespace ::com::sun::star::configuration::backend;

    maybeAddFlag(_eFlags,SchemaAttribute::REQUIRED,    ATTR_FLAG_NULLABLE, false);
    maybeAddFlag(_eFlags,SchemaAttribute::LOCALIZED,   ATTR_FLAG_LOCALIZED);
    maybeAddFlag(_eFlags,SchemaAttribute::EXTENSIBLE,  ATTR_FLAG_EXTENSIBLE);

    maybeAddFlag(_eFlags,NodeAttribute::FINALIZED, ATTR_FLAG_FINALIZED);
    maybeAddFlag(_eFlags,NodeAttribute::MANDATORY, ATTR_FLAG_MANDATORY);
    maybeAddFlag(_eFlags,NodeAttribute::READONLY,  ATTR_FLAG_READONLY);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addOperation(Operation::Enum _eOp)
{
    switch (_eOp)
    {
    case Operation::none:       break;
    case Operation::modify:     break ; //addAttribute(ATTR_OPERATION, static_cast<OUString const &>(OPERATION_MODIFY));  break;
    case Operation::clear:      OSL_ENSURE(false,"'clear' operation is not yet supported"); break ;
                                //addAttribute(ATTR_OPERATION, static_cast<OUString const &>(OPERATION_CLEAR));  break;
    case Operation::replace:    addAttribute(ATTR_OPERATION, static_cast<OUString const &>(OPERATION_REPLACE)); break;
    case Operation::remove:     addAttribute(ATTR_OPERATION, static_cast<OUString const &>(OPERATION_REMOVE));  break;

    case Operation::unknown:
        OSL_ENSURE(false, "ElementFormatter: Trying to add attribute for 'unknown' operation");
        break;
    default:
        OSL_ENSURE(false, "ElementFormatter: Trying to add attribute for invalid operation");
        break;
    }
}
// -----------------------------------------------------------------------------

void ElementFormatter::addInstanceType(OUString const & _aElementType, OUString const & _aElementTypeModule)
{
}
// -----------------------------------------------------------------------------

static ::rtl::OUString toXmlTypeName(const uno::TypeClass& _rTypeClass)
{
    ::rtl::OUString aRet;
    switch(_rTypeClass)
    {
    case uno::TypeClass_BOOLEAN:  aRet = VALUETYPE_BOOLEAN; break;
    case uno::TypeClass_SHORT:    aRet = VALUETYPE_SHORT; break;
    case uno::TypeClass_LONG:     aRet = VALUETYPE_INT; break;
    case uno::TypeClass_HYPER:    aRet = VALUETYPE_LONG; break;
    case uno::TypeClass_DOUBLE:   aRet = VALUETYPE_DOUBLE; break;
    case uno::TypeClass_STRING:   aRet = VALUETYPE_STRING; break;
    case uno::TypeClass_SEQUENCE: aRet = VALUETYPE_BINARY; break;
    case uno::TypeClass_ANY:      aRet = VALUETYPE_ANY; break;
    default:
        OSL_ENSURE(false,"Cannot get type name: unknown typeclass");
        break;
    }
    return aRet;
}
// -----------------------------------------------------------------------------

void ElementFormatter::addPropertyValueType(uno::Type const& _aType)
{
    if (_aType == uno::Type()) return;

    bool bList = false;
    uno::Type       aSimpleType         = getBasicType(_aType, bList);
    uno::TypeClass  aSimpleTypeClass    = aSimpleType.getTypeClass();
    OUString        aSimpleTypeName     = toXmlTypeName(aSimpleTypeClass);

    OUString sNsPrefix = (bList || aSimpleTypeClass == uno::TypeClass_ANY) ? NS_PREFIX_OOR : NS_PREFIX_XS;

    rtl::OUStringBuffer aTypeNameBuf(sNsPrefix);

    if (sNsPrefix.getLength())
        aTypeNameBuf. append(k_NS_SEPARATOR);

    aTypeNameBuf. append(aSimpleTypeName);

    if (bList)
        aTypeNameBuf. append(VALUETYPE_LIST_SUFFIX);

    addAttribute( ATTR_VALUETYPE, aTypeNameBuf.makeStringAndClear());
}
// -----------------------------------------------------------------------------

void ElementFormatter::addLanguage(OUString const & _sLanguage)
{
    OSL_ENSURE(_sLanguage.getLength(), "ElementFormatter: Trying to add empty language attribute");
    addAttribute(EXT_ATTR_LANGUAGE, _sLanguage);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addIsNull(bool _bIsNull)
{
    addAttribute( EXT_ATTR_NULL, _bIsNull);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addSeparator(OUString const& _sSeparator)
{
    addAttribute( ATTR_VALUESEPARATOR, _sSeparator);
}
// -----------------------------------------------------------------------------

OUString ElementFormatter::getElementTag() const
{
    switch (m_aElementType)
    {
    case ElementType::schema:    return TAG_SCHEMA;
    case ElementType::layer:     return TAG_LAYER;

    case ElementType::component: return TAG_COMPONENT;
    case ElementType::templates: return TAG_TEMPLATES;

    case ElementType::property:  return TAG_PROP;
    case ElementType::node:      return TAG_NODE;
    case ElementType::group:     return TAG_GROUP;
    case ElementType::set:       return TAG_SET;

    case ElementType::import:    return TAG_IMPORT;
    case ElementType::instance:  return TAG_INSTANCE;
    case ElementType::item_type: return TAG_ITEMTYPE;
    case ElementType::value:     return TAG_VALUE;
    case ElementType::uses:      return TAG_USES;

    case ElementType::unknown:
        OSL_ENSURE(false, "ElementFormatter: Trying to get Tag for 'unknown' element type");
        break;
    case ElementType::other:
        OSL_ENSURE(false, "ElementFormatter: Trying to get Tag for 'other' element type");
        break;
    default:
        OSL_ENSURE(false, "ElementFormatter: Trying to get Tag for invalid element type");
        break;
    }
    return OUString();
}
// -----------------------------------------------------------------------------

uno::Reference< sax::XAttributeList > ElementFormatter::getElementAttributes() const
{
    return m_xAttributes.get();
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
} // namespace
} // namespace

