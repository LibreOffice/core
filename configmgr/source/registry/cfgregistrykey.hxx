/*************************************************************************
 *
 *  $RCSfile: cfgregistrykey.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-27 10:27:47 $
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
#define _CONFIGMGR_REGISTRY_CFGREGISTRYKEY_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/Mutex.hxx>
#endif

//..........................................................................
namespace configmgr
{
//..........................................................................

//==========================================================================
//= OConfigurationRegistryKey
//==========================================================================
typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::registry::XRegistryKey
                                >   OConfigurationRegistryKey_Base;

/** wraps the registry-like access to a single node of a configuration sub tree
*/
class OConfigurationRegistryKey
        :public OConfigurationRegistryKey_Base
{
    ::osl::Mutex    m_aMutex;           /// access safety
    sal_Bool        m_bReadOnly;        /// is the key readonly ?

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xNode;        /// the config node object, if it is a container

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xParentNode;      /// if the key is not the root, this is it's parent.

    const ::rtl::OUString   m_sLocalName; /** the name of the element relative to the parent, which is
                                            m_xParentNode if that is present
                                        */

    // TODO : the current concept does not recognize when config keys are disposed (e.g. when the registry is closed)
    //
    // Possible solutions:
    // 1. each registry key is a listener on the component containing its node
    //      may be is expensive ?.
    //
    // At the moment we ignore this restriction, but perhaps we can't do that forever ....

public:
    /// when used as ctor parameter, this indicates that the key wraps a config tree subtree root
    struct SubtreeRoot { };

    /** builds an registry key which wraps the root of a configuration registry
    */
    OConfigurationRegistryKey(
             const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxRootNode
            ,sal_Bool _bWriteable
            ,SubtreeRoot
        );

    /** builds an registry key for a configuration node
        @param      _rxContainerNode    the node the key should represent
        @param      _bWriteable         should the key be writeable ?
    */
    OConfigurationRegistryKey(
             const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxNode
            ,sal_Bool _bWriteable
        );

    /** builds an registry key for a configuration child node.
        @param      _rxParentNode   the parent of the node. Used for update access and for obtaining the initial value.
        @param      _rRelativeName  te relative name within the parent
        @param      _bWriteable     should the key be writeable ?
    */
    OConfigurationRegistryKey(
             const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxParentNode
            ,const ::rtl::OUString& _rRelativeName
            ,sal_Bool _bWriteable
        );

    /** builds an registry key for a configuration value container node.
        @param      _rCurrentValue  the current value of the node. Must be the same as _rxParentNode->getByName(_rRelativeName) would provide
        @param      _rxParentNode   the parent of the value node. Used for update access and for obtaining the initial value.
        @param      _rRelativeName  te relative name within the parent
        @param      _bWriteable     should the key be writeable ?
    */
    OConfigurationRegistryKey(
             ::com::sun::star::uno::Any _rCurrentValue
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxParentNode
            ,const ::rtl::OUString& _rRelativeName
            ,sal_Bool _bWriteable
        );

    // XRegistryKey
    virtual ::rtl::OUString SAL_CALL getKeyName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::registry::RegistryKeyType SAL_CALL getKeyType( const ::rtl::OUString& rKeyName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::registry::RegistryValueType SAL_CALL getValueType(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getLongValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLongValue( sal_Int32 value ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getLongListValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLongListValue( const ::com::sun::star::uno::Sequence< sal_Int32 >& seqValue ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAsciiValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAsciiValue( const ::rtl::OUString& value ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAsciiListValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAsciiListValue( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& seqValue ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getStringValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStringValue( const ::rtl::OUString& value ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getStringListValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStringListValue( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& seqValue ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinaryValue(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::InvalidValueException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setBinaryValue( const ::com::sun::star::uno::Sequence< sal_Int8 >& value ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > SAL_CALL openKey( const ::rtl::OUString& aKeyName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > SAL_CALL createKey( const ::rtl::OUString& aKeyName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeKey(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteKey( const ::rtl::OUString& rKeyName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > > SAL_CALL openKeys(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getKeyNames(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL createLink( const ::rtl::OUString& aLinkName, const ::rtl::OUString& aLinkTarget ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deleteLink( const ::rtl::OUString& rLinkName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLinkTarget( const ::rtl::OUString& rLinkName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getResolvedName( const ::rtl::OUString& aKeyName ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

protected:
    /** specifies the kind of access to the key.
    */
    enum KEY_ACCESS_TYPE
    {
        KAT_META,           /// access on a meta level, e.g. asking for the read-onyl flag
        KAT_VALUE,          /// read access to the value the node represents
        KAT_VALUE_WRITE,    /// write access to the value the node represents
        KAT_CHILD           /// access to one of the (grand-)children of the node
    };

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
        implGetParentPropertyInfo()
            throw(::com::sun::star::uno::RuntimeException);

    sal_Bool implIsReadOnly()
            throw(::com::sun::star::uno::RuntimeException);

    sal_Bool implEnsureNode()
            throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Type implGetUnoType()
            throw(::com::sun::star::uno::RuntimeException);

    sal_Bool implEnsureValue()
            throw(::com::sun::star::uno::RuntimeException);

    sal_Bool implIsValid() throw ();

    /** check if the registry key is valid
        @param      _eIntentedAccess    type of access which the caller wants to perform on the object
        @throws             <type scope="com.sun.star.registry">InvalidRegistryException</type> if the key is invalid
    */
    void checkValid(KEY_ACCESS_TYPE _eIntentedAccess)
        throw (::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    /** return an child element.
        @param      _rDescendantName    the name of the descendant to open. May have a depth of more than 1, if
                                        the node container support XHierarchicalNameAccess
        @return             the requested element. The caller can assume that the returned
                            <type scope="com.sun.star.uno">Any</type> always contains an object, all other cases are
                            handled with exceptions
        @throws             <type scope="com.sun.star.registry">InvalidRegistryException</type> if the key is invalid,
                            the element refered by _rName does not exist, the configuration node threw an exception, or
                            the name has a depth of more than one and the config node does not support this.
    */
    ::com::sun::star::uno::Any
        implGetDescendant(const ::rtl::OUString& _rDescendantName)
            throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    /** write the given value into the configuration node the object represents.
        @throws             <type scope="com.sun.star.registry">InvalidRegistryException</type> if the key is invalid,
                            not opened for write access or the configurations parent is not able to provide a value access
        @throws             <type scope="com.sun.star.uno">RuntimeException</type> if a fatal runtime error occurs
    */
    void implSetValue(const ::com::sun::star::uno::Any& _rValue)
            throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Any implGetValue()
            throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    /** open the sub key (depth 1 or more) determined by the given name
        @param      _rKeyName       the name of the descendant node
        @return             a XRegistryKey wrapper for the requested configuration node
        @throws             <type scope="com.sun.star.registry">InvalidRegistryException</type> if the key is invalid,
                            the element refered by _rName does not exist, the configuration node threw an exception, or
                            the name has a depth of more than one and the config node does not support this.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >
        implGetKey( const ::rtl::OUString& _rKeyName )
            throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    /** check the given (relative) key name syntactically.

        <p>In particular, this means that no checks are made if a node with the given name exists or something like
        that ...<br/>
        In addition, the given name will be normalized. Basically, this means that it does not contain trailing slashes.
        </p>
        @throws InvalidRegistryException
            if the name is not relative (i.e. if it starts with an slash)
        @throws InvalidRegistryException
            if the name is empty or consists of slashes only
    */
    void checkRelativeKeyName(::rtl::OUString& _rKeyName)
            throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    /** get a default value for a value of a given type

        <p>Creates an Any with an empty or zero value of the given type.</p>

        @param  _rType          the type of the element to create
        @return
            an <type scope="com::sun::star::uno">Any</type> representing a default for the node value type. If the
            return value is still VOID, the node has a nullable type (e.g. any) or is of unsupported type

    */
    ::com::sun::star::uno::Any
        implCreateDefaultElement(::com::sun::star::uno::Type const& _rType)
            throw(::com::sun::star::uno::RuntimeException);
};


//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // _CONFIGMGR_REGISTRY_CFGREGISTRYKEY_HXX_


