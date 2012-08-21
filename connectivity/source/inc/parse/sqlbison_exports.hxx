/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _CONNECTIVITY_PARSE_SQLBISON_HXX_
#define _CONNECTIVITY_PARSE_SQLBISON_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <connectivity/sqlnode.hxx>

::rtl::OUString ConvertLikeToken(const ::connectivity::OSQLParseNode* pTokenNode, const ::connectivity::OSQLParseNode* pEscapeNode, sal_Bool bInternational);
int SQLyyparse (void);
void setParser( ::connectivity::OSQLParser* );

#endif //_CONNECTIVITY_PARSE_SQLBISON_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
