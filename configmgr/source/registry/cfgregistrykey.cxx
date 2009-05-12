/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cfgregistrykey.cxx,v $
 * $Revision: 1.18 $
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
#include "cfgregistrykey.hxx"
#include "datalock.hxx"
#include "typeconverter.hxx"
#include <osl/diagnose.h>
#include <cppuhelper/extract.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/util/XStringEscape.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <typelib/typedescription.hxx>

#include <limits>

#define THISREF()       static_cast< ::cppu::OWeakObject* >(this)
#define UNISTRING(c)    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(c) )

//..........................................................................
namespace configmgr
{
//..........................................................................

//--------------------------------------------------------------------------
namespace {
    inline
    com::sun::star::uno::Type getBinaryDataType()
    {
        com::sun::star::uno::Sequence<sal_Int8> const * const p= 0;
        return ::getCppuType(p);
    }
    inline
    bool isAscii(sal_Unicode ch)
    {
        return 0 < ch && ch < 128;
    }
    inline
    bool isAscii(sal_Unicode const * ps, sal_Int32 nLen )
    {
        for (int i= 0; i< nLen; ++i)
            if ( !isAscii( ps[i] ) )
                return false;
        return true;
    }
    inline
    bool isAscii(rtl::OUString const& str)
    {
        return isAscii(str.getStr(),str.getLength());
    }
    inline
    bool isAscii(com::sun::star::uno::Sequence< rtl::OUString > const& strList)
    {
        for (int i= 0; i< strList.getLength(); ++i)
            if ( !isAscii( strList[i] ) )
                return false;
        return true;
    }
}
// temporary helper
inline static void checkNullable() {}

//==========================================================================
//= OConfigurationRegistryKey
//==========================================================================

static
rtl::OUString getNodeName(const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& _xNode)
{
    com::sun::star::uno::Reference< com::sun::star::container::XNamed > xName( _xNode, com::sun::star::uno::UNO_QUERY );
    if (xName.is())
        return xName->getName();

    OSL_ENSURE( !_xNode.is(), "Cannot get name of node");
    return rtl::OUString();
}
//--------------------------------------------------------------------------

static bool splitPath(const rtl::OUString& _sPath, rtl::OUString& _rsParentPath, rtl::OUString& _rsLocalName);
//--------------------------------------------------------------------------

OConfigurationRegistryKey::OConfigurationRegistryKey
            (const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& _rxRootNode
            ,sal_Bool _bWriteable
            ,SubtreeRoot
            )
    :m_bReadOnly(!_bWriteable)
    ,m_xNode(_rxRootNode)
    ,m_xParentNode()
    ,m_sLocalName() // this will be treated as root - maybe use hierarchical name (ß)
{
    OSL_ENSURE(m_xNode.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid config node param !");
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey
            (const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& _rxNode
            ,sal_Bool _bWriteable
            )
    :m_bReadOnly(!_bWriteable)
    ,m_xNode(_rxNode)
    ,m_xParentNode()
    ,m_sLocalName( getNodeName(_rxNode) ) // this will not be treated as root
{
    OSL_ENSURE(m_xNode.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid config node param !");
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey(
                com::sun::star::uno::Any _rCurrentValue,
                const com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >& _rxParentNode,
                const ::rtl::OUString& _rLocalName,
                sal_Bool _bWriteable)
    :m_bReadOnly(!_bWriteable)
    ,m_xNode()
    ,m_xParentNode(_rxParentNode)
    ,m_sLocalName(_rLocalName)
{
    OSL_ENSURE(m_xParentNode.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid parent node param !");
    OSL_ENSURE(m_sLocalName.getLength(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid relative name !");
    OSL_ENSURE(m_xParentNode->hasByName( m_sLocalName ), "OConfigurationRegistryKey::OConfigurationRegistryKey : key not found in parent node !" ); //
    OSL_ENSURE(m_xParentNode->getByName( m_sLocalName ) == _rCurrentValue, "OConfigurationRegistryKey::OConfigurationRegistryKey : wrong value parameter !" ); //

    _rCurrentValue >>= m_xNode; // we don't care if that fails
}
//--------------------------------------------------------------------------

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo> OConfigurationRegistryKey::implGetParentPropertyInfo() throw(com::sun::star::uno::RuntimeException)
{
    if (!m_xParentNode.is())
    {
        com::sun::star::uno::Reference< com::sun::star::container::XChild > xChild(m_xNode, com::sun::star::uno::UNO_QUERY);
        if (xChild.is())
            m_xParentNode = m_xParentNode.query(xChild->getParent());
    }

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xParentInfo; // the result

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xParentProperties(m_xParentNode, com::sun::star::uno::UNO_QUERY);
    if (xParentProperties.is())
    {
        xParentInfo = xParentProperties->getPropertySetInfo();
        if (xParentInfo.is() && !xParentInfo->hasPropertyByName(m_sLocalName))
        {
            OSL_ENSURE(false, "OConfigurationRegistryKey: This key is unknown in the parent node's PropertySetInfo !");
            xParentInfo.clear(); // this key is unknow, so don't return the info
        }
    }

    return xParentInfo;
}
//--------------------------------------------------------------------------

static
sal_Bool isNodeReadOnly(com::sun::star::uno::Reference< com::sun::star::uno::XInterface > const& _xNode) throw(com::sun::star::uno::RuntimeException)
{
    OSL_ASSERT( _xNode.is() );

    com::sun::star::uno::Reference< com::sun::star::beans::XProperty > xProperty(_xNode, com::sun::star::uno::UNO_QUERY);
    if (xProperty.is())
    {
        com::sun::star::beans::Property aProperty = xProperty->getAsProperty();

        return (aProperty.Attributes & com::sun::star::beans::PropertyAttribute::READONLY) != 0;
    }

    com::sun::star::uno::Reference< com::sun::star::lang::XServiceInfo > xServiceInfo( _xNode, com::sun::star::uno::UNO_QUERY );
    if (xServiceInfo.is())
    {
        // does it announce update capability ?
        if (xServiceInfo->supportsService(UNISTRING("com.sun.star.configuration.ConfigurationUpdateAccess")))
            return false;

        // else does it announce the expected service at all ?
        else if (xServiceInfo->supportsService(UNISTRING("com.sun.star.configuration.ConfigurationAccess")))
            return true;
    }

    // no XProperty, no (meaningful) ServiceInfo - what can we do
    return false;
}

//--------------------------------------------------------------------------

sal_Bool OConfigurationRegistryKey::implIsReadOnly() throw (com::sun::star::uno::RuntimeException)
{
    sal_Bool bResult = m_bReadOnly;;

    // do checks only if this was requested to be writable
    if (m_bReadOnly)
    {
        // nothing to check
    }

    // try to ask the node itself
    else if (m_xNode.is())
    {
        bResult = m_bReadOnly = isNodeReadOnly( m_xNode );
    }

    // else use the parent
    else  if (m_xParentNode.is())
    {
        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xParentInfo = implGetParentPropertyInfo();

        if (xParentInfo.is())
        {
            com::sun::star::beans::Property aProperty = xParentInfo->getPropertyByName(m_sLocalName);

            bResult = m_bReadOnly = ((aProperty.Attributes & com::sun::star::beans::PropertyAttribute::READONLY) != 0);
        }
        else
        {
            // no property info about this key ? - check if the parent itself is writable

            // NOTE: do not set m_bReadOnly here, as we haven't really found out about this object
            bResult = isNodeReadOnly( m_xParentNode );
        }
    }
    else
    {
        // no data at all
        OSL_ENSURE(false, "implIsReadOnly called for invalid object");
        bResult = true; // no object is certainly not writable ;-)
    }

    return bResult;
}
//--------------------------------------------------------------------------

sal_Bool OConfigurationRegistryKey::implEnsureNode() throw (com::sun::star::registry::InvalidRegistryException,com::sun::star::uno::RuntimeException)
{
    if (!m_xNode.is())
    {
        OSL_ENSURE( m_xParentNode.is(), "implEnsureNode called for invalid registry key");
        if (m_xParentNode.is())
        {
            try
            {
                com::sun::star::uno::Any aNode = m_xParentNode->getByName( m_sLocalName );

                if ( !(aNode >>= m_xNode) )
                    OSL_ENSURE( ! (aNode.hasValue() && aNode.getValueTypeClass() == com::sun::star::uno::TypeClass_INTERFACE),
                                "OConfigurationRegistryKey: Node object does not implement expected interface");
            }
            catch (com::sun::star::container::NoSuchElementException& e)
            {
                m_xParentNode.clear();

                rtl::OUString sMessage = UNISTRING("Invalid OConfigurationRegistryKey. The node \"");
                sMessage += m_sLocalName;
                sMessage += UNISTRING("\" was not found in the parent. Parent error message: \n");
                sMessage += e.Message;

                throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
            }
        }
    }
    return m_xNode.is();
}
//--------------------------------------------------------------------------

com::sun::star::uno::Type OConfigurationRegistryKey::implGetUnoType() throw (com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Type aType;
    if (m_xNode.is())
    {
        aType = getCppuType(&m_xNode); // Its just an interface type
    }
    else if (m_xParentNode.is())
    {

        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xParentInfo = implGetParentPropertyInfo();
        if (xParentInfo.is())
        {
            aType = xParentInfo->getPropertyByName( m_sLocalName ).Type;
        }
        else
        {
            aType = m_xParentNode->getElementType();
        }
    }
    else
    {
        OSL_ASSERT( aType.getTypeClass() == com::sun::star::uno::TypeClass_VOID );
        OSL_ENSURE( false, "implGetUnoType called for invalid registry key");
    }
    return aType;
}
//--------------------------------------------------------------------------

sal_Bool OConfigurationRegistryKey::implEnsureValue() throw (com::sun::star::uno::RuntimeException)
{
    if (m_xNode.is())
        return false;

    OSL_ENSURE( m_xParentNode.is(), "implEnsureValue called for invalid registry key");
    if (!m_xParentNode.is())
        return false;

    switch (implGetUnoType().getTypeClass())
    {
    case com::sun::star::uno::TypeClass_INTERFACE:
        return false;

    case com::sun::star::uno::TypeClass_BYTE:
    case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
    case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
    case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
    case com::sun::star::uno::TypeClass_FLOAT:
        OSL_ENSURE(false, "Unexpected (UNSIGNED INTERGRAL or FLOAT) type found for configuration node");

    case com::sun::star::uno::TypeClass_STRING:
    case com::sun::star::uno::TypeClass_BOOLEAN:
    case com::sun::star::uno::TypeClass_SHORT:
    case com::sun::star::uno::TypeClass_LONG:
    case com::sun::star::uno::TypeClass_HYPER:
    case com::sun::star::uno::TypeClass_DOUBLE:
    case com::sun::star::uno::TypeClass_SEQUENCE:
        return true;

    case com::sun::star::uno::TypeClass_ANY:
        return true;

    case com::sun::star::uno::TypeClass_VOID:
        OSL_ENSURE(false, "OConfigurationRegistryKey: Key does not exist or has VOID type");
        return false;

    default:
        OSL_ENSURE(false, "OConfigurationRegistryKey: Key has unexpected UNO type (class)");
        return false;
    }
}
//--------------------------------------------------------------------------

sal_Bool OConfigurationRegistryKey::implIsValid() throw ()
{
    return m_xNode.is() || (m_xParentNode.is() && m_xParentNode->hasByName( m_sLocalName ) );
}
//--------------------------------------------------------------------------

void OConfigurationRegistryKey::checkValid(KEY_ACCESS_TYPE _eIntentedAccess) throw (com::sun::star::registry::InvalidRegistryException,com::sun::star::uno::RuntimeException)
{
    if (!implIsValid())
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The registry is not bound to a configuration node anymore."), THISREF());
        // "anymore", because at the moment the ctor was called it probably was bound ....

    switch (_eIntentedAccess)
    {
        case KAT_VALUE_WRITE:
            if (implIsReadOnly())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This configuration node is not writeable."), THISREF());

            // !!! NO !!! BREAK !!!
        case KAT_VALUE:
            if (m_xNode.is())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This configuration node is not a value, but an internal container."), THISREF());

            if (!m_xParentNode.is())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This configuration node is invalid. It has no parent."), THISREF());

            if (!implEnsureValue())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This configuration does not have a legal value type."), THISREF());
            break;

        case KAT_CHILD:
            if (!implEnsureNode())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This configuration node does not have children, it is a value node."), THISREF());
            break;

        case KAT_META:
            break;
    }
}

//--------------------------------------------------------------------------
com::sun::star::uno::Any    OConfigurationRegistryKey::implCreateDefaultElement(com::sun::star::uno::Type const& _aValueType) throw (com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aReturn;

    switch (_aValueType.getTypeClass())
    {
    case com::sun::star::uno::TypeClass_STRING:
        aReturn <<= rtl::OUString();
        break;

    // we don't distinguish between the different integer types or boolean
    // (the RegistryKeyType is not granular enough),
    // but we can't handle them all the same way here
    case com::sun::star::uno::TypeClass_BYTE:
    case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
    case com::sun::star::uno::TypeClass_SHORT:
        aReturn <<= (sal_Int16)0;
        break;

    case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
    case com::sun::star::uno::TypeClass_LONG:
        aReturn <<= (sal_Int32)0;
        break;

    case com::sun::star::uno::TypeClass_BOOLEAN:
        aReturn <<= sal_Bool(false);
        break;

    // we cannot really handle 64-bit ints in the registry (but here we can)
    case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
    case com::sun::star::uno::TypeClass_HYPER:
        OSL_ENSURE(false, "Warning: cannot handle 64-bit values correctly in registry");
        aReturn <<= (sal_Int64)0;
        break;

    // we cannot really handle doubles in the registry (but here we can)
    case com::sun::star::uno::TypeClass_FLOAT:
    case com::sun::star::uno::TypeClass_DOUBLE:
        OSL_ENSURE(false, "Warning: cannot handle DOUBLE correctly in registry");
        aReturn <<= (double)0;
        break;

    // we really want to leave an Any as NULL - hopefully this is acceptable to the set
    case com::sun::star::uno::TypeClass_ANY:
        break;

    case com::sun::star::uno::TypeClass_SEQUENCE:
        if (_aValueType == getBinaryDataType())
            aReturn <<= com::sun::star::uno::Sequence< sal_Int8 >();

        else
        {
            com::sun::star::uno::Type aElementType = getSequenceElementType(_aValueType);
            switch (aElementType.getTypeClass())
            {
            case com::sun::star::uno::TypeClass_STRING:
                aReturn <<= com::sun::star::uno::Sequence< rtl::OUString >();
                break;

            case com::sun::star::uno::TypeClass_BYTE:
            case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
            case com::sun::star::uno::TypeClass_SHORT:
                aReturn <<= com::sun::star::uno::Sequence< sal_Int16 >();
                break;

            case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
            case com::sun::star::uno::TypeClass_LONG:
                aReturn <<= com::sun::star::uno::Sequence< sal_Int32 >();
                break;

            case com::sun::star::uno::TypeClass_BOOLEAN:
                aReturn <<= com::sun::star::uno::Sequence< sal_Bool >();
                break;

            case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
            case com::sun::star::uno::TypeClass_HYPER:
                aReturn <<= com::sun::star::uno::Sequence< sal_Int64 >();
                break;

            case com::sun::star::uno::TypeClass_FLOAT:
            case com::sun::star::uno::TypeClass_DOUBLE:
                aReturn <<= com::sun::star::uno::Sequence< double >();
                break;

            case com::sun::star::uno::TypeClass_SEQUENCE:
                OSL_ENSURE(false, "Warning: cannot handle Sequence< BINARY > correctly in registry");
                if (aElementType == getBinaryDataType())
                {
                    OSL_ENSURE(false, "Warning: cannot handle Sequence< BINARY > correctly in registry");
                    aReturn <<= com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< sal_Int8 > >();
                    break;
                }

                // else FALL THRU to default
            default:
                OSL_ENSURE(false, "Unexpected sequence element type for configuration node - returning NULL");
                // throw here ??
                break;
            }
        }
        break;

    case com::sun::star::uno::TypeClass_INTERFACE:
        OSL_ENSURE(false, "Invalid call to OConfigurationRegistryKey::implCreateDefaultElement. Inner nodes must be created by a factory");
        break;

    default:
        OSL_ENSURE(false, "Unexpected value type for configuration node - returning NULL");
        // throw here ??
        break;
    }

    OSL_ENSURE( aReturn.getValueType() == _aValueType || (_aValueType.getTypeClass() == com::sun::star::uno::TypeClass_ANY && !aReturn.hasValue()),
                "Warning: Unexpected data type found in Registry - returning similar value or NULL");
    return aReturn;
}

//--------------------------------------------------------------------------
com::sun::star::uno::Any OConfigurationRegistryKey::implGetDescendant(const rtl::OUString& _rDescendantName) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aElementReturn;

    try
    {
        if (!m_xNode.is())
        {
            // implEnsureNode should have been called before this method
            OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getDescendant : invalid call !");

            // this method should not be called if the object does not represent a container node ...
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("invalid object."), THISREF());
        }

        try
        {
            // look for a local member first
            aElementReturn = m_xNode->getByName(_rDescendantName);
        }
        catch(com::sun::star::container::NoSuchElementException&)
        {
            // is it a (possibly) hierarchical name ?
            if ( _rDescendantName.indexOf('/') <0 ) throw;

            // Yes, so try deep access
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xDeepAccess( m_xNode, com::sun::star::uno::UNO_QUERY );
            if (!xDeepAccess.is())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("Nested element access not supported by this node."), THISREF());

            aElementReturn = xDeepAccess->getByHierarchicalName(_rDescendantName);
        }
    }
    catch(com::sun::star::container::NoSuchElementException&)
    {   // not allowed to leave the method, wrap it
        rtl::OUString sMessage(UNISTRING("There is no element named "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(".");
        throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
    }
    catch(com::sun::star::lang::WrappedTargetException& wte)
    {   // allowed to be thrown by XNameAccess::getByName, but not allowed to leave this method
        rtl::OUString sMessage(UNISTRING("The configuration node could not provide an element for "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(". Original Error: ");
        sMessage += wte.Message;
        throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
    }

    return aElementReturn;
}

//--------------------------------------------------------------------------
void OConfigurationRegistryKey::implSetValue(const com::sun::star::uno::Any& _rValue) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_VALUE_WRITE);

    // one possible interface
    com::sun::star::uno::Reference< com::sun::star::container::XNameReplace > xParentValueAccess(m_xParentNode, com::sun::star::uno::UNO_QUERY);
    if (xParentValueAccess.is())
    {
        try
        {
            xParentValueAccess->replaceByName(m_sLocalName, _rValue);
        }
        catch(com::sun::star::lang::IllegalArgumentException& iae)
        {
            rtl::OUString sMessage = UNISTRING("Unable to replace the old value. The configuration node threw an ");
            sMessage += UNISTRING("IllegalArgumentException: ");
            sMessage += iae.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        catch(com::sun::star::container::NoSuchElementException& nse)
        {
            OSL_ENSURE(false, "OConfigurationRegistryKey::writeValueNode : a NoSuchElementException should be impossible !");

            rtl::OUString sMessage = UNISTRING("Unable to replace the old value. The configuration node threw an ");
            sMessage += UNISTRING("NoSuchElementException: ");
            sMessage += nse.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        catch(com::sun::star::lang::WrappedTargetException& wte)
        {
            rtl::OUString sMessage = UNISTRING("Unable to replace the old value. The configuration node threw an ");
            sMessage += UNISTRING("WrappedTargetException: ");
            sMessage += wte.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        return;
    }

    // not found - try other interface
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xParentPropertySet(m_xParentNode, com::sun::star::uno::UNO_QUERY);
    if (xParentPropertySet.is())
    {
        try
        {
            xParentPropertySet->setPropertyValue(m_sLocalName, _rValue);
        }
        catch(com::sun::star::lang::IllegalArgumentException& iae)
        {
            rtl::OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("IllegalArgumentException: ");
            sMessage += iae.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        catch(com::sun::star::beans::UnknownPropertyException& upe)
        {
            OSL_ENSURE(false, "OConfigurationRegistryKey::writeValueNode : a UnknownPropertyException should be impossible !");

            rtl::OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("UnknownPropertyException: ");
            sMessage += upe.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        catch(com::sun::star::beans::PropertyVetoException& pve)
        {
            rtl::OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("PropertyVetoException: ");
            sMessage += pve.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        catch(com::sun::star::lang::WrappedTargetException& wte)
        {
            rtl::OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("WrappedTargetException: ");
            sMessage += wte.Message;
            throw com::sun::star::registry::InvalidRegistryException(sMessage, THISREF());
        }
        return;
    }

    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("No interface found on parent node for writing to configuration value node."), THISREF());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getKeyName() throw(com::sun::star::uno::RuntimeException)
{
    return m_sLocalName;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistryKey::isReadOnly() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    OSL_ASSERT(UnoApiLock::isHeld());
    checkValid(KAT_META);
    return m_bReadOnly;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistryKey::isValid() throw(com::sun::star::uno::RuntimeException)
{
    OSL_ASSERT(UnoApiLock::isHeld());
    // TODO : perhaps if the registry we're a part of is closed ....
    return implIsValid();
}

//--------------------------------------------------------------------------
com::sun::star::registry::RegistryKeyType SAL_CALL OConfigurationRegistryKey::getKeyType( const ::rtl::OUString& /*_rKeyName*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    OSL_ASSERT(UnoApiLock::isHeld());

    // no further checks are made (for performance reasons) ...
    // Maybe we should check only KAT_META for consistency ?
    checkValid(KAT_CHILD);

    return com::sun::star::registry::RegistryKeyType_KEY;
}

//--------------------------------------------------------------------------
com::sun::star::registry::RegistryValueType SAL_CALL OConfigurationRegistryKey::getValueType() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_META);

    const com::sun::star::uno::Type aUnoType = implGetUnoType();

    switch (aUnoType.getTypeClass())
    {
    case com::sun::star::uno::TypeClass_INTERFACE:  // this is really a case of 'no value type'
        return com::sun::star::registry::RegistryValueType_NOT_DEFINED;

    case com::sun::star::uno::TypeClass_ANY:    // this is really a case of 'all value types allowed'
        return com::sun::star::registry::RegistryValueType_NOT_DEFINED;

    case com::sun::star::uno::TypeClass_STRING:
        return com::sun::star::registry::RegistryValueType_STRING;

    case com::sun::star::uno::TypeClass_BYTE:
    case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
    case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        OSL_ENSURE(false, "Unexpected UNSIGNED type found for configuration node");
        // FALL THRU

    case com::sun::star::uno::TypeClass_BOOLEAN:
    case com::sun::star::uno::TypeClass_SHORT:
    case com::sun::star::uno::TypeClass_LONG:
        return com::sun::star::registry::RegistryValueType_LONG;

    case com::sun::star::uno::TypeClass_FLOAT:
    case com::sun::star::uno::TypeClass_DOUBLE:
        OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support floating point numbers !");
        return com::sun::star::registry::RegistryValueType_LONG;

    case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
    case com::sun::star::uno::TypeClass_HYPER:
        OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support 64-bit integer numbers !");
        return com::sun::star::registry::RegistryValueType_LONG;

    case com::sun::star::uno::TypeClass_SEQUENCE:
        if ( aUnoType.equals( getBinaryDataType() ) )
            return com::sun::star::registry::RegistryValueType_BINARY;

        else
        {
            com::sun::star::uno::Type aElementType = getSequenceElementType(aUnoType);

            switch (aElementType.getTypeClass())
            {
            case com::sun::star::uno::TypeClass_STRING:
                return com::sun::star::registry::RegistryValueType_STRINGLIST;

            case com::sun::star::uno::TypeClass_BYTE:
                OSL_ASSERT(false); // this is caught by the 'binary' case

            case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
            case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
                OSL_ENSURE(false, "Unexpected UNSIGNED-List type found for configuration node");
                // FALL THRU

            case com::sun::star::uno::TypeClass_BOOLEAN:
            case com::sun::star::uno::TypeClass_SHORT:
            case com::sun::star::uno::TypeClass_LONG:
                return com::sun::star::registry::RegistryValueType_LONGLIST;

            case com::sun::star::uno::TypeClass_FLOAT:
            case com::sun::star::uno::TypeClass_DOUBLE:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support floating point number lists !");
                return com::sun::star::registry::RegistryValueType_LONGLIST;

            case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
            case com::sun::star::uno::TypeClass_HYPER:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support 64-bit integer number lists !");
                return com::sun::star::registry::RegistryValueType_LONGLIST;

            case com::sun::star::uno::TypeClass_ANY:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : Unexpected: Any as sequence element type !");
                return com::sun::star::registry::RegistryValueType_NOT_DEFINED;

            default:
                if (aElementType.equals(getBinaryDataType()))
                    OSL_ENSURE(sal_False,"OConfigurationRegistryKey::getValueType : Registry cannot support LIST of BINARY");
                else
                    OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : unknown sequence element type !");

                return com::sun::star::registry::RegistryValueType_NOT_DEFINED;
            }
        }

    default:
        OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : unknown entry type !");
        return com::sun::star::registry::RegistryValueType_NOT_DEFINED;
    }
}

//--------------------------------------------------------------------------

com::sun::star::uno::Any OConfigurationRegistryKey::implGetValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_VALUE);

    return m_xParentNode->getByName( m_sLocalName );
}

//--------------------------------------------------------------------------
sal_Int32 SAL_CALL OConfigurationRegistryKey::getLongValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aValue = implGetValue();

    sal_Int32 nLongValue(0);
    switch (aValue.getValueTypeClass())
    {
    // integral types that are small enough are straightforward
    case com::sun::star::uno::TypeClass_BYTE                : { sal_Int8   nNativeValue = 0; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case com::sun::star::uno::TypeClass_BOOLEAN         : { sal_Bool   nNativeValue = false; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case com::sun::star::uno::TypeClass_SHORT           : { sal_Int16  nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case com::sun::star::uno::TypeClass_UNSIGNED_SHORT  : { sal_uInt16 nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case com::sun::star::uno::TypeClass_LONG                : { sal_Int32  nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;

    // this is lossless, but not value-preserving - use cast to avoid warnings
    case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        {
            sal_uInt32 nNativeValue;
            aValue >>= nNativeValue;
            nLongValue = sal_Int32(nNativeValue);
        }
        break;

    // the following are larger than Long - check for loss and throw if applicable
    case com::sun::star::uno::TypeClass_HYPER:
        {
            sal_Int64 nNativeValue;
            aValue >>= nNativeValue;
            nLongValue = sal_Int32(nNativeValue);

            // check for data loss
            if (sal_Int64(nLongValue) != nNativeValue)
                throw com::sun::star::registry::InvalidValueException(UNISTRING("Unsigned Hyper value too large for long; Value cannot be retrieved using registry."), THISREF());
        }
        break;

    case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
        {
            sal_uInt64 nNativeValue;
            aValue >>= nNativeValue;
            nLongValue = sal_Int32(nNativeValue);

            // check for data loss
            if (sal_uInt64(sal_uInt32(nLongValue)) != nNativeValue)
                throw com::sun::star::registry::InvalidValueException(UNISTRING("Unsigned Hyper value too large for long; Value cannot be retrieved using registry."), THISREF());
        }
        break;

    // for floating point types we need a limit for loss checking
    case com::sun::star::uno::TypeClass_FLOAT:
        OSL_ENSURE(false, "Unexpected type FLOAT in configuration node");
        {
            // treat as double
            float fNativeValue = 0;
            if (aValue >>= fNativeValue)
                aValue <<= double(fNativeValue);
        }
        // fall thru

    case com::sun::star::uno::TypeClass_DOUBLE:
        {
            double fNativeValue = 0;
            aValue >>= fNativeValue;

            // find a reasonable allowed imprecision
            const double fEps = (2.*fNativeValue + 5.) * std::numeric_limits<double>::epsilon();

            // should be rounding here
            nLongValue = sal_Int32(fNativeValue);

            // check for data loss
            bool bRecheck = false;

            double diff = fNativeValue-double(nLongValue);
            if ( diff > fEps)
            {
                // substitute for rounding here
                if (diff > .5)
                {
                    ++nLongValue;
                    diff = fNativeValue-double(nLongValue);
                }
                bRecheck = true;
            }
            else if ( diff < -fEps)
            {
                // substitute for rounding here
                if (diff < -.5)
                {
                    --nLongValue;
                    diff = fNativeValue-double(nLongValue);
                }
                bRecheck = true;
            }

            if (bRecheck)
            {
                if (diff > fEps || diff < -fEps)
                    throw com::sun::star::registry::InvalidValueException(UNISTRING("Double value cannot fit in Long; Value cannot be retrieved using registry."), THISREF());
            }
        }
        break;

    case com::sun::star::uno::TypeClass_VOID:
        // allow NULL values, if we maybe advertise this node as long
        if (this->getValueType() == com::sun::star::registry::RegistryValueType_LONG)
            break;
        // else FALL THRU to exception

    default:
        throw com::sun::star::registry::InvalidValueException(UNISTRING("This node does not contain a long (or a compatible) value."), THISREF());
    }
    return nLongValue;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setLongValue( sal_Int32 _nValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    implSetValue(com::sun::star::uno::makeAny(_nValue));
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL OConfigurationRegistryKey::getLongListValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aValue = implGetValue();

    com::sun::star::uno::Sequence< sal_Int32 > aReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= aReturn))
    {
        // TODO : maybe it's a sequence of sal_Int8 or anything like that which we're able to convert ....

        throw com::sun::star::registry::InvalidValueException(UNISTRING("This configuration node does not contain a list of longs !"), THISREF());
    }

    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setLongListValue( const com::sun::star::uno::Sequence< sal_Int32 >& _seqValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    implSetValue(com::sun::star::uno::makeAny(_seqValue));
}

//--------------------------------------------------------------------------
rtl::OUString SAL_CALL OConfigurationRegistryKey::getAsciiValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    rtl::OUString sReturn = getStringValue();

    if (!isAscii(sReturn))
        throw com::sun::star::registry::InvalidValueException(UNISTRING("This configuration node value (a string) is not pure ASCII !"), THISREF());

    return sReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setAsciiValue( const ::rtl::OUString& _rValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( isAscii(_rValue), "The string passesd to OConfigurationRegistryKey::setAsciiValue is not pure ASCII");

    setStringValue(_rValue);
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL OConfigurationRegistryKey::getAsciiListValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Sequence<rtl::OUString> aReturn = getStringListValue();

    if (!isAscii(aReturn))
        throw com::sun::star::registry::InvalidValueException(UNISTRING("This configuration node value (a string list) is not pure ASCII !"), THISREF());

    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setAsciiListValue( const com::sun::star::uno::Sequence< ::rtl::OUString >& _seqValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( isAscii(_seqValue), "The string passesd to OConfigurationRegistryKey::setAsciiValue is not pure ASCII");

    setStringListValue(_seqValue);
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getStringValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aValue = implGetValue();

    rtl::OUString sReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= sReturn))
        throw com::sun::star::registry::InvalidValueException(UNISTRING("This node does not contain a string value."), THISREF());

    return sReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setStringValue( const ::rtl::OUString& _rValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    implSetValue(com::sun::star::uno::makeAny(_rValue));
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getStringListValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aValue = implGetValue();

    com::sun::star::uno::Sequence< rtl::OUString > aReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= aReturn))
        throw com::sun::star::registry::InvalidValueException(UNISTRING("This configuration node does not contain a list of strings !"), THISREF());

    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setStringListValue( const com::sun::star::uno::Sequence< ::rtl::OUString >& _seqValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    implSetValue(com::sun::star::uno::makeAny(_seqValue));
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL OConfigurationRegistryKey::getBinaryValue() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::registry::InvalidValueException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aValue = implGetValue();

    com::sun::star::uno::Sequence< sal_Int8 > aReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= aReturn))
        return aReturn;

    throw com::sun::star::registry::InvalidValueException(UNISTRING("This configuration node does not contain a list of strings !"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setBinaryValue( const com::sun::star::uno::Sequence< sal_Int8 >& _rValue ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    implSetValue(com::sun::star::uno::makeAny(_rValue));
}

//--------------------------------------------------------------------------
com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > OConfigurationRegistryKey::implGetKey( const ::rtl::OUString& _rKeyName )
    throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aDescendant = implGetDescendant(_rKeyName);
    if (aDescendant.getValueType().getTypeClass() == com::sun::star::uno::TypeClass_INTERFACE)
    {
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xNode;
        ::cppu::extractInterface(xNode, aDescendant);
        if (!xNode.is())
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("invalid descendant node. No XNameAccess found."), THISREF());
        return new OConfigurationRegistryKey(xNode, !m_bReadOnly);
    }
    else
    {
#if OSL_DEBUG_LEVEL > 1
        switch (aDescendant.getValueType().getTypeClass())
        {
            case com::sun::star::uno::TypeClass_STRING:
            case com::sun::star::uno::TypeClass_SHORT:
            case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
            case com::sun::star::uno::TypeClass_BYTE:
            case com::sun::star::uno::TypeClass_LONG:
            case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
            case com::sun::star::uno::TypeClass_BOOLEAN:
            case com::sun::star::uno::TypeClass_SEQUENCE:
                break;
            case com::sun::star::uno::TypeClass_VOID: // NULL value found
                break;
            default:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::openKey : unknown, invalid or unhandled descendant value type !");
        }
#endif

        OSL_ASSERT(m_xNode.is());

        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xDescParent(m_xNode);  // the parent config node of the descandent
        rtl::OUString sDescRelativeName( _rKeyName );        // local name of the descendant within xDescParent

        if (!m_xNode->hasByName(_rKeyName)) // it is a hierarchical Path -> more work
        {
            rtl::OUString sParentLocation;

            if ( !splitPath(_rKeyName, sParentLocation, sDescRelativeName) )
            {
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("Cannot split path for value. The internal registry structure seems to be corrupt."), THISREF());
            }

            if (sParentLocation.getLength())
            {
                com::sun::star::uno::Any aDescParent = implGetDescendant(sParentLocation);
                ::cppu::extractInterface(xDescParent, aDescParent);
                if (!xDescParent.is())
                    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The internal registry structure seems to be corrupt."), THISREF());
            }
        }

        OSL_ENSURE(xDescParent.is(), "No Parent Node found for value ?");
        OSL_ENSURE(xDescParent->hasByName(sDescRelativeName), "Parent Node does not contain found value ?");

        return new OConfigurationRegistryKey(aDescendant, xDescParent, sDescRelativeName, !m_bReadOnly);
    }
}

//--------------------------------------------------------------------------
com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > SAL_CALL OConfigurationRegistryKey::openKey( const ::rtl::OUString& _rKeyName ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_CHILD);

    return implGetKey(_rKeyName);
}
//--------------------------------------------------------------------------
bool OConfigurationRegistryKey::checkRelativeKeyName(rtl::OUString& _rKeyName) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    // no empty names allowed
    if (!_rKeyName.getLength())
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key name is invalid."), THISREF());

    bool bCleanPath = true;

    // cut trailing slashes
    sal_Int32 nCleanEnd = _rKeyName.getLength();
    while (nCleanEnd > 0 && _rKeyName[nCleanEnd - 1] == '/' )
        --nCleanEnd;

    if (m_xNode.is())
    {
        if (m_xNode-> hasByName(_rKeyName))
        {
            bCleanPath = false;
        }

        else
        {
            com::sun::star::uno::Reference< com::sun::star::util::XStringEscape > xSE(m_xNode, com::sun::star::uno::UNO_QUERY);

            sal_Bool bPreferLocal = xSE.is();

            if (!bPreferLocal)
            {
                com::sun::star::uno::Reference< com::sun::star::lang::XServiceInfo > xSI(m_xNode, com::sun::star::uno::UNO_QUERY);
                if (xSI.is() && xSI->supportsService(rtl::OUString::createFromAscii("com.sun.star.configuration.SetAccess")))
                    bPreferLocal = true;
            }

            if (bPreferLocal)
            {
                com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xHA(m_xNode, com::sun::star::uno::UNO_QUERY);
                rtl::OUString sCleanName = _rKeyName.copy(0, nCleanEnd);

                if (xHA.is() && xHA->hasByHierarchicalName(sCleanName))
                    bPreferLocal = false;
            }

            if (bPreferLocal && xSE.is())
            {
                _rKeyName = xSE->escapeString(_rKeyName);
            }
            bCleanPath = !bPreferLocal;
        }
    }

    if (bCleanPath)
    {
        // no absolute names ("/...") allowed
        if (_rKeyName.getStr()[0] == '/')
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key name is invalid. It must be a relative, not an absolute name."), THISREF());

        if (nCleanEnd <= 0)
            // the original name consists of slashes only
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key name is invalid."), THISREF());


        _rKeyName = _rKeyName.copy(0, nCleanEnd);
    }
    return bCleanPath;
}

//--------------------------------------------------------------------------
com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > SAL_CALL OConfigurationRegistryKey::createKey( const ::rtl::OUString& _rKeyName ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_CHILD);

    if (m_bReadOnly)
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key is read only."), THISREF());

    OSL_ENSURE(m_xNode.is(), "OConfigurationRegistryKey::createKey : somebody changed the checkValid(KAT_CHILD) behaviour !");

    rtl::OUString sKeyName(_rKeyName);
    if (checkRelativeKeyName(sKeyName))
    {
        rtl::OUString sParentName, sLocalName;

        if (!splitPath(sKeyName,sParentName, sLocalName))
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key name is invalid."), THISREF());

        if (sParentName.getLength()) // it's a nested key name
        {
            // check if we have the key already
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xDeepAccess(m_xNode, com::sun::star::uno::UNO_QUERY);
            if (xDeepAccess.is() && xDeepAccess->hasByHierarchicalName(sKeyName))
            {
                // already there - just open it
                return implGetKey(sKeyName);
            }

            // deep access, but not found. delegate it to a registry key which is one level above the to-be-created one
            com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > xSetNode = implGetKey(sParentName);
            if (!xSetNode.is())
            {
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::createKey : somebody changed the implGetKey behaviour !");
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("An internal error occured."), THISREF());
            }
            return xSetNode->createKey(sLocalName); // problem: request for a/['b/c'] might find a/b/c
        }
        else
            sKeyName = sLocalName;
    }

    // The requested new key is one level below ourself. Can't delegate the creation.
    if (m_xNode->hasByName(sKeyName) )
    {
        // already there - just open it
        return implGetKey(sKeyName);
    }

    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > xContainer(m_xNode, com::sun::star::uno::UNO_QUERY);
    if (!xContainer.is())
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The configuration node represented by this key is not a set node, you can't insert keys."), THISREF());

    com::sun::star::uno::Any aValueToInsert;

    com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xChildFactory(xContainer, com::sun::star::uno::UNO_QUERY);
    if (xChildFactory.is())
    {
        // In the configuration API, the creation of a new child is two-stage process : first you create a child which
        // is "floating", i.e. does not belong to the configuration tree, yet. After filling it with values, you insert
        // it into the container node which was used for the creation.
        // We can't map this behaviour with the registry API, so we have to combine both steps

        // create a new floating child for the container node
        try
        {
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xFloatingChild = xChildFactory->createInstance();
            OSL_ENSURE( xFloatingChild.is(), "The newly created element is NULL !");

            com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xInsertedChild(xFloatingChild, com::sun::star::uno::UNO_QUERY);
            OSL_ENSURE( xInsertedChild.is(), "The newly created element does not provide the required interface");

            if (!xInsertedChild.is())
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("An internal error occured. The objects provided by the configuration API are invalid."), THISREF());

            aValueToInsert <<= xInsertedChild; // xFloatingChild;
        }
        catch (com::sun::star::uno::RuntimeException&)
        {   // allowed to leave this method
            throw;
        }
        catch (com::sun::star::uno::Exception& e)
        {   // not allowed to leave this method
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("Unable to create a new child for the configuration node. Original error message as provided by the configuration API : ") += e.Message,
                THISREF());
        }
        OSL_ENSURE(aValueToInsert.hasValue(), "New Child node did not get into the Any ?");
    }
    else
    {
        // If the elements of the set are simple values, we need to create a matching value
        com::sun::star::uno::Type aElementType = xContainer->getElementType();
        aValueToInsert = implCreateDefaultElement(aElementType);

        OSL_ENSURE(aValueToInsert.hasValue() || aElementType.getTypeClass() == com::sun::star::uno::TypeClass_ANY, "Internal error: NULL value created for new value element ?");
    }

    // and immediately insert it into the container
    try
    {
        xContainer->insertByName(sKeyName, aValueToInsert);
    }
    catch (com::sun::star::lang::IllegalArgumentException& e)
    {
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("illegal argument to InsertByName: ") += e.Message, THISREF());
    }
    catch (com::sun::star::container::ElementExistException& e)
    {
        OSL_ENSURE(false, "There was an element of the same name inserted just now");

        // try to return that one
        try { return implGetKey(sKeyName); }
        catch (com::sun::star::uno::Exception&) { OSL_ENSURE(false, "But the other element cannot be retrieved"); }


        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("Inserting raised a NoSuchElementException for an unavailable element ! Original error message : ") += e.Message, THISREF());
    }
    catch (com::sun::star::lang::WrappedTargetException& e)
    {
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("Inserting raised a WrappedTargetException. Original error message : ") += e.Message, THISREF());
    }

    return new OConfigurationRegistryKey(aValueToInsert, m_xNode, sKeyName, !m_bReadOnly);
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::closeKey() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    OSL_ASSERT(UnoApiLock::isHeld());
    checkValid(KAT_META);

    bool bRoot = (m_sLocalName.getLength() == 0);

    if (!bRoot) // don't close, if this is the root key ..
    {
        m_xNode.clear();
        m_xParentNode.clear();
//      m_sLocalName = rtl::OUString(); - local name is const ...
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::deleteKey( const rtl::OUString& _rKeyName ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_CHILD);
    if (m_bReadOnly)
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key is read only."), THISREF());

    rtl::OUString sKeyName(_rKeyName);
    if (checkRelativeKeyName(sKeyName))
    {
        rtl::OUString sParentName, sLocalName;

        if (!splitPath(sKeyName,sParentName, sLocalName))
            throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The key name is invalid."), THISREF());

        if (sParentName.getLength()) // it's a nested key name
        {
            com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > xSetNode = implGetKey(sParentName);
            if (!xSetNode.is())
            {
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::createKey : somebody changed the implGetKey behaviour !");
                throw com::sun::star::registry::InvalidRegistryException(UNISTRING("An internal error occured."), THISREF());
            }
            xSetNode->deleteKey(sLocalName);
            return;
        }
        else
            sKeyName = sLocalName;
    }

    // The requested new key is one level below ourself. Can't delegate the creation.
    com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > xContainer(m_xNode, com::sun::star::uno::UNO_QUERY);
    if (!xContainer.is())
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("The configuration node represented by this key is not a set node, you can't remove keys."), THISREF());

    // and immediately remove it from the container
    try
    {
        xContainer->removeByName(sKeyName);
    }
    catch (com::sun::star::container::NoSuchElementException& e)
    {
        if (e.Message.getLength())
            throw com::sun::star::registry::InvalidRegistryException(e.Message, THISREF());
        else
            throw com::sun::star::registry::InvalidRegistryException((UNISTRING("There is no element named ") += sKeyName) += UNISTRING(" to remove."), THISREF());
    }
    catch (com::sun::star::lang::WrappedTargetException& e)
    {
        throw com::sun::star::registry::InvalidRegistryException(UNISTRING("Removing a node caused a WrappedTargetException. Original error message : ") += e.Message, THISREF());
    }
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > > SAL_CALL OConfigurationRegistryKey::openKeys() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_CHILD);

    com::sun::star::uno::Sequence< ::rtl::OUString > aNames(m_xNode->getElementNames());

    sal_Int32 const nCount = aNames.getLength();

    com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > > aReturn(nCount);

    for (sal_Int32 i=0; i<nCount; ++i)
        aReturn[i] = implGetKey(aNames[i]);

    return aReturn;
}

//--------------------------------------------------------------------------
com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getKeyNames() throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    checkValid(KAT_CHILD);
    return m_xNode->getElementNames();
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistryKey::createLink( const ::rtl::OUString& /*aLinkName*/, const ::rtl::OUString& /*aLinkTarget*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::deleteLink( const ::rtl::OUString& /*rLinkName*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getLinkTarget( const ::rtl::OUString& /*rLinkName*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getResolvedName( const ::rtl::OUString& /*aKeyName*/ ) throw(com::sun::star::registry::InvalidRegistryException, com::sun::star::uno::RuntimeException)
{
    throw com::sun::star::registry::InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}
//--------------------------------------------------------------------------
//..........................................................................
}   // namespace configmgr
//..........................................................................
// split path
#include "configpath.hxx"
#include "configexcept.hxx"

bool configmgr::splitPath(const rtl::OUString& _sPath, rtl::OUString& _rsParentPath, rtl::OUString& _rsLocalName)
{
    bool bResult = false;
    try
    {
        bool bAbsolute = configmgr::configuration::Path::isAbsolutePath(_sPath);
        configmgr::configuration::Path::Rep aPath ;

        if (bAbsolute)
        {
            configmgr::configuration::AbsolutePath parsedPath = configmgr::configuration::AbsolutePath::parse(_sPath) ;

            aPath = parsedPath.rep() ;
        }
        else
        {
            configmgr::configuration::RelativePath parsedPath = configmgr::configuration::RelativePath::parse(_sPath) ;

            aPath = parsedPath.rep() ;
        }
        //configmgr::configuration::Path::Rep aPath = bAbsolute ? configmgr::configuration::AbsolutePath::parse(_sPath).rep() : configmgr::configuration::RelativePath::parse(_sPath).rep();

        OSL_ENSURE(!aPath.isEmpty(), "Trying to split an empty or root path");
        std::vector<configuration::Path::Component>::const_reverse_iterator aFirst = aPath.begin(), aLast = aPath.end();

        if (aFirst != aLast)
        {
            --aLast;

            _rsLocalName = aLast->getName();
            _rsParentPath = configmgr::configuration::Path::Rep(aFirst,aLast).toString(bAbsolute);

            bResult = true;
        }
        //  else go on to fail
    }
    catch (configuration::Exception&)
    {
    }
    return bResult;
}
//..........................................................................


