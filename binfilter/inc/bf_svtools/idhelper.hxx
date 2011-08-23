/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BF_SVTOOLS_ID_HELPER_HXX_
#define _BF_SVTOOLS_ID_HELPER_HXX_

#include <unotools/idhelper.hxx>

#define DECLARE_IMPLEMENTATIONID_HELPER_VSMALLSTART(_namespace, classname)		\
namespace _namespace {												\
class classname														\
{																	\
    friend class classname##Ref;								\
                                                                    \
    static sal_Int32	s_nReferenced;								\
    static void*		s_pMap;										\
                                                                    \
    static ::osl::Mutex	s_aMutex;									\
                                                                    \
public:																\
    static void acquire();											\
    static void release();											\
                                                                    \
                                                                \

#define DECLARE_IMPLEMENTATIONID_HELPER_VSMALLEND(_namespace, classname)              \
    classname() { }													\
};																	\
                                                                    \
/*=======================================================================*/	\
class classname##Ref												\
{																	\
public:																\
    classname##Ref() { classname::acquire(); }	\
    ~classname##Ref() { classname::release(); }	\
};																	\
                                                                    \
}	/* _namespace */												\
                                                                    \

#define DECLARE_IMPLEMENTATIONID_HELPER_VSMALL(_namespace, classname)            \
DECLARE_IMPLEMENTATIONID_HELPER_VSMALLSTART(_namespace, classname) \
DECLARE_IMPLEMENTATIONID_HELPER_VSMALLEND(_namespace, classname)


#define DECLARE_IMPLEMENTATIONID_HELPER_SMALL(_namespace, classname)		\
DECLARE_IMPLEMENTATIONID_HELPER_VSMALLSTART(_namespace, classname) \
    static ::com::sun::star::uno::Sequence< sal_Int8 > getImplementationId(	\
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _rTypes);	\
private:															\
    static void implCreateMap();									\
DECLARE_IMPLEMENTATIONID_HELPER_VSMALLEND(_namespace, classname)


/** implement an id helper
*/
#define IMPLEMENT_IMPLEMENTATIONID_HELPER_VSMALL(_namespace, classname)		\
namespace _namespace {	\
    \
/*=======================================================================*/	\
    \
sal_Int32		classname::s_nReferenced(0);	\
void*			classname::s_pMap = NULL;	\
::osl::Mutex	classname::s_aMutex;	\
    \
/*-----------------------------------------------------------------------*/	\
void classname::acquire()	\
{	\
    ::osl::MutexGuard aGuard(s_aMutex);	\
    ++s_nReferenced;	\
}	\
    \
/*-----------------------------------------------------------------------*/	\
void classname::release()	\
{	\
    ::osl::MutexGuard aGuard(s_aMutex);	\
    if (!--s_nReferenced)	\
    {	\
        delete static_cast< ::utl::MapType2Id *>( s_pMap );	\
        s_pMap = NULL;	\
    }	\
}	\
}	/* _namespace */

#define IMPLEMENT_IMPLEMENTATIONID_HELPER_SMALL(_namespace, classname)		\
IMPLEMENT_IMPLEMENTATIONID_HELPER_VSMALL(_namespace, classname)		\
namespace _namespace {	\
/*-----------------------------------------------------------------------*/	\
void classname::implCreateMap()	\
{	\
    if (s_pMap)	\
        return;	\
    s_pMap = new ::utl::MapType2Id();	\
}	\
    \
    \
::com::sun::star::uno::Sequence< sal_Int8 > classname::getImplementationId(	\
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _rTypes)	\
{	\
    ::osl::MutexGuard aGuard(s_aMutex);	\
    DBG_ASSERT(s_nReferenced,	\
        "classname::getImplementationId : you need to hold a reference to this class in order to use it !");	\
        /* give the calling class a member of type classname##Ref and all is fine .... */	\
    \
    implCreateMap();	\
    \
    ::utl::MapType2Id* pMap = static_cast< ::utl::MapType2Id *>(s_pMap);	\
    \
    ::cppu::OImplementationId& rId = (*pMap)[_rTypes];	\
    /* this will create an entry for the given type sequence, if neccessary */	\
    \
    return rId.getImplementationId();	\
}	\
}	

#endif // _BF_SVTOOLS_ID_HELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
