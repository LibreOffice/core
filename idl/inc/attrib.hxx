/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

