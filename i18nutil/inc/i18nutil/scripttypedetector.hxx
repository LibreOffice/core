/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _I18N_SCRIPTTYPEDETECTOR_HXX_
#define _I18N_SCRIPTTYPEDETECTOR_HXX_

#include "i18nutildllapi.h"
#include <rtl/ustring.hxx>

class I18NUTIL_DLLPUBLIC ScriptTypeDetector
{
public:
    static sal_Int32 beginOfScriptDirection( const rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection );
    static sal_Int32 endOfScriptDirection( const rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection );
    static sal_Int16 getScriptDirection( const rtl::OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection );
    static sal_Int32 beginOfCTLScriptType( const rtl::OUString& Text, sal_Int32 nPos );
    static sal_Int32 endOfCTLScriptType( const rtl::OUString& Text, sal_Int32 nPos );
    static sal_Int16 getCTLScriptType( const rtl::OUString& Text, sal_Int32 nPos );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
