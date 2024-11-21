/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <comphelper/hash.hxx>
#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>

#include <map>

namespace vcl::pdf
{
/* a crutch to transport a ::comphelper::Hash safely though UNO API
   this is needed for the PDF export dialog, which otherwise would have to pass
   clear text passwords down till they can be used in PDFWriter. Unfortunately
   the MD5 sum of the password (which is needed to create the PDF encryption key)
   is not sufficient, since an MD5 digest cannot be created in an arbitrary state
   which would be needed in computeEncryptionKey.
*/
class EncryptionHashTransporter : public cppu::WeakImplHelper<css::beans::XMaterialHolder>
{
    // V2R3
    std::unique_ptr<comphelper::Hash> m_pDigest;
    std::vector<sal_uInt8> maOValue;

    // V5R6
    std::vector<sal_uInt8> mU;
    std::vector<sal_uInt8> mUE;
    std::vector<sal_uInt8> mO;
    std::vector<sal_uInt8> mOE;
    std::vector<sal_uInt8> maEncryptionKey;

    // ID
    sal_IntPtr maID;

public:
    EncryptionHashTransporter();

    virtual ~EncryptionHashTransporter() override;

    comphelper::Hash* getUDigest() { return m_pDigest.get(); };

    std::vector<sal_uInt8>& getOValue() { return maOValue; }

    void invalidate() { m_pDigest.reset(); }

    std::vector<sal_uInt8> getU() { return mU; }
    void setU(std::vector<sal_uInt8> const& rU) { mU = rU; }

    std::vector<sal_uInt8> getUE() { return mUE; }
    void setUE(std::vector<sal_uInt8> const& rUE) { mUE = rUE; }

    std::vector<sal_uInt8> getO() { return mO; }
    void setO(std::vector<sal_uInt8> const& rO) { mO = rO; }

    std::vector<sal_uInt8> getOE() { return mOE; }
    void setOE(std::vector<sal_uInt8> const& rOE) { mOE = rOE; }

    std::vector<sal_uInt8> getEncryptionKey() { return maEncryptionKey; }
    void setEncryptionKey(std::vector<sal_uInt8> const& rEncryptionKey)
    {
        maEncryptionKey = rEncryptionKey;
    }

    // XMaterialHolder
    virtual css::uno::Any SAL_CALL getMaterial() override { return css::uno::Any(sal_Int64(maID)); }

    static EncryptionHashTransporter*
    getEncHashTransporter(const css::uno::Reference<css::beans::XMaterialHolder>& xReference);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
