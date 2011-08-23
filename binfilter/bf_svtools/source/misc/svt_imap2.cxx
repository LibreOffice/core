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

// MARKER(update_precomp.py): autogen include statement, do not remove


#ifdef WIN
#include <sysdep.hxx>
#endif
#include <string.h>

#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/wrkwin.hxx>
#include <sot/formats.hxx>

#include "urihelper.hxx"
#include <bf_svtools/imap.hxx>
#include <bf_svtools/imap.hxx>
#include <bf_svtools/imapobj.hxx>
#include <bf_svtools/imaprect.hxx>
#include <bf_svtools/imapcirc.hxx>
#include <bf_svtools/imappoly.hxx>

#define NOTEOL(c) ((c)!='\0')

namespace binfilter
{

TYPEINIT0_AUTOFACTORY( ImageMap );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
