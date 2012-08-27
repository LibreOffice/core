/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CLI_PROXY_H
#define INCLUDED_CLI_PROXY_H

#pragma warning(push, 1)
#include "uno/environment.hxx"
#pragma warning(pop)
#include "uno/mapping.hxx"
#include "uno/dispatcher.h"
#include "cli_bridge.h"
#include "cli_environment.h"

#using <cli_ure.dll>

namespace srrp = System::Runtime::Remoting::Proxies;
namespace srrm = System::Runtime::Remoting::Messaging;
namespace srr = System::Runtime::Remoting;
namespace sr = System::Reflection;
namespace sc = System::Collections;
using namespace uno;

namespace cli_uno
{

public ref class UnoInterfaceInfo
{
public:
    UnoInterfaceInfo(Bridge const * bridge, uno_Interface* unoI,
                     typelib_InterfaceTypeDescription* td);
    ~UnoInterfaceInfo();
    uno_Interface * m_unoI; // wrapped interface
    System::Type ^ m_type;
    typelib_InterfaceTypeDescription* m_typeDesc;

    Bridge const* m_bridge;
};

public ref class  UnoInterfaceProxy: public srrp::RealProxy,
                                      public srr::IRemotingTypeInfo
{
    /** used for IRemotingTypeInfo.TypeName
     */
    System::String^ m_sTypeName;
    /** The list is filled with UnoInterfaceInfo objects. The list can only
        grow and elements are never changed. If an element was added it
        must not be changed!
     */
    sc::ArrayList^ m_listIfaces;
    /** The number of UNO interfaces this proxy represents. It corresponds
        to the the number of elements in m_listIfaces.
    */
    int m_numUnoIfaces;
    /** The list is filled with additional UnoInterfaceProxy object due
        to aggregation via bridges.  Though the latter is strongly
        discouraged, this has to be supported.
    */
    sc::ArrayList^ m_listAdditionalProxies;
    int m_nlistAdditionalProxies;

    UnoInterfaceInfo ^ findInfo( ::System::Type ^ type );

    Bridge const* m_bridge;
     System::String^ m_oid;

#if OSL_DEBUG_LEVEL >= 2
    /** The string contains all names of UNO interfaces which are
        represented by this proxy. It is used to print out the interfaces
        when this proxy dies. In the destructor it is not allowed to
        access m_listIfaces or any other managed object.
    */
     rtl_uString * _sInterfaces;
//     /** Count of interfaces. Used in conjunction with _sInterfaces.
//      */
    int _numInterfaces;
#endif

public:

    /** Creates a proxy and registers it on the dot NET side.
     */
    static System::Object^ create(Bridge * bridge,
                                  uno_Interface * pUnoI,
                                  typelib_InterfaceTypeDescription* pTd,
                                  const rtl::OUString& oid);

    /** RealProxy::Invoke */
    virtual srrm::IMessage^ Invoke(srrm::IMessage^ msg) override;

    /** Must be called from within a synchronized section.
        Add only the interface if it is not already contained.
        This method is called from the constructor and as a result
        of IRemotingTypeInfo::CanCastTo
     */
    void addUnoInterface(uno_Interface* pUnoI,
                         typelib_InterfaceTypeDescription* pTd);
    ~UnoInterfaceProxy();

    /**
     */
    inline System::String ^ getOid()
        { return m_oid; }

    //IRemotingTypeInfo ----------------------------------------------
    virtual bool CanCastTo(System::Type^ fromType, System::Object^ o);

    virtual property System::String^ TypeName
    {
        System::String^ get()
        {
            return m_sTypeName;
        };
        void set(System::String^ name)
        {
            m_sTypeName = name;
        };
    }


private:
    UnoInterfaceProxy(
        Bridge * bridge,
        uno_Interface * pUnoI,
        typelib_InterfaceTypeDescription* pTD,
        const rtl::OUString& oid );

    static srrm::IMessage^ constructReturnMessage(System::Object^ retVal,
                           array<System::Object^>^ outArgs,
                           typelib_InterfaceMethodTypeDescription* mtd,
                           srrm::IMessage^ msg, System::Object^ exc);

    static System::String^ m_methodNameString =
                           gcnew System::String("__MethodName");
    static System::String^ m_typeNameString = gcnew System::String("__TypeName");
    static System::String^ m_ArgsString = gcnew System::String("__Args");
    static System::String^ m_CallContextString =
                           gcnew System::String("__CallContext");
    static System::String^ m_system_Object_String =
                           gcnew System::String("System.Object");
    static System::String^ m_methodSignatureString =
                           gcnew System::String("__MethodSignature");
    static System::String^ m_Equals_String =  gcnew System::String("Equals");
    static System::String^ m_GetHashCode_String =
                           gcnew System::String("GetHashCode");
    static System::String^ m_GetType_String = gcnew System::String("GetType");
    static System::String^ m_ToString_String = gcnew System::String("ToString");

protected:
     srrm::IMessage^ invokeObject(sc::IDictionary^ properties,
                                  srrm::LogicalCallContext^ context,
                                  srrm::IMethodCallMessage^ mcm);
};


//Cannot make this __gc because a managed type cannot derive from unmanaged type
struct CliProxy: public uno_Interface
{
    mutable oslInterlockedCount m_ref;
    const Bridge* m_bridge;
    const gcroot<System::Object^> m_cliI;
    gcroot<System::Type^> m_type;
    const com::sun::star::uno::TypeDescription m_unoType;
    const gcroot<System::String^> m_oid;
    const rtl::OUString m_usOid;

