/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
