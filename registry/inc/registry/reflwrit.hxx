/*************************************************************************
 *
 *  $RCSfile: reflwrit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-14 09:37:08 $
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

#ifndef _REGISTRY_REFLWRIT_HXX_
#define _REGISTRY_REFLWRIT_HXX_

#ifndef _REGISTRY_REFLTYPE_HXX_
#include <registry/refltype.hxx>
#endif
#ifndef _REGISTRY_REGTYPE_H_
#include <registry/regtype.h>
#endif
#ifndef _SALHELPER_DYNLOAD_HXX_
#include <salhelper/dynload.hxx>
#endif

// Implememetation handle
typedef void* TypeWriterImpl;

/****************************************************************************

    C-Api for load on call

*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

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

typedef RegistryTypeWriter_Api* (TYPEREG_CALLTYPE *InitRegistryTypeWriter_Api)(void);

#define REGISTRY_TYPE_WRITER_INIT_FUNCTION_NAME "initRegistryTypeWriter_Api"

#ifdef __cplusplus
}
#endif


/** RegistryTypeWriterLoader load the needed DLL for a RegistryTypeWriter.
    The loader can be checked if the DLL is loaded. If the DLL is loaded the
    loader provides a valid Api for the RegistryTypeWriter.
*/
class RegistryTypeWriterLoader
    : public ::salhelper::ODynamicLoader<RegistryTypeWriter_Api>
{
public:
    RegistryTypeWriterLoader()
        : ::salhelper::ODynamicLoader<RegistryTypeWriter_Api>
            (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_MODULENAME( "reg" LIBRARY_VERSION  ) ) ),
             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_TYPE_WRITER_INIT_FUNCTION_NAME) ))
        {}

    ~RegistryTypeWriterLoader()
        {}
};


/** RegistryTypeWriter writes type informations into a blop.
    Class is inline and use a load on call C-Api.
*/
class RegistryTypeWriter
{
public:

    inline RegistryTypeWriter(const RegistryTypeWriter_Api* pApi,
                              RTTypeClass               RTTypeClass,
                              const ::rtl::OUString&    typeName,
                              const ::rtl::OUString&    superTypeName,
                              sal_uInt16                fieldCount,
                              sal_uInt16                methodCount,
                              sal_uInt16                referenceCount);

    inline RegistryTypeWriter(const RegistryTypeWriterLoader& rLoader,
                              RTTypeClass               RTTypeClass,
                              const ::rtl::OUString&    typeName,
                              const ::rtl::OUString&    superTypeName,
                              sal_uInt16                fieldCount,
                              sal_uInt16                methodCount,
                              sal_uInt16                referenceCount);

    inline RegistryTypeWriter(const RegistryTypeWriter& toCopy);
    inline ~RegistryTypeWriter();

    inline RegistryTypeWriter& operator == (const RegistryTypeWriter& toAssign);

    inline void setUik(const RTUik& uik);

    inline void setDoku(const ::rtl::OUString& doku);

    inline void setFileName(const ::rtl::OUString& fileName);

    inline void setFieldData( sal_uInt16                index,
                              const ::rtl::OUString&    name,
                              const ::rtl::OUString&    typeName,
                              const ::rtl::OUString&    doku,
                              const ::rtl::OUString&    fileName,
                              RTFieldAccess             access,
                              RTConstValue              constValue = RTConstValue());

    inline void setMethodData(sal_uInt16                index,
                              const ::rtl::OUString&    name,
                              const ::rtl::OUString&    returnTypeName,
                              RTMethodMode              mode,
                              sal_uInt16                paramCount,
                              sal_uInt16                excCount,
                              const ::rtl::OUString&    doku);

    inline void setParamData(sal_uInt16             index,
                             sal_uInt16             paramIndex,
                             const ::rtl::OUString& type,
                             const ::rtl::OUString& name,
                             RTParamMode            mode);

    inline void setExcData(sal_uInt16               index,
                           sal_uInt16               excIndex,
                           const ::rtl::OUString&   type);

    inline const sal_uInt8*     getBlop();
    inline sal_uInt32       getBlopSize();

    inline void setReferenceData( sal_uInt16                index,
                                    const ::rtl::OUString&  name,
                                    RTReferenceType             refType,
                                    const ::rtl::OUString&  doku,
                                    RTFieldAccess               access = RT_ACCESS_INVALID);

protected:

    const RegistryTypeWriter_Api*                                m_pApi;
    const ::salhelper::ODynamicLoader< RegistryTypeWriter_Api >  m_Api;
    TypeWriterImpl                                               m_hImpl;
};



