/*************************************************************************
 *
 *  $RCSfile: reflwrit.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jsc $ $Date: 2001-11-15 18:01:32 $
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

/// Implememetation handle
typedef void* TypeWriterImpl;

/****************************************************************************

    C-Api for load on call

*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/** specifies a collection of function pointers which represents the complete registry type writer C-API.

    The function pointers of this struct will be initialized when the library is loaded over
    the load on call mechanism specified in 'salhelper/dynload.hxx'. This funtions pointers are
    used by the C++ wrapper to call the C-API.
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

/** specifies a function pointer of the initialization function which is called to initialize
    the RegistryTypeWriter_Api struct.

 */
typedef RegistryTypeWriter_Api* (TYPEREG_CALLTYPE *InitRegistryTypeWriter_Api)(void);

/** spedifies the name of the API initialization function.

    This function will be searched by the load on call mechanism specified
    in 'salhelper/dynload.hxx'.
*/
#define REGISTRY_TYPE_WRITER_INIT_FUNCTION_NAME "initRegistryTypeWriter_Api"

#ifdef __cplusplus
}
#endif


/** The RegistryTypeWriterLoader provides a load on call mechanism for the library
    used for the registry type writer api.

    Furthermore it provides a reference counter for the library. When the last reference will be
    destroyed the RegisteryTypeWriterLoader will unload the library. If the library is loaded the loader
    provides a valid Api for the type writer.
    @see salhelper::ODynamicLoader<>
*/
class RegistryTypeWriterLoader
    : public ::salhelper::ODynamicLoader<RegistryTypeWriter_Api>
{
public:
    /// Default constructor, try to load the registry library and initialize the needed Api.
    RegistryTypeWriterLoader()
        : ::salhelper::ODynamicLoader<RegistryTypeWriter_Api>
            (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_MODULENAME_WITH_VERSION( "reg", LIBRARY_VERSION  ) ) ),
             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_TYPE_WRITER_INIT_FUNCTION_NAME) ))
        {}

    /// Destructor, decrease the refcount and unload the library if the refcount is 0.
    ~RegistryTypeWriterLoader()
        {}
};


/** RegistryTypeWriter writes/creates a binary type blob.

    This class provides the necessary functions to write type informations
    for all kinds of types into a blob.
    The class is inline and use a load on call C-Api.
*/
class RegistryTypeWriter
{
public:

    /** Constructor using the registry Api directly.

        The constructor is used if the api is known.
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
    inline RegistryTypeWriter(const RegistryTypeWriter_Api* pApi,
                              RTTypeClass               RTTypeClass,
                              const ::rtl::OUString&    typeName,
                              const ::rtl::OUString&    superTypeName,
                              sal_uInt16                fieldCount,
                              sal_uInt16                methodCount,
                              sal_uInt16                referenceCount);

    /** Constructor using the loader mechanism.

        This constructor is called with a RegistryTypeWriterLoader.
        The RegistryTypeWriterLoader loads the needed DLL and provides the needed
        Api for the registry type writer.
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
    inline RegistryTypeWriter(const RegistryTypeWriterLoader& rLoader,
                              RTTypeClass               RTTypeClass,
                              const ::rtl::OUString&    typeName,
                              const ::rtl::OUString&    superTypeName,
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

    /** @deprecated
        sets the unique identifier for an interface type.

        An earlier version of UNO used an unique identifier for interfaces. In the
        current version of UNO this uik was eliminated and this function is
        not longer used.
     */
    inline void setUik(const RTUik& uik);

    /** sets a documentation string for the type.

        This documentation should be the same as the documentation which is provided
        for this type in IDL.
     */
    inline void setDoku(const ::rtl::OUString& doku);

    /** sets the IDL filename where this type is defined.
     */
    inline void setFileName(const ::rtl::OUString& fileName);

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
    inline void setFieldData( sal_uInt16                index,
                              const ::rtl::OUString&    name,
                              const ::rtl::OUString&    typeName,
                              const ::rtl::OUString&    doku,
                              const ::rtl::OUString&    fileName,
                              RTFieldAccess             access,
                              RTConstValue              constValue = RTConstValue());

    /** sets the data for a method.

        @param index indicates the index of the method.
        @param name specifies the name.
        @param typeName specifies the full qualified return typename.
        @param mode specifies the method mode.
        @param paramCount specifies the number of parameters.
        @param excCount specifies the number of exceptions.
        @param doku specifies the documentation string of the field.
     */
    inline void setMethodData(sal_uInt16                index,
                              const ::rtl::OUString&    name,
                              const ::rtl::OUString&    returnTypeName,
                              RTMethodMode              mode,
                              sal_uInt16                paramCount,
                              sal_uInt16                excCount,
                              const ::rtl::OUString&    doku);

    /** sets the data for the specified parameter of a method.

        @param index indicates the index of the method.
        @param paramIndex specifies the index of the parameter.
        @param type specifies the full qualified typename.
        @param name specifies the name.
        @param mode specifies the parameter mode.
     */
    inline void setParamData(sal_uInt16             index,
                             sal_uInt16             paramIndex,
                             const ::rtl::OUString& type,
                             const ::rtl::OUString& name,
                             RTParamMode            mode);

    /** sets the data for the specified exception of a mehtod.

        @param index indicates the index of the method.
        @param excIndex specifies the index of the exception.
        @param type specifies the full qualified typename of the exception.
     */
    inline void setExcData(sal_uInt16               index,
                           sal_uInt16               excIndex,
                           const ::rtl::OUString&   type);

    /** returns a pointer to the new type blob.

        The pointer will be invalid (NULL) if the instance of
        the RegistryTypeWriter will be destroyed.
     */
    inline const sal_uInt8*     getBlop();

    /** returns the size of the new type blob in bytes.
     */
    inline sal_uInt32       getBlopSize();

    /** sets the data for a reference member.

        @param index indicates the index of the reference.
        @param name specifies the name.
        @param refType specifies the full qualified typename of the reference.
        @param doku specifies the documentation string of the reference.
        @param access specifies the access mode of the reference.
     */
    inline void setReferenceData( sal_uInt16                index,
                                    const ::rtl::OUString&  name,
                                    RTReferenceType             refType,
                                    const ::rtl::OUString&  doku,
                                    RTFieldAccess               access = RT_ACCESS_INVALID);

protected:

    /// stores the registry type writer Api.
    const RegistryTypeWriter_Api*                                m_pApi;
    /// stores the dynamic loader which is used to hold the library.
    const ::salhelper::ODynamicLoader< RegistryTypeWriter_Api >  m_Api;
    /// stores the handle of an implementation class
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
