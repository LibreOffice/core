/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SQLBISON_EXPORTS_HXX
#define INCLUDED_CONNECTIVITY_SQLBISON_EXPORTS_HXX

#include <rtl/ustring.hxx>

namespace connectivity { class OSQLParseNode; }
namespace connectivity { class OSQLParser; }

OUString ConvertLikeToken(const ::connectivity::OSQLParseNode* pTokenNode, const ::connectivity::OSQLParseNode* pEscapeNode, bool bInternational);
int SQLyyparse();
void setParser( ::connectivity::OSQLParser* );

#endif // INCLUDED_CONNECTIVITY_SQLBISON_EXPORTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
