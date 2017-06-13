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

#ifndef INCLUDED_SC_INC_XMLWRAP_HXX
#define INCLUDED_SC_INC_XMLWRAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include "importfilterdata.hxx"
#include <sal/types.h>

#include <vcl/errcode.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
    namespace embed { class XStorage; }
    namespace xml {
        namespace sax { struct InputSource; class XParser; class XWriter; } }
} } }

class ScDocument;
class SfxMedium;
class ScMySharedData;
class ScDocShell;

enum class ImportFlags {
    Styles   = 0x01,
    Content  = 0x02,
    Metadata = 0x04,
    Settings = 0x08,
    All      = Styles | Content | Metadata | Settings
};
namespace o3tl
{
    template<> struct typed_flags<ImportFlags> : is_typed_flags<ImportFlags, 0x0f> {};
}


class ScXMLImportWrapper
{
    sc::ImportPostProcessData maPostProcessData;

    ScDocShell& mrDocShell;
    ScDocument&     rDoc;
    SfxMedium*      pMedium;
    css::uno::Reference< css::embed::XStorage > xStorage;

    css::uno::Reference< css::task::XStatusIndicator> GetStatusIndicator();

    ErrCode ImportFromComponent(const css::uno::Reference<css::uno::XComponentContext>& xContext,
        css::uno::Reference<css::frame::XModel>& xModel,
        css::uno::Reference<css::xml::sax::XParser>& xParser,
        css::xml::sax::InputSource& aParserInput,
        const OUString& sComponentName, const OUString& sDocName, const OUString& sOldDocName,
        css::uno::Sequence<css::uno::Any>& aArgs,
        bool bMustBeSuccessfull);

    bool ExportToComponent(const css::uno::Reference<css::uno::XComponentContext>& xContext,
        css::uno::Reference<css::frame::XModel>& xModel,
        css::uno::Reference<css::xml::sax::XWriter>& xWriter,
        css::uno::Sequence<css::beans::PropertyValue>& aDescriptor,
        const OUString& sName, const OUString& sMediaType, const OUString& sComponentName,
        css::uno::Sequence<css::uno::Any>& aArgs,
        ScMySharedData*& pSharedData);

public:

    ScXMLImportWrapper(
        ScDocShell& rDocSh, SfxMedium* pM, const css::uno::Reference<css::embed::XStorage>& xStor );

    bool Import( ImportFlags nMode, ErrCode& rError );
    bool Export(bool bStylesOnly);

    const sc::ImportPostProcessData& GetImportPostProcessData() const { return maPostProcessData;}
};

class ScXMLChartExportWrapper
{
public:
    ScXMLChartExportWrapper( css::uno::Reference< css::frame::XModel > const & xModel, SfxMedium& rMed );
    bool Export();

private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::embed::XStorage > mxStorage;
    SfxMedium& mrMedium;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
