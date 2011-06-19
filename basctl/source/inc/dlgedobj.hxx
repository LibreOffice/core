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

#ifndef _BASCTL_DLGEDOBJ_HXX
#define _BASCTL_DLGEDOBJ_HXX

#include <svx/svdouno.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

#include <vector>
#include <map>

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
    sal_Bool        bIsListening;
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

    DECL_LINK(OnCreate, void* );

    using SfxListener::StartListening;
    void StartListening();
    using SfxListener::EndListening;
    void EndListening(sal_Bool bRemoveListener = sal_True);
    sal_Bool    isListening() const { return bIsListening; }

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
    ::std::vector<DlgEdObj*> pChilds;

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
    virtual ::std::vector<DlgEdObj*> GetChilds() const { return pChilds; }

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
