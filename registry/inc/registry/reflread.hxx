/*************************************************************************
 *
 *  $RCSfile: reflread.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obr $ $Date: 2000-11-03 13:44:33 $
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

#ifndef __REGISTRY_REFLREAD_HXX__
#define __REGISTRY_REFLREAD_HXX__

#ifndef _REGISTRY_REFLTYPE_HXX_
#include <registry/refltype.hxx>
#endif
#ifndef _REGISTRY_REGTYPE_H_
#include <registry/regtype.h>
#endif
#ifndef _VOS_DYNLOAD_HXX_
#include <vos/dynload.hxx>
#endif

// Implememetation handle
typedef void* TypeReaderImpl;

/****************************************************************************

    C-Api for load on call

*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

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

typedef RegistryTypeReader_Api* (TYPEREG_CALLTYPE *InitRegistryTypeReader_Api)(void);

#define REGISTRY_TYPE_READER_INIT_FUNCTION_NAME "initRegistryTypeReader_Api"

#ifdef __cplusplus
}
#endif


/** RegistryTypeReaderLoader load the needed DLL for a RegistryTypeReader.
    The loader can be checked if the DLL is loaded. If the DLL is loaded the
    loader provides a valid Api for the RegistryTypeReader.
*/
class RegistryTypeReaderLoader
    : public ::vos::ODynamicLoader<RegistryTypeReader_Api>
{
public:
    RegistryTypeReaderLoader()
        : ::vos::ODynamicLoader<RegistryTypeReader_Api>
            (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_MODULENAME( "reg" LIBRARY_VERSION ) ) ),
             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_TYPE_READER_INIT_FUNCTION_NAME) ))
        {}

    ~RegistryTypeReaderLoader()
        {}
};


/** RegistryTypeReader reads type informations from a blop.
    Class is inline and use a load on call C-Api.
*/
class RegistryTypeReader
{
public:

    inline RegistryTypeReader(const RegistryTypeReader_Api* pApi,
                              const sal_uInt8* buffer,
                              sal_uInt32 bufferLen,
                              sal_Bool copyData);
    inline RegistryTypeReader(const RegistryTypeReaderLoader& rLoader,
                              const sal_uInt8* buffer,
                              sal_uInt32 bufferLen,
                              sal_Bool copyData);
    inline RegistryTypeReader(const RegistryTypeReader& toCopy);
    inline ~RegistryTypeReader();

    inline RegistryTypeReader& operator == (const RegistryTypeReader& toAssign);

    inline sal_Bool         isValid() const;

    inline sal_uInt16       getMinorVersion() const;
    inline sal_uInt16       getMajorVersion() const;
    inline RTTypeClass      getTypeClass() const;
    inline ::rtl::OUString  getTypeName() const;
    inline ::rtl::OUString  getSuperTypeName() const;
    inline void             getUik(RTUik& uik) const;
    inline ::rtl::OUString  getDoku() const;
    inline ::rtl::OUString  getFileName() const;
    inline sal_uInt32       getFieldCount() const;
    inline ::rtl::OUString  getFieldName( sal_uInt16 index ) const;
    inline ::rtl::OUString  getFieldType( sal_uInt16 index ) const;
    inline RTFieldAccess    getFieldAccess( sal_uInt16 index ) const;
    inline RTConstValue     getFieldConstValue( sal_uInt16 index ) const;
    inline ::rtl::OUString  getFieldDoku( sal_uInt16 index ) const;
    inline ::rtl::OUString  getFieldFileName( sal_uInt16 index ) const;
    inline sal_uInt32       getMethodCount() const;
    inline ::rtl::OUString  getMethodName( sal_uInt16 index ) const;
    inline sal_uInt32       getMethodParamCount( sal_uInt16 index ) const;
    inline ::rtl::OUString  getMethodParamType( sal_uInt16 index, sal_uInt16 paramIndex ) const;
    inline ::rtl::OUString  getMethodParamName( sal_uInt16 index, sal_uInt16 paramIndex ) const;
    inline RTParamMode      getMethodParamMode( sal_uInt16 index, sal_uInt16 paramIndex ) const;
    inline sal_uInt32       getMethodExcCount( sal_uInt16 index ) const;
    inline ::rtl::OUString  getMethodExcType( sal_uInt16 index, sal_uInt16 excIndex ) const;
    inline ::rtl::OUString  getMethodReturnType( sal_uInt16 index ) const;
    inline RTMethodMode     getMethodMode( sal_uInt16 index ) const;
    inline ::rtl::OUString  getMethodDoku( sal_uInt16 index ) const;

    inline sal_uInt32       getReferenceCount() const;
    inline ::rtl::OUString  getReferenceName( sal_uInt16 index ) const;
    inline RTReferenceType  getReferenceType( sal_uInt16 index ) const;
    inline ::rtl::OUString  getReferenceDoku( sal_uInt16 index ) const;
    inline RTFieldAccess    getReferenceAccess( sal_uInt16 index ) const;

protected:

    const RegistryTypeReader_Api*                                m_pApi;
    const NAMESPACE_VOS(ODynamicLoader)<RegistryTypeReader_Api>  m_Api;
    TypeReaderImpl                                               m_hImpl;
};



inline RegistryTypeReader::RegistryTypeReader(const RegistryTypeReader_Api* pApi,
                                                const sal_uInt8* buffer,
                                              sal_uInt32 bufferLen,
                                              sal_Bool copyData)
    : m_pApi(pApi)
    , m_Api()
    , m_hImpl(NULL)
    {
        m_hImpl = m_pApi->createEntry(buffer, bufferLen, copyData);
    }


inline RegistryTypeReader::RegistryTypeReader(const RegistryTypeReaderLoader& rLoader,
                                                const sal_uInt8* buffer,
                                              sal_uInt32 bufferLen,
                                              sal_Bool copyData)
    : m_pApi(NULL)
    , m_Api(rLoader)
    , m_hImpl(NULL)
    {
        m_pApi = m_Api.getApi();
        m_hImpl = m_pApi->createEntry(buffer, bufferLen, copyData);
    }


inline RegistryTypeReader::RegistryTypeReader(const RegistryTypeReader& toCopy)
    : m_pApi(toCopy.m_pApi)
    , m_Api(toCopy.m_Api)
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
