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

        class XHelperInterface;

        /** Returns the VBA document implementation object representing the passed UNO document model. */
        VBAHELPER_DLLPUBLIC css::uno::Reference< XHelperInterface > getVBADocument( const css::uno::Reference< css::frame::XModel >& xModel );
        VBAHELPER_DLLPUBLIC css::uno::Reference< XHelperInterface > getUnoDocModule( const String& aModName, SfxObjectShell* pShell );
        VBAHELPER_DLLPUBLIC SfxObjectShell* getSfxObjShell( const css::uno::Reference< css::frame::XModel >& xModel ) throw ( css::uno::RuntimeException);

        css::uno::Reference< css::frame::XModel > getCurrentDoc( const rtl::OUString& sKey ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getThisExcelDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getThisWordDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getCurrentExcelDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::frame::XModel > getCurrentWordDoc( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);

        VBAHELPER_DLLPUBLIC css::uno::Reference< css::beans::XIntrospectionAccess > getIntrospectionAccess( const css::uno::Any& aObject ) throw (css::uno::RuntimeException);
        VBAHELPER_DLLPUBLIC css::uno::Reference< css::script::XTypeConverter > getTypeConverter( const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);

        VBAHELPER_DLLPUBLIC void dispatchRequests( const css::uno::Reference< css::frame::XModel>& xModel, const rtl::OUString& aUrl );
     VBAHELPER_DLLPUBLIC void dispatchRequests (const css::uno::Reference< css::frame::XModel>& xModel, const rtl::OUString & aUrl, const css::uno::Sequence< css::beans::PropertyValue >& sProps );
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
        VBAHELPER_DLLPUBLIC void WaitUntilPreviewIsClosed( SfxViewFrame* );

        /** Extracts a boolean value from the passed Any, which may contain a Boolean or an integer or floating-point value.
            Throws, if the Any is empty or contains an incompatible type. */
        VBAHELPER_DLLPUBLIC bool extractBoolFromAny( const css::uno::Any& rAny ) throw (css::uno::RuntimeException);

        /** Extracts a string from the passed Any, which may contain a Boolean, a value, or a string.
            Throws, if the Any is empty or contains an incompatible type. */
        VBAHELPER_DLLPUBLIC ::rtl::OUString extractStringFromAny( const css::uno::Any& rAny, bool bUppercaseBool = false ) throw (css::uno::RuntimeException);
        /** Extracts a string from the passed Any, which may contain a Boolean, a value, or a string.
            Returns rDefault, if rAny is empty. Throws, if the Any contains an incompatible type. */
        VBAHELPER_DLLPUBLIC ::rtl::OUString extractStringFromAny( const css::uno::Any& rAny, const ::rtl::OUString& rDefault, bool bUppercaseBool = false ) throw (css::uno::RuntimeException);

        VBAHELPER_DLLPUBLIC rtl::OUString getAnyAsString( const css::uno::Any& pvargItem ) throw ( css::uno::RuntimeException );
        VBAHELPER_DLLPUBLIC rtl::OUString VBAToRegexp(const rtl::OUString &rIn, bool bForLike = false); // needs to be in an uno service ( already this code is duplicated in basic )
        VBAHELPER_DLLPUBLIC double getPixelTo100thMillimeterConversionFactor( const css::uno::Reference< css::awt::XDevice >& xDevice, sal_Bool bVertical);
        VBAHELPER_DLLPUBLIC double PointsToPixels( const css::uno::Reference< css::awt::XDevice >& xDevice, double fPoints, sal_Bool bVertical);
        VBAHELPER_DLLPUBLIC double PixelsToPoints( const css::uno::Reference< css::awt::XDevice >& xDevice, double fPixels, sal_Bool bVertical);
        VBAHELPER_DLLPUBLIC sal_Int32 PointsToHmm( double fPoints );
        VBAHELPER_DLLPUBLIC double HmmToPoints( sal_Int32 nHmm );
        VBAHELPER_DLLPUBLIC sal_Int32 getPointerStyle( const css::uno::Reference< css::frame::XModel >& );
        VBAHELPER_DLLPUBLIC void setCursorHelper( const css::uno::Reference< css::frame::XModel >& xModel, const Pointer& rPointer, sal_Bool bOverWrite );
        VBAHELPER_DLLPUBLIC void setDefaultPropByIntrospection( const css::uno::Any& aObj, const css::uno::Any& aValue  ) throw ( css::uno::RuntimeException );
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

    void setInPoints(double points) ;
    double getInHundredthsOfOneMillimeter();
    static sal_Int32 getInHundredthsOfOneMillimeter(double points);
    static double getInPoints(int _hmm);
};

