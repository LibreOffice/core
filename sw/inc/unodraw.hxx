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
#ifndef INCLUDED_SW_INC_UNODRAW_HXX
#define INCLUDED_SW_INC_UNODRAW_HXX

#include <svx/fmdpage.hxx>
#include <calbck.hxx>
#include <frmfmt.hxx>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <svl/itemprop.hxx>
#include <set>

class SdrMarkList;
class SdrView;
class SwDoc;

class SwFmDrawPage : public SvxFmDrawPage
{
    SdrPageView*        pPageView;
public:
    SwFmDrawPage( SdrPage* pPage );
    virtual ~SwFmDrawPage() throw () override;

    const SdrMarkList&  PreGroup(const css::uno::Reference< css::drawing::XShapes >& rShapes);
    void                PreUnGroup(const css::uno::Reference< css::drawing::XShapeGroup >& rShapeGroup);

    SdrView*            GetDrawView() {return mpView;}
    SdrPageView*        GetPageView();
    void                RemovePageView();
    static css::uno::Reference< css::uno::XInterface >       GetInterface( SdrObject* pObj );

    // The following method is called when a SvxShape-object is to be created.
    // Derived classes may obtain at this point a derivation or an object
    // that is aggregating a SvxShape.
    virtual css::uno::Reference< css::drawing::XShape >  CreateShape( SdrObject *pObj ) const throw (css::uno::RuntimeException, std::exception) override;
};

typedef cppu::WeakAggImplHelper
<
    css::container::XEnumerationAccess,
    css::drawing::XDrawPage,
    css::lang::XServiceInfo,
    css::drawing::XShapeGrouper
>
SwXDrawPageBaseClass;
class SwXDrawPage : public SwXDrawPageBaseClass
{
    SwDoc*          pDoc;
    css::uno::Reference< css::uno::XAggregation >     xPageAgg;
    SwFmDrawPage*   pDrawPage;
public:
    SwXDrawPage(SwDoc* pDoc);
    virtual ~SwXDrawPage() override;

    //XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XShapes
    virtual void SAL_CALL add(const css::uno::Reference< css::drawing::XShape > & xShape) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL remove(const css::uno::Reference< css::drawing::XShape > & xShape) throw( css::uno::RuntimeException, std::exception ) override;

    //XShapeGrouper
    virtual css::uno::Reference< css::drawing::XShapeGroup >  SAL_CALL group(const css::uno::Reference< css::drawing::XShapes > & xShapes)
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL ungroup(const css::uno::Reference< css::drawing::XShapeGroup > & aGroup) throw( css::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    SwFmDrawPage*   GetSvxPage();
    // renamed and outlined to detect where it's called
    void    InvalidateSwDoc(); // {pDoc = 0;}
};

class SwShapeDescriptor_Impl;
class SwXGroupShape;
typedef
cppu::WeakAggImplHelper
<
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::text::XTextContent,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel,
    css::drawing::XShape
>
SwXShapeBaseClass;
class SwXShape : public SwXShapeBaseClass,
    public SwClient
{
    friend class SwXGroupShape;
    friend class SwXDrawPage;

    css::uno::Reference< css::uno::XAggregation > xShapeAgg;
    // reference to <XShape>, determined in the
    // constructor by <queryAggregation> at <xShapeAgg>.
    css::uno::Reference< css::drawing::XShape > mxShape;

    const SfxItemPropertySet*           m_pPropSet;
    const SfxItemPropertyMapEntry*      m_pPropertyMapEntries;

    SwShapeDescriptor_Impl*     pImpl;

    bool                        m_bDescriptor;

    SvxShape*               GetSvxShape();

    /** method to determine top group object

        @author OD
    */
    SdrObject* GetTopGroupObj( SvxShape* _pSvxShape = nullptr );

    /** method to determine position according to the positioning attributes

        @author OD
    */
    css::awt::Point GetAttrPosition();

    /** method to convert the position (translation) of the drawing object to
        the layout direction horizontal left-to-right.

        @author OD
    */
    css::awt::Point ConvertPositionToHoriL2R(
                                    const css::awt::Point& rObjPos,
                                    const css::awt::Size& rObjSize );

    /** method to convert the transformation of the drawing object to the layout
        direction, the drawing object is in

        @author OD
    */
    css::drawing::HomogenMatrix3 ConvertTransformationToLayoutDir(
                const css::drawing::HomogenMatrix3& rMatrixInHoriL2R );

    /** method to adjust the positioning properties

        @author OD

        @param _aPosition
        input parameter - point representing the new shape position. The position
        has to be given in the layout direction the shape is in and relative to
        its position alignment areas.
    */
    void AdjustPositionProperties(const css::awt::Point& rPosition);

    /** method to convert start or end position of the drawing object to the
        Writer specific position, which is the attribute position in layout direction

        @author OD
    */
    css::awt::Point ConvertStartOrEndPosToLayoutDir(
                            const css::awt::Point& aStartOrEndPos );

    /** method to convert PolyPolygonBezier of the drawing object to the
        Writer specific position, which is the attribute position in layout direction

        @author OD
    */
    css::drawing::PolyPolygonBezierCoords ConvertPolyPolygonBezierToLayoutDir(
                    const css::drawing::PolyPolygonBezierCoords& aPath );

    /** method to get property from aggregation object

        @author OD
    */
    css::uno::Any _getPropAtAggrObj( const OUString& _rPropertyName )
            throw( css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException);

protected:
    virtual ~SwXShape() override;
    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwXShape(css::uno::Reference< css::uno::XInterface > & xShape);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XTextContent
    virtual void SAL_CALL attach(const css::uno::Reference< css::text::XTextRange > & xTextRange) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange >  SAL_CALL getAnchor() throw( css::uno::RuntimeException, std::exception ) override;

    //XComponent
    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL getPosition(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw (css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getShapeType(  ) throw (css::uno::RuntimeException, std::exception) override;

    SwShapeDescriptor_Impl*     GetDescImpl() {return pImpl;}
    SwFrameFormat*               GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
    const css::uno::Reference< css::uno::XAggregation >& GetAggregationInterface() {return xShapeAgg;}

    // helper
    static void AddExistingShapeToFormat( SdrObject& _rObj );
};

class SwXGroupShape :
    public SwXShape,
    public css::drawing::XShapes
{
protected:
    virtual ~SwXGroupShape() override;
public:
    SwXGroupShape(css::uno::Reference< css::uno::XInterface > & xShape);

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) throw (css::uno::RuntimeException, std::exception) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
