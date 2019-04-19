/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_writerperfect.h>

#include "wpftqahelperdllapi.h"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>

#include <test/bootstrapfixture.hxx>

#define REQUIRE_VERSION(major, minor, micro, req_major, req_minor, req_micro)                      \
    (major) > (req_major)                                                                          \
        || ((major) == (req_major)                                                                 \
            && ((minor) > (req_minor) || ((minor) == (req_minor) && ((micro) >= (req_micro)))))

#define REQUIRE_EBOOK_VERSION(major, minor, micro)                                                 \
    REQUIRE_VERSION(EBOOK_VERSION_MAJOR, EBOOK_VERSION_MINOR, EBOOK_VERSION_MICRO, major, minor,   \
                    micro)

#define REQUIRE_ETONYEK_VERSION(major, minor, micro)                                               \
    REQUIRE_VERSION(ETONYEK_VERSION_MAJOR, ETONYEK_VERSION_MINOR, ETONYEK_VERSION_MICRO, major,    \
                    minor, micro)

#define REQUIRE_MWAW_VERSION(major, minor, micro)                                                  \
    REQUIRE_VERSION(MWAW_VERSION_MAJOR, MWAW_VERSION_MINOR, MWAW_VERSION_MICRO, major, minor, micro)

#define REQUIRE_STAROFFICE_VERSION(major, minor, micro)                                            \
    REQUIRE_VERSION(STAROFFICE_VERSION_MAJOR, STAROFFICE_VERSION_MINOR, STAROFFICE_VERSION_MICRO,  \
                    major, minor, micro)

#define REQUIRE_WPS_VERSION(major, minor, micro)                                                   \
    REQUIRE_VERSION(WPS_VERSION_MAJOR, WPS_VERSION_MINOR, WPS_VERSION_MICRO, major, minor, micro)

namespace com
{
namespace sun
{
namespace star
{
namespace container
{
class XNameAccess;
}
namespace document
{
class XTypeDetection;
}
namespace frame
{
class XDesktop2;
}
}
}
}

namespace writerperfect
{
namespace test
{
class WPFTQAHELPER_DLLPUBLIC WpftFilterFixture : public ::test::BootstrapFixture
{
public:
    virtual void setUp() override;

protected:
    css::uno::Reference<css::frame::XDesktop2> m_xDesktop;
    css::uno::Reference<css::document::XTypeDetection> m_xTypeDetection;
    css::uno::Reference<css::container::XNameAccess> m_xTypeMap;
};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
