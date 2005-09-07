/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attrib.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:43:51 $
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
    void                PutBackClass( SvMetaObject * pClass )
                        { /* OptimierungaTmpClass = pClass*/; }
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

