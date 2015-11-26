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

#ifndef INCLUDED_XMLOFF_INC_XMLBACKGROUNDIMAGEEXPORT_HXX
#define INCLUDED_XMLOFF_INC_XMLBACKGROUNDIMAGEEXPORT_HXX

#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

class SvXMLExport;

class XMLBackgroundImageExport
{
    SvXMLExport&        rExport;

protected:

    SvXMLExport& GetExport() { return rExport; }
public:

    XMLBackgroundImageExport( SvXMLExport& rExport );

    ~XMLBackgroundImageExport();

    void exportXML( const css::uno::Any& rURL,
                    const css::uno::Any *pPos,
                    const css::uno::Any *pFilter,
                    const css::uno::Any *pTransparency,
                    sal_uInt16 nPrefix,
                    const OUString& rLocalName );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
