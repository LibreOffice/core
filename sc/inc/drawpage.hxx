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



#ifndef SC_DRAWPAGE_HXX
#define SC_DRAWPAGE_HXX

#ifndef _FM_FMPAGE_HXX
#include <svx/fmpage.hxx>
#endif


class ScDrawLayer;

// -----------------------------------------------------------------------

class ScDrawPage: public FmFormPage
{
private:
protected:
    /// method to copy all data from given source
    virtual void copyDataFromSdrPage(const SdrPage& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrPage* CloneSdrPage(SdrModel* pTargetModel = 0) const;

    ScDrawPage(ScDrawLayer& rNewModel, StarBASIC* pBasic, bool bMasterPage = false);
    ~ScDrawPage();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();
};


#endif


