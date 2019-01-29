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

#ifndef INCLUDED_SVTOOLS_DOCUMENTINFOPREVIEW_HXX
#define INCLUDED_SVTOOLS_DOCUMENTINFOPREVIEW_HXX

#include <sal/config.h>

#include <svtools/svtdllapi.h>
#include <tools/wintypes.hxx>
#include <vcl/window.hxx>

namespace com :: sun :: star :: uno { template <typename > class Reference; }

class ExtMultiLineEdit;

namespace com { namespace sun { namespace star {
    namespace document { class XDocumentProperties; }
    namespace util { struct DateTime; }
} } }

namespace svtools {

class SVT_DLLPUBLIC ODocumentInfoPreview: public vcl::Window {
public:
    ODocumentInfoPreview(vcl::Window * pParent, WinBits nBits);

    virtual ~ODocumentInfoPreview() override;
    virtual void dispose() override;

    virtual void Resize() override;

    void clear();

    void fill(css::uno::Reference< css::document::XDocumentProperties > const & xDocProps);

private:
    VclPtr<ExtMultiLineEdit> m_pEditWin;

    void insertEntry(OUString const & title, OUString const & value);

    void insertNonempty(long id, OUString const & value);

    void insertDateTime(long id, css::util::DateTime const & value);
};

}

#endif // INCLUDED_SVTOOLS_DOCUMENTINFOPREVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
