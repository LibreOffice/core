/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementformatter.cxx,v $
 * $Revision: 1.17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "elementformatter.hxx"
#include "xmlstrings.hxx"
#include "typeconverter.hxx"

#include <comphelper/attributelist.hxx>

#include <rtl/ustrbuf.hxx>

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

void ElementFormatter::addAttribute(rtl::OUString const & _anAttributeName, rtl::OUString const & _aValue)
{
    OSL_PRECOND(m_xAttributes.is(),"Trying to add an attribute to a non-existing list");

    m_xAttributes->AddAttribute(_anAttributeName,
                                XML_ATTRTYPE_CDATA,
                                _aValue);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addAttribute(rtl::OUString const & _anAttributeName, bool _bValue)
{
    OSL_PRECOND(m_xAttributes.is(),"Trying to add an attribute to a non-existing list");

    m_xAttributes->AddAttribute(_anAttributeName,
                                XML_ATTRTYPE_CDATA,
                                _bValue ? ATTR_VALUE_TRUE : ATTR_VALUE_FALSE);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addNamespaces()
{
    static rtl::OUString const sNamespaceDecl( RTL_CONSTASCII_USTRINGPARAM("xmlns:") );

    addAttribute( sNamespaceDecl.concat(NS_PREFIX_OOR), static_cast<rtl::OUString const &>(NS_URI_OOR));
    addAttribute( sNamespaceDecl.concat(NS_PREFIX_XS ), static_cast<rtl::OUString const &>(NS_URI_XS ));
}
// -----------------------------------------------------------------------------

void ElementFormatter::prepareElement(ElementInfo const& _aInfo)
{
    if (!m_xAttributes.is())
    {
        m_xAttributes.set( new ::comphelper::AttributeList() );
        addNamespaces();
    }
    else
        m_xAttributes->Clear();

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
        m_xAttributes.set( new ::comphelper::AttributeList() );
        addNamespaces();
    }
    else
        m_xAttributes->Clear();

    m_aElementType = _eType;
}
// -----------------------------------------------------------------------------

void ElementFormatter::addName(rtl::OUString const & _aName)
{
    if (_aName.getLength())
    {
        switch( m_aElementType )
        {
        case ElementType::schema:
        case ElementType::layer:
            {
                sal_Int32 nIndex = _aName.lastIndexOf('.');

                rtl::OUString aNodeName = _aName.copy(nIndex + 1);
                addAttribute(ATTR_NAME, aNodeName);

                OSL_ENSURE(nIndex > 0,"Found component root element without a package part in its name");
                if (nIndex > 0)
                {
                    rtl::OUString aPackage = _aName.copy(0, nIndex);
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
void ElementFormatter::maybeAddFlag(sal_Int16 _eFlags, sal_Int16 _eSelect, rtl::OUString const & _anAttributeName, bool _bValue)
{
    if (_eFlags & _eSelect) addAttribute(_anAttributeName,_bValue);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addNodeFlags(sal_Int16 _eFlags)
{
    maybeAddFlag(_eFlags,com::sun::star::configuration::backend::SchemaAttribute::REQUIRED,    ATTR_FLAG_NULLABLE, false);
    maybeAddFlag(_eFlags,com::sun::star::configuration::backend::SchemaAttribute::LOCALIZED,   ATTR_FLAG_LOCALIZED);
    maybeAddFlag(_eFlags,com::sun::star::configuration::backend::SchemaAttribute::EXTENSIBLE,  ATTR_FLAG_EXTENSIBLE);

    maybeAddFlag(_eFlags,com::sun::star::configuration::backend::NodeAttribute::FINALIZED, ATTR_FLAG_FINALIZED);
    maybeAddFlag(_eFlags,com::sun::star::configuration::backend::NodeAttribute::MANDATORY, ATTR_FLAG_MANDATORY);
    maybeAddFlag(_eFlags,com::sun::star::configuration::backend::NodeAttribute::READONLY,  ATTR_FLAG_READONLY);
}
// -----------------------------------------------------------------------------

void ElementFormatter::addOperation(Operation::Enum _eOp)
{
    switch (_eOp)
    {
    case Operation::none:       break;
    case Operation::modify:     break ; //addAttribute(ATTR_OPERATION, static_cast<rtl::OUString const &>(OPERATION_MODIFY));  break;
    case Operation::clear:      OSL_ENSURE(false,"'clear' operation is not yet supported"); break ;
                                //addAttribute(ATTR_OPERATION, static_cast<rtl::OUString const &>(OPERATION_CLEAR));  break;
    case Operation::replace:    addAttribute(ATTR_OPERATION, static_cast<rtl::OUString const &>(OPERATION_REPLACE)); break;
    case Operation::fuse:       addAttribute(ATTR_OPERATION, static_cast<rtl::OUString const &>(OPERATION_FUSE));    break;
    case Operation::remove:     addAttribute(ATTR_OPERATION, static_cast<rtl::OUString const &>(OPERATION_REMOVE));  break;

    case Operation::unknown:
        OSL_ENSURE(false, "ElementFormatter: Trying to add attribute for 'unknown' operation");
        break;
    default:
        OSL_ENSURE(false, "ElementFormatter: Trying to add attribute for invalid operation");
        break;
    }
}
// -----------------------------------------------------------------------------

void ElementFormatter::addInstanceType(rtl::OUString const & /*_aElementType*/, rtl::OUString const & /*_aElementTypeModule*/)
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
    rtl::OUString        aSimpleTypeName     = toXmlTypeName(aSimpleTypeClass);

    rtl::OUString sNsPrefix = (bList || aSimpleTypeClass == uno::TypeClass_ANY) ?
                         rtl::OUString( NS_PREFIX_OOR ) : rtl::OUString( NS_PREFIX_XS );

    rtl::OUStringBuffer aTypeNameBuf(sNsPrefix);

    if (sNsPrefix.getLength())
        aTypeNameBuf. append(k_NS_SEPARATOR);

    aTypeNameBuf. append(aSimpleTypeName);

    if (bList)
        aTypeNameBuf. append(VALUETYPE_LIST_SUFFIX);

    addAttribute( ATTR_VALUETYPE, aTypeNameBuf.makeStringAndClear());
}
// -----------------------------------------------------------------------------

void ElementFormatter::addLanguage(rtl::OUString const & _sLanguage)
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

void ElementFormatter::addSeparator(rtl::OUString const& _sSeparator)
{
    addAttribute( ATTR_VALUESEPARATOR, _sSeparator);
}
// -----------------------------------------------------------------------------

rtl::OUString ElementFormatter::getElementTag() const
{
    switch (m_aElementType)
    {
    case ElementType::schema:    return rtl::OUString( TAG_SCHEMA );
    case ElementType::layer:     return rtl::OUString( TAG_LAYER );

    case ElementType::component: return rtl::OUString( TAG_COMPONENT );
    case ElementType::templates: return rtl::OUString( TAG_TEMPLATES );

    case ElementType::property:  return rtl::OUString( TAG_PROP );
    case ElementType::node:      return rtl::OUString( TAG_NODE );
    case ElementType::group:     return rtl::OUString( TAG_GROUP );
    case ElementType::set:       return rtl::OUString( TAG_SET );

    case ElementType::import:    return rtl::OUString( TAG_IMPORT );
    case ElementType::instance:  return rtl::OUString( TAG_INSTANCE );
    case ElementType::item_type: return rtl::OUString( TAG_ITEMTYPE );
    case ElementType::value:     return rtl::OUString( TAG_VALUE );
    case ElementType::uses:      return rtl::OUString( TAG_USES );

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
    return rtl::OUString();
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

