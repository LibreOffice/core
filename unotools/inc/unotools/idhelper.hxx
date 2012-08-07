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

#ifndef _UNOTOOLS_ID_HELPER_HXX_
#define _UNOTOOLS_ID_HELPER_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>

//.........................................................................
namespace utl
{
//.........................................................................

//=========================================================================
// to shorten some lines ...
typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  TypeSequence;

// compare to Sequences of Types
struct TypeSequenceLess : public ::std::binary_function<TypeSequence, TypeSequence, bool>
{
public:
    inline bool operator() (const TypeSequence& lhs, const TypeSequence& rhs) const
    {
        sal_Int32 nLengthLeft = lhs.getLength();
        sal_Int32 nLengthRight = rhs.getLength();

        // first check the two lengths
        if (nLengthLeft < nLengthRight)
            return sal_True;
        if (nLengthLeft > nLengthRight)
            return sal_False;

        // both sequences have the same length -> check the type names
        const ::com::sun::star::uno::Type* pTypesLeft = lhs.getConstArray();
        const ::com::sun::star::uno::Type* pTypesRight = rhs.getConstArray();
        for (sal_Int32 i=0; i<nLengthLeft; ++i, ++pTypesLeft, ++pTypesRight)
        {
            sal_Int32 nTypeNameCompare = pTypesLeft->getTypeName().compareTo(pTypesRight->getTypeName());
            if (nTypeNameCompare < 0)
                return sal_True;
            if (nTypeNameCompare > 0)
                return sal_False;
        }

        // both sequences are equal ...
        return sal_False;
    }
};

// declare the map
DECLARE_STL_MAP (   TypeSequence,
                    ::cppu::OImplementationId,
                    TypeSequenceLess,
                    MapType2Id
                );

//.........................................................................
}   // namespace utl
//.........................................................................

//=========================================================================
/** defines a helper class for implementing the XTypeProvider::getImplementationId.
    it maps sequences of ::com::sun::star::uno::Type to implementation ids
    (which means sequences of bytes).<BR>
    As there is no possibility to determine the time where the id's are no longer
    needed (e.g. because the last instance of the class using this mechanism died)
    the helper is "refcounted", i.e. there are acquire and release methods.
    To simplify this there is a class classname##Ref which you may want to
    use as an member of your classes.
    <BR><BR>
    As we don't want a global helper class which handles implementation id's
    of components from all over the office (supposing somebody want's to use this :)
    this is only a define. Wherever you have a "closed" area (which is small enough
    and large enough :), see below) where diffenrent components want to use an id helper,
    define your own one with this macro.<BR>
    The more classes use this helper, the later redundant map entries will be
    cleared. The less classes use it, the earlier map entries which may have
    been reused will be cleared.
*/
#define DECLARE_IMPLEMENTATIONID_HELPER(_namespace, classname)      \
namespace _namespace {                                              \
class classname                                                     \
{                                                                   \
    friend class classname##Ref;                                \
                                                                    \
    static sal_Int32    s_nReferenced;                              \
    static void*        s_pMap;                                     \
                                                                    \
    static ::osl::Mutex s_aMutex;                                   \
                                                                    \
public:                                                             \
    static void acquire();                                          \
    static void release();                                          \
                                                                    \
    static ::com::sun::star::uno::Sequence< sal_Int8 > getImplementationId( \
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _rTypes); \
                                                                    \
private:                                                            \
    static void implCreateMap();                                    \
                                                                    \
    classname() { }                                                 \
};                                                                  \
                                                                    \
/*=======================================================================*/ \
class classname##Ref                                                \
{                                                                   \
public:                                                             \
    classname##Ref() { classname::acquire(); }  \
    ~classname##Ref() { classname::release(); } \
};                                                                  \
                                                                    \
}   /* _namespace */                                                \
                                                                    \

/** implement an id helper
*/
#define IMPLEMENT_IMPLEMENTATIONID_HELPER(_namespace, classname)        \
namespace _namespace {  \
    \
/*=======================================================================*/ \
    \
sal_Int32       classname::s_nReferenced(0);    \
void*           classname::s_pMap = NULL;   \
::osl::Mutex    classname::s_aMutex;    \
    \
/*-----------------------------------------------------------------------*/ \
void classname::acquire()   \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ++s_nReferenced;    \
}   \
    \
/*-----------------------------------------------------------------------*/ \
void classname::release()   \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    if (!--s_nReferenced)   \
    {   \
        delete static_cast< ::utl::MapType2Id *>( s_pMap ); \
        s_pMap = NULL;  \
    }   \
}   \
    \
/*-----------------------------------------------------------------------*/ \
::com::sun::star::uno::Sequence< sal_Int8 > classname::getImplementationId( \
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _rTypes)  \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    DBG_ASSERT(s_nReferenced,   \
        "classname::getImplementationId : you need to hold a reference to this class in order to use it !");    \
        /* give the calling class a member of type classname##Ref and all is fine .... */   \
    \
    implCreateMap();    \
    \
    ::utl::MapType2Id* pMap = static_cast< ::utl::MapType2Id *>(s_pMap);    \
    \
    ::cppu::OImplementationId& rId = (*pMap)[_rTypes];  \
    /* this will create an entry for the given type sequence, if neccessary */  \
    \
    return rId.getImplementationId();   \
}   \
    \
/*-----------------------------------------------------------------------*/ \
void classname::implCreateMap() \
{   \
    if (s_pMap) \
        return; \
    s_pMap = new ::utl::MapType2Id();   \
}   \
    \
    \
}   /* _namespace */    \
    \


#endif // _UNOTOOLS_ID_HELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
