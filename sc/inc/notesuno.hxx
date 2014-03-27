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

#ifndef SC_NOTESUNO_HXX
#define SC_NOTESUNO_HXX

#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <cppuhelper/implbase5.hxx>
#include <svl/lstner.hxx>
#include "address.hxx"

class ScDocShell;
class SvxUnoText;
class ScPostIt;


class ScAnnotationObj : public cppu::WeakImplHelper5<
                            com::sun::star::container::XChild,
                            com::sun::star::text::XSimpleText,
                            com::sun::star::sheet::XSheetAnnotation,
                            com::sun::star::sheet::XSheetAnnotationShapeSupplier,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
public:
                            ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos);
    virtual                 ~ScAnnotationObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            /// XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            getParent() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setParent( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::uno::XInterface >& Parent )
                                    throw(::com::sun::star::lang::NoSupportException,
                                            ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setString( const OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XSheetAnnotation
    virtual ::com::sun::star::table::CellAddress SAL_CALL getPosition()
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAuthor()
        throw(::com::sun::star::uno::RuntimeException,
              std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDate()
        throw(::com::sun::star::uno::RuntimeException,
              std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getIsVisible()
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setIsVisible( sal_Bool bIsVisible )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            /// XSheetAnnotationShapeSupplier
    virtual ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > SAL_CALL
                            getAnnotationShape()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

                            /// XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    SvxUnoText&         GetUnoText();

    const ScPostIt*     ImplGetNote() const;

private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    SvxUnoText*             pUnoText;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
