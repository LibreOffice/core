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
#ifndef INCLUDED_REPORTDESIGN_INC_RPTOBJECT_HXX
#define INCLUDED_REPORTDESIGN_INC_RPTOBJECT_HXX

#include "dllapi.h"

#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>


#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <svx/svdoashp.hxx>

#include <map>

namespace rptui
{
typedef ::std::multimap< sal_Int16, OUString > IndexToNameMap;
    enum DlgEdHintKind
    {
        RPTUI_HINT_WINDOWSCROLLED,
        RPTUI_HINT_SELECTIONCHANGED
    };

    class OUnoObject;
    class REPORTDESIGN_DLLPUBLIC DlgEdHint final : public SfxHint
    {
    private:
        DlgEdHintKind   eHintKind;

        DlgEdHint(DlgEdHint const &) = delete;
        void operator =(DlgEdHint const &) = delete;
    public:
        DlgEdHint( DlgEdHintKind eHint );
        virtual ~DlgEdHint() override;

        DlgEdHintKind    GetKind() const { return eHintKind; }
    };


class OReportPage;
class OPropertyMediator;

class REPORTDESIGN_DLLPUBLIC OObjectBase
{
protected:
    mutable rtl::Reference<OPropertyMediator>                         m_xMediator;
    mutable css::uno::Reference< css::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    mutable css::uno::Reference< css::report::XReportComponent>       m_xReportComponent;
    css::uno::Reference< css::uno::XInterface >                       m_xKeepShapeAlive;
    OUString m_sComponentName;
    bool        m_bIsListening;

    OObjectBase(const css::uno::Reference< css::report::XReportComponent>& _xComponent);
    OObjectBase(const OUString& _sComponentName);

    virtual ~OObjectBase();

    bool isListening() const { return m_bIsListening; }

    void SetPropsFromRect(const tools::Rectangle& _rRect);

    virtual SdrPage* GetImplPage() const = 0;

    /** called by instances of derived classes to implement their overriding of getUnoShape
    */
    css::uno::Reference< css::drawing::XShape >
            getUnoShapeOf( SdrObject& _rSdrObject );

private:
    static void    ensureSdrObjectOwnership(
                    const css::uno::Reference< css::uno::XInterface >& _rxShape );

public:
    OObjectBase(const OObjectBase&) = delete;
    OObjectBase& operator=(const OObjectBase&) = delete;
    void StartListening();
    void EndListening();
    // PropertyChangeListener
    /// @throws css::uno::RuntimeException
    virtual void _propertyChange( const  css::beans::PropertyChangeEvent& evt );
    virtual void initializeOle() {}

    bool        supportsService( const OUString& _sServiceName ) const;

    const css::uno::Reference< css::report::XReportComponent>& getReportComponent() const { return m_xReportComponent;}
    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent();
    css::uno::Reference< css::report::XSection> getSection() const;
    const OUString& getServiceName() const { return m_sComponentName; }

    /** releases the reference to our UNO shape (m_xKeepShapeAlive)
    */
    void    releaseUnoShape() { m_xKeepShapeAlive.clear(); }

    static SdrObject* createObject(
        SdrModel& rTargetModel,
        const css::uno::Reference< css::report::XReportComponent>& _xComponent);
    static SdrObjKind getObjectType(const css::uno::Reference< css::report::XReportComponent>& _xComponent);
};

// OCustomShape

class REPORTDESIGN_DLLPUBLIC OCustomShape final : public SdrObjCustomShape , public OObjectBase
{
    friend class OReportPage;
    friend class DlgEdFactory;

private:
    // protected destructor - due to final, make private
    virtual ~OCustomShape() override;

public:
    static OCustomShape* Create(
        SdrModel& rSdrModel,
        const css::uno::Reference< css::report::XReportComponent>& _xComponent)
    {
        return new OCustomShape(rSdrModel, _xComponent );
    }

    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent() override;

    virtual css::uno::Reference< css::drawing::XShape > getUnoShape() override;
    virtual SdrObjKind GetObjIdentifier() const override;
    virtual SdrInventor GetObjInventor() const override;

private:
    virtual void setUnoShape( const css::uno::Reference< css::drawing::XShape >& rxUnoShape ) override;
    virtual void setUnoShape( SvxShape& rNewShape ) override;

