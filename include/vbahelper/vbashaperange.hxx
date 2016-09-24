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
#ifndef INCLUDED_VBAHELPER_VBASHAPERANGE_HXX
#define INCLUDED_VBAHELPER_VBASHAPERANGE_HXX

#include <exception>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <ooo/vba/msforms/XShapeRange.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbacollectionimpl.hxx>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XEnumeration; }
    namespace container { class XIndexAccess; }
    namespace drawing { class XDrawPage; }
    namespace drawing { class XShapes; }
    namespace frame { class XModel; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class XHelperInterface;
    namespace msforms { class XFillFormat; }
    namespace msforms { class XLineFormat; }
    namespace msforms { class XShape; }
} }

typedef CollTestImplHelper< ov::msforms::XShapeRange > ScVbaShapeRange_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShapeRange : public ScVbaShapeRange_BASE
{
private:
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    css::uno::Reference< css::drawing::XShapes > const & getShapes() throw (css::uno::RuntimeException) ;
public:
    ScVbaShapeRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, const css::uno::Reference< css::drawing::XDrawPage>& xDrawShape, const css::uno::Reference< css::frame::XModel >& xModel );

    // Methods
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ::ooo::vba::msforms::XShape > SAL_CALL Group() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL IncrementRotation( double Increment ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL IncrementLeft( double Increment ) throw (css::uno::RuntimeException, std::exception) override ;
    virtual void SAL_CALL IncrementTop( double Increment ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& _name ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHeight( double _height ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setWidth( double _width ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XLineFormat > SAL_CALL getLine() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XFillFormat > SAL_CALL getFill() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getLockAspectRatio() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLockAspectRatio( sal_Bool _lockaspectratio ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getLockAnchor() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLockAnchor( sal_Bool _lockanchor ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getRelativeHorizontalPosition() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRelativeHorizontalPosition( ::sal_Int32 _relativehorizontalposition ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getRelativeVerticalPosition() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRelativeVerticalPosition( ::sal_Int32 _relativeverticalposition ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL SAL_CALL TextFrame(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL SAL_CALL WrapFormat(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ZOrder( sal_Int32 ZOrderCmd ) throw (css::uno::RuntimeException, std::exception) override;
    //XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
};

#endif // INCLUDED_VBAHELPER_VBASHAPERANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
