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

#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <memory>
#include <svx/svdpage.hxx>
#include <svx/svxdllapi.h>
#include <comphelper/uno3.hxx>

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
                                    ,public css::form::XFormsSupplier2
                                    ,public css::container::XNamed
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
    // XNamed
    virtual OUString SAL_CALL getName() override { return m_sPageName; }
    virtual void SAL_CALL setName( const OUString& rName ) override { m_sPageName = rName; }

    static bool         RequestHelp(
                            vcl::Window* pWin,
                            SdrView const * pView,
                            const HelpEvent& rEvt );

protected:
    // lateInit -> copyValuesToClonedInstance (?)
    void lateInit(const FmFormPage& rPage);

protected:

    // Creating a SdrObject based on a Description. Can be used by derived classes to
    // support own css::drawing::Shapes (for example Controls)
    virtual SdrObject *CreateSdrObject_( const css::uno::Reference< css::drawing::XShape > & xShape ) override;

    // The following method is called when a SvxShape object should be created.
    // Derived classes can create a derivation or an object aggregating SvxShape.
    virtual css::uno::Reference< css::drawing::XShape >  CreateShape( SdrObject *pObj ) const override;

public:

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(FmFormPage, SdrPage)

    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // XFormsSupplier
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getForms() override;

    // XFormsSupplier2
    virtual sal_Bool SAL_CALL hasForms() override;

};

#endif // INCLUDED_SVX_FMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
