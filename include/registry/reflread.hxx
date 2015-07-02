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

#ifndef INCLUDED_REGISTRY_REFLREAD_HXX
#define INCLUDED_REGISTRY_REFLREAD_HXX

#include <registry/regdllapi.h>
#include <registry/refltype.hxx>
#include <registry/regtype.h>
#include <rtl/ustring.hxx>

/// Implememetation handle
typedef void* TypeReaderImpl;

/****************************************************************************

    C-Api

*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** specifies a collection of function pointers which represents the complete registry type reader C-API.

    This funtions pointers are used by the C++ wrapper to call the C-API.
*/
struct RegistryTypeReader_Api
{
    TypeReaderImpl      (TYPEREG_CALLTYPE *createEntry)         (const sal_uInt8*, sal_uInt32, bool);
    void                (TYPEREG_CALLTYPE *acquire)             (TypeReaderImpl);
    void                (TYPEREG_CALLTYPE *release)             (TypeReaderImpl);
    sal_uInt16          (TYPEREG_CALLTYPE *getMinorVersion)     (TypeReaderImpl);
    sal_uInt16          (TYPEREG_CALLTYPE *getMajorVersion)     (TypeReaderImpl);
    RTTypeClass         (TYPEREG_CALLTYPE *getTypeClass)        (TypeReaderImpl);
    void                (TYPEREG_CALLTYPE *getUik)              (TypeReaderImpl, RTUik*);
    void                (TYPEREG_CALLTYPE *getDoku)             (TypeReaderImpl, rtl_uString**);
    void                (TYPEREG_CALLTYPE *getFileName)         (TypeReaderImpl, rtl_uString**);
    void                (TYPEREG_CALLTYPE *getTypeName)         (TypeReaderImpl, rtl_uString**);
    void                (TYPEREG_CALLTYPE *getSuperTypeName)    (TypeReaderImpl, rtl_uString**);
    sal_uInt32          (TYPEREG_CALLTYPE *getFieldCount)       (TypeReaderImpl);
    void                (TYPEREG_CALLTYPE *getFieldName)        (TypeReaderImpl, rtl_uString**, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getFieldType)        (TypeReaderImpl, rtl_uString**, sal_uInt16);
    RTFieldAccess       (TYPEREG_CALLTYPE *getFieldAccess)      (TypeReaderImpl, sal_uInt16);
    RTValueType         (TYPEREG_CALLTYPE *getFieldConstValue)  (TypeReaderImpl, sal_uInt16, RTConstValueUnion*);
    void                (TYPEREG_CALLTYPE *getFieldDoku)        (TypeReaderImpl, rtl_uString**, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getFieldFileName)    (TypeReaderImpl, rtl_uString**, sal_uInt16);
    sal_uInt32          (TYPEREG_CALLTYPE *getMethodCount)      (TypeReaderImpl);
    void                (TYPEREG_CALLTYPE *getMethodName)       (TypeReaderImpl, rtl_uString**, sal_uInt16);
    sal_uInt32          (TYPEREG_CALLTYPE *getMethodParamCount) (TypeReaderImpl, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getMethodParamType)  (TypeReaderImpl, rtl_uString**, sal_uInt16, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getMethodParamName)  (TypeReaderImpl, rtl_uString**, sal_uInt16, sal_uInt16);
    RTParamMode         (TYPEREG_CALLTYPE *getMethodParamMode)  (TypeReaderImpl, sal_uInt16, sal_uInt16);
    sal_uInt32          (TYPEREG_CALLTYPE *getMethodExcCount)   (TypeReaderImpl, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getMethodExcType)    (TypeReaderImpl, rtl_uString**, sal_uInt16, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getMethodReturnType) (TypeReaderImpl, rtl_uString**, sal_uInt16);
    RTMethodMode        (TYPEREG_CALLTYPE *getMethodMode)       (TypeReaderImpl, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getMethodDoku)       (TypeReaderImpl, rtl_uString**, sal_uInt16);

    sal_uInt32          (TYPEREG_CALLTYPE *getReferenceCount)   (TypeReaderImpl);
    void                (TYPEREG_CALLTYPE *getReferenceName)    (TypeReaderImpl, rtl_uString**, sal_uInt16);
    RTReferenceType     (TYPEREG_CALLTYPE *getReferenceType)    (TypeReaderImpl, sal_uInt16);
    void                (TYPEREG_CALLTYPE *getReferenceDoku)    (TypeReaderImpl, rtl_uString**, sal_uInt16);
    RTFieldAccess       (TYPEREG_CALLTYPE *getReferenceAccess)  (TypeReaderImpl, sal_uInt16);
};

/** the API initialization function.
*/
REG_DLLPUBLIC RegistryTypeReader_Api* TYPEREG_CALLTYPE initRegistryTypeReader_Api();

#ifdef __cplusplus
}
#endif

/** RegistryTypeReades reads a binary type blob.

    This class provides the necessary functions to read type information
    for all kinds of types of a type blob.
    The class is inline and use a C-Api.

    @deprecated
    use typereg::Reader instead
*/
class RegistryTypeReader
{
public:

