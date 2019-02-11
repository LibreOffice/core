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

#ifndef INCLUDED_REGISTRY_SOURCE_REFLREAD_HXX
#define INCLUDED_REGISTRY_SOURCE_REFLREAD_HXX

#include <registry/refltype.hxx>
#include <rtl/ustring.hxx>

/// Implememetation handle
typedef void* TypeReaderImpl;

/** RegistryTypeReades reads a binary type blob.

    This class provides the necessary functions to read type information
    for all kinds of types of a type blob.

    @deprecated
    use typereg::Reader instead
*/
class RegistryTypeReader
{
public:

    /** Constructor.

        @param buffer points to the binary data block.
        @param bufferLen specifies the size of the binary data block.
     */
    RegistryTypeReader(const sal_uInt8* buffer,
                              sal_uInt32 bufferLen);

    /// Destructor. The Destructor frees the data block if the copyData flag was TRUE.
    ~RegistryTypeReader();

    /** returns the typeclass of the type represented by this blob.

        This function will always return the type class without the internal
        RT_TYPE_PUBLISHED flag set.
     */
    RTTypeClass      getTypeClass() const;

    /** returns the full qualified name of the type.
     */
    OUString  getTypeName() const;

    /** returns the full qualified name of the supertype.
     */
    OUString  getSuperTypeName() const;

    /** returns the number of fields (attributes/properties, enum values or number
        of constants in a module).

     */
    sal_uInt32       getFieldCount() const;

    /** returns the name of the field specified by index.
     */
    OUString  getFieldName( sal_uInt16 index ) const;

    /** returns the full qualified name of the field specified by index.
     */
    OUString  getFieldType( sal_uInt16 index ) const;

    /** returns the access mode of the field specified by index.
     */
    RTFieldAccess    getFieldAccess( sal_uInt16 index ) const;

    /** returns the value of the field specified by index.

        This function returns the value of an enum value or of a constant.
     */
    RTConstValue     getFieldConstValue( sal_uInt16 index ) const;

    /** returns the documentation string for the field specified by index.

        Each field of a type can have their own documentation.
     */
    OUString  getFieldDoku( sal_uInt16 index ) const;

    /** returns the IDL filename of the field specified by index.

        The IDL filename of a field can differ from the filename of the ype itself
        because modules and also constants can be defined in different IDL files.
     */
    OUString  getFieldFileName( sal_uInt16 index ) const;

private:
    RegistryTypeReader(RegistryTypeReader const &) = delete;
    void operator =(RegistryTypeReader const &) = delete;

    /// stores the handle of an implementation class
    TypeReaderImpl                                              m_hImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
