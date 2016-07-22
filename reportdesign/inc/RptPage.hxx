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
#ifndef INCLUDED_REPORTDESIGN_INC_RPTPAGE_HXX
#define INCLUDED_REPORTDESIGN_INC_RPTPAGE_HXX

#include "dllapi.h"
#include <svx/svdpage.hxx>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XSection.hpp>

namespace rptui
{

// OReportPage


class OReportModel;

class REPORTDESIGN_DLLPUBLIC OReportPage : public SdrPage
{
    OReportPage& operator=(const OReportPage&) = delete;

    OReportModel&           rModel;
    css::uno::Reference< css::report::XSection > m_xSection;
    bool                    m_bSpecialInsertMode;
    std::vector<SdrObject*> m_aTemporaryObjectList;

    OReportPage(const OReportPage&);

    // methode to remove temporary objects, created by 'special mode'
    // (BegDragObj)
    void removeTempObject(SdrObject *_pToRemoveObj);

    virtual ~OReportPage();

protected:
    virtual css::uno::Reference< css::uno::XInterface > createUnoPage() override;
public:

    OReportPage( OReportModel& rModel
                ,const css::uno::Reference< css::report::XSection >& _xSection );


    virtual SdrPage* Clone() const override;
    virtual SdrPage* Clone( SdrModel* pNewModel ) const override;

    virtual void NbcInsertObject(SdrObject* pObj, size_t nPos=SAL_MAX_SIZE, const SdrInsertReason* pReason=nullptr) override;
    virtual SdrObject* RemoveObject(size_t nObjNum) override;

    /** returns the index inside the object list which belongs to the report component.
        @param  _xObject    the report component
    */
    sal_uLong getIndexOf(const css::uno::Reference< css::report::XReportComponent >& _xObject);

    /** removes the SdrObject which belongs to the report component.
        @param  _xObject    the report component
    */
    void removeSdrObject(const css::uno::Reference< css::report::XReportComponent >& _xObject);

    void setSpecialMode() {m_bSpecialInsertMode = true;}
    bool getSpecialMode() {return m_bSpecialInsertMode;}
    // all temporary objects will remove and destroy
    void resetSpecialMode();

    /** insert a new SdrObject which belongs to the report component.
        @param  _xObject    the report component
    */
    void insertObject(const css::uno::Reference< css::report::XReportComponent >& _xObject);

    const css::uno::Reference< css::report::XSection >& getSection() const { return m_xSection;}
};
}
#endif // INCLUDED_REPORTDESIGN_INC_RPTPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
