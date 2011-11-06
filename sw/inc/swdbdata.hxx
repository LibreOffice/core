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


#ifndef _SWDBDATA_HXX
#define _SWDBDATA_HXX

#include <tools/solar.h>
#include <rtl/ustring.hxx>

// contains the description of a data source
struct SwDBData
{
    ::rtl::OUString     sDataSource;
    ::rtl::OUString     sCommand;       //table, query or statement
    sal_Int32           nCommandType; //com::sun::star::sdb::CommandType
    SwDBData() :
        nCommandType(0){}

    sal_Bool operator !=(const SwDBData& rCmp) const
        {return rCmp.sDataSource != sDataSource || rCmp.sCommand != sCommand || rCmp.nCommandType != nCommandType;}
    sal_Bool operator ==(const SwDBData& rCmp) const
        {return rCmp.sDataSource == sDataSource && rCmp.sCommand == sCommand && rCmp.nCommandType == nCommandType;}
};

#endif

