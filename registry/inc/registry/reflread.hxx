/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: reflread.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _REGISTRY_REFLREAD_HXX_
#define _REGISTRY_REFLREAD_HXX_

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
    TypeReaderImpl      (TYPEREG_CALLTYPE *createEntry)         (const sal_uInt8*, sal_uInt32, sal_Bool);
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
RegistryTypeReader_Api* TYPEREG_CALLTYPE initRegistryTypeReader_Api(void);

#ifdef __cplusplus
}
#endif

/** RegistryTypeReades reads a binary type blob.

    This class provides the necessary functions to read type informations
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
        @param bufferlen specifies the size of the binary data block.
        @param copyData specifies if the data block should be copied.
                        The block can be copied to ensure that the data
                        is valid for the lifetime of this instance.
     */
    inline RegistryTypeReader(const sal_uInt8* buffer,
                              sal_uInt32 bufferLen,
                              sal_Bool copyData);

    /// Copy constructcor
    inline RegistryTypeReader(const RegistryTypeReader& toCopy);

    /// Destructor. The Destructor frees the data block if the copyData flag was TRUE.
    inline ~RegistryTypeReader();

    /// Assign operator
    inline RegistryTypeReader& operator == (const RegistryTypeReader& toAssign);

    /// checks if the registry type reader points to a valid Api.
    inline sal_Bool         isValid() const;

    /** @deprecated
        returns the minor version number.

        We currently don't support a versioning concept of IDL interfaces and
        so this function is currently not used.
     */
    inline sal_uInt16       getMinorVersion() const;

    /** @deprecated
        returns the major version number.

        We currently don't support a versioning concept of IDL interfaces and
        so this function is currently not used.
     */
    inline sal_uInt16       getMajorVersion() const;

    /** returns the typeclass of the type represented by this blob.

        This function will always return the type class without the internal
        RT_TYPE_PUBLISHED flag set.
     */
    inline RTTypeClass      getTypeClass() const;

    /** returns the full qualified name of the type.
     */
    inline ::rtl::OUString  getTypeName() const;

    /** returns the full qualified name of the supertype.
     */
    inline ::rtl::OUString  getSuperTypeName() const;

    /** @deprecated
        returns the unique identifier for an interface type as an out parameter.

        An earlier version of UNO used an unique identifier for interfaces. In the
        current version of UNO this uik was eliminated and this function is
        not longer used.
     */
    inline void             getUik(RTUik& uik) const;

    /** returns the documentation string of this type.
     */
    inline ::rtl::OUString  getDoku() const;

    /** returns the IDL filename where the type is defined.
     */
    inline ::rtl::OUString  getFileName() const;

    /** returns the number of fields (attributes/properties, enum values or number
        of constants in a module).

     */
    inline sal_uInt32       getFieldCount() const;

    /** returns the name of the field specified by index.
     */
    inline ::rtl::OUString  getFieldName( sal_uInt16 index ) const;

    /** returns the full qualified name of the field specified by index.
     */
    inline ::rtl::OUString  getFieldType( sal_uInt16 index ) const;

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
    inline ::rtl::OUString  getFieldDoku( sal_uInt16 index ) const;

    /** returns the IDL filename of the field specified by index.

        The IDL filename of a field can differ from the filename of the ype itself
        because modules and also constants can be defined in different IDL files.
     */
    inline ::rtl::OUString  getFieldFileName( sal_uInt16 index ) const;

    /** returns the number of methods of an interface type.
     */
    inline sal_uInt32       getMethodCount() const;

    /** returns the name of the method specified by index.
     */
    inline ::rtl::OUString  getMethodName( sal_uInt16 index ) const;

    /** returns number of parameters of the method specified by index.
     */
    inline sal_uInt32       getMethodParamCount( sal_uInt16 index ) const;

    /** returns the full qualified parameter typename.

        @param index indicates the method
        @param paramIndex indeciates the parameter which type will be returned.
     */
    inline ::rtl::OUString  getMethodParamType( sal_uInt16 index, sal_uInt16 paramIndex ) const;

    /** returns the name of a parameter.

        @param index indicates the method
        @param paramIndex indiciates the parameter which name will be returned.
     */
    inline ::rtl::OUString  getMethodParamName( sal_uInt16 index, sal_uInt16 paramIndex ) const;

    /** returns the parameter mode, if it is an in, out or inout parameter.

        @param index indicates the method
        @param paramIndex indeciates the parameter which mode will be returned.
     */
    inline RTParamMode      getMethodParamMode( sal_uInt16 index, sal_uInt16 paramIndex ) const;

    /** returns the number of exceptions which are declared for the method specified by index.

        @param index indicates the method
     */
    inline sal_uInt32       getMethodExcCount( sal_uInt16 index ) const;

    /** returns the full qualified exception type of the specified exception.

        @param index indicates the method
        @param paramIndex indeciates the exception which typename will be returned.
     */
    inline ::rtl::OUString  getMethodExcType( sal_uInt16 index, sal_uInt16 excIndex ) const;

    /** returns the full qualified return type of the method specified by index.
     */
    inline ::rtl::OUString  getMethodReturnType( sal_uInt16 index ) const;

    /** returns the full qualified exception type of the specified exception.

        @param index indicates the method
        @param paramIndex indeciates the exception which typename will be returned.
     */
    inline RTMethodMode     getMethodMode( sal_uInt16 index ) const;

    /** returns the documentation string of the method specified by index.

        @param index indicates the method.
     */
    inline ::rtl::OUString  getMethodDoku( sal_uInt16 index ) const;

    /** returns the number of references (supported interfaces, exported services).
     */
    inline sal_uInt32       getReferenceCount() const;

    /** returns the full qualified typename of the reference specified by index.

        @param index indicates the reference.
     */
    inline ::rtl::OUString  getReferenceName( sal_uInt16 index ) const;

    /** returns the type of the reference specified by index.

        @param index indicates the reference.
     */
    inline RTReferenceType  getReferenceType( sal_uInt16 index ) const;

    /** returns the documentation string of the reference specified by index.

        @param index indicates the reference.
    */
    inline ::rtl::OUString  getReferenceDoku( sal_uInt16 index ) const;

    /** returns the access mode of the reference specified by index.

        The only valid value is RT_ACCESS_OPTIONAL in the context of
        references.
        @param index indicates the reference.
     */
    inline RTFieldAccess    getReferenceAccess( sal_uInt16 index ) const;

