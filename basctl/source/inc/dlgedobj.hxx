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

#ifndef _BASCTL_DLGEDOBJ_HXX
#define _BASCTL_DLGEDOBJ_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/svdouno.hxx>

#include <boost/optional.hpp>

typedef ::std::multimap< sal_Int16, ::rtl::OUString, ::std::less< sal_Int16 > > IndexToNameMap;


class DlgEdForm;
class DlgEditor;

//============================================================================
// DlgEdObj
//============================================================================

class DlgEdObj: public SdrUnoObj
{
    friend class DlgEditor;
    friend class DlgEdFactory;
    friend class DlgEdPropListenerImpl;
    friend class DlgEdForm;

private:
    bool        bIsListening;
    DlgEdForm*      pDlgEdForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>  m_xContainerListener;

protected:
    DlgEdObj();
    DlgEdObj(const ::rtl::OUString& rModelName,
             const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac);

    virtual void NbcMove( const Size& rSize );
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    using SfxListener::StartListening;
    void StartListening();
    using SfxListener::EndListening;
    void EndListening(bool bRemoveListener = true);
    bool    isListening() const { return bIsListening; }

    virtual bool TransformSdrToControlCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    virtual bool TransformSdrToFormCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    virtual bool TransformControlToSdrCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    virtual bool TransformFormToSdrCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );

public:
    TYPEINFO();

    virtual ~DlgEdObj();
    virtual void SetPage(SdrPage* pNewPage);

    virtual void SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    virtual DlgEdForm* GetDlgEdForm() const { return pDlgEdForm; }

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual DlgEdObj*   Clone() const;                                          // not working yet
    virtual void clonedFrom(const DlgEdObj* _pSource);                          // not working yet

    // FullDrag support
    virtual SdrObject* getFullDragClone() const;

    virtual sal_Bool        supportsService( const sal_Char* _pServiceName ) const;
    virtual ::rtl::OUString GetDefaultName() const;
    virtual ::rtl::OUString GetUniqueName() const;

    virtual sal_Int32   GetStep() const;
    virtual void        UpdateStep();

    virtual void SetDefaults();
    virtual void SetRectFromProps();
    virtual void SetPropsFromRect();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > GetControl() const;

    virtual void PositionAndSizeChange( const ::com::sun::star::beans::PropertyChangeEvent& evt );
    virtual void SAL_CALL NameChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL TabIndexChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);

    // PropertyChangeListener
    virtual void SAL_CALL _propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

    // ContainerListener
    virtual void SAL_CALL _elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    virtual void SetLayer(SdrLayerID nLayer);
    bool MakeDataAware( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
};


//============================================================================
// DlgEdForm
//============================================================================

class DlgEdForm: public DlgEdObj
{
    friend class DlgEditor;
    friend class DlgEdFactory;

private:
    DlgEditor* pDlgEditor;
    ::std::vector<DlgEdObj*> pChildren;

    mutable ::boost::optional< ::com::sun::star::awt::DeviceInfo >   mpDeviceInfo;


protected:
    DlgEdForm();

    virtual void NbcMove( const Size& rSize );
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

public:
    TYPEINFO();

    virtual ~DlgEdForm();

    virtual void SetDlgEditor( DlgEditor* pEditor );
    virtual DlgEditor* GetDlgEditor() const { return pDlgEditor; }

    virtual void AddChild( DlgEdObj* pDlgEdObj );
    virtual void RemoveChild( DlgEdObj* pDlgEdObj );
    virtual ::std::vector<DlgEdObj*> GetChildren() const { return pChildren; }

    virtual void UpdateStep();

    virtual void SetRectFromProps();
    virtual void SetPropsFromRect();

    virtual void PositionAndSizeChange( const ::com::sun::star::beans::PropertyChangeEvent& evt );

    virtual void UpdateTabIndices();
    virtual void UpdateTabOrder();
    virtual void UpdateGroups();
    virtual void UpdateTabOrderAndGroups();

    ::com::sun::star::awt::DeviceInfo getDeviceInfo() const;

private:
    void    ImplInvalidateDeviceInfo();
};

#endif // _BASCTL_DLGEDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
