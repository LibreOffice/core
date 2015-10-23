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

#include <boost/noncopyable.hpp>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>


#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <svx/svdocirc.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoashp.hxx>

namespace rptui
{
typedef ::std::multimap< sal_Int16, OUString, ::std::less< sal_Int16 > > IndexToNameMap;
    enum DlgEdHintKind
    {
        RPTUI_HINT_UNKNOWN,
        RPTUI_HINT_WINDOWSCROLLED,
        RPTUI_HINT_LAYERCHANGED,
        RPTUI_HINT_OBJORDERCHANGED,
        RPTUI_HINT_SELECTIONCHANGED
    };

    class OUnoObject;
    class REPORTDESIGN_DLLPUBLIC DlgEdHint: public SfxHint
    {
    private:
        DlgEdHintKind   eHintKind;

        DlgEdHint(DlgEdHint&) = delete;
        void operator =(DlgEdHint&) = delete;
    public:
        DlgEdHint( DlgEdHintKind eHint );
        virtual ~DlgEdHint();

        inline DlgEdHintKind    GetKind() const { return eHintKind; }
    };


class OReportPage;
class OPropertyMediator;

class REPORTDESIGN_DLLPUBLIC OObjectBase: private boost::noncopyable
{
public:
    typedef rtl::Reference<OPropertyMediator> TMediator;

protected:
    mutable TMediator                                                 m_xMediator;
    mutable css::uno::Reference< css::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    mutable css::uno::Reference< css::report::XReportComponent>       m_xReportComponent;
    css::uno::Reference< css::container::XContainerListener>          m_xContainerListener;
    css::uno::Reference< css::report::XSection>                       m_xSection;
    css::uno::Reference< css::uno::XInterface >                       m_xKeepShapeAlive;
    OUString m_sComponentName;
    bool        m_bIsListening;

    OObjectBase(const css::uno::Reference< css::report::XReportComponent>& _xComponent);
    OObjectBase(const OUString& _sComponentName);

    virtual ~OObjectBase();

    inline bool isListening() const { return m_bIsListening; }

    void SetPropsFromRect(const Rectangle& _rRect);

    virtual SdrPage* GetImplPage() const = 0;

    /** called by instances of derived classes to implement their overriding of getUnoShape
    */
    css::uno::Reference< css::uno::XInterface >
            getUnoShapeOf( SdrObject& _rSdrObject );

private:
    static void    ensureSdrObjectOwnership(
                    const css::uno::Reference< css::uno::XInterface >& _rxShape );

public:
    void StartListening();
    void EndListening(bool bRemoveListener = true);
    // PropertyChangeListener
    virtual void _propertyChange( const  css::beans::PropertyChangeEvent& evt ) throw(css::uno::RuntimeException);
    virtual void initializeOle() {}

    bool        supportsService( const OUString& _sServiceName ) const;

    css::uno::Reference< css::report::XReportComponent> getReportComponent() const { return m_xReportComponent;}
    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent();
    css::uno::Reference< css::report::XSection> getSection() const;
    inline const OUString getServiceName() const { return m_sComponentName; }

    /** releases the reference to our UNO shape (m_xKeepShapeAlive)
    */
    void    releaseUnoShape() { m_xKeepShapeAlive.clear(); }

    static SdrObject* createObject(const css::uno::Reference< css::report::XReportComponent>& _xComponent);
    static sal_uInt16 getObjectType(const css::uno::Reference< css::report::XReportComponent>& _xComponent);
};

// OCustomShape

class REPORTDESIGN_DLLPUBLIC OCustomShape: public SdrObjCustomShape , public OObjectBase
{
    friend class OReportPage;
    friend class DlgEdFactory;

public:
    static OCustomShape* Create( const css::uno::Reference< css::report::XReportComponent>& _xComponent )
    {
        return new OCustomShape( _xComponent );
    }

protected:
    OCustomShape(const css::uno::Reference< css::report::XReportComponent>& _xComponent);
    OCustomShape(const OUString& _sComponentName);

    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    virtual SdrPage* GetImplPage() const override;

public:
    TYPEINFO_OVERRIDE();

