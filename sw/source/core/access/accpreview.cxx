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

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <strings.hrc>
#include "accpreview.hxx"

constexpr OUStringLiteral sImplementationName
    = u"com.sun.star.comp.Writer.SwAccessibleDocumentPageView";

using ::com::sun::star::uno::Sequence;

SwAccessiblePreview::SwAccessiblePreview(std::shared_ptr<SwAccessibleMap> const& pMap)
    : SwAccessibleDocumentBase(pMap)
{
    SetName( GetResource( STR_ACCESS_PREVIEW_DOC_NAME ) );
}

SwAccessiblePreview::~SwAccessiblePreview()
{
}

OUString SwAccessiblePreview::getImplementationName( )
{
    return sImplementationName;
}

sal_Bool SwAccessiblePreview::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SwAccessiblePreview::getSupportedServiceNames( )
{
    return {u"com.sun.star.text.AccessibleTextDocumentPageView"_ustr,
            sAccessibleServiceName};
}

Sequence< sal_Int8 > SAL_CALL SwAccessiblePreview::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SAL_CALL SwAccessiblePreview::getAccessibleDescription()
{
    return GetResource( STR_ACCESS_PREVIEW_DOC_NAME );
}

OUString SAL_CALL SwAccessiblePreview::getAccessibleName()
{
    return SwAccessibleDocumentBase::getAccessibleName() + " " + GetResource( STR_ACCESS_PREVIEW_DOC_SUFFIX );
}

void SwAccessiblePreview::InvalidateFocus_()
{
    FireStateChangedEvent( css::accessibility::AccessibleStateType::FOCUSED, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
