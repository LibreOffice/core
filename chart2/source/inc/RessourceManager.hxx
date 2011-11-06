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


#ifndef CHART2_RESSOURCEMANAGER_HXX
#define CHART2_RESSOURCEMANAGER_HXX

#include "charttoolsdllapi.hxx"

// defined in <tools/resmgr.hxx>
class ResMgr;

namespace chart
{

class RessourceManager
{
public:
    static ::ResMgr &  getRessourceManager();

private:
    // not implemented -- avoid instantiation
    RessourceManager();

    static ::ResMgr *  m_pRessourceManager;
};

} //  namespace chart

// CHART2_RESSOURCEMANAGER_HXX
#endif
