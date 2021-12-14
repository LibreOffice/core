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

#pragma once

#include <sal/config.h>

#include <osl/mutex.hxx>
#include <com/sun/star/lang/EventObject.hpp>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/interfacecontainer2.hxx>
#include <memory>
#include <vector>
#include <utility>

namespace com::sun::star::uno
{
class XInterface;
}

/** */ //for docpp
namespace comphelper
{
/** This is a copy of cppu::OMultiTypeInterfaceContainerHelper2 in include/cppuhelper/interfacecontainer.h,
    except that it uses comphelper::OInterfaceContainerHelper2, which is more efficient.
*/
class COMPHELPER_DLLPUBLIC OMultiTypeInterfaceContainerHelper2
{
public:
    /**
      Create a container of interface containers.

      @param rMutex the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    OMultiTypeInterfaceContainerHelper2(::osl::Mutex& rMutex);
    /**
      Delete all containers.
     */
    ~OMultiTypeInterfaceContainerHelper2();

    /**
      Return all id's under which at least one interface is added.
     */
    std::vector<css::uno::Type> getContainedTypes() const;

    /**
      Return the container created under this key.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    OInterfaceContainerHelper2* getContainer(const css::uno::Type& rKey) const;

    /** Inserts an element into the container with the specified key.
        The position is not specified, thus it is not specified in which order events are fired.

        @attention
        If you add the same interface more than once, then it will be added to the elements list
        more than once and thus if you want to remove that interface from the list, you have to call
        removeInterface() the same number of times.
        In the latter case, you will also get events fired more than once (if the interface is a
        listener interface).

        @param rKey
               the id of the container
        @param r
               interface to be added; it is allowed, to insert null or
               the same interface more than once
        @return
                the new count of elements in the container
    */
    sal_Int32 addInterface(const css::uno::Type& rKey,
                           const css::uno::Reference<css::uno::XInterface>& r);

    /** Removes an element from the container with the specified key.
        It uses interface equality to remove the interface.

        @param rKey
               the id of the container
        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    sal_Int32 removeInterface(const css::uno::Type& rKey,
                              const css::uno::Reference<css::uno::XInterface>& rxIFace);

    /**
      Call disposing on all object in the container that
      support XEventListener. Then clear the container.
     */
    void disposeAndClear(const css::lang::EventObject& rEvt);
    /**
      Remove all elements of all containers. Does not delete the container.
     */
    void clear();

    typedef css::uno::Type keyType;

private:
    typedef std::vector<std::pair<css::uno::Type, std::unique_ptr<OInterfaceContainerHelper2>>>
        t_type2ptr;

    t_type2ptr::iterator findType(const css::uno::Type& rKey);
    t_type2ptr::const_iterator findType(const css::uno::Type& rKey) const;

    t_type2ptr m_aMap;
    ::osl::Mutex& rMutex;

    OMultiTypeInterfaceContainerHelper2(const OMultiTypeInterfaceContainerHelper2&) = delete;
    OMultiTypeInterfaceContainerHelper2& operator=(const OMultiTypeInterfaceContainerHelper2&)
        = delete;
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
