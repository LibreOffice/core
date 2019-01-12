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

#ifndef INCLUDED_XMLOFF_XMLSETTINGSEXPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_XMLSETTINGSEXPORTCONTEXT_HXX

#include <xmloff/xmltoken.hxx>

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }


namespace xmloff
{


    //= XMLExporter

    class SAL_NO_VTABLE XMLSettingsExportContext
    {
    public:
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      const OUString& i_rValue ) = 0;
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      enum ::xmloff::token::XMLTokenEnum i_eValue ) = 0;

        virtual void    StartElement( enum ::xmloff::token::XMLTokenEnum i_eName ) = 0;
        virtual void    EndElement(   const bool i_bIgnoreWhitespace ) = 0;

        virtual void    Characters( const OUString& i_rCharacters ) = 0;

        virtual css::uno::Reference< css::uno::XComponentContext >
                        GetComponentContext() const = 0;

    protected:
        ~XMLSettingsExportContext() {}
    };


} // namespace xmloff


#endif // INCLUDED_XMLOFF_XMLSETTINGSEXPORTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
