/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/pdf/PDFEncryptionInitialization.hxx>
#include <pdf/EncryptionHashTransporter.hxx>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <rtl/ref.hxx>
#include <pdf/PDFEncryptor.hxx>
#include <pdf/PDFEncryptorR6.hxx>

using namespace css;

namespace vcl::pdf
{
css::uno::Reference<css::beans::XMaterialHolder> initEncryption(const OUString& i_rOwnerPassword,
                                                                const OUString& i_rUserPassword)
{
    rtl::Reference<EncryptionHashTransporter> pTransporter = new EncryptionHashTransporter;
    PDFEncryptor::initEncryption(*pTransporter, i_rOwnerPassword, i_rUserPassword);
    PDFEncryptorR6::initEncryption(*pTransporter, i_rOwnerPassword, i_rUserPassword);
    return pTransporter;
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
