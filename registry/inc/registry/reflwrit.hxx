/*************************************************************************
 *
 *  $RCSfile: reflwrit.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:42 $
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

/*************************************************************************

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.20  2000/09/17 12:30:32  willem.vandorp
    OpenOffice header added.

    Revision 1.19  2000/08/04 15:31:39  jsc
    extend biniary type format

    Revision 1.18  2000/07/26 17:44:32  willem.vandorp
    Headers/footers replaced

    Revision 1.17  2000/05/02 06:06:21  jsc
    insert LIBRARY_VERSION for dynamic loading

    Revision 1.16  2000/04/18 10:09:48  jsc
    modified for unicode changes

    Revision 1.15  2000/03/24 15:00:36  jsc
    modified for unicode changes

    Revision 1.14  1999/10/19 16:14:49  kr
    Version parameter for dynload added

    Revision 1.13  1999/09/06 08:33:00  jsc
    insert IDL filename in registry

    Revision 1.12  1999/07/02 09:25:40  jsc
    #67179# Binaerblop der CoreReflection ist erweitert worden

    Revision 1.11  1999/03/12 14:16:38  jsc
    #61011# Anpassung an osl-Typen

    Revision 1.10  1999/02/05 09:44:33  jsc
    #61011# Typanpassung

    Revision 1.9  1998/12/11 11:57:42  jsc
    Anpassung an ODynamicLoader

    Revision 1.8  1998/08/06 09:43:44  jsc
    #54698# Anpassung an DynamicLoader

    Revision 1.7  1998/07/06 17:02:03  jsc
    Calltype von InitRegistryTypeWriter_Api gesetzt

    Revision 1.6  1998/07/03 11:20:08  ts
    prefixen von typen

    Revision 1.5  1998/07/03 08:41:03  ts
    bugs und set/getUik()

    Revision 1.4  1998/07/02 11:58:39  jsc
    Typ Anpassung, Schnittstelle frei von solar.h

    Revision 1.3  1998/07/01 16:55:19  ts
    alignment unter soaris beachten

    Revision 1.2  1998/07/01 13:39:01  ts
    bugs, bugs, bugs

    Revision 1.1.1.1  1998/06/30 11:17:45  jsc
    neu


*************************************************************************/

#ifndef __REGISTRY_REFLWRIT_HXX__
#define __REGISTRY_REFLWRIT_HXX__

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
    : public ::vos::ODynamicLoader<RegistryTypeWriter_Api>
{
public:
    RegistryTypeWriterLoader()
        : ::vos::ODynamicLoader<RegistryTypeWriter_Api>
            (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("reg") ),
             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_TYPE_WRITER_INIT_FUNCTION_NAME) ),
             sal_True, LIBRARY_VERSION, sal_False)
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
    const NAMESPACE_VOS(ODynamicLoader)<RegistryTypeWriter_Api>  m_Api;
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