    OCustomShape(
        SdrModel& rSdrModel,
        const css::uno::Reference< css::report::XReportComponent>& _xComponent);
    OCustomShape(
        SdrModel& rSdrModel,
        const OUString& _sComponentName);

    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    virtual SdrPage* GetImplPage() const override;
};


// OOle2Obj

class REPORTDESIGN_DLLPUBLIC OOle2Obj final : public SdrOle2Obj , public OObjectBase
{
    friend class OReportPage;
    friend class DlgEdFactory;

private:
    // protected destructor - due to final, make private
    virtual ~OOle2Obj() override;

public:
    static OOle2Obj* Create(
        SdrModel& rSdrModel,
        const css::uno::Reference< css::report::XReportComponent>& _xComponent,
        SdrObjKind _nType)
    {
        return new OOle2Obj(rSdrModel, _xComponent, _nType);
    }

    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent() override;

    virtual css::uno::Reference< css::drawing::XShape > getUnoShape() override;
    virtual SdrObjKind GetObjIdentifier() const override;
    virtual SdrInventor GetObjInventor() const override;
    // Clone() should make a complete copy of the object.
    virtual OOle2Obj* CloneSdrObject(SdrModel& rTargetModel) const override;
    virtual void initializeOle() override;

    void initializeChart( const css::uno::Reference< css::frame::XModel>& _xModel);

private:
    OOle2Obj(
        SdrModel& rSdrModel,
        const css::uno::Reference< css::report::XReportComponent>& _xComponent,
        SdrObjKind _nType);
    OOle2Obj(
        SdrModel& rSdrModel,
        const OUString& _sComponentName,
        SdrObjKind _nType);
    // copy constructor
    OOle2Obj(SdrModel& rSdrModel, const OOle2Obj& rSource);

    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    virtual SdrPage* GetImplPage() const override;

    void impl_createDataProvider_nothrow( const css::uno::Reference< css::frame::XModel>& _xModel);
    virtual void setUnoShape( const css::uno::Reference< css::drawing::XShape >& rxUnoShape ) override;
    virtual void setUnoShape( SvxShape& rNewShape ) override;

    SdrObjKind m_nType;
    bool    m_bOnlyOnce;
};


// OUnoObject

class REPORTDESIGN_DLLPUBLIC OUnoObject final : public SdrUnoObj , public OObjectBase
{
    friend class OReportPage;
    friend class OObjectBase;
    friend class DlgEdFactory;

    SdrObjKind m_nObjectType;
    // tdf#118730 remember if this object was created interactively (due to ::EndCreate being called)
    bool         m_bSetDefaultLabel;

    OUnoObject(SdrModel& rSdrModel,
        const OUString& _sComponentName,
        const OUString& rModelName,
        SdrObjKind _nObjectType);
    OUnoObject(
        SdrModel& rSdrModel,
        const css::uno::Reference< css::report::XReportComponent>& _xComponent,
        const OUString& rModelName,
        SdrObjKind _nObjectType);
    // copy constructor
    OUnoObject(SdrModel& rSdrModel, OUnoObject const & rSource);

    // protected destructor
    virtual ~OUnoObject() override;

    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    virtual SdrPage* GetImplPage() const override;

public:

    virtual void _propertyChange( const  css::beans::PropertyChangeEvent& evt ) override;

    /** creates the m_xMediator when it doesn't already exist.
        @param  _bReverse   when set to <TRUE/> then the properties from the uno control will be copied into report control
    */
    void CreateMediator(bool _bReverse = false);

    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent() override;

    static OUString GetDefaultName(const OUnoObject* _pObj);

    virtual css::uno::Reference< css::drawing::XShape > getUnoShape() override;
    virtual SdrObjKind GetObjIdentifier() const override;
    virtual SdrInventor GetObjInventor() const override;
    virtual OUnoObject* CloneSdrObject(SdrModel& rTargetModel) const override;

private:
    virtual void setUnoShape( const css::uno::Reference< css::drawing::XShape >& rxUnoShape ) override;
    virtual void setUnoShape( SvxShape& rNewShape ) override;
    void    impl_initializeModel_nothrow();
};


} // rptui

#endif // INCLUDED_REPORTDESIGN_INC_RPTOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
