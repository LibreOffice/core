/*************************************************************************
 *
 *  $RCSfile: object.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:12 $
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


#ifndef _VOS_OBJECT_HXX_
#define _VOS_OBJECT_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_MACROS_HXX_
#   include <vos/macros.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

// ***************************************
// Object super class

struct OClassInfo;
struct OCreateParam;

/** OObject
    common base class for all framework classes. Used for memory-management
    and runtime type-info.
*/
class OObject
{
public:

    ///
    OObject();

    ///
    OObject(const OCreateParam& rParam);

    // Disable the copy constructor and assignment by default so you will get
    // compiler errors instead of unexpected behaviour if you pass objects
    // by value or assign objects.
private:
    OObject(const OObject& objectSrc);          // no implementation
    void SAL_CALL operator=(const OObject& objectSrc);  // no implementation

public:
    virtual ~OObject();

public:

    /** Define private new and delete operator because of compiler bug,
        when allocating and deleteing a exported class
    */
#if defined MACOS || defined MACOSX
    void* SAL_CALL operator new(size_t size);
    void* SAL_CALL operator new(size_t size, void* p);
#else
    void* SAL_CALL operator new(unsigned int size);
    void* SAL_CALL operator new(unsigned int size, void* p);
#endif

    void  SAL_CALL operator delete(void* p);

// Attributes
public:

    ///
    virtual const OClassInfo& SAL_CALL getClassInfo() const;

    ///
    sal_Bool SAL_CALL isKindOf(const OClassInfo& rClass) const;

// Implementation
public:
    static const OClassInfo& SAL_CALL classInfo();

public:
    static OClassInfo __ClassInfo__;
};


/**
    Basic class information
*/
struct OCreateParam
{
    sal_uInt32 m_Size;
    void*    m_pParam;

    ///
    OCreateParam(void *pParam)
    {
        m_Size = sizeof(OCreateParam);
        m_pParam = pParam;
    }
};

/**
*/
struct OClassInfo
{
    ///
    const sal_Char  *m_pClassName;
    ///
    sal_Int32            m_nObjectSize;
    /// schema number of the loaded class
    sal_uInt32   m_wSchema;

    ///
    OObject* (SAL_CALL * m_pfnCreateObject)(const OCreateParam&);   // NULL => abstract class

    /// linked list of registered classes
    const OClassInfo* m_pBaseClass;
    /// linked list of registered classes
    const OClassInfo* m_pNextClass;

    ///
    OObject* SAL_CALL createObject(const OCreateParam& rParam) const;

    ///
    sal_Bool SAL_CALL isDerivedFrom(const OClassInfo& rBaseClass) const;

    ///
    static const OClassInfo* SAL_CALL getClassInfo(const sal_Char* pClassName);

    ///
    OClassInfo(const sal_Char *pClassName, sal_Int32 ObjectSize,
               const OClassInfo* pBaseClass = NULL, sal_uInt32 Schema = (sal_uInt32)-1,
               OObject* (SAL_CALL * fnCreateObject)(const OCreateParam&) = NULL);
};

// *****************************************************************
// Helper macros for declaring OClassInfo data


#define VOS_STRINGIZE(name) #name

#define VOS_CLASSNAME(class_name, domain_name) VOS_STRINGIZE(domain_name.class_name)

#define VOS_CLASSINFO(class_name) (class_name::classInfo())

// generate static object constructor for class registration
struct VOS_CLASSINIT
{ VOS_CLASSINIT(VOS_NAMESPACE(OClassInfo, vos)* pNewClass); };

#define VOS_CLASSDATA(class_spec, class_name, base_class_name, wSchema, constructor) \
    VOS_NAMESPACE(OClassInfo, vos) class_name::__ClassInfo__(class_spec, \
        sizeof(class_name), &VOS_CLASSINFO(base_class_name), wSchema, constructor); \
    const VOS_NAMESPACE(VOS_CLASSINIT, vos) class_name::__ClassInit__(&class_name::__ClassInfo__); \
    const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL class_name::getClassInfo() const \
        { return (VOS_CLASSINFO(class_name)); } \
    const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL class_name::classInfo() \
        { return (__ClassInfo__); }

#define VOS_DECLARE_CLASSINFO(class_name) \
public: \
    static const VOS_NAMESPACE(VOS_CLASSINIT, vos) __ClassInit__; \
    static VOS_NAMESPACE(OClassInfo, vos) __ClassInfo__; \
public: \
    virtual const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL getClassInfo() const; \
    static const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL classInfo()

#define VOS_IMPLEMENT_CLASSINFO(class_spec, class_name, base_class_name, wSchema) \
    VOS_CLASSDATA(class_spec, class_name, base_class_name, wSchema, NULL)

#define VOS_DECLARE_CLASSTYPE(class_name) \
    VOS_DECLARE_CLASSINFO(class_name); \
public: \
    static VOS_NAMESPACE(OObject, vos)* SAL_CALL createObject(const VOS_NAMESPACE(OCreateParam, vos)& rParam);

#define VOS_IMPLEMENT_CLASSTYPE(class_spec, class_name, base_class_name, wSchema) \
    VOS_CLASSDATA(class_spec, class_name, base_class_name, wSchema, class_name::createObject) \
    VOS_NAMESPACE(OObject, vos)* class_name::createObject(const VOS_NAMESPACE(OCreateParam, vos)& rParam) \
        { return new class_name(rParam); }

#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_OBJECT_HXX_

