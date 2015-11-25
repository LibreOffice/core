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


#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_FILEOPENDIALOG_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_FILEOPENDIALOG_HXX

#include <vector>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class FileOpenDialog
{
    // a list of filters that are provided within the SaveDialog
    struct FilterEntry
    {
        OUString maName;
        OUString maType;
        OUString maUIName;
        sal_Int32 maFlags;

        FilterEntry() : maFlags( 0 ) {}
    };
    css::uno::Reference < css::ui::dialogs::XFilePicker3 > mxFilePicker;

    std::vector< FilterEntry > aFilterEntryList;

public:
        explicit FileOpenDialog( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        ~FileOpenDialog();

        sal_Int16 execute();

        void setDefaultName( const OUString& );
//      void setDefaultDirectory( const OUString& );

        OUString getURL() const;
        OUString getFilterName() const;
};

#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_FILEOPENDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
