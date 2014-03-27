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
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase6.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <svl/itemprop.hxx>

class SdrMarkList;
class SdrView;
class SwDoc;

class SwFmDrawPage : public SvxFmDrawPage
{
    SdrPageView*        pPageView;
protected:

    // Create a SdrObject according to a description. Can be used by derived classes to
    // support own ::com::sun::star::drawing::Shapes (e.g. controls).
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape ) throw (std::exception) SAL_OVERRIDE;

public:
    SwFmDrawPage( SdrPage* pPage );
    virtual ~SwFmDrawPage() throw ();

    const SdrMarkList&  PreGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > & xShapes);
    void                PreUnGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup >   xShapeGroup);

    SdrView*            GetDrawView() {return mpView;}
    SdrPageView*        GetPageView();
    void                RemovePageView();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >       GetInterface( SdrObject* pObj );

    // The following method is called when a SvxShape-object is to be created.
    // Derived classes may obtain at this point a derivation or an object
    // that is aggregating a SvxShape.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw (std::exception) SAL_OVERRIDE;
};

typedef cppu::WeakAggImplHelper4
<
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::drawing::XDrawPage,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::drawing::XShapeGrouper
>
SwXDrawPageBaseClass;
class SwXDrawPage : public SwXDrawPageBaseClass
{
    SwDoc*          pDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >     xPageAgg;
    SwFmDrawPage*   pDrawPage;
public:
    SwXDrawPage(SwDoc* pDoc);
    ~SwXDrawPage();

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XShapes
    virtual void SAL_CALL add(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL remove(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XShapeGrouper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup >  SAL_CALL group(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > & xShapes)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL ungroup(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup > & aGroup) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    SwFmDrawPage*   GetSvxPage();
    // renamed and outlined to detect where it's called
    void    InvalidateSwDoc(); // {pDoc = 0;}
};

class SwShapeDescriptor_Impl;
class SwXGroupShape;
typedef
cppu::WeakAggImplHelper6
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::text::XTextContent,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::drawing::XShape
>
SwXShapeBaseClass;
class SwXShape : public SwXShapeBaseClass,
    public SwClient
{
    friend class SwXGroupShape;
    friend class SwXDrawPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation > xShapeAgg;
    // reference to <XShape>, determined in the
    // constructor by <queryAggregation> at <xShapeAgg>.
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;

    const SfxItemPropertySet*           m_pPropSet;
    const SfxItemPropertyMapEntry*      m_pPropertyMapEntries;

    SwShapeDescriptor_Impl*     pImpl;

    sal_Bool                        m_bDescriptor;

    SvxShape*               GetSvxShape();

    /** method to determine top group object

        @author OD
    */
    SdrObject* _GetTopGroupObj( SvxShape* _pSvxShape = 0L );

    /** method to determine position according to the positioning attributes

        @author OD
    */
    com::sun::star::awt::Point _GetAttrPosition();

    /** method to convert the position (translation) of the drawing object to
        the layout direction horizontal left-to-right.

        @author OD
    */
    ::com::sun::star::awt::Point _ConvertPositionToHoriL2R(
                                    const ::com::sun::star::awt::Point _aObjPos,
                                    const ::com::sun::star::awt::Size _aObjSize );

    /** method to convert the transformation of the drawing object to the layout
        direction, the drawing object is in

        @author OD
    */
    ::com::sun::star::drawing::HomogenMatrix3 _ConvertTransformationToLayoutDir(
                ::com::sun::star::drawing::HomogenMatrix3 _aMatrixInHoriL2R );

    /** method to adjust the positioning properties

        @author OD

        @param _aPosition
        input parameter - point representing the new shape position. The position
        has to be given in the layout direction the shape is in and relative to
        its position alignment areas.
    */
    void _AdjustPositionProperties( const ::com::sun::star::awt::Point _aPosition );

    /** method to convert start or end position of the drawing object to the
        Writer specific position, which is the attribute position in layout direction

        @author OD
    */
    ::com::sun::star::awt::Point _ConvertStartOrEndPosToLayoutDir(
                            const ::com::sun::star::awt::Point& aStartOrEndPos );

    /** method to convert PolyPolygonBezier of the drawing object to the
        Writer specific position, which is the attribute position in layout direction

        @author OD
    */
    ::com::sun::star::drawing::PolyPolygonBezierCoords _ConvertPolyPolygonBezierToLayoutDir(
                    const ::com::sun::star::drawing::PolyPolygonBezierCoords& aPath );

    /** method to get property from aggregation object

        @author OD
    */
    ::com::sun::star::uno::Any _getPropAtAggrObj( const OUString& _rPropertyName )
            throw( ::com::sun::star::beans::UnknownPropertyException,
                   ::com::sun::star::lang::WrappedTargetException,
                   ::com::sun::star::uno::RuntimeException);

protected:
    virtual ~SwXShape();
    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;

public:
    SwXShape(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xShape);

    TYPEINFO_OVERRIDE();
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getShapeType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    SwShapeDescriptor_Impl*     GetDescImpl() {return pImpl;}
    SwFrmFmt*               GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >                 GetAggregationInterface() {return xShapeAgg;}

    // helper
    static void AddExistingShapeToFmt( SdrObject& _rObj );
};

class SwXGroupShape :
    public SwXShape,
    public ::com::sun::star::drawing::XShapes
{
protected:
    virtual ~SwXGroupShape();
public:
    SwXGroupShape(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xShape);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw() SAL_OVERRIDE;

    //XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
