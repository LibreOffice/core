/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#if ! defined INCLUDED_CLI_ENVIRONMENT_H
#define INCLUDED_CLI_ENVIRONMENT_H

#include "cli_base.h"
#using <mscorlib.dll>

using namespace System;
using namespace System::Collections;
using namespace System::Runtime::Serialization;

namespace cli_uno
{

public __gc class Cli_environment
{
    static System::String* sOidPart;
    static  Hashtable* m_objects;
    static System::Runtime::Serialization::ObjectIDGenerator* m_IDGen;
    inline static System::String* createKey(System::String* oid, System::Type* t);

#if OSL_DEBUG_LEVEL >= 1
    int _numRegisteredObjects;
#endif

public:

    static Cli_environment()
    {
        m_objects = Hashtable::Synchronized(new Hashtable());
        m_IDGen = new System::Runtime::Serialization::ObjectIDGenerator();
        System::Text::StringBuilder* buffer = new System::Text::StringBuilder(256);
        Guid gd = Guid::NewGuid();
        buffer->Append(S";cli[0];");
        buffer->Append(gd.ToString());
        sOidPart = buffer->ToString();
    }

    inline Cli_environment();

    ~Cli_environment();

    /**
       Registers an UNO object as being mapped by this bridge. The resulting
       cli object is represents all interfaces of the UNO object. Therefore the
       object can be registered only with its OID; a type is not necessary.
    */
    Object* registerInterface(Object* obj, System::String* oid);
    /**
       Registers a CLI object as being mapped by this bridge. The resulting
       object represents exactly one UNO interface.
     */
    Object* registerInterface(Object* obj, System::String* oid, System::Type* type);

    /**
       By revoking an interface it is declared that the respective interface has
       not been mapped. The proxy implementations call revoke interface in their
       destructors.
     */
    inline void revokeInterface(System::String* oid);

    void revokeInterface(System::String* oid, System::Type* type);
    /**
     * Retrieves an interface identified by its object id and type from this
     * environment.
     *
     * @param oid object id of interface to be retrieved
     * @param type the type description of the interface to be retrieved
     * @see com.sun.star.uno.IEnvironment#getRegisteredInterface
     */
    Object* getRegisteredInterface(System::String* oid, System::Type* type);

    /**
     * Generates a worldwide unique object identifier (oid) for the given object. It is
     * guaranteed, that subsequent calls to the method with the same object
     * will give the same id.
     * <p>
     * @return     the generated oid.
     * @param      object     the object for which a Oid should be generated.
     */
    static System::String* getObjectIdentifier(Object* obj);

};

} //namespace cli_uno


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
