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

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_EVENTIMP_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_EVENTIMP_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <anim.hxx>

// office:events inside a shape

class SdXMLEventsContext : public SvXMLImportContext
{
private:
    css::uno::Reference< css::drawing::XShape > mxShape;

public:

    SdXMLEventsContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        const css::uno::Reference< css::drawing::XShape >& rxShape );
    virtual ~SdXMLEventsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

struct SdXMLEventContextData
{
    SdXMLEventContextData(const css::uno::Reference<css::drawing::XShape>& rxShape);
    void ApplyProperties();

    css::uno::Reference<css::drawing::XShape> mxShape;

    bool mbValid;
    bool mbScript;
    css::presentation::ClickAction meClickAction;
    XMLEffect meEffect;
    XMLEffectDirection meDirection;
    sal_Int16 mnStartScale;
    css::presentation::AnimationSpeed meSpeed;
    sal_Int32 mnVerb;
    OUString msSoundURL;
    bool mbPlayFull;
    OUString msMacroName;
    OUString msBookmark;
    OUString msLanguage;
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_EVENTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
