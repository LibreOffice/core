/*************************************************************************
 *
 *  $RCSfile: cfgregistrykey.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:13:41 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif

#define THISREF()       static_cast< ::cppu::OWeakObject* >(this)
#define UNISTRING(c)    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(c) )

//..........................................................................
namespace configmgr
{
//..........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::cppu;

//==========================================================================
//= OConfigurationRegistryKey
//==========================================================================

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey
            (const Reference< XNameAccess >& _rxContainerNode
            ,sal_Bool _bWriteable
            ,SubtreeRoot
            )
    :m_xNodeAccess(_rxContainerNode)
    ,m_bReadOnly(!_bWriteable)
    ,m_xNodeDeepAccess(_rxContainerNode, UNO_QUERY)
{
    OSL_ENSHURE(m_xNodeAccess.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid config node param !");
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey(
            const Reference< XNameAccess >& _rxContainerNode,
            const ::rtl::OUString& _rLocalName,
            sal_Bool _bWriteable)
    :m_xNodeAccess(_rxContainerNode)
    ,m_bReadOnly(!_bWriteable)
    ,m_xNodeDeepAccess(_rxContainerNode, UNO_QUERY)
    ,m_sLocalName(_rLocalName)
{
    OSL_ENSHURE(m_xNodeAccess.is(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid config node param !");
    OSL_ENSHURE(m_sLocalName.getLength(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid relative name !");
}

//--------------------------------------------------------------------------
OConfigurationRegistryKey::OConfigurationRegistryKey(
                Any _rCurrentValue,
                const Reference< XNameAccess >& _rxParentNode,
                const ::rtl::OUString& _rRelativeName,
                sal_Bool _bWriteable)
    :m_aLeafElement(_rCurrentValue)
    ,m_xLeafParent(_rxParentNode)
    ,m_sLocalName(_rRelativeName)
    ,m_bReadOnly(!_bWriteable)
{
    OSL_ENSHURE(_rxParentNode.is() && _rCurrentValue.hasValue(),
        "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid value and/or parent !");
    OSL_ENSHURE(m_bReadOnly || Reference< XNameReplace >(_rxParentNode, UNO_QUERY).is(),
        "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid parent (no value write access) !");
    OSL_ENSHURE(m_sLocalName.getLength(), "OConfigurationRegistryKey::OConfigurationRegistryKey : invalid relative name !");
}

//--------------------------------------------------------------------------
void OConfigurationRegistryKey::checkValid(KEY_ACCESS_TYPE _eIntentedAccess) throw (InvalidRegistryException)
{
    if (!isValid())
        throw InvalidRegistryException(UNISTRING("The registry is not bound to a configuration node anymore."), THISREF());
        // "anymore", because at the moment the ctor was called it probably was bound ....
    switch (_eIntentedAccess)
    {
        case KAT_VALUE_WRITE:
            if (m_bReadOnly)
                throw InvalidRegistryException(UNISTRING("This configuration node is not writeable."), THISREF());
            // !!! NO !!! BREAK !!!
        case KAT_VALUE:
            if (!m_aLeafElement.hasValue())
                throw InvalidRegistryException(UNISTRING("This configuration node does not have a value, it is a container."), THISREF());
            if (!m_xLeafParent.is())
                throw InvalidRegistryException(UNISTRING("This configuration nodes parent is invalid."), THISREF());
            break;
        case KAT_CHILD:
            if (!m_xNodeAccess.is())
                throw InvalidRegistryException(UNISTRING("This configuration node does not have children, it is a value node."), THISREF());
            break;
    }
}

//--------------------------------------------------------------------------
Any OConfigurationRegistryKey::getDescendant(const ::rtl::OUString& _rDescendantName) throw(InvalidRegistryException)
{
    Any aElementReturn;

    try
    {
        if (-1 != _rDescendantName.indexOf('/'))
        {
            if (m_xNodeDeepAccess.is())
                aElementReturn = m_xNodeDeepAccess->getByHierarchicalName(_rDescendantName);
            else
                throw InvalidRegistryException(UNISTRING("Nested element access not supported by this node."), THISREF());
        }
        else
        {
            if (m_xNodeAccess.is())
                aElementReturn = m_xNodeAccess->getByName(_rDescendantName);
            else
            {
                OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::getDescendant : invalid call !");
                    // this method should not be called if the object does not represent a container node ...
                throw InvalidRegistryException(UNISTRING("invalid object."), THISREF());
            }
        }
    }
    catch(NoSuchElementException&)
    {   // not allowed to leave the method, wrap it
        ::rtl::OUString sMessage(UNISTRING("There is no element named "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(".");
        throw InvalidRegistryException(sMessage, THISREF());
    }
    catch(WrappedTargetException&)
    {   // allowed to be thrown by XNameAccess::getByName, but not allowed to leave this method
        ::rtl::OUString sMessage(UNISTRING("The configuration node could not provide an element for "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(".");
        throw InvalidRegistryException(sMessage, THISREF());
    }

    if (!aElementReturn.hasValue())
    {   // suspicious .... either there is an element, or there is none, but in the latter case an exception should
        // have been thrown.
        ::rtl::OUString sMessage(UNISTRING("There is no element named "));
        sMessage += _rDescendantName;
        sMessage += UNISTRING(".");
        throw InvalidRegistryException(sMessage, THISREF());
    }

    return aElementReturn;
}

//--------------------------------------------------------------------------
void OConfigurationRegistryKey::writeValueNode(const Any& _rValue) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE_WRITE);

    Reference< XNameReplace > xParentValueAccess(m_xLeafParent, UNO_QUERY);
    if (!xParentValueAccess.is())
        throw InvalidRegistryException(UNISTRING("The parent configuration node does not allow write access to it's children."), THISREF());

    try
    {
        xParentValueAccess->replaceByName(m_sLocalName, _rValue);
    }
    catch(IllegalArgumentException&)
    {
        throw InvalidRegistryException(UNISTRING("Unable to replace the old value. The configuration node threw an IllegalArgumentException."), THISREF());
    }
    catch(NoSuchElementException&)
    {
        OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::writeValueNode : a NoSuchElementException should be impossible !");
        throw InvalidRegistryException(UNISTRING("Unable to replace the old value. The configuration node threw an NoSuchElementException."), THISREF());
    }
    catch(WrappedTargetException&)
    {
        throw InvalidRegistryException(UNISTRING("Unable to replace the old value. The configuration node threw an WrappedTargetException."), THISREF());
    }
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
    return m_xNodeAccess.is() || m_aLeafElement.hasValue();
}

//--------------------------------------------------------------------------
RegistryKeyType SAL_CALL OConfigurationRegistryKey::getKeyType( const ::rtl::OUString& _rKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);

    // no further checks are made (for performance reasons) ...
    return RegistryKeyType_KEY;
}

//--------------------------------------------------------------------------
namespace {
    inline
    Type getBinaryDataType() {
        Sequence<sal_Int8> const * const p= 0;
        return ::getCppuType(p);
    }
}
//--------------------------------------------------------------------------
typedef typelib_TypeDescriptionReference ElementTypeDesc;

RegistryValueType SAL_CALL OConfigurationRegistryKey::getValueType(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE);

    switch (m_aLeafElement.getValueType().getTypeClass())
    {
        case TypeClass_STRING:
            return RegistryValueType_STRING;
        case TypeClass_SHORT:
        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_BYTE:
        case TypeClass_LONG:
        case TypeClass_UNSIGNED_LONG:
        case TypeClass_BOOLEAN:
            return RegistryValueType_LONG;
        case TypeClass_SEQUENCE:
            if (m_aLeafElement.getValueType() == getBinaryDataType())
                return RegistryValueType_BINARY;
            else
            {
                TypeDescription aType( m_aLeafElement.getValueType() );
                typelib_IndirectTypeDescription* pSequenceType
                    = reinterpret_cast< typelib_IndirectTypeDescription* >(aType.get());
                ElementTypeDesc* pElementType = pSequenceType->pType;

                switch (pElementType->eTypeClass)
                {
                    case TypeClass_SHORT:
                    case TypeClass_UNSIGNED_SHORT:
                    case TypeClass_BYTE:
                    case TypeClass_LONG:
                    case TypeClass_UNSIGNED_LONG:
                    case TypeClass_BOOLEAN:
                        return RegistryValueType_LONGLIST;
                    case TypeClass_STRING:
                        return RegistryValueType_STRINGLIST;
                    case TypeClass_DOUBLE:
                        OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support lists of floating point numebrs !");
                    default:
                        if (Type(pElementType) == getBinaryDataType())
                            OSL_ENSHURE(sal_False,"OConfigurationRegistryKey::getValueType : Registry cannot support LIST of BINARY");
                        else
                            OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::getValueType : unknown sequence element type !");
                        return RegistryValueType_NOT_DEFINED;
                }
            }
        case TypeClass_DOUBLE:
            OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::getValueType : registry does not support floating point numebrs !");
            return RegistryValueType_NOT_DEFINED;
        default:
            OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::getValueType : unknown entry type !");
            return RegistryValueType_NOT_DEFINED;
    }
}

//--------------------------------------------------------------------------
sal_Int32 SAL_CALL OConfigurationRegistryKey::getLongValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE);

#define EXTRACT(tcname, type)   \
    case TypeClass_##tcname : { type nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; }

    sal_Int32 nLongValue(0);
    switch (m_aLeafElement.getValueTypeClass())
    {
        case TypeClass_SHORT            : { sal_Int16 nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; } break;
        case TypeClass_UNSIGNED_SHORT   : { sal_uInt16 nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; } break;
        case TypeClass_BYTE             : { sal_Int8 nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; } break;
        case TypeClass_LONG             : { sal_Int32 nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; } break;
        case TypeClass_UNSIGNED_LONG    : { sal_uInt32 nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; } break;
        case TypeClass_BOOLEAN          : { sal_Bool nNativeValue; m_aLeafElement >>= nNativeValue; nLongValue = nNativeValue; } break;
        default:
            throw InvalidValueException(UNISTRING("This node does not contain a long (or a compatible) value."), THISREF());
    }
    return nLongValue;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setLongValue( sal_Int32 _nValue ) throw(InvalidRegistryException, RuntimeException)
{
    writeValueNode(makeAny(_nValue));
}

//--------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OConfigurationRegistryKey::getLongListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE);

    Sequence< sal_Int32 > aReturn;
    if (m_aLeafElement >>= aReturn)
        return aReturn;

    // TODO : maybe it's a sequence of sal_Int8 or anything like that which we're able to convert ....

    throw InvalidValueException(UNISTRING("This configuration node does not contain a list of longs !"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setLongListValue( const Sequence< sal_Int32 >& _seqValue ) throw(InvalidRegistryException, RuntimeException)
{
    writeValueNode(makeAny(_seqValue));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getAsciiValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    ::rtl::OUString sReturn = getStringValue();
    // TODO : check if it's really ascii ...
    return sReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setAsciiValue( const ::rtl::OUString& _rValue ) throw(InvalidRegistryException, RuntimeException)
{
    setStringValue(_rValue);
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getAsciiListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    return getStringListValue();
        // TODO : it's not really an "ascii list" .... perhaps we should throw an exception here ...
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setAsciiListValue( const Sequence< ::rtl::OUString >& _seqValue ) throw(InvalidRegistryException, RuntimeException)
{
    setStringListValue(_seqValue);
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConfigurationRegistryKey::getStringValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE);

    ::rtl::OUString sReturn;
    if (!(m_aLeafElement >>= sReturn))
        throw InvalidValueException(UNISTRING("This node does not contain a string value."), THISREF());
    return sReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setStringValue( const ::rtl::OUString& _rValue ) throw(InvalidRegistryException, RuntimeException)
{
    writeValueNode(makeAny(_rValue));
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getStringListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE);

    Sequence< ::rtl::OUString > aReturn;
    if (m_aLeafElement >>= aReturn)
        return aReturn;

    throw InvalidValueException(UNISTRING("This configuration node does not contain a list of strings !"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setStringListValue( const Sequence< ::rtl::OUString >& _seqValue ) throw(InvalidRegistryException, RuntimeException)
{
    writeValueNode(makeAny(_seqValue));
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OConfigurationRegistryKey::getBinaryValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_VALUE);

    Sequence< sal_Int8 > aReturn;
    if (m_aLeafElement >>= aReturn)
        return aReturn;

    throw InvalidValueException(UNISTRING("This configuration node does not contain a list of strings !"), THISREF());
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::setBinaryValue( const Sequence< sal_Int8 >& _rValue ) throw(InvalidRegistryException, RuntimeException)
{
    writeValueNode(makeAny(_rValue));
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > OConfigurationRegistryKey::implGetKey( const ::rtl::OUString& _rKeyName )
    throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sDescRelativeName(_rKeyName);
        // will be the name of the new key relative to it's parent
    sal_Bool bDeepAccess = sal_False;
    sal_Int32 nSeparatorPos = _rKeyName.lastIndexOf('/');
    if ((nSeparatorPos >= 0) && (nSeparatorPos == (_rKeyName.getLength() - 1)))
        // recognize a trailing slashs
        nSeparatorPos = _rKeyName.lastIndexOf('/', nSeparatorPos - 1);
    if (nSeparatorPos >= 1)
    {
        sDescRelativeName = _rKeyName.copy(nSeparatorPos + 1);
        bDeepAccess = sal_True;
    }

    Any aDescendant = getDescendant(_rKeyName);
    if (aDescendant.getValueType().getTypeClass() == TypeClass_INTERFACE)
    {
        Reference< XNameAccess > xContainerNode;
        ::cppu::extractInterface(xContainerNode, aDescendant);
        if (!xContainerNode.is())
            throw InvalidRegistryException(UNISTRING("invalid descendant node."), THISREF());
        return new OConfigurationRegistryKey(xContainerNode, sDescRelativeName, !m_bReadOnly);
    }
    else
    {
        Reference< XNameAccess > xDescParent(m_xNodeAccess);    // the parent config node of the descandent

        OSL_ENSHURE(aDescendant.hasValue(), "OConfigurationRegistryKey::openKey : invalid return from getDescendant.");
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
            default:
                OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::openKey : unknown or invalid descendant value type !");
        }
#endif
        if (bDeepAccess)
        {
            Any aDescParent = getDescendant(_rKeyName.copy(0, nSeparatorPos));
            ::cppu::extractInterface(xDescParent, aDescParent);
            if (!xDescParent.is())
                throw InvalidRegistryException(UNISTRING("The internal registry structure seems to be corrupt."), THISREF());
        }

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
void OConfigurationRegistryKey::checkRelativeKeyName(::rtl::OUString& _rKeyName) throw(InvalidRegistryException, RuntimeException)
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

    ::rtl::OUString sKeyName(_rKeyName);
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
            OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::createKey : somebody changed the implGetKey behaviour !");
            throw InvalidRegistryException(UNISTRING("An internal error occured."), THISREF());
        }
        return xSetNode->createKey(sKeyName);
    }

    // The requested new key is one level below ourself. Can't delegate the creation.
    Reference< XNameContainer > xContainer(m_xNodeAccess, UNO_QUERY);
    Reference< XSingleServiceFactory > xChildFactory(xContainer, UNO_QUERY);
    if (!xChildFactory.is())
        throw InvalidRegistryException(UNISTRING("The configuration node represented by this key is not a container node, you can't insert keys."), THISREF());

    // In the configuration API, the creation of a new child is two-stage process : first you create a child which
    // is "floating", i.e. does not belong to the configuration tree, yet. After filling it with values, you insert
    // it into the container node which was used for the creation.
    // We can't map this behaviour with the registry API, so we have to combine both steps

    // create a new floating child for the container node
    Reference< XInterface > xFloatingChild;
    try
    {
        xFloatingChild = xChildFactory->createInstance();
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

    // and immediately insert it into the container
    try
    {
        xContainer->insertByName(sKeyName, makeAny(xFloatingChild));
    }
    catch (IllegalArgumentException& e)
    {
        throw InvalidRegistryException(UNISTRING("illegal argument : ") += e.Message, THISREF());
    }
    catch (ElementExistException& e)
    {
        if (e.Message.getLength())
            throw InvalidRegistryException(e.Message, THISREF());
        else
            throw InvalidRegistryException((UNISTRING("There already is an element named ") += sKeyName) += UNISTRING("."), THISREF());
    }
    catch (WrappedTargetException& e)
    {
        throw InvalidRegistryException(UNISTRING("Caught an WrappedTargetException. Original error message : ") += e.Message, THISREF());
    }

    Reference< XNameAccess > xInsertedChild(xFloatingChild, UNO_QUERY);
    if (!xInsertedChild.is())
        throw InvalidRegistryException(UNISTRING("An internal error occured. The objects provided by the configuration API are invalid."), THISREF());

    return new OConfigurationRegistryKey(xInsertedChild, sKeyName, !m_bReadOnly);
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::closeKey(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    m_xNodeAccess = NULL;
    m_xNodeDeepAccess = NULL;
    m_aLeafElement.clear();
    m_xLeafParent = NULL;
    m_sLocalName = ::rtl::OUString();
}

//--------------------------------------------------------------------------
void SAL_CALL OConfigurationRegistryKey::deleteKey( const ::rtl::OUString& _rKeyName ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    checkValid(KAT_CHILD);
    if (m_bReadOnly)
        throw InvalidRegistryException(UNISTRING("The key is read only."), THISREF());

    ::rtl::OUString sKeyName(_rKeyName);
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
            OSL_ENSHURE(sal_False, "OConfigurationRegistryKey::createKey : somebody changed the implGetKey behaviour !");
            throw InvalidRegistryException(UNISTRING("An internal error occured."), THISREF());
        }
        xSetNode->deleteKey(sKeyName);
        return;
    }

    // The requested new key is one level below ourself. Can't delegate the creation.
    Reference< XNameContainer > xContainer(m_xNodeAccess, UNO_QUERY);
    if (!xContainer.is())
        throw InvalidRegistryException(UNISTRING("The configuration node represented by this key is not a container node, you can't remove keys."), THISREF());

    // and immediately insert it into the container
    try
    {
        xContainer->removeByName(sKeyName);
    }
    catch (NoSuchElementException& e)
    {
        if (e.Message.getLength())
            throw InvalidRegistryException(e.Message, THISREF());
        else
            throw InvalidRegistryException((UNISTRING("There is no element named ") += sKeyName) += UNISTRING("."), THISREF());
    }
    catch (WrappedTargetException& e)
    {
        throw InvalidRegistryException(UNISTRING("Caught an WrappedTargetException. Original error message : ") += e.Message, THISREF());
    }
}

//--------------------------------------------------------------------------
Sequence< Reference< XRegistryKey > > SAL_CALL OConfigurationRegistryKey::openKeys(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);

    Sequence< ::rtl::OUString > aNames(m_xNodeAccess->getElementNames());
    const ::rtl::OUString* pNames = aNames.getConstArray();
    Sequence< Reference< XRegistryKey > > aReturn(aNames.getLength());
    Reference< XRegistryKey >* pReturn = aReturn.getArray();
    for (sal_Int32 i=0; i<aNames.getLength(); ++i, ++pNames, ++pReturn)
        *pReturn = implGetKey(*pNames);

    return aReturn;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConfigurationRegistryKey::getKeyNames(  ) throw(InvalidRegistryException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    checkValid(KAT_CHILD);
    return m_xNodeAccess->getElementNames();
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


