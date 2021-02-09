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
#ifndef INCLUDED_SVX_SOURCE_INC_FMOBJ_HXX
#define INCLUDED_SVX_SOURCE_INC_FMOBJ_HXX

#include <config_options.h>
#include <svx/svdouno.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XForms.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>


// FmFormObj

class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) FmFormObj : public SdrUnoObj
{
    FmFormObj( const FmFormObj& ) = delete;

    css::uno::Sequence< css::script::ScriptEventDescriptor >  aEvts;  // events of the object
    css::uno::Sequence< css::script::ScriptEventDescriptor>   m_aEventsHistory;
                // valid if and only if m_pEnvironmentHistory != NULL, this are the events which we're set when
                // m_pEnvironmentHistory was created

    // information for the control environment is only maintained if an object is not in an
    // object list
    css::uno::Reference< css::container::XIndexContainer>     m_xParent;
    css::uno::Reference< css::form::XForms >                  m_xEnvironmentHistory;
    sal_Int32           m_nPos;

    VclPtr<OutputDevice>   m_pLastKnownRefDevice;
                            // the last ref device we know, as set at the model
                            // only to be used for comparison with the current ref device!

protected:
    // protected destructor
    SAL_DLLPRIVATE virtual ~FmFormObj() override;

public:
    FmFormObj(
        SdrModel& rSdrModel,
        const OUString& rModelName);
    FmFormObj(SdrModel& rSdrModel);
    // Copy constructor
    FmFormObj(SdrModel& rSdrModel, FmFormObj const & rSource);

    SAL_DLLPRIVATE const css::uno::Reference< css::container::XIndexContainer>&
        GetOriginalParent() const { return m_xParent; }
    SAL_DLLPRIVATE const css::uno::Sequence< css::script::ScriptEventDescriptor >&
        GetOriginalEvents() const { return aEvts; }
    SAL_DLLPRIVATE sal_Int32
        GetOriginalIndex() const { return m_nPos; }

    SAL_DLLPRIVATE void SetObjEnv(
            const css::uno::Reference< css::container::XIndexContainer>& xForm,
            const sal_Int32 nIdx,
            const css::uno::Sequence< css::script::ScriptEventDescriptor >& rEvts );
    SAL_DLLPRIVATE void ClearObjEnv();

public:
    // react on page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

    SAL_DLLPRIVATE virtual SdrInventor GetObjInventor() const override;
    SAL_DLLPRIVATE virtual SdrObjKind GetObjIdentifier() const override;
    SAL_DLLPRIVATE virtual void NbcReformatText() override;

    SAL_DLLPRIVATE virtual FmFormObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    SAL_DLLPRIVATE static css::uno::Reference< css::uno::XInterface> ensureModelEnv(
                  const css::uno::Reference< css::uno::XInterface>& _rSourceContainer,
                  const css::uno::Reference< css::form::XForms>& _rTopLevelDestContainer);

    /** returns the FmFormObj behind the given SdrObject

        In case the SdrObject *is* a FmFormObject, this is a simple cast. In case the SdrObject
        is a virtual object whose referenced object is a FmFormObj, then this referenced
        object is returned. In all other cases, NULL is returned.
    */
    SAL_DLLPRIVATE static       FmFormObj* GetFormObject( SdrObject* _pSdrObject );
    SAL_DLLPRIVATE static const FmFormObj* GetFormObject( const SdrObject* _pSdrObject );

    SAL_DLLPRIVATE virtual void SetUnoControlModel( const css::uno::Reference< css::awt::XControlModel >& _rxModel ) override;

protected:
    SAL_DLLPRIVATE virtual bool        EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd ) override;
    SAL_DLLPRIVATE virtual void        BrkCreate( SdrDragStat& rStat ) override;

private:
    /** isolates the control model from its form component hierarchy, i.e. removes it from
        its parent.
    */
    SAL_DLLPRIVATE void    impl_isolateControlModel_nothrow();

    /** forwards the reference device of our SdrModel to the control model
    */
    SAL_DLLPRIVATE void    impl_checkRefDevice_nothrow( bool _force = false );
};


#endif // _FM_FMOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
