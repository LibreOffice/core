/*************************************************************************
 *
 *  $RCSfile: cfgregistrykey.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2001-03-02 15:52:25 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONFIGMGR_REGISTRY_CFGREGISTRYKEY_HXX_
#include "cfgregistrykey.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTY_HPP_
#include <com/sun/star/beans/XProperty.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif

#include <limits>

#define THISREF()       static_cast< ::cppu::OWeakObject* >(this)
#define UNISTRING(c)    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(c) )

//..........................................................................
namespace configmgr
{
//..........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::cppu;
using ::rtl::OUString;

//--------------------------------------------------------------------------
namespace {
    inline
    Type getBinaryDataType()
    {
        Sequence<sal_Int8> const * const p= 0;
        return ::getCppuType(p);
    }
    inline
    bool isAscii(sal_Unicode ch)
    {
        return 0 <= ch && ch < 128;
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
    bool isAscii(OUString const& str)
    {
        return isAscii(str.getStr(),str.getLength());
    }
    inline
    bool isAscii(Sequence< OUString > const& strList)
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
OUString getNodeName(const Reference< XNameAccess >& _xNode)
{
    Reference< XNamed > xName( _xNode, UNO_QUERY );
    if (xName.is())
        return xName->getName();

    // fallback
    Reference< XHierarchicalName > xPath( _xNode, UNO_QUERY );
    if (xPath.is())
        return xPath->getHierarchicalName();

    OSL_ENSURE( !_xNode.is(), "Cannot get name of node");
    return OUString();
}
//--------------------------------------------------------------------------

OConfigurationRegistryKey::OConfigurationRegistryKey
            (const Reference< XNameAccess >& _rxRootNode
            ,sal_Bool _bWriteable
            ,SubtreeRoot
            )
    :m_aMutex()
    ,m_bReadOnly(!_bWriteable)
    ,m_xNode(_rxRootNode)
    ,m_xParentNode()
    ,m_sLocalName() // this will be treated as root - maybe use hierarchical name (ß)
{
    OSL_ENSURE(m_xNode.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid config node param !");
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey
            (const Reference< XNameAccess >& _rxNode
            ,sal_Bool _bWriteable
            )
    :m_aMutex()
    ,m_bReadOnly(!_bWriteable)
    ,m_xNode(_rxNode)
    ,m_xParentNode()
    ,m_sLocalName( getNodeName(_rxNode) ) // this will be treated as root
{
    OSL_ENSURE(m_xNode.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid config node param !");
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey(
            const Reference< XNameAccess >& _rxParentNode,
            const ::rtl::OUString& _rLocalName,
            sal_Bool _bWriteable)
    :m_aMutex()
    ,m_bReadOnly(!_bWriteable)
    ,m_xNode()
    ,m_xParentNode(_rxParentNode)
    ,m_sLocalName(_rLocalName)
{
    OSL_ENSURE(m_xParentNode.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid parent node param !");
    OSL_ENSURE(m_sLocalName.getLength(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid relative name !");
    OSL_ENSURE(m_xParentNode->hasByName( m_sLocalName ), "OConfigurationRegistryKey::OConfigurationRegistryKey : key not found in parent node !" ); //
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey(
                Any _rCurrentValue,
                const Reference< XNameAccess >& _rxParentNode,
                const ::rtl::OUString& _rLocalName,
                sal_Bool _bWriteable)
    :m_aMutex()
    ,m_bReadOnly(!_bWriteable)
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

Reference<XPropertySetInfo> OConfigurationRegistryKey::implGetParentPropertyInfo() throw(RuntimeException)
{
    if (!m_xParentNode.is())
    {
        Reference< XChild > xChild(m_xNode, UNO_QUERY);
        if (xChild.is())
            m_xParentNode = m_xParentNode.query(xChild->getParent());
    }

    Reference< XPropertySetInfo > xParentInfo; // the result

    Reference< XPropertySet > xParentProperties(m_xParentNode, UNO_QUERY);
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
sal_Bool isNodeReadOnly(Reference< XInterface > const& _xNode) throw(RuntimeException)
{
    OSL_ASSERT( _xNode.is() );

    Reference< XProperty > xProperty(_xNode, UNO_QUERY);
    if (xProperty.is())
    {
        Property aProperty = xProperty->getAsProperty();

        return (aProperty.Attributes & PropertyAttribute::READONLY) != 0;
    }

    Reference< XServiceInfo > xServiceInfo( _xNode, UNO_QUERY );
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

sal_Bool OConfigurationRegistryKey::implIsReadOnly() throw (RuntimeException)
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
        Reference< XPropertySetInfo > xParentInfo = implGetParentPropertyInfo();

        if (xParentInfo.is())
        {
            Property aProperty = xParentInfo->getPropertyByName(m_sLocalName);

            bResult = m_bReadOnly = ((aProperty.Attributes & PropertyAttribute::READONLY) != 0);
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

sal_Bool OConfigurationRegistryKey::implEnsureNode() throw (InvalidRegistryException,RuntimeException)
{
    if (!m_xNode.is())
    {
        OSL_ENSURE( m_xParentNode.is(), "implEnsureNode called for invalid registry key");
        if (m_xParentNode.is())
        {
            try
            {
                Any aNode = m_xParentNode->getByName( m_sLocalName );

                if ( !(aNode >>= m_xNode) )
                    OSL_ENSURE( ! (aNode.hasValue() && aNode.getValueTypeClass() == TypeClass_INTERFACE),
                                "OConfigurationRegistryKey: Node object does not implement expected interface");
            }
            catch (NoSuchElementException& e)
            {
                m_xParentNode.clear();

                OUString sMessage = UNISTRING("Invalid OConfigurationRegistryKey. The node \"");
                sMessage += m_sLocalName;
                sMessage += UNISTRING("\" was not found in the parent. Parent error message: \n");
                sMessage += e.Message;

                throw InvalidRegistryException(sMessage, THISREF());
            }
        }
    }
    return m_xNode.is();
}
//--------------------------------------------------------------------------

Type OConfigurationRegistryKey::implGetUnoType() throw (RuntimeException)
{
    Type aType;
    if (m_xNode.is())
    {
        aType = getCppuType(&m_xNode); // Its just an interface type
    }
    else if (m_xParentNode.is())
    {

        Reference< XPropertySetInfo > xParentInfo = implGetParentPropertyInfo();
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
        OSL_ASSERT( aType.getTypeClass() == TypeClass_VOID );
        OSL_ENSURE( false, "implGetUnoType called for invalid registry key");
    }
    return aType;
}
//--------------------------------------------------------------------------

sal_Bool OConfigurationRegistryKey::implEnsureValue() throw (RuntimeException)
{
    if (m_xNode.is())
        return false;

    OSL_ENSURE( m_xParentNode.is(), "implEnsureValue called for invalid registry key");
    if (!m_xParentNode.is())
        return false;

    switch (implGetUnoType().getTypeClass())
    {
    case TypeClass_INTERFACE:
        return false;

    case TypeClass_BYTE:
    case TypeClass_UNSIGNED_SHORT:
    case TypeClass_UNSIGNED_LONG:
    case TypeClass_UNSIGNED_HYPER:
    case TypeClass_FLOAT:
        OSL_ENSURE(false, "Unexpected (UNSIGNED INTERGRAL or FLOAT) type found for configuration node");

    case TypeClass_STRING:
    case TypeClass_BOOLEAN:
    case TypeClass_SHORT:
    case TypeClass_LONG:
    case TypeClass_HYPER:
    case TypeClass_DOUBLE:
    case TypeClass_SEQUENCE:
        return true;

    case TypeClass_ANY:
        return true;

    case TypeClass_VOID:
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

void OConfigurationRegistryKey::checkValid(KEY_ACCESS_TYPE _eIntentedAccess) throw (InvalidRegistryException,RuntimeException)
{
    if (!implIsValid())
        throw InvalidRegistryException(UNISTRING("The registry is not bound to a configuration node anymore."), THISREF());
        // "anymore", because at the moment the ctor was called it probably was bound ....

    switch (_eIntentedAccess)
    {
        case KAT_VALUE_WRITE:
            if (implIsReadOnly())
                throw InvalidRegistryException(UNISTRING("This configuration node is not writeable."), THISREF());

            // !!! NO !!! BREAK !!!
        case KAT_VALUE:
            if (m_xNode.is())
                throw InvalidRegistryException(UNISTRING("This configuration node is not a value, but an internal container."), THISREF());

            if (!m_xParentNode.is())
                throw InvalidRegistryException(UNISTRING("This configuration node is invalid. It has no parent."), THISREF());

            if (!implEnsureValue())
                throw InvalidRegistryException(UNISTRING("This configuration does not have a legal value type."), THISREF());
            break;

        case KAT_CHILD:
            if (!implEnsureNode())
                throw InvalidRegistryException(UNISTRING("This configuration node does not have children, it is a value node."), THISREF());
            break;

        case KAT_META:
            break;
    }
}

//--------------------------------------------------------------------------
Any OConfigurationRegistryKey::implCreateDefaultElement(Type const& _aValueType) throw (RuntimeException)
{
    Any aReturn;

    switch (_aValueType.getTypeClass())
    {
    case TypeClass_STRING:
        aReturn <<= OUString();
        break;

    // we don't distinguish between the different integer types or boolean
    // (the RegistryKeyType is not granular enough),
    // but we can't handle them all the same way here
    case TypeClass_BYTE:
    case TypeClass_UNSIGNED_SHORT:
    case TypeClass_SHORT:
        aReturn <<= (sal_Int16)0;
        break;

    case TypeClass_UNSIGNED_LONG:
    case TypeClass_LONG:
        aReturn <<= (sal_Int32)0;
        break;

    case TypeClass_BOOLEAN:
        aReturn <<= sal_Bool(false);
        break;

    // we cannot really handle 64-bit ints in the registry (but here we can)
    case TypeClass_UNSIGNED_HYPER:
    case TypeClass_HYPER:
        OSL_ENSURE(false, "Warning: cannot handle 64-bit values correctly in registry");
        aReturn <<= (sal_Int64)0;
        break;

    // we cannot really handle doubles in the registry (but here we can)
    case TypeClass_FLOAT:
    case TypeClass_DOUBLE:
        OSL_ENSURE(false, "Warning: cannot handle DOUBLE correctly in registry");
        aReturn <<= (double)0;
        break;

    // we really want to leave an Any as NULL - hopefully this is acceptable to the set
    case TypeClass_ANY:
        break;

    case TypeClass_SEQUENCE:
        if (_aValueType == getBinaryDataType())
            aReturn <<= Sequence< sal_Int8 >();

        else
        {
            Type aElementType = getSequenceElementType(_aValueType);
            switch (aElementType.getTypeClass())
            {
            case TypeClass_STRING:
                aReturn <<= Sequence< OUString >();
                break;

            case TypeClass_BYTE:
            case TypeClass_UNSIGNED_SHORT:
            case TypeClass_SHORT:
                aReturn <<= Sequence< sal_Int16 >();
                break;

            case TypeClass_UNSIGNED_LONG:
            case TypeClass_LONG:
                aReturn <<= Sequence< sal_Int32 >();
                break;

            case TypeClass_BOOLEAN:
                aReturn <<= Sequence< sal_Bool >();
                break;

            case TypeClass_UNSIGNED_HYPER:
            case TypeClass_HYPER:
                aReturn <<= Sequence< sal_Int64 >();
                break;

            case TypeClass_FLOAT:
            case TypeClass_DOUBLE:
                aReturn <<= Sequence< double >();
                break;

            case TypeClass_SEQUENCE:
                OSL_ENSURE(false, "Warning: cannot handle Sequence< BINARY > correctly in registry");
                if (aElementType == getBinaryDataType())
                {
                    OSL_ENSURE(false, "Warning: cannot handle Sequence< BINARY > correctly in registry");
                    aReturn <<= Sequence< Sequence< sal_Int8 > >();
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

    case TypeClass_INTERFACE:
        OSL_ENSURE(false, "Invalid call to OConfigurationRegistryKey::implCreateDefaultElement. Inner nodes must be created by a factory");
        break;

    default:
        OSL_ENSURE(false, "Unexpected value type for configuration node - returning NULL");
        // throw here ??
        break;
    }

    OSL_ENSURE( aReturn.getValueType() == _aValueType || (_aValueType.getTypeClass() == TypeClass_ANY && !aReturn.hasValue()),
                "Warning: Unexpected data type found in Registry - returning similar value or NULL");
    return aReturn;
}

//--------------------------------------------------------------------------
Any OConfigurationRegistryKey::implGetDescendant(const OUString& _rDescendantName) throw(InvalidRegistryException, RuntimeException)
{
    Any aElementReturn;

    try
    {
        if (-1 != _rDescendantName.indexOf('/'))
        {
            Reference< XHierarchicalNameAccess > xDeepAccess( m_xNode, UNO_QUERY );
            if (xDeepAccess.is())
                aElementReturn = xDeepAccess->getByHierarchicalName(_rDescendantName);
            else
                throw InvalidRegistryException(UNISTRING("Nested element access not supported by this node."), THISREF());
        }
        else
        {
            if (m_xNode.is())
                aElementReturn = m_xNode->getByName(_rDescendantName);

            else
            {
                // implEnsureNode should have been called before this method
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getDescendant : invalid call !");

                // this method should not be called if the object does not represent a container node ...
                throw InvalidRegistryException(UNISTRING("invalid object."), THISREF());
            }
        }
    }
    catch(NoSuchElementException&)
    {   // not allowed to leave the method, wrap it
        OUString sMessage(UNISTRING("There is no element named "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(".");
        throw InvalidRegistryException(sMessage, THISREF());
    }
    catch(WrappedTargetException& wte)
    {   // allowed to be thrown by XNameAccess::getByName, but not allowed to leave this method
        OUString sMessage(UNISTRING("The configuration node could not provide an element for "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(". Original Error: ");
        sMessage += wte.Message;
        throw InvalidRegistryException(sMessage, THISREF());
    }

    return aElementReturn;
}

//--------------------------------------------------------------------------
void OConfigurationRegistryKey::implSetValue(const Any& _rValue) throw(InvalidRegistryException, RuntimeException)
{
    checkValid(KAT_VALUE_WRITE);

    // one possible interface
    Reference< XNameReplace > xParentValueAccess(m_xParentNode, UNO_QUERY);
    if (xParentValueAccess.is())
    {
        try
        {
            xParentValueAccess->replaceByName(m_sLocalName, _rValue);
        }
        catch(IllegalArgumentException& iae)
        {
            OUString sMessage = UNISTRING("Unable to replace the old value. The configuration node threw an ");
            sMessage += UNISTRING("IllegalArgumentException: ");
            sMessage += iae.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        catch(NoSuchElementException& nse)
        {
            OSL_ENSURE(false, "OConfigurationRegistryKey::writeValueNode : a NoSuchElementException should be impossible !");

            OUString sMessage = UNISTRING("Unable to replace the old value. The configuration node threw an ");
            sMessage += UNISTRING("NoSuchElementException: ");
            sMessage += nse.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        catch(WrappedTargetException& wte)
        {
            OUString sMessage = UNISTRING("Unable to replace the old value. The configuration node threw an ");
            sMessage += UNISTRING("WrappedTargetException: ");
            sMessage += wte.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        return;
    }

    // not found - try other interface
    Reference< XPropertySet > xParentPropertySet(m_xParentNode, UNO_QUERY);
    if (xParentPropertySet.is())
    {
        try
        {
            xParentPropertySet->setPropertyValue(m_sLocalName, _rValue);
        }
        catch(IllegalArgumentException& iae)
        {
            OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("IllegalArgumentException: ");
            sMessage += iae.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        catch(UnknownPropertyException& upe)
        {
            OSL_ENSURE(false, "OConfigurationRegistryKey::writeValueNode : a UnknownPropertyException should be impossible !");

            OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("UnknownPropertyException: ");
            sMessage += upe.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        catch(PropertyVetoException& pve)
        {
            OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("PropertyVetoException: ");
            sMessage += pve.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        catch(WrappedTargetException& wte)
        {
            OUString sMessage = UNISTRING("Unable to set a new value. The configuration node threw an ");
            sMessage += UNISTRING("WrappedTargetException: ");
            sMessage += wte.Message;
            throw InvalidRegistryException(sMessage, THISREF());
        }
        return;
    }

    throw InvalidRegistryException(UNISTRING("No interface found on parent node for writing to configuration value node."), THISREF());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getKeyName() throw(RuntimeException)
{
    return m_sLocalName;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistryKey::isReadOnly(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_META);
    return m_bReadOnly;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistryKey::isValid(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    // TODO : perhaps if the registry we're a part of is closed ....
    return implIsValid();
}

//--------------------------------------------------------------------------
RegistryKeyType SAL_CALL OConfigurationRegistryKey::getKeyType( const ::rtl::OUString& _rKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    // no further checks are made (for performance reasons) ...
    // Maybe we should check only KAT_META for consistency ?
    checkValid(KAT_CHILD);

    return RegistryKeyType_KEY;
}

//--------------------------------------------------------------------------
RegistryValueType SAL_CALL OConfigurationRegistryKey::getValueType(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_META);

    const Type aUnoType = implGetUnoType();

    switch (aUnoType.getTypeClass())
    {
    case TypeClass_INTERFACE:   // this is really a case of 'no value type'
        return RegistryValueType_NOT_DEFINED;

    case TypeClass_ANY: // this is really a case of 'all value types allowed'
        return RegistryValueType_NOT_DEFINED;

    case TypeClass_STRING:
        return RegistryValueType_STRING;

    case TypeClass_BYTE:
    case TypeClass_UNSIGNED_SHORT:
    case TypeClass_UNSIGNED_LONG:
        OSL_ENSURE(false, "Unexpected UNSIGNED type found for configuration node");
        // FALL THRU

    case TypeClass_BOOLEAN:
    case TypeClass_SHORT:
    case TypeClass_LONG:
        return RegistryValueType_LONG;

    case TypeClass_FLOAT:
    case TypeClass_DOUBLE:
        OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support floating point numbers !");
        return RegistryValueType_LONG;

    case TypeClass_UNSIGNED_HYPER:
    case TypeClass_HYPER:
        OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support 64-bit integer numbers !");
        return RegistryValueType_LONG;

    case TypeClass_SEQUENCE:
        if ( aUnoType.equals( getBinaryDataType() ) )
            return RegistryValueType_BINARY;

        else
        {
            Type aElementType = getSequenceElementType(aUnoType);

            switch (aElementType.getTypeClass())
            {
            case TypeClass_STRING:
                return RegistryValueType_STRINGLIST;

            case TypeClass_BYTE:
                OSL_ASSERT(false); // this is caught by the 'binary' case

            case TypeClass_UNSIGNED_SHORT:
            case TypeClass_UNSIGNED_LONG:
                OSL_ENSURE(false, "Unexpected UNSIGNED-List type found for configuration node");
                // FALL THRU

            case TypeClass_BOOLEAN:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                return RegistryValueType_LONGLIST;

            case TypeClass_FLOAT:
            case TypeClass_DOUBLE:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support floating point number lists !");
                return RegistryValueType_LONGLIST;

            case TypeClass_UNSIGNED_HYPER:
            case TypeClass_HYPER:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support 64-bit integer number lists !");
                return RegistryValueType_LONGLIST;

            case TypeClass_ANY:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : Unexpected: Any as sequence element type !");
                return RegistryValueType_NOT_DEFINED;

            default:
                if (aElementType.equals(getBinaryDataType()))
                    OSL_ENSURE(sal_False,"OConfigurationRegistryKey::getValueType : Registry cannot support LIST of BINARY");
                else
                    OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : unknown sequence element type !");

                return RegistryValueType_NOT_DEFINED;
            }
        }

    default:
        OSL_ENSURE(sal_False, "OConfigurationRegistryKey::getValueType : unknown entry type !");
        return RegistryValueType_NOT_DEFINED;
    }
}

//--------------------------------------------------------------------------

Any OConfigurationRegistryKey::implGetValue() throw(InvalidRegistryException, RuntimeException)
{
    checkValid(KAT_VALUE);

    return m_xParentNode->getByName( m_sLocalName );
}

//--------------------------------------------------------------------------
sal_Int32 SAL_CALL OConfigurationRegistryKey::getLongValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Any aValue = implGetValue();

    sal_Int32 nLongValue(0);
    switch (aValue.getValueTypeClass())
    {
    // integral types that are small enough are straightforward
    case TypeClass_BYTE             : { sal_Int8   nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case TypeClass_BOOLEAN          : { sal_Bool   nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case TypeClass_SHORT            : { sal_Int16  nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case TypeClass_UNSIGNED_SHORT   : { sal_uInt16 nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;
    case TypeClass_LONG             : { sal_Int32  nNativeValue; aValue >>= nNativeValue; nLongValue = nNativeValue; } break;

    // this is lossless, but not value-preserving - use cast to avoid warnings
    case TypeClass_UNSIGNED_LONG:
        {
            sal_uInt32 nNativeValue;
            aValue >>= nNativeValue;
            nLongValue = sal_Int32(nNativeValue);
        }
        break;

    // the following are larger than Long - check for loss and throw if applicable
    case TypeClass_HYPER:
        {
            sal_Int64 nNativeValue;
            aValue >>= nNativeValue;
            nLongValue = sal_Int32(nNativeValue);

            // check for data loss
            if (sal_Int64(nLongValue) != nNativeValue)
                throw InvalidValueException(UNISTRING("Unsigned Hyper value too large for long; Value cannot be retrieved using registry."), THISREF());
        }
        break;

    case TypeClass_UNSIGNED_HYPER:
        {
            sal_uInt64 nNativeValue;
            aValue >>= nNativeValue;
            nLongValue = sal_Int32(nNativeValue);

            // check for data loss
            if (sal_uInt64(sal_uInt32(nLongValue)) != nNativeValue)
                throw InvalidValueException(UNISTRING("Unsigned Hyper value too large for long; Value cannot be retrieved using registry."), THISREF());
        }
        break;

    // for floating point types we need a limit for loss checking
    case TypeClass_FLOAT:
        OSL_ENSURE(false, "Unexpected type FLOAT in configuration node");
        {
            // treat as double
            float fNativeValue;
            if (aValue >>= fNativeValue)
                aValue <<= double(fNativeValue);
        }
        // fall thru

    case TypeClass_DOUBLE:
        {
            double fNativeValue;
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
                    throw InvalidValueException(UNISTRING("Double value cannot fit in Long; Value cannot be retrieved using registry."), THISREF());
            }
        }
        break;

    case TypeClass_VOID:
        // allow NULL values, if we maybe advertise this node as long
        if (this->getValueType() == RegistryValueType_LONG)
            break;
        // else FALL THRU to exception

    default:
        throw InvalidValueException(UNISTRING("This node does not contain a long (or a compatible) value."), THISREF());
    }
    return nLongValue;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setLongValue( sal_Int32 _nValue ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    implSetValue(makeAny(_nValue));
}

//--------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OConfigurationRegistryKey::getLongListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Any aValue = implGetValue();

    Sequence< sal_Int32 > aReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= aReturn))
    {
        // TODO : maybe it's a sequence of sal_Int8 or anything like that which we're able to convert ....

        throw InvalidValueException(UNISTRING("This configuration node does not contain a list of longs !"), THISREF());
    }

    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setLongListValue( const Sequence< sal_Int32 >& _seqValue ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    implSetValue(makeAny(_seqValue));
}

//--------------------------------------------------------------------------
OUString SAL_CALL OConfigurationRegistryKey::getAsciiValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    OUString sReturn = getStringValue();

    if (!isAscii(sReturn))
        throw InvalidValueException(UNISTRING("This configuration node value (a string) is not pure ASCII !"), THISREF());

    return sReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setAsciiValue( const ::rtl::OUString& _rValue ) throw(InvalidRegistryException, RuntimeException)
{
    OSL_ENSURE( isAscii(_rValue), "The string passesd to OConfigurationRegistryKey::setAsciiValue is not pure ASCII");

    setStringValue(_rValue);
}

//--------------------------------------------------------------------------
Sequence< OUString > SAL_CALL OConfigurationRegistryKey::getAsciiListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Sequence<OUString> aReturn = getStringListValue();

    if (!isAscii(aReturn))
        throw InvalidValueException(UNISTRING("This configuration node value (a string list) is not pure ASCII !"), THISREF());

    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setAsciiListValue( const Sequence< ::rtl::OUString >& _seqValue ) throw(InvalidRegistryException, RuntimeException)
{
    OSL_ENSURE( isAscii(_seqValue), "The string passesd to OConfigurationRegistryKey::setAsciiValue is not pure ASCII");

    setStringListValue(_seqValue);
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getStringValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Any aValue = implGetValue();

    OUString sReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= sReturn))
        throw InvalidValueException(UNISTRING("This node does not contain a string value."), THISREF());

    return sReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setStringValue( const ::rtl::OUString& _rValue ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    implSetValue(makeAny(_rValue));
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getStringListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Any aValue = implGetValue();

    Sequence< OUString > aReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= aReturn))
        throw InvalidValueException(UNISTRING("This configuration node does not contain a list of strings !"), THISREF());

    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setStringListValue( const Sequence< ::rtl::OUString >& _seqValue ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    implSetValue(makeAny(_seqValue));
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OConfigurationRegistryKey::getBinaryValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Any aValue = implGetValue();

    Sequence< sal_Int8 > aReturn;
    if (!aValue.hasValue())
        checkNullable();// let NULL values pass

    else if (!(aValue >>= aReturn))
        return aReturn;

    throw InvalidValueException(UNISTRING("This configuration node does not contain a list of strings !"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setBinaryValue( const Sequence< sal_Int8 >& _rValue ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    implSetValue(makeAny(_rValue));
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > OConfigurationRegistryKey::implGetKey( const ::rtl::OUString& _rKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Any aDescendant = implGetDescendant(_rKeyName);
    if (aDescendant.getValueType().getTypeClass() == TypeClass_INTERFACE)
    {
        Reference< XNameAccess > xNode;
        ::cppu::extractInterface(xNode, aDescendant);
        if (!xNode.is())
            throw InvalidRegistryException(UNISTRING("invalid descendant node. No XNameAccess found."), THISREF());
        return new OConfigurationRegistryKey(xNode, !m_bReadOnly);
    }
    else
    {
#ifdef DEBUG
        switch (aDescendant.getValueType().getTypeClass())
        {
            case TypeClass_STRING:
            case TypeClass_SHORT:
            case TypeClass_UNSIGNED_SHORT:
            case TypeClass_BYTE:
            case TypeClass_LONG:
            case TypeClass_UNSIGNED_LONG:
            case TypeClass_BOOLEAN:
            case TypeClass_SEQUENCE:
                break;
            case TypeClass_VOID: // NULL value found
                break;
            default:
                OSL_ENSURE(sal_False, "OConfigurationRegistryKey::openKey : unknown, invalid or unhandled descendant value type !");
        }
#endif

        Reference< XNameAccess > xDescParent(m_xNode);  // the parent config node of the descandent

        OUString sDescRelativeName(_rKeyName);
        sal_Int32 nSeparatorPos = _rKeyName.lastIndexOf('/');

        if ( (nSeparatorPos > 0) && (nSeparatorPos == (_rKeyName.getLength() - 1)) )
        {
            // recognize a trailing slash
            sDescRelativeName = sDescRelativeName.copy(0, nSeparatorPos);
            OSL_ASSERT(sDescRelativeName.getLength() == nSeparatorPos);

            nSeparatorPos = sDescRelativeName.lastIndexOf('/');
            OSL_ASSERT(sDescRelativeName.getLength() > nSeparatorPos);
        }

        if (nSeparatorPos >= 1)
        {
            Any aDescParent = implGetDescendant(_rKeyName.copy(0, nSeparatorPos));
            ::cppu::extractInterface(xDescParent, aDescParent);
            if (!xDescParent.is())
                throw InvalidRegistryException(UNISTRING("The internal registry structure seems to be corrupt."), THISREF());

            // will be the name of the new key relative to it's parent
            sDescRelativeName = sDescRelativeName.copy(nSeparatorPos + 1);
        }
        else if (nSeparatorPos == 0)
        {
            OSL_ENSURE(false, "Component root path specified for value ?");
            throw InvalidRegistryException(UNISTRING("Component root path found for value. The internal registry structure seems to be corrupt."), THISREF());

        }

        OSL_ENSURE(xDescParent.is(), "No Parent Node found for value ?");
        OSL_ENSURE(xDescParent->hasByName(sDescRelativeName), "Parent Node does not contain found value ?");

        return new OConfigurationRegistryKey(aDescendant, xDescParent, sDescRelativeName, !m_bReadOnly);
    }
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > SAL_CALL OConfigurationRegistryKey::openKey( const ::rtl::OUString& _rKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);

    return implGetKey(_rKeyName);
}

//--------------------------------------------------------------------------
void OConfigurationRegistryKey::checkRelativeKeyName(OUString& _rKeyName) throw(InvalidRegistryException, RuntimeException)
{
    // no empty names allowed
    if (!_rKeyName.getLength())
        throw InvalidRegistryException(UNISTRING("The key name is invalid."), THISREF());

    // no absolute names ("/...") allowed
    if (_rKeyName.getStr()[0] == '/')
        throw InvalidRegistryException(UNISTRING("The key name is invalid. It must be a relative, not an absolute name."), THISREF());

    // cut trailing slashes
    while (_rKeyName.getLength() && (_rKeyName.getStr()[_rKeyName.getLength() - 1] == '/'))
        _rKeyName = _rKeyName.copy(0, _rKeyName.getLength() - 1);

    if (!_rKeyName.getLength())
        // the original name consists of slashes only
        throw InvalidRegistryException(UNISTRING("The key name is invalid."), THISREF());
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > SAL_CALL OConfigurationRegistryKey::createKey( const ::rtl::OUString& _rKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);

    if (m_bReadOnly)
        throw InvalidRegistryException(UNISTRING("The key is read only."), THISREF());

    OSL_ENSURE(m_xNode.is(), "OConfigurationRegistryKey::createKey : somebody changed the checkValid(KAT_CHILD) behaviour !");

    OUString sKeyName(_rKeyName);
    checkRelativeKeyName(sKeyName);

    sal_Int32 nSeparatorPos = sKeyName.lastIndexOf('/');
    if (-1 != nSeparatorPos)
    {
        // check if we have the key already
        Reference< XHierarchicalNameAccess > xDeepAccess(m_xNode, UNO_QUERY);
        if (xDeepAccess.is() && xDeepAccess->hasByHierarchicalName(sKeyName))
        {
            // already there - just open it
            return implGetKey(sKeyName);
        }

        // deep access, but not found. delegate it to a registry key which is one level above the to-be-created one
        OUString sSetNodeName = sKeyName.copy(0, nSeparatorPos);
        sKeyName = sKeyName.copy(nSeparatorPos + 1);

        Reference< XRegistryKey > xSetNode = implGetKey(sSetNodeName);
        if (!xSetNode.is())
        {
            OSL_ENSURE(sal_False, "OConfigurationRegistryKey::createKey : somebody changed the implGetKey behaviour !");
            throw InvalidRegistryException(UNISTRING("An internal error occured."), THISREF());
        }
        return xSetNode->createKey(sKeyName);
    }

    // The requested new key is one level below ourself. Can't delegate the creation.
    if (m_xNode->hasByName(sKeyName) )
    {
        // already there - just open it
        return implGetKey(sKeyName);
    }

    Reference< XNameContainer > xContainer(m_xNode, UNO_QUERY);
    if (!xContainer.is())
        throw InvalidRegistryException(UNISTRING("The configuration node represented by this key is not a set node, you can't insert keys."), THISREF());

    Any aValueToInsert;

    Reference< XSingleServiceFactory > xChildFactory(xContainer, UNO_QUERY);
    if (xChildFactory.is())
    {
        // In the configuration API, the creation of a new child is two-stage process : first you create a child which
        // is "floating", i.e. does not belong to the configuration tree, yet. After filling it with values, you insert
        // it into the container node which was used for the creation.
        // We can't map this behaviour with the registry API, so we have to combine both steps

        // create a new floating child for the container node
        try
        {
            Reference< XInterface > xFloatingChild = xChildFactory->createInstance();
            OSL_ENSURE( xFloatingChild.is(), "The newly created element is NULL !");

            Reference< XNameAccess > xInsertedChild(xFloatingChild, UNO_QUERY);
            OSL_ENSURE( xInsertedChild.is(), "The newly created element does not provide the required interface");

            if (!xInsertedChild.is())
                throw InvalidRegistryException(UNISTRING("An internal error occured. The objects provided by the configuration API are invalid."), THISREF());

            aValueToInsert <<= xInsertedChild; // xFloatingChild;
        }
        catch (RuntimeException&)
        {   // allowed to leave this method
            throw;
        }
        catch (Exception& e)
        {   // not allowed to leave this method
            throw InvalidRegistryException(UNISTRING("Unable to create a new child for the configuration node. Original error message as provided by the configuration API : ") += e.Message,
                THISREF());
        }
        OSL_ENSURE(aValueToInsert.hasValue(), "New Child node did not get into the Any ?");
    }
    else
    {
        // If the elements of the set are simple values, we need to create a matching value
        Type aElementType = xContainer->getElementType();
        aValueToInsert = implCreateDefaultElement(aElementType);

        OSL_ENSURE(aValueToInsert.hasValue() || aElementType.getTypeClass() == TypeClass_ANY, "Internal error: NULL value created for new value element ?");
    }

    // and immediately insert it into the container
    try
    {
        xContainer->insertByName(sKeyName, aValueToInsert);
    }
    catch (IllegalArgumentException& e)
    {
        throw InvalidRegistryException(UNISTRING("illegal argument to InsertByName: ") += e.Message, THISREF());
    }
    catch (ElementExistException& e)
    {
        OSL_ENSURE(false, "There was an element of the same name inserted just now");

        // try to return that one
        try { return implGetKey(sKeyName); }
        catch (Exception&) { OSL_ENSURE(false, "But the other element cannot be retrieved"); }


        throw InvalidRegistryException(UNISTRING("Inserting raised a NoSuchElementException for an unavailable element ! Original error message : ") += e.Message, THISREF());
    }
    catch (WrappedTargetException& e)
    {
        throw InvalidRegistryException(UNISTRING("Inserting raised a WrappedTargetException. Original error message : ") += e.Message, THISREF());
    }

    return new OConfigurationRegistryKey(aValueToInsert, m_xNode, sKeyName, !m_bReadOnly);
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::closeKey(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    checkValid(KAT_META);

    m_xNode.clear();
    m_xParentNode.clear();
//  m_sLocalName = OUString(); - local name is const ...
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::deleteKey( const OUString& _rKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    checkValid(KAT_CHILD);
    if (m_bReadOnly)
        throw InvalidRegistryException(UNISTRING("The key is read only."), THISREF());

    OUString sKeyName(_rKeyName);
    checkRelativeKeyName(sKeyName);

    sal_Int32 nSeparatorPos = sKeyName.lastIndexOf('/');
    if (-1 != nSeparatorPos)
    {
        // deep access. delegate it to a registry key which is one level above the to-be-created one
        ::rtl::OUString sSetNodeName = sKeyName.copy(0, nSeparatorPos);
        sKeyName = sKeyName.copy(nSeparatorPos + 1);

        Reference< XRegistryKey > xSetNode = implGetKey(sSetNodeName);
        if (!xSetNode.is())
        {
            OSL_ENSURE(sal_False, "OConfigurationRegistryKey::createKey : somebody changed the implGetKey behaviour !");
            throw InvalidRegistryException(UNISTRING("An internal error occured."), THISREF());
        }
        xSetNode->deleteKey(sKeyName);
        return;
    }

    // The requested new key is one level below ourself. Can't delegate the creation.
    Reference< XNameContainer > xContainer(m_xNode, UNO_QUERY);
    if (!xContainer.is())
        throw InvalidRegistryException(UNISTRING("The configuration node represented by this key is not a set node, you can't remove keys."), THISREF());

    // and immediately remove it from the container
    try
    {
        xContainer->removeByName(sKeyName);
    }
    catch (NoSuchElementException& e)
    {
        if (e.Message.getLength())
            throw InvalidRegistryException(e.Message, THISREF());
        else
            throw InvalidRegistryException((UNISTRING("There is no element named ") += sKeyName) += UNISTRING(" to remove."), THISREF());
    }
    catch (WrappedTargetException& e)
    {
        throw InvalidRegistryException(UNISTRING("Removing a node caused a WrappedTargetException. Original error message : ") += e.Message, THISREF());
    }
}

//--------------------------------------------------------------------------
Sequence< Reference< XRegistryKey > > SAL_CALL OConfigurationRegistryKey::openKeys(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);

    Sequence< ::rtl::OUString > aNames(m_xNode->getElementNames());

    sal_Int32 const nCount = aNames.getLength();

    Sequence< Reference< XRegistryKey > > aReturn(nCount);

    for (sal_Int32 i=0; i<nCount; ++i)
        aReturn[i] = implGetKey(aNames[i]);

    return aReturn;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getKeyNames(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);
    return m_xNode->getElementNames();
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConfigurationRegistryKey::createLink( const ::rtl::OUString& aLinkName, const ::rtl::OUString& aLinkTarget ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    throw InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::deleteLink( const ::rtl::OUString& rLinkName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    throw InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getLinkTarget( const ::rtl::OUString& rLinkName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    throw InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getResolvedName( const ::rtl::OUString& aKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    throw InvalidRegistryException(UNISTRING("This registry, which is base on a configuration tree, does not support links."), THISREF());
}
//--------------------------------------------------------------------------
//..........................................................................
}   // namespace configmgr
//..........................................................................


