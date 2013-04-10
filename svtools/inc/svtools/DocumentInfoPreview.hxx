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

#ifndef SVTOOLS_DOCUMENTINFOPREVIEW_HXX
#define SVTOOLS_DOCUMENTINFOPREVIEW_HXX

#include "sal/config.h"

#include "boost/scoped_ptr.hpp"
#include "i18nlangtag/languagetag.hxx"
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
        OUString const & rURL);

private:
    ExtMultiLineEdit m_pEditWin;
    boost::scoped_ptr< SvtDocInfoTable_Impl > m_pInfoTable;
    LanguageTag m_aLanguageTag;

    void insertEntry(OUString const & title, OUString const & value);

    void insertNonempty(long id, OUString const & value);

    void insertDateTime(long id, com::sun::star::util::DateTime const & value);
};

}

#endif // SVTOOLS_DOCUMENTINFOPREVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
