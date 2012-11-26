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



#ifndef _SVX_FMPAGE_HXX
#define _SVX_FMPAGE_HXX

#include <svx/svdpage.hxx>
#include <comphelper/uno3.hxx>
#include "svx/svxdllapi.h"

class StarBASIC;
class FmFormModel;
class FmFormPageImpl;   // haelt die Liste aller Forms

FORWARD_DECLARE_INTERFACE(container,XNameContainer)

class SdrView;
class SfxJSArray;
class HelpEvent;

class SVX_DLLPUBLIC FmFormPage : public SdrPage
{
private:
    friend class FmFormObj;

    FmFormPageImpl*     m_pImpl;
    String              m_sPageName;
    StarBASIC*          m_pBasic;

protected:
    /// method to copy all data from given source
    virtual void copyDataFromSdrPage(const SdrPage& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrPage* CloneSdrPage(SdrModel* pTargetModel = 0) const;

    FmFormPage(FmFormModel& rModel, StarBASIC*, bool bMasterPage = false);
    ~FmFormPage();

    virtual void InsertObjectToSdrObjList(SdrObject& rObj, sal_uInt32 nPos = CONTAINER_APPEND);
    virtual SdrObject* RemoveObjectFromSdrObjList(sal_uInt32 nObjNum);

    // Zugriff auf alle Formulare
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& GetForms( bool _bForceCreate = true ) const;

#ifndef SVX_LIGHT
    FmFormPageImpl& GetImpl() const { return *m_pImpl; }
#endif // SVX_LIGHT

    const String&       GetName() const { return m_sPageName; }
    void                SetName( const String& rName ) { m_sPageName = rName; }
    StarBASIC*          GetBasic() const { return m_pBasic; }
    sal_Bool            RequestHelp(
                            Window* pWin,
                            SdrView* pView,
                            const HelpEvent& rEvt );
};

#endif          // _SVX_FMPAGE_HXX

