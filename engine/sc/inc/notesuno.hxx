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

#pragma once

#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include <rtl/ref.hxx>
#include "address.hxx"

class ScDocShell;
class SvxUnoText;
class ScPostIt;

class ScAnnotationObj final : public cppu::WeakImplHelper<
                            css::container::XChild,
                            css::text::XSimpleText,
                            css::sheet::XSheetAnnotation,
                            css::sheet::XSheetAnnotationShapeSupplier,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
public:
                            ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos);
    virtual                 ~ScAnnotationObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XChild
    virtual cpo::uno::Reference< cpo::uno::XInterface >
                            getParent() override;
    virtual void   setParent( const cpo::uno::Reference<
                                        cpo::uno::XInterface >& Parent ) override;

                            /// XSimpleText
    virtual cpo::uno::Reference< css::text::XTextCursor >
                            createTextCursor() override;
    virtual cpo::uno::Reference< css::text::XTextCursor >
                            createTextCursorByRange( const cpo::uno::Reference< css::text::XTextRange >& aTextPosition ) override;
    virtual void   insertString( const cpo::uno::Reference<
                                        css::text::XTextRange >& xRange,
                                        const OUString& aString, bool bAbsorb ) override;
    virtual void   insertControlCharacter( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, bool bAbsorb ) override;

                            /// XTextRange
    virtual cpo::uno::Reference< css::text::XText >
                            getText() override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getStart() override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getEnd() override;
    virtual OUString getString() override;
    virtual void   setString( const OUString& aString ) override;

                            /// XSheetAnnotation
    virtual css::table::CellAddress getPosition() override;
    virtual OUString getAuthor() override;
    virtual OUString getDate() override;
    virtual bool getIsVisible() override;
    virtual void setIsVisible( bool bIsVisible ) override;

                            /// XSheetAnnotationShapeSupplier
    virtual cpo::uno::Reference < css::drawing::XShape >
                            getAnnotationShape() override;

                            /// XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

private:
    SvxUnoText&         GetUnoText();

    const ScPostIt*     ImplGetNote() const;

private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    rtl::Reference<SvxUnoText> pUnoText;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
