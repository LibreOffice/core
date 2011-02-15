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

#ifndef _SOT_SOTREF_HXX
#define _SOT_SOTREF_HXX

#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif

//========================================================================
#ifndef SVT_DECL_SOTOBJECT_DEFINED
#define SVT_DECL_SOTOBJECT_DEFINED
class SotObject;
class SotObjectRef
{
    PRV_SV_DECL_REF(SotObject)
};
#endif

//========================================================================
#define SO2_DECL_REF(ClassName)                                           \
class ClassName;                                                          \
class ClassName##Ref                                                      \
{                                                                         \
    PRV_SV_DECL_REF(ClassName)                                            \
    inline          ClassName##Ref( const SotObjectRef & );               \
    inline          ClassName##Ref( SotObject * pObjP );                  \
};

#define SO2_IMPL_REF(ClassName)                                           \
SV_IMPL_REF(ClassName)                                                    \
inline ClassName##Ref::ClassName##Ref( const SotObjectRef & r )           \
{                                                                         \
    pObj = (ClassName *)ClassName::ClassFactory()->CastAndAddRef( &r );   \
}                                                                         \
inline ClassName##Ref::ClassName##Ref( SotObject * pObjP )                \
{                                                                         \
    pObj = (ClassName *)ClassName::ClassFactory()->CastAndAddRef( pObjP );\
}


#define SO2_DECL_IMPL_REF(ClassName)                                      \
    SO2_DECL_REF(ClassName)                                               \
    SO2_IMPL_REF(ClassName)

#endif // _SO2REF_HXX
