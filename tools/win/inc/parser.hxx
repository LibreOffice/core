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
#ifndef _PARSER_HXX
#define _PARSER_HXX

#if defined WNT

#include <tools/string.hxx>
#include <winshell.hxx>

void * NewBinaryFromString( const String & rBinStr );
String CreateStringFromData( const void *pData, ULONG nBytes );

String CreateStringFromItemIDList( const CItemIDList & rIDList );

String GetURLFromHostNotation( const String & rPath );
String GetHostNotationFromURL( const String & rURL );

CItemIDList MakeIDToken( const String &rToken );
CItemIDList ParseSpecialURL( const String & rURL );

#endif

#endif // _PARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