    /** Constructor.

        @param buffer points to the binary data block.
        @param bufferLen specifies the size of the binary data block.
        @param copyData specifies if the data block should be copied.
                        The block can be copied to ensure that the data
                        is valid for the lifetime of this instance.
     */
    inline RegistryTypeReader(const sal_uInt8* buffer,
                              sal_uInt32 bufferLen,
                              bool copyData);

    /// Copy constructcor
    inline RegistryTypeReader(const RegistryTypeReader& toCopy);

    /// Destructor. The Destructor frees the data block if the copyData flag was TRUE.
    inline ~RegistryTypeReader();

    /// Assign operator
    inline RegistryTypeReader& operator == (const RegistryTypeReader& toAssign);


    /** returns the typeclass of the type represented by this blob.

        This function will always return the type class without the internal
        RT_TYPE_PUBLISHED flag set.
     */
    inline RTTypeClass      getTypeClass() const;

    /** returns the full qualified name of the type.
     */
    inline rtl::OUString  getTypeName() const;

    /** returns the full qualified name of the supertype.
     */
    inline rtl::OUString  getSuperTypeName() const;

    /** returns the number of fields (attributes/properties, enum values or number
        of constants in a module).

     */
    inline sal_uInt32       getFieldCount() const;

    /** returns the name of the field specified by index.
     */
    inline rtl::OUString  getFieldName( sal_uInt16 index ) const;

    /** returns the full qualified name of the field specified by index.
     */
    inline rtl::OUString  getFieldType( sal_uInt16 index ) const;

    /** returns the access mode of the field specified by index.
     */
    inline RTFieldAccess    getFieldAccess( sal_uInt16 index ) const;

    /** returns the value of the field specified by index.

        This function returns the value of an enum value or of a constant.
     */
    inline RTConstValue     getFieldConstValue( sal_uInt16 index ) const;

    /** returns the documentation string for the field specified by index.

        Each field of a type can have their own documentation.
     */
    inline rtl::OUString  getFieldDoku( sal_uInt16 index ) const;

    /** returns the IDL filename of the field specified by index.

        The IDL filename of a field can differ from the filename of the ype itself
        because modules and also constants can be defined in different IDL files.
     */
    inline rtl::OUString  getFieldFileName( sal_uInt16 index ) const;

protected:

    /// stores the registry type reader Api.
    const RegistryTypeReader_Api*                               m_pApi;
    /// stores the handle of an implementation class
    TypeReaderImpl                                              m_hImpl;
};



inline RegistryTypeReader::RegistryTypeReader(const sal_uInt8* buffer,
                                              sal_uInt32 bufferLen,
                                              bool copyData)
    : m_pApi(initRegistryTypeReader_Api())
    , m_hImpl(NULL)
    {
        m_hImpl = m_pApi->createEntry(buffer, bufferLen, copyData);
    }


inline RegistryTypeReader::RegistryTypeReader(const RegistryTypeReader& toCopy)
    : m_pApi(toCopy.m_pApi)
    , m_hImpl(toCopy.m_hImpl)
    { m_pApi->acquire(m_hImpl); }


inline RegistryTypeReader::~RegistryTypeReader()
    {  m_pApi->release(m_hImpl); }

inline RegistryTypeReader& RegistryTypeReader::operator == (const RegistryTypeReader& toAssign)
{
    if (m_hImpl != toAssign.m_hImpl)
    {
        m_pApi->release(m_hImpl);
        m_hImpl = toAssign.m_hImpl;
        m_pApi->acquire(m_hImpl);
    }

    return *this;
}







inline RTTypeClass RegistryTypeReader::getTypeClass() const
    {  return m_pApi->getTypeClass(m_hImpl); }

inline rtl::OUString RegistryTypeReader::getTypeName() const
    {
        rtl::OUString sRet;
        m_pApi->getTypeName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline rtl::OUString RegistryTypeReader::getSuperTypeName() const
    {
        rtl::OUString sRet;
        m_pApi->getSuperTypeName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline sal_uInt32 RegistryTypeReader::getFieldCount() const
    {   return m_pApi->getFieldCount(m_hImpl); }

inline rtl::OUString RegistryTypeReader::getFieldName( sal_uInt16 index ) const
    {
        rtl::OUString sRet;
        m_pApi->getFieldName(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline rtl::OUString RegistryTypeReader::getFieldType( sal_uInt16 index ) const
    {
        rtl::OUString sRet;
        m_pApi->getFieldType(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline RTFieldAccess RegistryTypeReader::getFieldAccess( sal_uInt16 index ) const
    {  return m_pApi->getFieldAccess(m_hImpl, index); }

inline RTConstValue RegistryTypeReader::getFieldConstValue( sal_uInt16 index ) const
    {
        RTConstValue ret;
        ret.m_type = m_pApi->getFieldConstValue(m_hImpl, index, &ret.m_value);
        return ret;
    }

inline rtl::OUString RegistryTypeReader::getFieldDoku( sal_uInt16 index ) const
    {
        rtl::OUString sRet;
        m_pApi->getFieldDoku(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline rtl::OUString RegistryTypeReader::getFieldFileName( sal_uInt16 index ) const
    {
        rtl::OUString sRet;
        m_pApi->getFieldFileName(m_hImpl, &sRet.pData, index);
        return sRet;
    }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
