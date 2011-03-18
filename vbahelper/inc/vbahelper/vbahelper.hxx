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
#ifndef OOVBAAPI_VBA_HELPER_HXX
#define OOVBAAPI_VBA_HELPER_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <basic/basmgr.hxx>
#include <basic/sberrors.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/pointr.hxx>
#include <memory>
#include "vbaaccesshelper.hxx"

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
        VBAHELPER_DLLPUBLIC css::uno::Reference<  css::uno::XInterface > getUnoDocModule( const String& aModName, SfxObjectShell* pShell );
        VBAHELPER_DLLPUBLIC SfxObjectShell* getSfxObjShell( const css::uno::Reference< css::frame::XModel >& xModel ) throw ( css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::uno::XInterface > createVBAUnoAPIService( SfxObjectShell* pShell,  const sal_Char* _pAsciiName ) throw (css::uno::RuntimeException);

        css::uno::Reference< css::frame::XModel > getCurrentDoc( const rtl::OUString& sKey ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getThisExcelDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getThisWordDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getCurrentExcelDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getCurrentWordDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);

        VBAHELPER_DLLPUBLIC css::uno::Reference< css::beans::XIntrospectionAccess > getIntrospectionAccess( const css::uno::Any& aObject ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::script::XTypeConverter > getTypeConverter( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);

        VBAHELPER_DLLPUBLIC void dispatchRequests( const css::uno::Reference< css::frame::XModel>& xModel, const rtl::OUString& aUrl );
     VBAHELPER_DLLPUBLIC void dispatchRequests (const css::uno::Reference< css::frame::XModel>& xModel, const rtl::OUString & aUrl, const css::uno::Sequence< css::beans::PropertyValue >& sProps, const css::uno::Reference< css::frame::XDispatchResultListener >& rListener = css::uno::Reference< css::frame::XDispatchResultListener >(), const sal_Bool bSilent = sal_True );
        VBAHELPER_DLLPUBLIC void dispatchExecute(SfxViewShell* pView, sal_uInt16 nSlot, SfxCallMode nCall = SFX_CALLMODE_SYNCHRON );
        VBAHELPER_DLLPUBLIC sal_Int32 OORGBToXLRGB( sal_Int32 );
        VBAHELPER_DLLPUBLIC sal_Int32 XLRGBToOORGB( sal_Int32 );
        VBAHELPER_DLLPUBLIC css::uno::Any OORGBToXLRGB( const css::uno::Any& );
        VBAHELPER_DLLPUBLIC css::uno::Any XLRGBToOORGB( const css::uno::Any& );
        // provide a NULL object that can be passed as variant so that
        // the object when passed to IsNull will return true. aNULL
        // contains an empty object reference
        VBAHELPER_DLLPUBLIC const css::uno::Any& aNULL();
        VBAHELPER_DLLPUBLIC void PrintOutHelper( SfxViewShell* pViewShell, const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName, sal_Bool bSelection  );
        VBAHELPER_DLLPUBLIC void PrintPreviewHelper( const css::uno::Any& EnableChanges,  SfxViewShell* );

        /** Extracts a boolean value from the passed Any, which may contain sal_Bool or an integer or floating-point value.
            Returns false, if the Any is empty or contains an incompatible type. */
        VBAHELPER_DLLPUBLIC bool extractBoolFromAny( bool& rbValue, const css::uno::Any& rAny );
        /** Extracts a boolean value from the passed Any, which may contain sal_Bool or an integer or floating-point value.
            Throws, if the Any is empty or contains an incompatible type. */
        VBAHELPER_DLLPUBLIC bool extractBoolFromAny( const css::uno::Any& rAny ) throw (css::uno::RuntimeException);

        VBAHELPER_DLLPUBLIC rtl::OUString getAnyAsString( const css::uno::Any& pvargItem ) throw ( css::uno::RuntimeException );
        VBAHELPER_DLLPUBLIC rtl::OUString VBAToRegexp(const rtl::OUString &rIn, bool bForLike = false); // needs to be in an uno service ( already this code is duplicated in basic )
        VBAHELPER_DLLPUBLIC double getPixelTo100thMillimeterConversionFactor( css::uno::Reference< css::awt::XDevice >& xDevice, sal_Bool bVertical);
        VBAHELPER_DLLPUBLIC double PointsToPixels( css::uno::Reference< css::awt::XDevice >& xDevice, double fPoints, sal_Bool bVertical);
        VBAHELPER_DLLPUBLIC double PixelsToPoints( css::uno::Reference< css::awt::XDevice >& xDevice, double fPixels, sal_Bool bVertical);
        VBAHELPER_DLLPUBLIC sal_Int32 PointsToHmm( double fPoints );
        VBAHELPER_DLLPUBLIC double HmmToPoints( sal_Int32 nHmm );
        VBAHELPER_DLLPUBLIC sal_Int32 getPointerStyle( const css::uno::Reference< css::frame::XModel >& );
        VBAHELPER_DLLPUBLIC void setCursorHelper( const css::uno::Reference< css::frame::XModel >& xModel, const Pointer& rPointer, sal_Bool bOverWrite );
        VBAHELPER_DLLPUBLIC void setDefaultPropByIntrospection( const css::uno::Any& aObj, const css::uno::Any& aValue  ) throw ( css::uno::RuntimeException );
        VBAHELPER_DLLPUBLIC css::uno::Any getDefaultPropByIntrospection( const css::uno::Any& aObj ) throw ( css::uno::RuntimeException );
        VBAHELPER_DLLPUBLIC css::uno::Any getPropertyValue( const css::uno::Sequence< css::beans::PropertyValue >& aProp, const rtl::OUString& aName );
        VBAHELPER_DLLPUBLIC sal_Bool setPropertyValue( css::uno::Sequence< css::beans::PropertyValue >& aProp, const rtl::OUString& aName, const css::uno::Any& aValue );
        VBAHELPER_DLLPUBLIC void setOrAppendPropertyValue( css::uno::Sequence< css::beans::PropertyValue >& aProp, const rtl::OUString& aName, const css::uno::Any& aValue );

class VBAHELPER_DLLPUBLIC Millimeter
{
//Factor to translate between points and hundredths of millimeters:
private:
    double m_nMillimeter;

public:
    Millimeter();

    Millimeter(double mm);

    void set(double mm);
    void setInPoints(double points) ;
    void setInHundredthsOfOneMillimeter(double hmm);
    double get();
    double getInHundredthsOfOneMillimeter();
    double getInPoints();
    static sal_Int32 getInHundredthsOfOneMillimeter(double points);
    static double getInPoints(int _hmm);
};

class VBAHELPER_DLLPUBLIC AbstractGeometryAttributes // probably should replace the ShapeHelper below
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

namespace msforms {
    class XShape;
}

class VBAHELPER_DLLPUBLIC ShapeHelper
{
protected:
    css::uno::Reference< css::drawing::XShape > xShape;
public:
    ShapeHelper( const css::uno::Reference< css::drawing::XShape >& _xShape) throw (css::script::BasicErrorException );

    double getHeight();

        void setHeight(double _fheight) throw ( css::script::BasicErrorException );

    double getWidth();

    void setWidth(double _fWidth) throw ( css::script::BasicErrorException );

    double getLeft();

    void setLeft(double _fLeft);

    double getTop();

    void setTop(double _fTop);
};

class VBAHELPER_DLLPUBLIC ConcreteXShapeGeometryAttributes : public AbstractGeometryAttributes
{
    std::auto_ptr< ShapeHelper > m_pShapeHelper;
public:
    ConcreteXShapeGeometryAttributes( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape );
    virtual double getLeft();
    virtual void setLeft( double nLeft );
    virtual double getTop();
    virtual void setTop( double nTop );
    virtual double getHeight();
    virtual void setHeight( double nHeight );
    virtual double getWidth();
    virtual void setWidth( double nWidth);
    virtual ~ConcreteXShapeGeometryAttributes();
};

#define VBA_LEFT "PositionX"
#define VBA_TOP "PositionY"
#define VBA_HEIGHT "Height"
#define VBA_WIDTH "Width"
class VBAHELPER_DLLPUBLIC UserFormGeometryHelper : public AbstractGeometryAttributes
{
    css::uno::Reference< css::awt::XWindow > mxWindow;
    sal_Bool mbDialog;

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

class VBAHELPER_DLLPUBLIC ContainerUtilities
{

public:
    static rtl::OUString getUniqueName( const css::uno::Sequence< ::rtl::OUString >&  _slist, const rtl::OUString& _sElementName, const ::rtl::OUString& _sSuffixSeparator);
    static rtl::OUString getUniqueName( const css::uno::Sequence< rtl::OUString >& _slist, const rtl::OUString _sElementName, const rtl::OUString& _sSuffixSeparator, sal_Int32 _nStartSuffix );

    static sal_Int32 FieldInList( const css::uno::Sequence< rtl::OUString >& SearchList, const rtl::OUString& SearchString );
};

// really just a a place holder to ease the porting pain
class VBAHELPER_DLLPUBLIC DebugHelper
{
public:
    static void exception( const rtl::OUString&  DetailedMessage, const css::uno::Exception& ex,  int err, const rtl::OUString& /*additionalArgument*/ ) throw( css::script::BasicErrorException );

    static void exception( int err,  const rtl::OUString& additionalArgument ) throw( css::script::BasicErrorException );

    static void exception( css::uno::Exception& ex ) throw( css::script::BasicErrorException );
};

class VBAHELPER_DLLPUBLIC VBADispatchListener : public cppu::WeakImplHelper1< css::frame::XDispatchResultListener >
{
private:
    css::uno::Any m_Result;
    sal_Bool m_State;

public:
    VBADispatchListener();
    ~VBADispatchListener();

    css::uno::Any getResult() { return m_Result; }
    sal_Bool getState() { return m_State; }

    // XDispatchResultListener
    virtual void SAL_CALL dispatchFinished( const css::frame::DispatchResultEvent& aEvent ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw ( css::uno::RuntimeException );
};

    } // openoffice
} // org

namespace ov = ooo::vba;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