protected:

    /// stores the registry type reader Api.
    const RegistryTypeReader_Api*                               m_pApi;
    /// stores the handle of an implementation class
    TypeReaderImpl                                              m_hImpl;
};



inline RegistryTypeReader::RegistryTypeReader(const sal_uInt8* buffer,
                                              sal_uInt32 bufferLen,
                                              sal_Bool copyData)
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

inline sal_uInt16 RegistryTypeReader::getMinorVersion() const
    {  return m_pApi->getMinorVersion(m_hImpl); }

inline sal_Bool RegistryTypeReader::isValid() const
    {  return (m_hImpl != NULL); }

inline sal_uInt16 RegistryTypeReader::getMajorVersion() const
    {  return m_pApi->getMajorVersion(m_hImpl); }

inline RTTypeClass RegistryTypeReader::getTypeClass() const
    {  return m_pApi->getTypeClass(m_hImpl); }

inline ::rtl::OUString RegistryTypeReader::getTypeName() const
    {
        ::rtl::OUString sRet;
        m_pApi->getTypeName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline ::rtl::OUString RegistryTypeReader::getSuperTypeName() const
    {
        ::rtl::OUString sRet;
        m_pApi->getSuperTypeName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline void RegistryTypeReader::getUik(RTUik& uik) const
    {  m_pApi->getUik(m_hImpl, &uik); }

inline ::rtl::OUString RegistryTypeReader::getDoku() const
    {
        ::rtl::OUString sRet;
        m_pApi->getDoku(m_hImpl, &sRet.pData);
        return sRet;
    }

inline ::rtl::OUString RegistryTypeReader::getFileName() const
    {
        ::rtl::OUString sRet;
        m_pApi->getFileName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline sal_uInt32 RegistryTypeReader::getFieldCount() const
    {   return m_pApi->getFieldCount(m_hImpl); }

inline ::rtl::OUString RegistryTypeReader::getFieldName( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getFieldName(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline ::rtl::OUString RegistryTypeReader::getFieldType( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
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

inline ::rtl::OUString RegistryTypeReader::getFieldDoku( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getFieldDoku(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline ::rtl::OUString RegistryTypeReader::getFieldFileName( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getFieldFileName(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline sal_uInt32 RegistryTypeReader::getMethodCount() const
    {  return m_pApi->getMethodCount(m_hImpl); }

inline ::rtl::OUString RegistryTypeReader::getMethodName( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getMethodName(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline sal_uInt32 RegistryTypeReader::getMethodParamCount( sal_uInt16 index ) const
    {  return m_pApi->getMethodParamCount(m_hImpl, index); }

inline ::rtl::OUString RegistryTypeReader::getMethodParamType( sal_uInt16 index, sal_uInt16 paramIndex ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getMethodParamType(m_hImpl, &sRet.pData, index, paramIndex);
        return sRet;
    }

inline ::rtl::OUString RegistryTypeReader::getMethodParamName( sal_uInt16 index, sal_uInt16 paramIndex ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getMethodParamName(m_hImpl, &sRet.pData, index, paramIndex);
        return sRet;
    }

inline RTParamMode RegistryTypeReader::getMethodParamMode( sal_uInt16 index, sal_uInt16 paramIndex ) const
    {  return m_pApi->getMethodParamMode(m_hImpl, index, paramIndex); }

inline sal_uInt32 RegistryTypeReader::getMethodExcCount( sal_uInt16 index ) const
    {  return m_pApi->getMethodExcCount(m_hImpl, index); }

inline ::rtl::OUString RegistryTypeReader::getMethodExcType( sal_uInt16 index, sal_uInt16 excIndex ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getMethodExcType(m_hImpl, &sRet.pData, index, excIndex);
        return sRet;
    }

inline ::rtl::OUString RegistryTypeReader::getMethodReturnType( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getMethodReturnType(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline RTMethodMode RegistryTypeReader::getMethodMode( sal_uInt16 index ) const
    {  return m_pApi->getMethodMode(m_hImpl, index); }

inline ::rtl::OUString RegistryTypeReader::getMethodDoku( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getMethodDoku(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline sal_uInt32 RegistryTypeReader::getReferenceCount() const
    {  return m_pApi->getReferenceCount(m_hImpl); }

inline ::rtl::OUString RegistryTypeReader::getReferenceName( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getReferenceName(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline RTReferenceType RegistryTypeReader::getReferenceType( sal_uInt16 index ) const
    {  return m_pApi->getReferenceType(m_hImpl, index); }

inline ::rtl::OUString RegistryTypeReader::getReferenceDoku( sal_uInt16 index ) const
    {
        ::rtl::OUString sRet;
        m_pApi->getReferenceDoku(m_hImpl, &sRet.pData, index);
        return sRet;
    }

inline RTFieldAccess RegistryTypeReader::getReferenceAccess( sal_uInt16 index ) const
    {  return m_pApi->getReferenceAccess(m_hImpl, index); }

#endif
