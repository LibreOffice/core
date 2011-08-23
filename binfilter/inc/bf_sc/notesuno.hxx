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

#ifndef SC_NOTESUNO_HXX
#define SC_NOTESUNO_HXX

#include "global.hxx"			// ScRange, ScAddress

#include <bf_svtools/lstner.hxx>

#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/XSimpleText.hpp>

#include <cppuhelper/implbase4.hxx>
namespace binfilter {


class ScDocShell;
class SvxUnoText;


class ScAnnotationObj : public cppu::WeakImplHelper4<
                            ::com::sun::star::container::XChild,
                            ::com::sun::star::text::XSimpleText,
                            ::com::sun::star::sheet::XSheetAnnotation,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    ScAddress				aCellPos;
    SvxUnoText*				pUnoText;

private:
    SvxUnoText&	GetUnoText();

public:
                            ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos);
    virtual					~ScAnnotationObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setParent( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::uno::XInterface >& Parent )
                                    throw(::com::sun::star::lang::NoSupportException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setString( const ::rtl::OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetAnnotation
    virtual ::com::sun::star::table::CellAddress SAL_CALL getPosition()
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAuthor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDate() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getIsVisible() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setIsVisible( sal_Bool bIsVisible )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
