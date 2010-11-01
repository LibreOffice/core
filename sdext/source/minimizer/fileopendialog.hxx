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


#ifndef _FILEOPEN_DIALOG_HXX_
#define _FILEOPEN_DIALOG_HXX_

#include <vector>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class FileOpenDialog
{
    // a list of filters that are provided within the SaveDialog
    struct FilterEntry
    {
        rtl::OUString maName;
        rtl::OUString maType;
        rtl::OUString maUIName;
        rtl::OUString maFilter;
        sal_Int32 maFlags;
        com::sun::star::uno::Sequence< rtl::OUString > maExtensions;

        FilterEntry() :
            maFlags( 0 ) {}
    };
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxMSF;
    com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > mxFilePicker;

    std::vector< FilterEntry > aFilterEntryList;

public :
        FileOpenDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF );
        ~FileOpenDialog();

        sal_Int16 execute();

        void setDefaultName( const rtl::OUString& );
//      void setDefaultDirectory( const rtl::OUString& );

        ::rtl::OUString getURL() const;
        ::rtl::OUString getFilterName() const;
};

#endif  // _FILEOPEN_DIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
