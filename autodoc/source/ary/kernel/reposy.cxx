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

#include <precomp.h>
#include <reposy.hxx>


// NOT FULLY DECLARED SERVICES
#include <cpp_internalgate.hxx>
#include <idl_internalgate.hxx>


namespace ary
{


//*****************     Repository          ************//

DYN Repository &
Repository::Create_()
{
    return *new RepositoryCenter;
}




RepositoryCenter::RepositoryCenter()
    :   sDisplayedName(),
        aLocation(),
        pCppPartition(0),
        pIdlPartition(0)
{
    pCppPartition = & cpp::InternalGate::Create_Partition_(*this);
    pIdlPartition = & idl::InternalGate::Create_Partition_(*this);
}

RepositoryCenter::~RepositoryCenter()
{
}

const ::ary::cpp::Gate &
RepositoryCenter::Gate_Cpp() const
{
    csv_assert(pCppPartition);
    return *pCppPartition;
}

const ::ary::idl::Gate &
RepositoryCenter::Gate_Idl() const
{
    csv_assert(pIdlPartition);
    return *pIdlPartition;
}

const String &
RepositoryCenter::Title() const
{
    return sDisplayedName;
}


::ary::cpp::Gate &
RepositoryCenter::Gate_Cpp()
{
    csv_assert(pCppPartition);
    return *pCppPartition;
}

::ary::idl::Gate &
RepositoryCenter::Gate_Idl()
{
    csv_assert(pIdlPartition);
    return *pIdlPartition;
}

void
RepositoryCenter::Set_Title(const String & i_sName)
{
    sDisplayedName = i_sName;
}




//*********************     Repository Type Info Data       ****************//

// !!! IMPORTANT - NEVER DELETE OR CHANGE - ADDING ALLOWED



/*  ClassType-Ids
    -------------

    cpp                 1000
    idl                 2000
    corba               3000
    java                4000
    information         5000
    logic location      6000
    phys location       7000
    sec. prod.          8000


    cpp
    ---
    Namespace           1000
    Class               1001
    Enum                1002
    Typedef             1003
    Function            1004
    Variable            1005
    EnumValue           1006
    NamespaceAlias      1007

    BuiltInType         1200
    CeType_Final        1201
    CeType_Extern       1202
    UsedType            1203
    PtrType             1211
    RefType             1212
    ConstType           1221
    VolatileType        1222
    ArrayType           1230
    TemplateInstance    1235
    FunctionPtr         1240
    DataMemberPtr       1250
    OperationMemberPtr  1260

    TplParam_Type       1301
    TplParam_Value      1302

    OpSignature         1400

    Define              1601
    Macro               1602

    ProjectGroup        1901
    FileGroup           1902

    TopProject          1921



    idl
    ---

    Module              2000
    Interface           2001
    Function            2002
    Service             2003
    Property            2004
    Enum                2005
    EnumValue           2006
    Typedef             2007
    Struct              2008
    StructElement       2009
    Exception           2010
    ConstantGroup       2011
    Constant            2012
    Singleton           2013
    Attribute           2014
    SglIfcService       2015
    SglIfcSingleton     2016

    BuiltInType         2200
    CeType              2201
    Sequence            2202
    ExplicitType        2203
    ExplicitNameRoom    2204
    TemplateParamType   2205


    java
    ----
    Package             4000
    Interface           4001
    Class               4002

    physical location
    -----------------
    Root                7000
    Directory           7030
    File                7100


    info
    ----
    CodeInformation
        (IDL)          11002
*/


}   // namespace ary
