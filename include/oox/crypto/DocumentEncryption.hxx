/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef DOCUMENT_ENCRYPTION_HXX
#define DOCUMENT_ENCRYPTION_HXX

#include "oox/dllapi.h"

#include "oox/ole/olestorage.hxx"

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include "CryptTools.hxx"
#include "Standard2007Engine.hxx"

#include <vector>


namespace oox {
namespace core {

class OOX_DLLPUBLIC DocumentEncryption
{
private:
    com::sun::star::uno::Reference< com::sun::star::io::XStream > mxDocumentStream;
    oox::ole::OleStorage& mrOleStorage;
    OUString maPassword;

    Standard2007Engine mEngine;

public:
    DocumentEncryption(
        com::sun::star::uno::Reference< com::sun::star::io::XStream > xDocumentStream,
        oox::ole::OleStorage& rOleStorage,
        OUString aPassword);

    bool encrypt();

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
