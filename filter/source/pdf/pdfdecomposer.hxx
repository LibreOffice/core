/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_SOURCE_PDF_PDFDECOMPOSER_HXX
#define INCLUDED_FILTER_SOURCE_PDF_PDFDECOMPOSER_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

OUString PDFDecomposer_getImplementationName();
css::uno::Sequence<OUString> PDFDecomposer_getSupportedServiceNames();
css::uno::Reference<css::uno::XInterface>
PDFDecomposer_createInstance(const css::uno::Reference<css::lang::XMultiServiceFactory>& rSMgr);

#endif // INCLUDED_FILTER_SOURCE_PDF_PDFDECOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
