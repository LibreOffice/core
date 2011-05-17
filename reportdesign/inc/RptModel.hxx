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
#ifndef REPORT_RPTMODEL_HXX
#define REPORT_RPTMODEL_HXX

#include "dllapi.h"
#include <svx/svdmodel.hxx>
#include <com/sun/star/report/XReportDefinition.hpp>


class Window;
namespace dbaui
{
    class DBSubComponentController;
}
namespace reportdesign
{
    class OReportDefinition;
}
namespace rptui
{
//============================================================================
// OReportModel
//============================================================================

class OReportPage;
class OXUndoEnvironment;

class REPORTDESIGN_DLLPUBLIC OReportModel : public SdrModel
{
    friend class OReportPage;

private:
    OXUndoEnvironment*                  m_pUndoEnv;
    ::dbaui::DBSubComponentController*  m_pController;
    ::reportdesign::OReportDefinition*  m_pReportDefinition;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

    OReportModel( const OReportModel& );
    void operator=(const OReportModel& rSrcModel);
public:
    TYPEINFO();

    OReportModel(::reportdesign::OReportDefinition* _pReportDefinition);
    virtual ~OReportModel();

    virtual void        SetChanged(sal_Bool bFlg = sal_True);
    virtual SdrPage*    AllocPage(bool bMasterPage);
    virtual Window*     GetCurDocViewWin();
    virtual SdrPage*    RemovePage(sal_uInt16 nPgNum);
    /** returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType  GetPageNumType() const;

    OXUndoEnvironment&  GetUndoEnv();
    void                SetModified(sal_Bool _bModified);

    inline dbaui::DBSubComponentController* getController() const { return m_pController; }
    inline void attachController( dbaui::DBSubComponentController& _rController ) { m_pController = &_rController; }
    void detachController();

    OReportPage* createNewPage(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);

    /** returns the page which belongs to a section
    *
    * \param _xSection
    * \return The page or <NULL/> when t´no page could be found.
    */
    OReportPage* getPage(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);

    /// returns the XReportDefinition which the OReportModel belongs to
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >
                getReportDefinition() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createShape(const ::rtl::OUString& aServiceSpecifier,::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _rShape,sal_Int32 nOrientation = -1);
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
