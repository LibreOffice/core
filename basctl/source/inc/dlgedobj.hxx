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

#ifndef INCLUDED_BASCTL_SOURCE_INC_DLGEDOBJ_HXX
#define INCLUDED_BASCTL_SOURCE_INC_DLGEDOBJ_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/svdouno.hxx>

#include <boost/optional.hpp>

namespace basctl
{

typedef ::std::multimap< sal_Int16, OUString, ::std::less< sal_Int16 > > IndexToNameMap;


class DlgEdForm;
class DlgEditor;


// DlgEdObj


class DlgEdObj: public SdrUnoObj
{
    friend class DlgEditor;
    friend class DlgEdFactory;
    friend class DlgEdPropListenerImpl;
    friend class DlgEdForm;

private:
    bool            bIsListening;
    DlgEdForm*      pDlgEdForm;
    css::uno::Reference< css::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    css::uno::Reference< css::container::XContainerListener>  m_xContainerListener;

private:
    DlgEditor& GetDialogEditor ();

protected:
    DlgEdObj();
    DlgEdObj(const OUString& rModelName,
             const css::uno::Reference< css::lang::XMultiServiceFactory >& rxSFac);

    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    using SfxListener::StartListening;
    void StartListening();
    using SfxListener::EndListening;
    void    EndListening(bool bRemoveListener = true);
    bool    isListening() const { return bIsListening; }

    bool TransformSdrToControlCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    bool TransformSdrToFormCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    bool TransformControlToSdrCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    bool TransformFormToSdrCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );

public:

    virtual ~DlgEdObj();
    virtual void SetPage(SdrPage* pNewPage) override;

    void SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm* GetDlgEdForm() const { return pDlgEdForm; }

    virtual sal_uInt32 GetObjInventor() const override;
    virtual sal_uInt16 GetObjIdentifier() const override;

    virtual DlgEdObj*   Clone() const override;                                          // not working yet
    void clonedFrom(const DlgEdObj* _pSource);                          // not working yet

    // FullDrag support
    virtual SdrObject* getFullDragClone() const override;

    bool supportsService( OUString const & serviceName ) const;
    OUString GetDefaultName() const;
    OUString GetUniqueName() const;

    sal_Int32     GetStep() const;
    virtual void  UpdateStep();

    void SetDefaults();
    virtual void SetRectFromProps();
    virtual void SetPropsFromRect();

    css::uno::Reference< css::awt::XControl > GetControl() const;

    virtual void PositionAndSizeChange( const css::beans::PropertyChangeEvent& evt );
    void SAL_CALL NameChange( const  css::beans::PropertyChangeEvent& evt ) throw(css::container::NoSuchElementException, css::uno::RuntimeException);
    void SAL_CALL TabIndexChange( const  css::beans::PropertyChangeEvent& evt ) throw( css::uno::RuntimeException);

    // PropertyChangeListener
    void SAL_CALL _propertyChange(const css::beans::PropertyChangeEvent& evt) throw (css::uno::RuntimeException, std::exception);

    // ContainerListener
    void SAL_CALL _elementInserted( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException);
    void SAL_CALL _elementReplaced( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException);
    void SAL_CALL _elementRemoved( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException);

    virtual void SetLayer(SdrLayerID nLayer) override;
    bool MakeDataAware( const css::uno::Reference< css::frame::XModel >& xModel );
};



// DlgEdForm


class DlgEdForm: public DlgEdObj
{
    friend class DlgEditor;
    friend class DlgEdFactory;

private:
    DlgEditor& rDlgEditor;
    ::std::vector<DlgEdObj*> pChildren;

    mutable ::boost::optional< css::awt::DeviceInfo >   mpDeviceInfo;

private:
    explicit DlgEdForm (DlgEditor&);

protected:
    virtual void NbcMove( const Size& rSize ) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

public:

    virtual ~DlgEdForm();

    DlgEditor& GetDlgEditor () const { return rDlgEditor; }

    void AddChild( DlgEdObj* pDlgEdObj );
    void RemoveChild( DlgEdObj* pDlgEdObj );
    std::vector<DlgEdObj*> const& GetChildren() const { return pChildren; }

    virtual void UpdateStep() override;

    virtual void SetRectFromProps() override;
    virtual void SetPropsFromRect() override;

    virtual void PositionAndSizeChange( const css::beans::PropertyChangeEvent& evt ) override;

    void UpdateTabIndices();
    void UpdateTabOrder();
    void UpdateGroups();
    void UpdateTabOrderAndGroups();

    css::awt::DeviceInfo getDeviceInfo() const;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_DLGEDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
