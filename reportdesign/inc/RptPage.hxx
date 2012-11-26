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


#ifndef _REPORT_RPTUIPAGE_HXX
#define _REPORT_RPTUIPAGE_HXX

#include "dllapi.h"
#include "svx/svdpage.hxx"
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XSection.hpp>

namespace rptui
{
//============================================================================
// OReportPage
//============================================================================

class OReportModel;
//class OReportSection;

class REPORTDESIGN_DLLPUBLIC OReportPage : public SdrPage
{
private:
    OReportModel&           rModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > m_xSection;
    bool                    m_bSpecialInsertMode;
    SdrObjectVector m_aTemporaryObjectList;

//    OReportPage(const OReportPage&);

    // methode to remove temporary objects, created by 'special mode'
    // (BegDragObj)
    void removeTempObject(SdrObject *_pToRemoveObj);

    virtual ~OReportPage();

protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

    /// method to copy all data from given source
    virtual void copyDataFromSdrPage(const SdrPage& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrPage* CloneSdrPage(SdrModel* pTargetModel = 0) const;

    OReportPage( OReportModel& rModel
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                ,bool bMasterPage = false );

    virtual void InsertObjectToSdrObjList(SdrObject& rObj, sal_uInt32 nPos = CONTAINER_APPEND);
    virtual SdrObject* RemoveObjectFromSdrObjList(sal_uInt32 nObjNum);

    /** returns the index inside the object list which belongs to the report component.
        @param  _xObject    the report component
    */
    sal_uLong getIndexOf(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

    /** removes the SdrObject which belongs to the report component.
        @param  _xObject    the report component
    */
    void removeSdrObject(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

    void setSpecialMode() {m_bSpecialInsertMode = true;}
    bool getSpecialMode() {return m_bSpecialInsertMode;}
    // all temporary objects will remove and destroy
    void resetSpecialMode();

    /** insert a new SdrObject which belongs to the report component.
        @param  _xObject    the report component
    */
    void insertObject(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getSection() const;
};
}
#endif //_REPORT_RPTUIPAGE_HXX
