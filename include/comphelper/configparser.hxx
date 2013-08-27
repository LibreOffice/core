/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPHELPER_SERVICEDECL_XML_CONFIGPARSING_HXX
#define COMPHELPER_SERVICEDECL_XML_CONFIGPARSING_HXX
#include "rtl/ustring.hxx"
#include "comphelper/comphelperdllapi.h"
#include <boost/shared_ptr.hpp>

namespace comphelper
{
    COMPHELPER_DLLPUBLIC std::auto_ptr< rtl::OUString > getExpandedFilePath(const rtl::OUString&);
}
#endif