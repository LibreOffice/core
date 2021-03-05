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

#include <memory>
#include <svx/svdpage.hxx>
#include <svx/svxdllapi.h>

class FmFormModel;
class FmFormPageImpl;   // contains a list of all forms

namespace com::sun::star {
    namespace form {
        class XForms;
    }
}

class SdrView;
class HelpEvent;

class SVXCORE_DLLPUBLIC FmFormPage : public SdrPage
{
    FmFormPage& operator=(const FmFormPage&) = delete;
    FmFormPage(const FmFormPage&) = delete;

    friend class FmFormObj;
    std::unique_ptr<FmFormPageImpl>     m_pImpl;
    OUString            m_sPageName;

public:

    explicit FmFormPage(FmFormModel& rModel, bool bMasterPage=false);
    virtual ~FmFormPage() override;

    virtual rtl::Reference<SdrPage> CloneSdrPage(SdrModel& rTargetModel) const override;

    virtual void    InsertObject(SdrObject* pObj, size_t nPos = SAL_MAX_SIZE) override;

    virtual SdrObject* RemoveObject(size_t nObjNum) override;

    // access to all forms
    const css::uno::Reference< css::form::XForms>& GetForms( bool _bForceCreate = true ) const;

    FmFormPageImpl& GetImpl() const { return *m_pImpl; }

public:
    const OUString&     GetName() const { return m_sPageName; }
    void                SetName( const OUString& rName ) { m_sPageName = rName; }
    static bool         RequestHelp(
                            vcl::Window* pWin,
                            SdrView const * pView,
                            const HelpEvent& rEvt );

protected:
    // lateInit -> copyValuesToClonedInstance (?)
    void lateInit(const FmFormPage& rPage);
};

#endif // INCLUDED_SVX_FMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
