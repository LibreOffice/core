/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbahelper.hxx,v $
 * $Revision: 1.5.32.1 $
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
#ifndef SC_VBA_HELPER_HXX
#define SC_VBA_HELPER_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <basic/sberrors.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <docsh.hxx>
#include <sfx2/dispatch.hxx>
#include <ooo/vba/msforms/XShape.hpp>
#include "cellsuno.hxx"

namespace css = ::com::sun::star;

namespace ooo
{
    namespace vba
    {
        template < class T >
        css::uno::Reference< T > getXSomethingFromArgs( css::uno::Sequence< css::uno::Any > const & args, sal_Int32 nPos, bool bCanBeNull = true ) throw (css::lang::IllegalArgumentException)
        {
            if ( args.getLength() < ( nPos + 1) )
                throw css::lang::IllegalArgumentException();
            css::uno::Reference< T > aSomething( args[ nPos ], css::uno::UNO_QUERY );
            if ( !bCanBeNull && !aSomething.is() )
                throw css::lang::IllegalArgumentException();
            return aSomething;
        }
        css::uno::Reference< css::beans::XIntrospectionAccess > getIntrospectionAccess( const css::uno::Any& aObject ) throw (css::uno::RuntimeException);
        css::uno::Reference< css::script::XTypeConverter > getTypeConverter( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);

        void dispatchRequests (css::uno::Reference< css::frame::XModel>& xModel,rtl::OUString & aUrl) ;
        void dispatchRequests (css::uno::Reference< css::frame::XModel>& xModel,rtl::OUString & aUrl, css::uno::Sequence< css::beans::PropertyValue >& sProps ) ;
        void dispatchExecute(css::uno::Reference< css::frame::XModel>& xModel, USHORT nSlot, SfxCallMode nCall = SFX_CALLMODE_SYNCHRON );
        void implnCopy();
        void implnPaste();
        void implnCut();
        void implnPasteSpecial(sal_uInt16 nFlags,sal_uInt16 nFunction,sal_Bool bSkipEmpty, sal_Bool bTranspose);
        css::uno::Reference< css::frame::XModel >
            getCurrentDocument() throw (css::uno::RuntimeException);
        ScTabViewShell* getBestViewShell( css::uno::Reference< css::frame::XModel>& xModel ) ;
        ScDocShell* getDocShell( css::uno::Reference< css::frame::XModel>& xModel ) ;
        ScTabViewShell* getCurrentBestViewShell();
        SfxViewFrame* getCurrentViewFrame();
        sal_Int32 OORGBToXLRGB( sal_Int32 );
        sal_Int32 XLRGBToOORGB( sal_Int32 );
        css::uno::Any OORGBToXLRGB( const css::uno::Any& );
        css::uno::Any XLRGBToOORGB( const css::uno::Any& );
        // provide a NULL object that can be passed as variant so that
        // the object when passed to IsNull will return true. aNULL
        // contains an empty object reference
        const css::uno::Any& aNULL();
        void PrintOutHelper( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName, css::uno::Reference< css::frame::XModel >& xModel, sal_Bool bSelection  );
        void PrintPreviewHelper( const css::uno::Any& EnableChanges, css::uno::Reference< css::frame::XModel >& xModel );

        rtl::OUString getAnyAsString( const css::uno::Any& pvargItem ) throw ( css::uno::RuntimeException );
        rtl::OUString VBAToRegexp(const rtl::OUString &rIn, bool bForLike = false); // needs to be in an uno service ( already this code is duplicated in basic )
    double getPixelTo100thMillimeterConversionFactor( css::uno::Reference< css::awt::XDevice >& xDevice, sal_Bool bVertical);
    double PointsToPixels( css::uno::Reference< css::awt::XDevice >& xDevice, double fPoints, sal_Bool bVertical);
    double PixelsToPoints( css::uno::Reference< css::awt::XDevice >& xDevice, double fPoints, sal_Bool bVertical);


class ScVbaCellRangeAccess
{
public:
    static SfxItemSet* GetDataSet( ScCellRangeObj* pRangeObj );
};

class Millimeter
{
//Factor to translate between points and hundredths of millimeters:
private:
    static const double factor;

    double m_nMillimeter;

public:
    Millimeter():m_nMillimeter(0) {}

    Millimeter(double mm):m_nMillimeter(mm) {}

    void set(double mm) { m_nMillimeter = mm; }
    void setInPoints(double points)
    {
        m_nMillimeter = points * 0.352777778;
        // 25.4mm / 72
    }

    void setInHundredthsOfOneMillimeter(double hmm)
    {
        m_nMillimeter = hmm / 100;
    }

    double get()
    {
        return m_nMillimeter;
    }
    double getInHundredthsOfOneMillimeter()
    {
        return m_nMillimeter * 100;
    }
    double getInPoints()
    {
        return m_nMillimeter * 2.834645669; // 72 / 25.4mm
    }

    static sal_Int32 getInHundredthsOfOneMillimeter(double points)
    {
        sal_Int32 mm = static_cast<sal_Int32>(points * factor);
        return mm;
    }

    static double getInPoints(int _hmm)
    {
        double points = double( static_cast<double>(_hmm) / factor);
        return points;
    }
};

class AbstractGeometryAttributes // probably should replace the ShapeHelper below
{
public:
    virtual ~AbstractGeometryAttributes() {}
    virtual double getLeft() = 0;
    virtual void setLeft( double ) = 0;
    virtual double getTop() = 0;
    virtual void setTop( double ) = 0;
    virtual double getHeight() = 0;
    virtual void setHeight( double ) = 0;
    virtual double getWidth() = 0;
    virtual void setWidth( double ) = 0;
};

class ConcreteXShapeGeometryAttributes : public AbstractGeometryAttributes
{
public:
    css::uno::Reference< ooo::vba::msforms::XShape > m_xShape;
    ConcreteXShapeGeometryAttributes( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape );
    virtual double getLeft()
    {
        return m_xShape->getLeft();
    }
    virtual void setLeft( double nLeft )
    {
        m_xShape->setLeft( nLeft );
    }
    virtual double getTop()
    {
        return m_xShape->getTop();
    }
    virtual void setTop( double nTop )
    {
        m_xShape->setTop( nTop );
    }

