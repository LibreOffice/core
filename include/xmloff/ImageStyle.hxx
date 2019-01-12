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

#ifndef INCLUDED_XMLOFF_IMAGESTYLE_HXX
#define INCLUDED_XMLOFF_IMAGESTYLE_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }
namespace com { namespace sun { namespace star { namespace xml { namespace sax { class XAttributeList; } } } } }

class SvXMLExport;
class SvXMLImport;

namespace XMLImageStyle
{

XMLOFF_DLLPUBLIC void exportXML(OUString const & rStrName, css::uno::Any const & rValue, SvXMLExport& rExport);
XMLOFF_DLLPUBLIC bool importXML(css::uno::Reference<css::xml::sax::XAttributeList> const & xAttrList,
                                css::uno::Any& rValue, OUString& rStrName, SvXMLImport& rImport);

}

#endif // INCLUDED_XMLOFF_IMAGESTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
