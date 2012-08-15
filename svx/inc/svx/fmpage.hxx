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
    namespace container {
        class XNameContainer;
    }
}}}

class SdrView;
class HelpEvent;

class SVX_DLLPUBLIC FmFormPage : public SdrPage
{
    friend class FmFormObj;
    FmFormPageImpl*     m_pImpl;
    String              m_sPageName;
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
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& GetForms( bool _bForceCreate = true ) const;

    FmFormPageImpl& GetImpl() const { return *m_pImpl; }

public:
    const String&       GetName() const { return m_sPageName; }
    void                SetName( const String& rName ) { m_sPageName = rName; }
    StarBASIC*          GetBasic() const { return m_pBasic; }
    sal_Bool            RequestHelp(
                            Window* pWin,
                            SdrView* pView,
                            const HelpEvent& rEvt );
};

#endif          // _SVX_FMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
