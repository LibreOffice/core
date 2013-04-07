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
#ifndef _SVX_FMOBJ_HXX
#define _SVX_FMOBJ_HXX

#include <svx/svdouno.hxx>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

//==================================================================
// FmFormObj
//==================================================================
class FmFormObj: public SdrUnoObj
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >  aEvts;  // events des Objects
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>   m_aEventsHistory;
                // valid if and only if m_pEnvironmentHistory != NULL, this are the events which we're set when
                // m_pEnvironmentHistory was created

    // Informationen fuer die Controlumgebung
    // werden nur vorgehalten, wenn ein Object sich nicht in einer Objectliste befindet
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>     m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >    m_xEnvironmentHistory;
    sal_Int32           m_nPos;

    OutputDevice*       m_pLastKnownRefDevice;
                            // the last ref device we know, as set at the model
                            // only to be used for comparison with the current ref device!

public:
    SVX_DLLPUBLIC FmFormObj(const OUString& rModelName);
    SVX_DLLPUBLIC FmFormObj();

    TYPEINFO();

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>&
        GetOriginalParent() const { return m_xParent; }
    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >&
        GetOriginalEvents() const { return aEvts; }
    sal_Int32
        GetOriginalIndex() const { return m_nPos; }

    void SetObjEnv(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xForm,
            const sal_Int32 nIdx,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& rEvts );
    void ClearObjEnv();

public:
    virtual ~FmFormObj();
    virtual void SetPage(SdrPage* pNewPage);

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void NbcReformatText();

    virtual FmFormObj* Clone() const;
    // #116235# virtual SdrObject*  Clone(SdrPage* pPage, SdrModel* pModel) const;
    FmFormObj& operator= (const FmFormObj& rObj);

    virtual void SetModel(SdrModel* pNewModel);

    virtual void clonedFrom(const FmFormObj* _pSource);

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> ensureModelEnv(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rSourceContainer, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> _rTopLevelDestContainer);

    /** returns the FmFormObj behind the given SdrObject

        In case the SdrObject *is* an FmFormObject, this is a simple cast. In case the SdrObject
        is a virtual object whose referenced object is an FmFormObj, then this referenced
        object is returned. In all other cases, NULL is returned.
    */
    static       FmFormObj* GetFormObject( SdrObject* _pSdrObject );
    static const FmFormObj* GetFormObject( const SdrObject* _pSdrObject );

    virtual void SetUnoControlModel( const ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& _rxModel );

protected:
    virtual bool        EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd );
    virtual void        BrkCreate( SdrDragStat& rStat );

    // #i70852# overload Layer interface to force to FormColtrol layer
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);

private:
    /** isolates the control model from its form component hierarchy, i.e. removes it from
        its parent.
    */
    void    impl_isolateControlModel_nothrow();

    /** forwards the reference device of our SdrModel to the control model
    */
    void    impl_checkRefDevice_nothrow( bool _force = false );
};


#endif // _FM_FMOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
