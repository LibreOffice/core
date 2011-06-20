/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
class SfxProgress;
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

    ::osl::Module*              OpenLibrary( const ::rtl::OUString& rLibraryName ) const;
    void                        CreateStatusIndicator();

private:

    ::rtl::OUString             ImplGetFullLibraryName( const ::rtl::OUString& rLibraryName ) const;

};

#endif // _SD_SDFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
