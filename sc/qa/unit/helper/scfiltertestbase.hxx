/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <docsh.hxx>
#include <address.hxx>

#include <comphelper/fileformat.h>
#include "scqahelperdllapi.h"

#define ODS_FORMAT_TYPE                                                                            \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::TEMPLATE                    \
     | SfxFilterFlags::OWN | SfxFilterFlags::DEFAULT | SfxFilterFlags::ENCRYPTION                  \
     | SfxFilterFlags::PASSWORDTOMODIFY)
#define XLS_FORMAT_TYPE                                                                            \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN                       \
     | SfxFilterFlags::ENCRYPTION | SfxFilterFlags::PASSWORDTOMODIFY | SfxFilterFlags::PREFERED)
#define XLSX_FORMAT_TYPE                                                                           \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN                       \
     | SfxFilterFlags::STARONEFILTER | SfxFilterFlags::PREFERED)
#define LOTUS123_FORMAT_TYPE                                                                       \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED)
#define CSV_FORMAT_TYPE (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN)
#define HTML_FORMAT_TYPE (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN)
#define DIF_FORMAT_TYPE (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN)
#define XLS_XML_FORMAT_TYPE                                                                        \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED)
#define XLSB_XML_FORMAT_TYPE                                                                       \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER                \
     | SfxFilterFlags::PREFERED)
#define FODS_FORMAT_TYPE                                                                           \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::OWN                         \
     | SfxFilterFlags::STARONEFILTER)
#define GNUMERIC_FORMAT_TYPE                                                                       \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED)
#define XLTX_FORMAT_TYPE                                                                           \
    (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::TEMPLATE                    \
     | SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER | SfxFilterFlags::PREFERED)

class SCQAHELPER_DLLPUBLIC ScFilterTestBase : public test::BootstrapFixture,
                                              public test::FiltersTest
{
protected:
    ScDocShellRef loadDoc(const OUString& rURL, const OUString& rFilter, const OUString& rUserData,
                          const OUString& rTypeName, SfxFilterFlags nFilterFlags,
                          SotClipboardFormatId nClipboardID,
                          sal_Int32 nFilterVersion = SOFFICE_FILEFORMAT_CURRENT);

private:
    // reference to document interface that we are testing
    css::uno::Reference<css::uno::XInterface> m_xCalcComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
