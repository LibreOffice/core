/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

class REPORTDESIGN_DLLPUBLIC OReportPage : public SdrPage
{
    OReportModel&           rModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > m_xSection;
    bool                    m_bSpecialInsertMode;
    std::vector<SdrObject*> m_aTemporaryObjectList;

    OReportPage(const OReportPage&);

    // methode to remove temporary objects, created by 'special mode'
    // (BegDragObj)
    void removeTempObject(SdrObject *_pToRemoveObj);

    virtual ~OReportPage();

protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();
public:
    TYPEINFO();

    OReportPage( OReportModel& rModel
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                ,bool bMasterPage = false );


    virtual SdrPage* Clone() const;
    using SdrPage::Clone;

    virtual void NbcInsertObject(SdrObject* pObj, sal_uLong nPos, const SdrInsertReason* pReason);
    virtual SdrObject* RemoveObject(sal_uLong nObjNum);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
