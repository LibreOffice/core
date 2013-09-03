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

#ifndef _SVX_FMPAGE_HXX
#define _SVX_FMPAGE_HXX

#include <svx/svdpage.hxx>
#include <comphelper/uno3.hxx>
#include "svx/svxdllapi.h"
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
    friend class FmFormObj;
    FmFormPageImpl*     m_pImpl;
    OUString            m_sPageName;
    StarBASIC*          m_pBasic;

public:
    TYPEINFO();

    FmFormPage(FmFormModel& rModel,StarBASIC*, bool bMasterPage=sal_False);
    FmFormPage(const FmFormPage& rPage);
    ~FmFormPage();

    virtual void    SetModel(SdrModel* pNewModel);

    virtual SdrPage* Clone() const;
    using SdrPage::Clone;

    virtual void    InsertObject(SdrObject* pObj, sal_uLong nPos = CONTAINER_APPEND,
                                    const SdrInsertReason* pReason=NULL);

    virtual SdrObject* RemoveObject(sal_uLong nObjNum);

    // access to all forms
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForms>& GetForms( bool _bForceCreate = true ) const;

    FmFormPageImpl& GetImpl() const { return *m_pImpl; }

public:
    const OUString&     GetName() const { return m_sPageName; }
    void                SetName( const OUString& rName ) { m_sPageName = rName; }
    StarBASIC*          GetBasic() const { return m_pBasic; }
    sal_Bool            RequestHelp(
                            Window* pWin,
                            SdrView* pView,
                            const HelpEvent& rEvt );
};

#endif          // _SVX_FMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
