/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_MAIN_DUMMYXSHAPE_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_MAIN_DUMMYXSHAPE_HXX

#include <cppuhelper/implbase.hxx>

#include <vcl/opengl/OpenGLContext.hxx>
#include "OpenGLRender.hxx"

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Type.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>

#include "PropertyMapper.hxx"
#include "VLineProperties.hxx"
#include "Stripe.hxx"

#include <rtl/ustring.hxx>

#include <map>
#include <unordered_map>
#include <vector>

namespace chart {

namespace dummy {

class DummyChart;

class TextCache
{
public:
    struct TextCacheKey
    {
        OUString maText;
        std::map<OUString, css::uno::Any> maProperties;

        bool operator==(const TextCacheKey& rKey) const
        {
            return maText == rKey.maText && maProperties == rKey.maProperties;
        }
    };

    struct TextCacheKeyHash
    {
        size_t operator()(const TextCacheKey& rKey) const
        {
            return rKey.maText.hashCode();
        }
    };

    bool hasEntry(const TextCacheKey& rKey);
    BitmapEx& getBitmap(const TextCacheKey& rKey);
    void insertBitmap(const TextCacheKey& rKey, const BitmapEx& rBitmap);

private:

    std::unordered_map<TextCacheKey, BitmapEx, TextCacheKeyHash> maCache;
};

class DummyXShape : public cppu::WeakAggImplHelper<
                    css::drawing::XShape,
                    css::beans::XPropertySet,
                    css::beans::XMultiPropertySet,
                    css::container::XNamed,
                    css::container::XChild,
                    css::lang::XServiceInfo >
{
public:
    DummyXShape();

    // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames,
            const css::uno::Sequence< css::uno::Any >& aValues )
        throw (css::beans::PropertyVetoException, css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues(
            const css::uno::Sequence< OUString >& aPropertyNames )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames,
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;

    // normal non UNO methods

    virtual DummyChart* getRootShape();
    /**
     * Implement this method for all subclasses that should be rendered
     */
    virtual void render();

    const css::awt::Point& getPos() { return maPosition;} // internal API

protected:

    std::map<OUString, css::uno::Any> maProperties;
    css::awt::Point maPosition;
    css::awt::Size maSize;

private:
    OUString maName;

    css::uno::Reference< css::uno::XInterface > mxParent;

};

class DummyCube : public DummyXShape
{
public:
    DummyCube(const css::drawing::Position3D &rPos, const css::drawing::Direction3D& rSize,
            const css::uno::Reference< css::beans::XPropertySet > & xPropSet,
            const tPropertyNameMap& rPropertyNameMap );
};

class DummyCylinder : public DummyXShape
{
public:
    DummyCylinder(const css::drawing::Position3D&, const css::drawing::Direction3D& rSize );
};

class DummyPyramid : public DummyXShape
{
public:
    DummyPyramid(const css::drawing::Position3D& rPosition, const css::drawing::Direction3D& rSize,
            const css::uno::Reference< css::beans::XPropertySet > & xPropSet,
            const tPropertyNameMap& rPropertyNameMap );
};

class DummyCone : public DummyXShape
{
public:
    DummyCone(const css::drawing::Position3D& rPosition, const css::drawing::Direction3D& rSize);
};

class DummyPieSegment2D : public DummyXShape
{
public:
    DummyPieSegment2D(double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree,
            double fUnitCircleInnerRadius, double fUnitCircleOuterRadius,
            const css::drawing::Direction3D& rOffset, const css::drawing::HomogenMatrix& rUnitCircleToScene);
    void render() override;
private:
    double mfUnitCircleStartAngleDegree;
    double mfUnitCircleWidthAngleDegree;
    double mfUnitCircleInnerRadius;
    double mfUnitCircleOuterRadius;

    css::drawing::Direction3D maOffset;
    css::drawing::HomogenMatrix maUnitCircleToScene;
};

class DummyPieSegment : public DummyXShape {};

class DummyStripe : public DummyXShape
{
public:
    DummyStripe(const css::uno::Reference< css::beans::XPropertySet > & xPropSet,
            const tPropertyNameMap& rPropertyNameMap );
};

class DummyArea3D : public DummyXShape {};

class DummyArea2D : public DummyXShape
{
public:
    explicit DummyArea2D(const css::drawing::PointSequenceSequence& rShape);

    virtual void render() override;

private:
    css::drawing::PointSequenceSequence maShapes;
};

class DummySymbol2D : public DummyXShape
{
public:
    DummySymbol2D(const css::drawing::Position3D& rPosition, const css::drawing::Direction3D& rSize,
            sal_Int32 nStandardSymbol, sal_Int32 nFillColor);
    void render() override;
private:
    sal_Int32 mnStandardSymbol;
    sal_Int32 mnFillColor;
};

class DummyGraphic2D : public DummyXShape
{
public:
    DummyGraphic2D(const css::drawing::Position3D& rPosition, const css::drawing::Direction3D& rSize );
};

class DummyCircle : public DummyXShape
{
public:
    DummyCircle(const css::awt::Point& rPosition, const css::awt::Size& rSize);

    virtual void render() override;
};

class DummyLine3D : public DummyXShape
{
public:
    explicit DummyLine3D(const VLineProperties& rProperties);
};

class DummyLine2D : public DummyXShape
{
public:
    DummyLine2D(const css::drawing::PointSequenceSequence& rPoints, const VLineProperties* pProperties);
    DummyLine2D(const css::awt::Size& rSize, const css::awt::Point& rPosition);

    virtual void render() override;

private:
    css::drawing::PointSequenceSequence maPoints;
};

class DummyRectangle : public DummyXShape
{
public:
    DummyRectangle();
    explicit DummyRectangle(const css::awt::Size& rSize);
    DummyRectangle(const css::awt::Size& rSize, const css::awt::Point& rPoint, const tNameSequence& rNames,
            const tAnySequence& rValues );

    virtual void render() override;
};

class DummyText : public DummyXShape
{
public:
    DummyText(const OUString& rText, const tNameSequence& rNames,
            const tAnySequence& rValues, const css::uno::Any& rTrans, css::uno::Reference<
            css::drawing::XShapes > const & xTarget, double nRotation);

    virtual void render() override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;

private:
    void setTransformatAsProperty(const css::drawing::HomogenMatrix3& rMatrix);

    OUString maText;
    css::uno::Any maTrans;
    BitmapEx maBitmap;
    double mnRotation;
};

class DummyXShapes : public DummyXShape, public css::drawing::XShapes
{
public:

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // normal methods
    virtual void render() override;

protected:
    std::vector<css::uno::Reference< css::drawing::XShape > > maUNOShapes;
    std::vector<DummyXShape*> maShapes;
};

class DummyChart : public DummyXShapes
{
public:
    DummyChart();
    virtual ~DummyChart() override;
    virtual DummyChart* getRootShape() override;

    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    virtual void render() override;

    void clear();
    void invalidateInit() { mbNotInit = true; }
    TextCache& getTextCache() { return maTextCache;}

    OpenGLRender& getRenderer() { return m_GLRender; }

private:

    TextCache maTextCache;
    bool mbNotInit;

public:
    OpenGLRender m_GLRender;

};

class DummyGroup2D : public DummyXShapes
{
public:
    explicit DummyGroup2D(const OUString& rName);

    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;
};

class DummyGroup3D : public DummyXShapes
{
public:
    explicit DummyGroup3D(const OUString& rName);
};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
