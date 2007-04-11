/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sotref.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:53:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SOT_SOTREF_HXX
#define _SOT_SOTREF_HXX

#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif

//========================================================================
enum SvCastEnum { SV_AGGREGATION_CAST };
#ifndef SVT_DECL_SOTOBJECT_DEFINED
#define SVT_DECL_SOTOBJECT_DEFINED
class SotObject;
class SotObjectRef
{
    PRV_SV_DECL_REF(SotObject)
    inline SotObjectRef( SotObject * pObjP, SvCastEnum );
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
    inline          ClassName##Ref( SotObject * pObjP, SvCastEnum );      \
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
}                                                                         \
inline ClassName##Ref::ClassName##Ref( SotObject * pObjP, SvCastEnum )    \
{                                                                         \
    pObj = (ClassName *)ClassName::ClassFactory()->AggCastAndAddRef( pObjP );\
}

#define SO2_DECL_IMPL_REF(ClassName)                                      \
    SO2_DECL_REF(ClassName)                                               \
    SO2_IMPL_REF(ClassName)

#endif // _SO2REF_HXX
