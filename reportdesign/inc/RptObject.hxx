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
    mutable TMediator                                                                           m_xMediator;
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    //mutable ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>       m_xReportComponent;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>          m_xContainerListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>                       m_xSection;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >                       m_xKeepShapeAlive;
    OUString m_sComponentName;
    bool        m_bIsListening;

    OObjectBase(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent);
    OObjectBase(const OUString& _sComponentName);

    virtual ~OObjectBase();

    inline bool isListening() const { return m_bIsListening; }

    void SetPropsFromRect(const Rectangle& _rRect);

    virtual SdrPage* GetImplPage() const = 0;

    /** called by instances of derived classes to implement their overriding of getUnoShape
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            getUnoShapeOf( SdrObject& _rSdrObject );

private:
    static void    ensureSdrObjectOwnership(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxShape );

public:
    void StartListening();
    void EndListening(bool bRemoveListener = true);
    // PropertyChangeListener
    virtual void _propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);
    virtual void initializeOle() {}

    bool        supportsService( const OUString& _sServiceName ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> getReportComponent() const { return m_xReportComponent;}
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getAwtComponent();
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> getSection() const;
    inline const OUString getServiceName() const { return m_sComponentName; }

    /** releases the reference to our UNO shape (m_xKeepShapeAlive)
    */
    void    releaseUnoShape() { m_xKeepShapeAlive.clear(); }

    static SdrObject* createObject(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent);
    static sal_uInt16 getObjectType(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent);
};

// OCustomShape

class REPORTDESIGN_DLLPUBLIC OCustomShape: public SdrObjCustomShape , public OObjectBase
{
    friend class OReportPage;
    friend class DlgEdFactory;

public:
    static OCustomShape* Create( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent )
    {
        return new OCustomShape( _xComponent );
    }

protected:
    OCustomShape(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent);
    OCustomShape(const OUString& _sComponentName);

    virtual void NbcMove( const Size& rSize ) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;

    virtual SdrPage* GetImplPage() const SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    virtual ~OCustomShape();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getAwtComponent() SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape() SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual sal_uInt32 GetObjInventor() const SAL_OVERRIDE;

private:
    virtual void impl_setUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxUnoShape ) SAL_OVERRIDE;
};


// OOle2Obj

class REPORTDESIGN_DLLPUBLIC OOle2Obj: public SdrOle2Obj , public OObjectBase
{
    friend class OReportPage;
    friend class DlgEdFactory;

    sal_uInt16 m_nType;
    bool    m_bOnlyOnce;
    void impl_createDataProvider_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& _xModel);
    virtual void impl_setUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxUnoShape ) SAL_OVERRIDE;

public:
    static OOle2Obj* Create( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent,sal_uInt16 _nType )
    {
        return new OOle2Obj( _xComponent,_nType );
    }
protected:
    OOle2Obj(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent,sal_uInt16 _nType);
    OOle2Obj(const OUString& _sComponentName,sal_uInt16 _nType);


    virtual void NbcMove( const Size& rSize ) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;

    virtual SdrPage* GetImplPage() const SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    virtual ~OOle2Obj();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getAwtComponent() SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape() SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual sal_uInt32 GetObjInventor() const SAL_OVERRIDE;
    // Clone() should make a complete copy of the object.
    virtual OOle2Obj* Clone() const SAL_OVERRIDE;
    virtual void initializeOle() SAL_OVERRIDE;

    OOle2Obj& operator=(const OOle2Obj& rObj);

    void initializeChart( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& _xModel);
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
    OUnoObject(  const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent
                ,const OUString& rModelName
                ,sal_uInt16   _nObjectType);

    virtual ~OUnoObject();

    virtual void NbcMove( const Size& rSize ) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;

    virtual SdrPage* GetImplPage() const SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    virtual void _propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    /** creates the m_xMediator when it doesn't already exist.
        @param  _bReverse   when set to <TRUE/> then the properties from the uno control will be copied into report control
    */
    void CreateMediator(bool _bReverse = false);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getAwtComponent() SAL_OVERRIDE;

    static OUString GetDefaultName(const OUnoObject* _pObj);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape() SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual sal_uInt32 GetObjInventor() const SAL_OVERRIDE;
    virtual OUnoObject* Clone() const SAL_OVERRIDE;

    OUnoObject& operator=(const OUnoObject& rObj);

private:
    virtual void impl_setUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxUnoShape ) SAL_OVERRIDE;
    void    impl_setReportComponent_nothrow();
    void    impl_initializeModel_nothrow();
};


} // rptui

#endif // INCLUDED_REPORTDESIGN_INC_RPTOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
