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

#ifndef SVTOOLS_DOCUMENTINFOPREVIEW_HXX
#define SVTOOLS_DOCUMENTINFOPREVIEW_HXX

#include "sal/config.h"

#include "boost/scoped_ptr.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "svtools/svmedit2.hxx"
#include "svtools/svtdllapi.h"
#include "tools/wintypes.hxx"
#include "vcl/window.hxx"

class SvtDocInfoTable_Impl;

namespace com { namespace sun { namespace star {
    namespace document { class XDocumentProperties; }
    namespace util { struct DateTime; }
} } }
namespace rtl { class OUString; }

namespace svtools {

class SVT_DLLPUBLIC ODocumentInfoPreview: public Window {
public:
    ODocumentInfoPreview(Window * pParent, WinBits nBits);

    virtual ~ODocumentInfoPreview();

    virtual void Resize();

    void clear();

    void fill(
        com::sun::star::uno::Reference<
            com::sun::star::document::XDocumentProperties > const & xDocProps,
        rtl::OUString const & rURL);

private:
    ExtMultiLineEdit m_pEditWin;
    boost::scoped_ptr< SvtDocInfoTable_Impl > m_pInfoTable;
    com::sun::star::lang::Locale m_aLocale;

    void insertEntry(rtl::OUString const & title, rtl::OUString const & value);

    void insertNonempty(long id, rtl::OUString const & value);

    void insertDateTime(long id, com::sun::star::util::DateTime const & value);
};

}

#endif // SVTOOLS_DOCUMENTINFOPREVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