inline RegistryTypeWriter::RegistryTypeWriter(const RegistryTypeWriter_Api* pApi,
                                              RTTypeClass               RTTypeClass,
                                              const ::rtl::OUString&    typeName,
                                              const ::rtl::OUString&    superTypeName,
                                              sal_uInt16                fieldCount,
                                              sal_uInt16                methodCount,
                                              sal_uInt16                referenceCount)
    : m_pApi(pApi)
    , m_Api()
    , m_hImpl(NULL)
{
    m_hImpl = m_pApi->createEntry(RTTypeClass,
                                  typeName.pData,
                                  superTypeName.pData,
                                  fieldCount,
                                  methodCount,
                                  referenceCount);
}


inline RegistryTypeWriter::RegistryTypeWriter(const RegistryTypeWriterLoader& rLoader,
                                              RTTypeClass               RTTypeClass,
                                              const ::rtl::OUString&    typeName,
                                              const ::rtl::OUString&    superTypeName,
                                              sal_uInt16                fieldCount,
                                              sal_uInt16                methodCount,
                                              sal_uInt16                referenceCount)
    : m_pApi(NULL)
    , m_Api(rLoader)
    , m_hImpl(NULL)
{
    m_pApi = m_Api.getApi();
    m_hImpl = m_pApi->createEntry(RTTypeClass,
                                  typeName.pData,
                                  superTypeName.pData,
                                  fieldCount,
                                  methodCount,
                                  referenceCount);
}


inline RegistryTypeWriter::RegistryTypeWriter(const RegistryTypeWriter& toCopy)
    : m_pApi(toCopy.m_pApi)
    , m_Api(toCopy.m_Api)
    , m_hImpl(toCopy.m_hImpl)
{
    m_Api->acquire(m_hImpl);
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

inline void RegistryTypeWriter::setFieldData( sal_uInt16                index,
                                              const ::rtl::OUString&    name,
                                              const ::rtl::OUString&    typeName,
                                              const ::rtl::OUString&    doku,
                                              const ::rtl::OUString&    fileName,
                                              RTFieldAccess             access,
                                              RTConstValue              constValue)
{
    m_pApi->setFieldData(m_hImpl, index, name.pData, typeName.pData, doku.pData, fileName.pData, access, constValue.m_type, constValue.m_value);
}


inline void RegistryTypeWriter::setMethodData(sal_uInt16                index,
                                              const ::rtl::OUString&    name,
                                              const ::rtl::OUString&    returnTypeName,
                                              RTMethodMode              mode,
                                              sal_uInt16                paramCount,
                                              sal_uInt16                excCount,
                                              const ::rtl::OUString&    doku)
{
    m_pApi->setMethodData(m_hImpl, index, name.pData, returnTypeName.pData, mode, paramCount, excCount, doku.pData);
}


inline void RegistryTypeWriter::setUik(const RTUik& uik)
{
    m_pApi->setUik(m_hImpl, &uik);
}

inline void RegistryTypeWriter::setDoku(const ::rtl::OUString& doku)
{
    m_pApi->setDoku(m_hImpl, doku.pData);
}

inline void RegistryTypeWriter::setFileName(const ::rtl::OUString& doku)
{
    m_pApi->setFileName(m_hImpl, doku.pData);
}

inline void RegistryTypeWriter::setParamData(sal_uInt16             index,
                                             sal_uInt16             paramIndex,
                                             const ::rtl::OUString& type,
                                             const ::rtl::OUString& name,
                                             RTParamMode            mode)
{
    m_pApi->setParamData(m_hImpl, index, paramIndex, type.pData, name.pData, mode);
}

inline void RegistryTypeWriter::setExcData(sal_uInt16               index,
                                           sal_uInt16               excIndex,
                                           const ::rtl::OUString&   type)
{
    m_pApi->setExcData(m_hImpl, index, excIndex, type.pData);
}

inline const sal_uInt8* RegistryTypeWriter::getBlop()
{
    return m_pApi->getBlop(m_hImpl);
}

inline sal_uInt32 RegistryTypeWriter::getBlopSize()
{
    return m_pApi->getBlopSize(m_hImpl);
}


inline void RegistryTypeWriter::setReferenceData( sal_uInt16                index,
                                                    const ::rtl::OUString&  name,
                                                    RTReferenceType             refType,
                                                    const ::rtl::OUString&  doku,
                                                    RTFieldAccess           access)
{
    m_pApi->setReferenceData(m_hImpl, index, name.pData, refType, doku.pData, access);
}

#endif