    virtual ~OCustomShape();

    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent() override;

    virtual css::uno::Reference< css::uno::XInterface > getUnoShape() override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual sal_uInt32 GetObjInventor() const override;

private:
    virtual void impl_setUnoShape( const css::uno::Reference< css::uno::XInterface >& rxUnoShape ) override;
};


// OOle2Obj

class REPORTDESIGN_DLLPUBLIC OOle2Obj: public SdrOle2Obj , public OObjectBase
{
    friend class OReportPage;
    friend class DlgEdFactory;

    sal_uInt16 m_nType;
    bool    m_bOnlyOnce;
    void impl_createDataProvider_nothrow( const css::uno::Reference< css::frame::XModel>& _xModel);
    virtual void impl_setUnoShape( const css::uno::Reference< css::uno::XInterface >& rxUnoShape ) override;

public:
    static OOle2Obj* Create( const css::uno::Reference< css::report::XReportComponent>& _xComponent,sal_uInt16 _nType )
    {
        return new OOle2Obj( _xComponent,_nType );
    }
protected:
    OOle2Obj(const css::uno::Reference< css::report::XReportComponent>& _xComponent,sal_uInt16 _nType);
    OOle2Obj(const OUString& _sComponentName,sal_uInt16 _nType);


    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    virtual SdrPage* GetImplPage() const override;

public:
    TYPEINFO_OVERRIDE();

    virtual ~OOle2Obj();

    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent() override;

    virtual css::uno::Reference< css::uno::XInterface > getUnoShape() override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual sal_uInt32 GetObjInventor() const override;
    // Clone() should make a complete copy of the object.
    virtual OOle2Obj* Clone() const override;
    virtual void initializeOle() override;

    OOle2Obj& operator=(const OOle2Obj& rObj);

    void initializeChart( const css::uno::Reference< css::frame::XModel>& _xModel);
};


// OUnoObject

class REPORTDESIGN_DLLPUBLIC OUnoObject: public SdrUnoObj , public OObjectBase
{
    friend class OReportPage;
    friend class OObjectBase;
    friend class DlgEdFactory;

    sal_uInt16   m_nObjectType;
protected:
    OUnoObject(const OUString& _sComponentName
                ,const OUString& rModelName
                ,sal_uInt16   _nObjectType);
    OUnoObject(  const css::uno::Reference< css::report::XReportComponent>& _xComponent
                ,const OUString& rModelName
                ,sal_uInt16   _nObjectType);

    virtual ~OUnoObject();

    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    virtual SdrPage* GetImplPage() const override;

public:
    TYPEINFO_OVERRIDE();

    virtual void _propertyChange( const  css::beans::PropertyChangeEvent& evt ) throw(css::uno::RuntimeException) override;

    /** creates the m_xMediator when it doesn't already exist.
        @param  _bReverse   when set to <TRUE/> then the properties from the uno control will be copied into report control
    */
    void CreateMediator(bool _bReverse = false);

    virtual css::uno::Reference< css::beans::XPropertySet> getAwtComponent() override;

    static OUString GetDefaultName(const OUnoObject* _pObj);

    virtual css::uno::Reference< css::uno::XInterface > getUnoShape() override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual sal_uInt32 GetObjInventor() const override;
    virtual OUnoObject* Clone() const override;

    OUnoObject& operator=(const OUnoObject& rObj);

private:
    virtual void impl_setUnoShape( const css::uno::Reference< css::uno::XInterface >& rxUnoShape ) override;
    void    impl_setReportComponent_nothrow();
    void    impl_initializeModel_nothrow();
};


} // rptui

#endif // INCLUDED_REPORTDESIGN_INC_RPTOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
