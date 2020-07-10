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

#ifndef INCLUDED_VCL_UNOHELP_HXX
#define INCLUDED_VCL_UNOHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/FontSlant.hpp>
#include <tools/fontenum.hxx>
#include <vcl/dllapi.h>

namespace com::sun::star::i18n {
    class XBreakIterator;
    class XCharacterClassification;
}
namespace com::sun::star::accessibility {
    struct AccessibleEventObject;
}

namespace vcl::unohelper
{
VCL_DLLPUBLIC css::uno::Reference < css::i18n::XBreakIterator > CreateBreakIterator();
VCL_DLLPUBLIC css::uno::Reference < css::i18n::XCharacterClassification> CreateCharacterClassification();
VCL_DLLPUBLIC void NotifyAccessibleStateEventGlobally( const css::accessibility::AccessibleEventObject& rEventObject );
VCL_DLLPUBLIC float               ConvertFontWidth( FontWidth eWidth );
VCL_DLLPUBLIC FontWidth           ConvertFontWidth( float f );
VCL_DLLPUBLIC float               ConvertFontWeight( FontWeight eWeight );
VCL_DLLPUBLIC FontWeight          ConvertFontWeight( float f );
VCL_DLLPUBLIC css::awt::FontSlant ConvertFontSlant( FontItalic eWeight );
VCL_DLLPUBLIC FontItalic          ConvertFontSlant( css::awt::FontSlant );
}  // namespace vcl::unohelper

#endif // INCLUDED_VCL_UNOHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
