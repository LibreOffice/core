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


#ifndef RPT_SHARED_GROUPS_PROPERTIES_HXX
#define RPT_SHARED_GROUPS_PROPERTIES_HXX

namespace rptshared
{
    /** struct for the group properties
    */
    struct GroupProperties
    {
        ::sal_Int32                                                                 m_nGroupInterval;
        ::rtl::OUString                                                             m_sExpression;
        ::sal_Int16                                                                 m_nGroupOn;
        ::sal_Int16                                                                 m_nKeepTogether;
        ::sal_Bool                                                                  m_eSortAscending;
        ::sal_Bool                                                                  m_bStartNewColumn;
        ::sal_Bool                                                                  m_bResetPageNumber;

        GroupProperties()
        :m_nGroupInterval(1)
        ,m_nGroupOn(0)
        ,m_nKeepTogether(0)
        ,m_eSortAscending(sal_True)
        ,m_bStartNewColumn(sal_False)
        ,m_bResetPageNumber(sal_False)
        {}
    };
}
#endif //RPT_SHARED_GROUPS_PROPERTIES_HXX
