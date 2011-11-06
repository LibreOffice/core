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


#ifndef CONNECTIVITY_ADO_WRAP_TYPEDEFS_HXX
#define CONNECTIVITY_ADO_WRAP_TYPEDEFS_HXX


namespace connectivity
{
    namespace ado
    {
        class WpADOTable;
        class WpADOKey;
        class WpADOIndex;
        class WpADOColumn;
        class WpADOGroup;
        class WpADOView;
        class WpADOUser;

        typedef WpOLEAppendCollection<ADOTables,    _ADOTable,  WpADOTable>     WpADOTables;
        typedef WpOLEAppendCollection<ADOKeys,      ADOKey,     WpADOKey>       WpADOKeys;
        typedef WpOLEAppendCollection<ADOIndexes,   _ADOIndex,  WpADOIndex>     WpADOIndexes;
        typedef WpOLEAppendCollection<ADOColumns,   _ADOColumn, WpADOColumn>    WpADOColumns;
        typedef WpOLEAppendCollection<ADOGroups,    ADOGroup,   WpADOGroup>     WpADOGroups;
        typedef WpOLEAppendCollection<ADOViews,     ADOView,    WpADOView>      WpADOViews;
        typedef WpOLEAppendCollection<ADOUsers,     _ADOUser,   WpADOUser>      WpADOUsers;
    }
}

#endif //CONNECTIVITY_ADO_WRAP_TYPEDEFS_HXX
