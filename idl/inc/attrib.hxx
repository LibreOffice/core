/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: attrib.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _ATTRIB_HXX
#define _ATTRIB_HXX

#ifdef IDL_COMPILER
#include <hash.hxx>
#include <object.hxx>

/******************** class SvAttribute **********************************/
class SvAttribute
{
    SvStringHashEntryRef    aName;
    CreateMetaObjectType    pCreateMethod;
    SvMetaObjectRef         aTmpClass;
public:
                        SvAttribute( SvStringHashEntry * pAttribName,
                                     CreateMetaObjectType pMethod )
                            : aName( pAttribName ),
                              pCreateMethod( pMethod ) {}

    SvMetaObjectRef     CreateClass()
                        {
                            if( aTmpClass.Is() )
                            {
                                SvMetaObjectRef aTmp( aTmpClass );
                                aTmpClass.Clear();
                                return aTmp;
                            }
                            return pCreateMethod();
                        }
};

#define SV_ATTRIBUTE( AttributeName, PostfixClassName )                 \
        SvAttribute( SvHash_##AttributeName(),                          \
                     SvMeta##PostfixClassName::Create )

/******************** class SvAttributeList ******************************/
DECLARE_LIST(SvAttributeListImpl,SvAttribute*)
class SvAttributeList : public SvAttributeListImpl
{
public:
            SvAttributeList() : SvAttributeListImpl() {}

    void    Append( SvAttribute * pObj )
            { Insert( pObj, LIST_APPEND ); }
};


#endif // IDL_COMPILER
#endif // _ATTRIB_HXX

