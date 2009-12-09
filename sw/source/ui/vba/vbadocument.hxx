/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbadocument.hxx,v $
 * $Revision: 1.4 $
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
#ifndef SW_VBA_DOCUMENT_HXX
#define SW_VBA_DOCUMENT_HXX

#include <ooo/vba/word/XDocument.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadocumentbase.hxx>
#include <com/sun/star/text/XTextDocument.hpp>

typedef cppu::ImplInheritanceHelper1< VbaDocumentBase, ooo::vba::word::XDocument > SwVbaDocument_BASE;

class SwVbaDocument : public SwVbaDocument_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;

    void Initialize();
    css::uno::Any getControlShape( const rtl::OUString& sName );
    css::uno::Reference< css::container::XNameAccess > getFormControls();

public:
    SwVbaDocument( const css::uno::Reference< ooo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& m_xContext, css::uno::Reference< css::frame::XModel > xModel );
    SwVbaDocument(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~SwVbaDocument();

    // XDocument
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getContent() throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL Range( const css::uno::Any& rStart, const css::uno::Any& rEnd ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL BuiltInDocumentProperties( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL CustomDocumentProperties( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Bookmarks( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL Variables( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getAttachedTemplate() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAttachedTemplate( const css::uno::Any& _attachedtemplate ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Paragraphs( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL Styles( const css::uno::Any& rIndex ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL Tables( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sections( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL PageSetup() throw (css::uno::RuntimeException);
    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL invoke( const ::rtl::OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const ::rtl::OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_DOCUMENT_HXX */
