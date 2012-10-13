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

#ifndef SC_XMLWRAP_HXX
#define SC_XMLWRAP_HXX

#include <tools/solar.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

class ScDocument;
class SfxMedium;
class ScMySharedData;

#include <tools/errcode.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
    namespace embed { class XStorage; }
    namespace xml {
        namespace sax { struct InputSource; class XParser; } }
} } }

class ScXMLImportWrapper
{
    ScDocument&     rDoc;
    SfxMedium*      pMedium;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xStorage;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator> GetStatusIndicator();

    sal_uInt32 ImportFromComponent(com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xModel,
        com::sun::star::uno::Reference<com::sun::star::xml::sax::XParser>& xParser,
        com::sun::star::xml::sax::InputSource& aParserInput,
        const rtl::OUString& sComponentName, const rtl::OUString& sDocName, const rtl::OUString& sOldDocName,
        com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArgs,
        sal_Bool bMustBeSuccessfull);

    sal_Bool ExportToComponent(com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceFactory,
        com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xModel,
        com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& xWriter,
        com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aDescriptor,
        const rtl::OUString& sName, const rtl::OUString& sMediaType, const rtl::OUString& sComponentName,
        const sal_Bool bPlainText, com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArgs,
        ScMySharedData*& pSharedData);

public:
    ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >&);
    sal_Bool Import(sal_Bool bStylesOnly, ErrCode& );
    sal_Bool Export(sal_Bool bStylesOnly);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
