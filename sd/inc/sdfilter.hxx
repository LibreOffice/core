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

#ifndef _SD_SDFILTER_HXX
#define _SD_SDFILTER_HXX

#include <tools/gen.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>

// ------------
// - SdFilter -
// ------------

class SfxMedium;
namespace sd {
class DrawDocShell;
}
class SdDrawDocument;
namespace osl { class Module; }

class SdFilter
{
public:
    SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress );
    virtual ~SdFilter();

    sal_Bool                    IsProgress() const { return mbShowProgress; }
    sal_Bool                    IsDraw() const { return mbIsDraw; }
    sal_Bool                    IsImpress() const { return !mbIsDraw; }
    virtual sal_Bool            Export() = 0;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >             mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >    mxStatusIndicator;

    SfxMedium&                  mrMedium;
    ::sd::DrawDocShell&         mrDocShell;
    SdDrawDocument&             mrDocument;
    sal_Bool                    mbIsDraw : 1;
    sal_Bool                    mbShowProgress : 1;
#ifndef DISABLE_DYNLOADING
    ::osl::Module*              OpenLibrary( const ::rtl::OUString& rLibraryName ) const;
#endif
    void                        CreateStatusIndicator();

private:

    ::rtl::OUString             ImplGetFullLibraryName( const ::rtl::OUString& rLibraryName ) const;

};

#endif // _SD_SDFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
