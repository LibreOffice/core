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

#ifndef INCLUDED_SC_INC_SERVUNO_HXX
#define INCLUDED_SC_INC_SERVUNO_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Sequence.hxx>

class ScDocShell;

class ScServiceProvider
{
public:
    enum class Type
    {
        SHEET , URLFIELD , PAGEFIELD , PAGESFIELD , DATEFIELD , TIMEFIELD , TITLEFIELD , FILEFIELD ,
        SHEETFIELD , CELLSTYLE , PAGESTYLE , AUTOFORMAT , CELLRANGES ,
        //  drawing layer tables
        GRADTAB , HATCHTAB , BITMAPTAB , TRGRADTAB , MARKERTAB , DASHTAB , NUMRULES ,

        DOCDEFLTS , DRAWDEFLTS ,

        DOCSPRSETT , DOCCONF ,

        IMAP_RECT , IMAP_CIRC , IMAP_POLY ,
        // Support creation of GraphicObjectResolver and EmbeddedObjectResolver
        EXPORT_GOR , IMPORT_GOR , EXPORT_EOR , IMPORT_EOR ,

        VALBIND , LISTCELLBIND , LISTSOURCE ,

        CELLADDRESS , RANGEADDRESS ,

        SHEETDOCSET ,

        // BM
        CHDATAPROV ,
        // formula parser
        FORMULAPARS , OPCODEMAPPER ,
        // VBA specific
        VBAOBJECTPROVIDER , VBACODENAMEPROVIDER , VBAGLOBALS ,

        EXT_TIMEFIELD ,

        INVALID
    };

                            // pDocShell is not needed for all Services
    static css::uno::Reference< css::uno::XInterface >
                            MakeInstance( Type nType, ScDocShell* pDocShell );
    static css::uno::Sequence<OUString> GetAllServiceNames();
    static Type             GetProviderType(const OUString& rServiceName);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
