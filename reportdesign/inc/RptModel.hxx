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

#ifndef INCLUDED_REPORTDESIGN_INC_RPTMODEL_HXX
#define INCLUDED_REPORTDESIGN_INC_RPTMODEL_HXX

#include "dllapi.h"
#include <svx/svdmodel.hxx>
#include <com/sun/star/report/XReportDefinition.hpp>

namespace vcl { class Window; }
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
class OReportPage;
class OXUndoEnvironment;

class REPORTDESIGN_DLLPUBLIC OReportModel : public SdrModel
{
    friend class OReportPage;

private:
    OXUndoEnvironment*                  m_pUndoEnv;
    ::dbaui::DBSubComponentController*  m_pController;
    ::reportdesign::OReportDefinition*  m_pReportDefinition;

    virtual css::uno::Reference< css::uno::XInterface > createUnoModel() override;

    OReportModel( const OReportModel& ) = delete;
    void operator=(const OReportModel& rSrcModel) = delete;

public:

    OReportModel(::reportdesign::OReportDefinition* _pReportDefinition);
    virtual ~OReportModel();

    virtual void        SetChanged(bool bFlg = true) override;
    virtual SdrPage*    AllocPage(bool bMasterPage) override;
    virtual SdrPage*    RemovePage(sal_uInt16 nPgNum) override;
    /** @returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType  GetPageNumType() const override;

    OXUndoEnvironment&  GetUndoEnv() { return *m_pUndoEnv;}
    void                SetModified(bool _bModified);

    inline dbaui::DBSubComponentController* getController() const { return m_pController; }
    inline void attachController( dbaui::DBSubComponentController& _rController ) { m_pController = &_rController; }
    void detachController();

    OReportPage* createNewPage(const css::uno::Reference< css::report::XSection >& _xSection);

    /** returns the page which belongs to a section
    *
    * @param _xSection
    * @return The page or <NULL/> when no page could be found.
    */
    OReportPage* getPage(const css::uno::Reference< css::report::XSection >& _xSection);

    /// returns the XReportDefinition which the OReportModel belongs to
    css::uno::Reference< css::report::XReportDefinition >
                getReportDefinition() const;

    css::uno::Reference< css::uno::XInterface > createShape(const OUString& aServiceSpecifier,css::uno::Reference< css::drawing::XShape >& _rShape,sal_Int32 nOrientation = -1);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