    enum MethodKind {MK_METHOD = 0, MK_SET, MK_GET};
    /** The array contains MethodInfos  of the cli object. Each one reflects an
        implemented interface method of the interface for which this proxy was
        created. The MethodInfos are from the object's method and not from the
        interface type. That is, they can be used to invoke the methods. The
        order of the MethodInfo objects corresponds to the order of the
        interface methods (see member m_type). Position 0 contains the
        MethodInfo of the first method of the interface which represents the
        root of the inheritance chain. The last MethodInfo represents the last
        method of the furthest derived interface.

        The array is completely initialized in the constructor of this object.

        When the uno_DispatchMethod is called for this proxy then it receives
        a typelib_TypeDescription of the member which is either an attribute
        (setter or getter) or method. After determining the position of the
        method within the UNO interface one can use the position to obtain the
        MethodInfo of the corresponding cli method. To obtain the index for the
        m_arMethodInfos array the function position has to be decreased by 3.
        This is becaus, the cli interface does not contain the XInterface
        methods.
    */
    gcroot<array<sr::MethodInfo^>^> m_arMethodInfos;

    /** This array is similar to m_arMethodInfos but it contains the MethodInfo
        objects of the interface (not the object). When a call is made from uno
        to cli then the uno method name is compared to the cli method name. The
        cli method name can be obtained from the MethodInfo object in this
        array. The name of the actual implemented method may not be the same as
        the interface method.
    */
    gcroot<array<sr::MethodInfo^>^> m_arInterfaceMethodInfos;

    /** Maps the position of the method in the UNO interface to the position of
        the corresponding MethodInfo in m_arMethodInfos. The Uno position must
        not include the XInterface methods. For example,
        pos 0 = XInterface::queryInterface
        pos 1 = XInterface::acquire
        pos 2 = XInterface::release

        That is the real Uno position has to be deducted by 3. Then
        arUnoPosToCliPos[pos] contains the index for m_arMethodInfos.

     */
    gcroot<array<System::Int32>^> m_arUnoPosToCliPos;

    /** Count of inherited interfaces of the cli interface.
     */
    int m_nInheritedInterfaces;
    /** Contains the number of methods of each interface.
     */
    gcroot<array<System::Int32^>^> m_arInterfaceMethodCount;

    CliProxy( Bridge const* bridge, System::Object^ cliI,
                 typelib_TypeDescription const* pTD,
                 const rtl::OUString& usOid);
    ~CliProxy();

    static uno_Interface* create(Bridge const * bridge,
                                 System::Object^ cliI,
                                 typelib_TypeDescription const * TD,
                                 rtl::OUString const & usOid );

    /** Prepares an array (m_arMethoInfos) containing MethodInfo object of the
        interface and all inherited interfaces. At index null is the first
        method of the base interface and at the last position is the last method
        of the furthest derived interface.
        If a UNO call is received then one can determine the position of the
        method (or getter or setter for an attribute) from the passed type
        information. The position minus 3 (there is no XInterface in the cli
        mapping) corresponds to the index of the cli interface method in the
        array.
    */
    void makeMethodInfos();

    /**Obtains a MethodInfo which can be used to invoke the cli object.
       Internally it maps nUnoFunctionPos to an index that is used to get the
       corresponding MethodInfo object from m_arMethoInfos. The mapping table
       is dynamically initialized. If the cli interface has no base interface
       or exactly one then the mapping table is initialized in one go at the
       first call. In all ensuing calls the MethodInfo object is immediately
       retrieved through the mapping table.

       If the interface has more then one interface in its inheritance chain,
       that is Type.GetInterfaces return more then one Type, then the mapping
       table is partially initiallized. On the first call the mappings for the
       methods of the belonging interface are created.

       The implementation assumes that the order of interface methods as
       provided by InterfaceMapping.InterfaceMethods corresponds to the order
       of methods in the interface declaration.

       @param nUnoFunctionPos
       Position of the method in the uno interface.
     */
    sr::MethodInfo^ getMethodInfo(int nUnoFunctionPos,
                                  const rtl::OUString & usMethodName,
                                  MethodKind mk);

    void SAL_CALL uno_DispatchMethod(
        struct _uno_Interface * pUnoI,
        const struct _typelib_TypeDescription * pMemberType,
        void * pReturn,
        void * pArgs[],
        uno_Any ** ppException );

    inline void acquire() const;
    inline void release() const;
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
