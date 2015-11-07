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

#ifndef INCLUDED_SVX_FMPAGE_HXX
#define INCLUDED_SVX_FMPAGE_HXX

#include <svx/svdpage.hxx>
#include <comphelper/uno3.hxx>
#include <svx/svxdllapi.h>
#include <tools/contnr.hxx>

class StarBASIC;
class FmFormModel;
class FmFormPageImpl;   // contains a list of all forms

namespace com { namespace sun { namespace star {
    namespace form {
        class XForms;
    }
}}}

class SdrView;
class HelpEvent;

class SVX_DLLPUBLIC FmFormPage : public SdrPage
{
    FmFormPage& operator=(const FmFormPage&) = delete;

    friend class FmFormObj;
    FmFormPageImpl*     m_pImpl;
    OUString            m_sPageName;

public:

    explicit FmFormPage(FmFormModel& rModel, bool bMasterPage=false);
    virtual ~FmFormPage();

    virtual void    SetModel(SdrModel* pNewModel) override;

    virtual SdrPage* Clone() const override;
    virtual SdrPage* Clone(SdrModel* pNewModel) const override;

    virtual void    InsertObject(SdrObject* pObj, size_t nPos = SAL_MAX_SIZE,
                                    const SdrInsertReason* pReason=NULL) override;

    virtual SdrObject* RemoveObject(size_t nObjNum) override;

    // access to all forms
    const css::uno::Reference< css::form::XForms>& GetForms( bool _bForceCreate = true ) const;

    FmFormPageImpl& GetImpl() const { return *m_pImpl; }

public:
    const OUString&     GetName() const { return m_sPageName; }
    void                SetName( const OUString& rName ) { m_sPageName = rName; }
    static bool         RequestHelp(
                            vcl::Window* pWin,
                            SdrView* pView,
                            const HelpEvent& rEvt );

protected:
    FmFormPage(const FmFormPage& rPage);

    void lateInit(const FmFormPage& rPage, FmFormModel* pNewModel = 0);
};

#endif // INCLUDED_SVX_FMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
