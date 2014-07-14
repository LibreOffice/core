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


#ifndef _DPAGE_HXX
#define _DPAGE_HXX

#ifndef _FM_FMPAGE_HXX
#include <svx/fmpage.hxx>
#endif
#include <svx/svdobj.hxx>

class SdrPageGridFrameList;
class SwDrawModel;
class SwDoc;

class SwDPage : public FmFormPage
{
private:
    SdrPageGridFrameList*   pGridLst;
    SwDoc&                  rDoc;

protected:
    /// method to copy all data from given source
    virtual void copyDataFromSdrPage(const SdrPage& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrPage* CloneSdrPage(SdrModel* pTargetModel = 0) const;

    SwDPage(SwDrawModel& rNewModel, bool bMasterPage = false);
    ~SwDPage();

    virtual SdrObject* ReplaceObjectInSdrObjList( SdrObject& rNewObj, sal_uInt32 nObjNum );

    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrView& rSdrView,
                                    const Rectangle *pRect) const;

    sal_Bool RequestHelp( Window* pWindow, SdrView* pView, const HelpEvent& rEvt );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();
};

#endif     // _DPAGE_HXX



