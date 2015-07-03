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

#include <registry/refltype.hxx>
#include <registry/regtype.h>
#include <rtl/ustring.hxx>

/// Implememetation handle
typedef void* TypeWriterImpl;

/****************************************************************************

    C-Api

*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** specifies a collection of function pointers which represents the complete registry type writer C-API.

    This funtions pointers are used by the C++ wrapper to call the C-API.
*/
struct RegistryTypeWriter_Api
{
    TypeWriterImpl          (TYPEREG_CALLTYPE *createEntry)         (RTTypeClass, rtl_uString*, rtl_uString*, sal_uInt16, sal_uInt16, sal_uInt16);
    void                    (TYPEREG_CALLTYPE *acquire)             (TypeWriterImpl);
    void                    (TYPEREG_CALLTYPE *release)             (TypeWriterImpl);
    void                    (TYPEREG_CALLTYPE *setUik)              (TypeWriterImpl, const RTUik*);
    void                    (TYPEREG_CALLTYPE *setDoku)             (TypeWriterImpl, rtl_uString*);
    void                    (TYPEREG_CALLTYPE *setFileName)         (TypeWriterImpl, rtl_uString*);
    void                    (TYPEREG_CALLTYPE *setFieldData)        (TypeWriterImpl, sal_uInt16, rtl_uString*, rtl_uString*, rtl_uString*, rtl_uString*, RTFieldAccess, RTValueType, RTConstValueUnion);
    void                    (TYPEREG_CALLTYPE *setMethodData)       (TypeWriterImpl, sal_uInt16, rtl_uString*, rtl_uString*, RTMethodMode, sal_uInt16, sal_uInt16, rtl_uString*);
    void                    (TYPEREG_CALLTYPE *setParamData)        (TypeWriterImpl, sal_uInt16, sal_uInt16, rtl_uString*, rtl_uString*, RTParamMode);
    void                    (TYPEREG_CALLTYPE *setExcData)          (TypeWriterImpl, sal_uInt16, sal_uInt16, rtl_uString*);
    const sal_uInt8*        (TYPEREG_CALLTYPE *getBlop)             (TypeWriterImpl);
    sal_uInt32              (TYPEREG_CALLTYPE *getBlopSize)         (TypeWriterImpl);

    void                    (TYPEREG_CALLTYPE *setReferenceData)    (TypeWriterImpl, sal_uInt16, rtl_uString*, RTReferenceType, rtl_uString*, RTFieldAccess);
};

/** the API initialization function.
*/
RegistryTypeWriter_Api* TYPEREG_CALLTYPE initRegistryTypeWriter_Api();

#ifdef __cplusplus
}
#endif

/** RegistryTypeWriter writes/creates a binary type blob.

    This class provides the necessary functions to write type information
    for all kinds of types into a blob.
    The class is inline and use a C-Api.

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
        @param methodCount specifies the number of methods.
        @param referenceCount specifies the number of references (eg. number of supported interfaces,
                              exported services ...)
     */
    inline RegistryTypeWriter(RTTypeClass               RTTypeClass,
                              const rtl::OUString&    typeName,
                              const rtl::OUString&    superTypeName,
                              sal_uInt16                fieldCount,
                              sal_uInt16                methodCount,
                              sal_uInt16                referenceCount);

    /// Copy constructcor
    inline RegistryTypeWriter(const RegistryTypeWriter& toCopy);

    /** Destructor. The Destructor frees the internal data block.

        The pointer (returned by getBlop) will be set to NULL.
     */
    inline ~RegistryTypeWriter();

    /// Assign operator
    inline RegistryTypeWriter& operator == (const RegistryTypeWriter& toAssign);

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
    inline void setFieldData( sal_uInt16              index,
                              const rtl::OUString&    name,
                              const rtl::OUString&    typeName,
                              const rtl::OUString&    doku,
                              const rtl::OUString&    fileName,
                              RTFieldAccess           access,
                              const RTConstValue&     constValue = RTConstValue());

    /** returns a pointer to the new type blob.

        The pointer will be invalid (NULL) if the instance of
        the RegistryTypeWriter will be destroyed.
     */
    inline const sal_uInt8*     getBlop();

    /** returns the size of the new type blob in bytes.
     */
    inline sal_uInt32       getBlopSize();

protected:

    /// stores the registry type writer Api.
    const RegistryTypeWriter_Api*                                m_pApi;
    /// stores the handle of an implementation class
    TypeWriterImpl                                               m_hImpl;
};



inline RegistryTypeWriter::RegistryTypeWriter(RTTypeClass               RTTypeClass,
                                              const rtl::OUString&    typeName,
                                              const rtl::OUString&    superTypeName,
                                              sal_uInt16                fieldCount,
                                              sal_uInt16                methodCount,
                                              sal_uInt16                referenceCount)
    : m_pApi(initRegistryTypeWriter_Api())
    , m_hImpl(NULL)
{
    m_hImpl = m_pApi->createEntry(RTTypeClass,
                                  typeName.pData,
                                  superTypeName.pData,
                                  fieldCount,
                                  methodCount,
                                  referenceCount);
}


inline RegistryTypeWriter::RegistryTypeWriter(const RegistryTypeWriter& toCopy)
    : m_pApi(toCopy.m_pApi)
    , m_hImpl(toCopy.m_hImpl)
{
    m_pApi->acquire(m_hImpl);
}

inline RegistryTypeWriter::~RegistryTypeWriter()
{
    m_pApi->release(m_hImpl);
}

inline RegistryTypeWriter& RegistryTypeWriter::operator == (const RegistryTypeWriter& toAssign)
{
    if (m_hImpl != toAssign.m_hImpl)
    {
        m_pApi->release(m_hImpl);
        m_hImpl = toAssign.m_hImpl;
        m_pApi->acquire(m_hImpl);
    }

    return *this;
}

inline void RegistryTypeWriter::setFieldData( sal_uInt16              index,
                                              const rtl::OUString&    name,
                                              const rtl::OUString&    typeName,
                                              const rtl::OUString&    doku,
                                              const rtl::OUString&    fileName,
                                              RTFieldAccess           access,
                                              const RTConstValue&     constValue)
{
    m_pApi->setFieldData(m_hImpl, index, name.pData, typeName.pData, doku.pData, fileName.pData, access, constValue.m_type, constValue.m_value);
}

inline const sal_uInt8* RegistryTypeWriter::getBlop()
{
    return m_pApi->getBlop(m_hImpl);
}

inline sal_uInt32 RegistryTypeWriter::getBlopSize()
{
    return m_pApi->getBlopSize(m_hImpl);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
