/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/uno/Reference.hxx>
#include <ooo/vba/msforms/XShapeRange.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbacollectionimpl.hxx>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>

namespace com::sun::star {
    namespace container { class XEnumeration; }
    namespace container { class XIndexAccess; }
    namespace drawing { class XDrawPage; }
    namespace drawing { class XShapes; }
    namespace frame { class XModel; }
}
namespace cpo::uno { class XComponentContext; }

namespace ooo::vba {
    class XHelperInterface;
    namespace msforms { class XFillFormat; }
    namespace msforms { class XLineFormat; }
    namespace msforms { class XShape; }
}

typedef CollTestImplHelper< ov::msforms::XShapeRange > ScVbaShapeRange_BASE;

class UNLESS_MERGELIBS(SAL_DLLPUBLIC_RTTI) ScVbaShapeRange final : public ScVbaShapeRange_BASE
{
private:
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::frame::XModel > m_xModel;
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
    /// @throws cpo::uno::RuntimeException
    css::uno::Reference< css::drawing::XShapes > const & getShapes() ;
public:
    VBAHELPER_DLLPUBLIC ScVbaShapeRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< cpo::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, css::uno::Reference< css::drawing::XDrawPage> xDrawShape, css::uno::Reference< css::frame::XModel > xModel );

    // Methods
    virtual void Select(  ) override;
    virtual css::uno::Reference< ::ooo::vba::msforms::XShape > Group() override;
    virtual void IncrementRotation( double Increment ) override;
    virtual void IncrementLeft( double Increment ) override ;
    virtual void IncrementTop( double Increment ) override;
    virtual OUString getName() override;
    virtual void setName( const OUString& _name ) override;
    virtual double getHeight() override;
    virtual void setHeight( double _height ) override;
    virtual double getWidth() override;
    virtual void setWidth( double _width ) override;
    virtual double getLeft() override;
    virtual void setLeft( double _left ) override;
    virtual double getTop() override;
    virtual void setTop( double _top ) override;
    virtual css::uno::Reference< ov::msforms::XLineFormat > getLine() override;
    virtual css::uno::Reference< ov::msforms::XFillFormat > getFill() override;
    virtual bool getLockAspectRatio() override;
    virtual void setLockAspectRatio( bool _lockaspectratio ) override;
    virtual bool getLockAnchor() override;
    virtual void setLockAnchor( bool _lockanchor ) override;
    virtual ::sal_Int32 getRelativeHorizontalPosition() override;
    virtual void setRelativeHorizontalPosition( ::sal_Int32 _relativehorizontalposition ) override;
    virtual ::sal_Int32 getRelativeVerticalPosition() override;
    virtual void setRelativeVerticalPosition( ::sal_Int32 _relativeverticalposition ) override;
    virtual cpo::uno::Any TextFrame(  ) override;
    virtual cpo::uno::Any WrapFormat(  ) override;
    virtual void ZOrder( sal_Int32 ZOrderCmd ) override;
    //XEnumerationAccess
    virtual cpo::uno::Type getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > createEnumeration() override;
    // ScVbaCollectionBaseImpl
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;
};

#endif // INCLUDED_VBAHELPER_VBASHAPERANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
