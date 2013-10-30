/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _DOCXTABLESTYLEXPORT_HXX_
#define _DOCXTABLESTYLEXPORT_HXX_

#include <boost/shared_ptr.hpp>
#include <sax/fshelper.hxx>

class SwDoc;

/// Handles DOCX export of table styles, based on InteropGrabBag.
class DocxTableStyleExport
{
    class Impl;
    boost::shared_ptr<Impl> m_pImpl;
public:
    void TableStyles();
    void SetSerializer(sax_fastparser::FSHelperPtr pSerializer);
    DocxTableStyleExport(SwDoc* pDoc, sax_fastparser::FSHelperPtr pSerializer);
    ~DocxTableStyleExport();
};

#endif // _DOCXTABLESTYLEXPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
