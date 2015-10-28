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

#ifndef INCLUDED_SD_INC_SDFILTER_HXX
#define INCLUDED_SD_INC_SDFILTER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>

// SdFilter
class SfxMedium;
namespace sd {
class DrawDocShell;
}
class SdDrawDocument;
namespace osl { class Module; }

class SdFilter
{
public:
    SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, bool bShowProgress );
    virtual ~SdFilter();

    bool                    IsDraw() const { return mbIsDraw; }
    virtual bool            Export() = 0;

protected:
    css::uno::Reference< css::frame::XModel >             mxModel;
    css::uno::Reference< css::task::XStatusIndicator >    mxStatusIndicator;

    SfxMedium&                  mrMedium;
    ::sd::DrawDocShell&         mrDocShell;
    SdDrawDocument&             mrDocument;
    bool                        mbIsDraw : 1;
    bool                        mbShowProgress : 1;
#ifndef DISABLE_DYNLOADING
    static ::osl::Module*       OpenLibrary( const OUString& rLibraryName );
#endif
    void                        CreateStatusIndicator();

private:

    static OUString             ImplGetFullLibraryName( const OUString& rLibraryName );

};

#endif // INCLUDED_SD_INC_SDFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