class VBAHELPER_DLLPUBLIC AbstractGeometryAttributes // probably should replace the ShapeHelper below
{
public:
    virtual ~AbstractGeometryAttributes() {}
    virtual double getLeft() const = 0;
    virtual void setLeft( double ) = 0;
    virtual double getTop() const = 0;
    virtual void setTop( double ) = 0;
    virtual double getHeight() const = 0;
    virtual void setHeight( double ) = 0;
    virtual double getWidth() const = 0;
    virtual void setWidth( double ) = 0;

    virtual double getInnerHeight() const { return 0.0; }
    virtual void setInnerHeight( double ) {}
    virtual double getInnerWidth() const { return 0.0; }
    virtual void setInnerWidth( double ) {}
    virtual double getOffsetX() const { return 0.0; }
    virtual double getOffsetY() const { return 0.0; }
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

    double getHeight() const;
       void setHeight(double _fheight) throw ( css::script::BasicErrorException );
    double getWidth() const;
    void setWidth(double _fWidth) throw ( css::script::BasicErrorException );
    double getLeft() const;
    void setLeft(double _fLeft);
    double getTop() const;
    void setTop(double _fTop);
};

class VBAHELPER_DLLPUBLIC ConcreteXShapeGeometryAttributes : public AbstractGeometryAttributes
{
    std::auto_ptr< ShapeHelper > m_pShapeHelper;
public:
    ConcreteXShapeGeometryAttributes( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape );
    virtual double getLeft() const;
    virtual void setLeft( double nLeft );
    virtual double getTop() const;
    virtual void setTop( double nTop );
    virtual double getHeight() const;
    virtual void setHeight( double nHeight );
    virtual double getWidth() const;
    virtual void setWidth( double nWidth);
    virtual ~ConcreteXShapeGeometryAttributes();
};

#define VBA_LEFT "PositionX"
#define VBA_TOP "PositionY"
class VBAHELPER_DLLPUBLIC UserFormGeometryHelper : public AbstractGeometryAttributes
{
public:
    UserFormGeometryHelper(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::awt::XControl >& xControl,
        double fOffsetX, double fOffsetY );
    virtual double getLeft() const;
    virtual void setLeft( double fLeft );
    virtual double getTop() const;
    virtual void setTop( double fTop );
    virtual double getWidth() const;
    virtual void setWidth( double fWidth );
    virtual double getHeight() const;
    virtual void setHeight( double fHeight );
    virtual double getInnerWidth() const;
    virtual void setInnerWidth( double fWidth );
    virtual double getInnerHeight() const;
    virtual void setInnerHeight( double fHeight );
    virtual double getOffsetX() const;
    virtual double getOffsetY() const;

private:
    double implGetPos( bool bPosY ) const;
    void implSetPos( double fPos, bool bPosY );
    double implGetSize( bool bHeight, bool bOuter ) const;
    void implSetSize( double fSize, bool bHeight, bool bOuter );

private:
    css::uno::Reference< css::awt::XWindow > mxWindow;
    css::uno::Reference< css::beans::XPropertySet > mxModelProps;
    css::uno::Reference< css::awt::XUnitConversion > mxUnitConv;
    double mfOffsetX;
    double mfOffsetY;
    sal_Bool mbDialog;
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

    static void exception( const css::uno::Exception& ex ) throw( css::script::BasicErrorException );
};

    } // vba
} // ooo

namespace ov = ooo::vba;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