    virtual double getHeight()
    {
        return m_xShape->getHeight();
    }
    virtual void setHeight( double nHeight )
    {
        m_xShape->setHeight( nHeight );
    }
    virtual double getWidth()
    {
        return m_xShape->getWidth();
    }
    virtual void setWidth( double nWidth)
    {
        m_xShape->setHeight( nWidth );
    }


};
#define VBA_LEFT "PositionX"
#define VBA_TOP "PositionY"
class UserFormGeometryHelper : public AbstractGeometryAttributes
{

    css::uno::Reference< css::beans::XPropertySet > mxModel;
public:
    UserFormGeometryHelper( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::awt::XControl >& xControl );
    virtual double getLeft();
    virtual void setLeft( double nLeft );
    virtual double getTop();
    virtual void setTop( double nTop );
    virtual double getHeight();
    virtual void setHeight( double nHeight );
    virtual double getWidth();
    virtual void setWidth( double nWidth);
};

class ShapeHelper
{
protected:
    css::uno::Reference< css::drawing::XShape > xShape;
public:
    ShapeHelper( const css::uno::Reference< css::drawing::XShape >& _xShape) throw (css::script::BasicErrorException ) : xShape( _xShape )
    {
        if( !xShape.is() )
            throw css::uno::RuntimeException( rtl::OUString::createFromAscii("No valid shape for helper"), css::uno::Reference< css::uno::XInterface >() );
    }

    double getHeight()
    {
            return  Millimeter::getInPoints(xShape->getSize().Height);
        }


        void setHeight(double _fheight) throw ( css::script::BasicErrorException )
    {
        try
        {
            css::awt::Size aSize = xShape->getSize();
            aSize.Height = Millimeter::getInHundredthsOfOneMillimeter(_fheight);
            xShape->setSize(aSize);
        }
        catch ( css::uno::Exception& /*e*/)
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
            }
    }


    double getWidth()
    {
        return Millimeter::getInPoints(xShape->getSize().Width);
        }

    void setWidth(double _fWidth) throw ( css::script::BasicErrorException )
    {
        try
        {
            css::awt::Size aSize = xShape->getSize();
            aSize.Width = Millimeter::getInHundredthsOfOneMillimeter(_fWidth);
            xShape->setSize(aSize);
        }
        catch (css::uno::Exception& /*e*/)
        {
            throw css::script::BasicErrorException( rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
        }
    }


    double getLeft()
    {
        return Millimeter::getInPoints(xShape->getPosition().X);
    }


    void setLeft(double _fLeft)
    {
        css::awt::Point aPoint = xShape->getPosition();
        aPoint.X = Millimeter::getInHundredthsOfOneMillimeter(_fLeft);
        xShape->setPosition(aPoint);
    }


    double getTop()
    {
            return Millimeter::getInPoints(xShape->getPosition().Y);
    }


    void setTop(double _fTop)
    {
        css::awt::Point aPoint = xShape->getPosition();
        aPoint.Y = Millimeter::getInHundredthsOfOneMillimeter(_fTop);
        xShape->setPosition(aPoint);
    }

};

class ContainerUtilities
{

public:
    static rtl::OUString getUniqueName( const css::uno::Sequence< ::rtl::OUString >&  _slist, const rtl::OUString& _sElementName, const ::rtl::OUString& _sSuffixSeparator);
    static rtl::OUString getUniqueName( const css::uno::Sequence< rtl::OUString >& _slist, const rtl::OUString _sElementName, const rtl::OUString& _sSuffixSeparator, sal_Int32 _nStartSuffix );

    static sal_Int32 FieldInList( const css::uno::Sequence< rtl::OUString >& SearchList, const rtl::OUString& SearchString );
};

// really just a a place holder to ease the porting pain
class DebugHelper
{
public:
    static void exception( const rtl::OUString&  DetailedMessage, const css::uno::Exception& ex,  int err, const rtl::OUString& /*additionalArgument*/ ) throw( css::script::BasicErrorException )
    {
        // #TODO #FIXME ( do we want to support additionalArg here )
        throw css::script::BasicErrorException( DetailedMessage.concat( rtl::OUString::createFromAscii(" ") ).concat( ex.Message ), css::uno::Reference< css::uno::XInterface >(), err, rtl::OUString() );
    }

    static void exception( int err,  const rtl::OUString& additionalArgument ) throw( css::script::BasicErrorException )
    {
        exception( rtl::OUString(), css::uno::Exception(), err, additionalArgument );
    }

    static void exception( css::uno::Exception& ex ) throw( css::script::BasicErrorException )
    {
        exception( rtl::OUString(), ex, SbERR_INTERNAL_ERROR, rtl::OUString() );
    }
};
    } // openoffice
} // org

namespace ov = ooo::vba;

#ifdef DEBUG
#  define SC_VBA_FIXME(a) OSL_TRACE( a )
#  define SC_VBA_STUB() SC_VBA_FIXME(( "%s - stubbed\n", __FUNCTION__ ))
#else
#  define SC_VBA_FIXME(a)
#  define SC_VBA_STUB()
#endif

#endif
