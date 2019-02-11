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

#ifndef INCLUDED_REGISTRY_SOURCE_REFLWRIT_HXX
#define INCLUDED_REGISTRY_SOURCE_REFLWRIT_HXX

#include <registry/types.hxx>
#include <rtl/ustring.hxx>

class RTConstValue;

/// Implememetation handle
typedef void* TypeWriterImpl;

/** RegistryTypeWriter writes/creates a binary type blob.

    This class provides the necessary functions to write type information
    for all kinds of types into a blob.

    @deprecated
    use typereg::Writer instead
*/
class RegistryTypeWriter
{
public:

    /** Constructor.

        @param RTTypeClass specifies the type of the new blob.
        @param typeName specifies the full qualified type name with '/' as separator.
        @param superTypeName specifies the full qualified type name of the base type
                             with '/' as separator.
        @param fieldCount specifies the number of fields (eg. number of attrbutes/properties,
                          enum values or constants).
     */
    RegistryTypeWriter(RTTypeClass               RTTypeClass,
                              const OUString&    typeName,
                              const OUString&    superTypeName,
                              sal_uInt16                fieldCount);

    /** Destructor. The Destructor frees the internal data block.

        The pointer (returned by getBlop) will be set to NULL.
     */
    ~RegistryTypeWriter();

    /** sets the data for a field member of a type blob.

        @param index indicates the index of the field.
        @param name specifies the name.
        @param typeName specifies the full qualified typename.
        @param doku specifies the documentation string of the field.
        @param fileName specifies the name of the IDL file where the field is defined.
        @param access specifies the access mode of the field.
        @param constValue specifies the value of the field. The value is only interesting
                          for enum values or constants.
     */
    void setFieldData( sal_uInt16              index,
                              const OUString&    name,
                              const OUString&    typeName,
                              const OUString&    doku,
                              const OUString&    fileName,
                              RTFieldAccess           access,
                              const RTConstValue&     constValue);

    /** returns a pointer to the new type blob.

        The pointer will be invalid (NULL) if the instance of
        the RegistryTypeWriter will be destroyed.
     */
    const sal_uInt8*     getBlop();

    /** returns the size of the new type blob in bytes.
     */
    sal_uInt32       getBlopSize();

private:
    RegistryTypeWriter(RegistryTypeWriter const &) = delete;
    void operator =(RegistryTypeWriter const &) = delete;

    /// stores the handle of an implementation class
    TypeWriterImpl                                               m_hImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
