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
#ifndef _SOERR_HXX
#define _SOERR_HXX

#include <tools/errcode.hxx>

// Fehler Codes
#define RID_SO_ERROR_HANDLER	32000

#define SO_ERR() (ERRCODE_AREA_SO | ERRCODE_CLASS_SO)
#define SO_WRN() (ERRCODE_AREA_SO | ERRCODE_CLASS_SO | ERRCODE_WARNING_MASK)

#define ERRCODE_SO_GENERALERROR 			(SO_ERR() | 1)
#define ERRCODE_SO_CANT_BINDTOSOURCE    	(SO_ERR() | 2)
#define ERRCODE_SO_NOCACHE_UPDATED			(SO_ERR() | 3)
#define ERRCODE_SO_SOMECACHES_NOTUPDATED	(SO_WRN() | 4)
#define ERRCODE_SO_MK_UNAVAILABLE			(SO_ERR() | 5)
#define ERRCODE_SO_E_CLASSDIFF				(SO_ERR() | 6)
#define ERRCODE_SO_MK_NO_OBJECT				(SO_ERR() | 7)
#define ERRCODE_SO_MK_EXCEEDED_DEADLINE		(SO_ERR() | 8)
#define ERRCODE_SO_MK_CONNECT_MANUALLY   	(SO_ERR() | 9)
#define ERRCODE_SO_MK_INTERMEDIATE_INTERFACE_NOT_SUPPORTED	(SO_ERR() | 10)
#define ERRCODE_SO_NO_INTERFACE				(SO_ERR() | 11)
#define ERRCODE_SO_OUT_OF_MEMORY			(SO_ERR() | 12)
#define ERRCODE_SO_MK_SYNTAX				(SO_ERR() | 13)
#define ERRCODE_SO_MK_REDUCED_TO_SELF		(SO_WRN() | 14)
#define ERRCODE_SO_MK_NO_INVERSE            (SO_ERR() | 15)
#define ERRCODE_SO_MK_NO_PREFIX				(SO_ERR() | 16)
#define ERRCODE_SO_MK_HIM					(SO_WRN() | 17)
#define ERRCODE_SO_MK_US					(SO_WRN() | 18)
#define ERRCODE_SO_MK_ME					(SO_WRN() | 19)
#define ERRCODE_SO_MK_NOT_BINDABLE			(SO_ERR() | 20)
#define ERRCODE_SO_NOT_IMPLEMENTED			(SO_ERR() | 21)
#define ERRCODE_SO_MK_NO_STORAGE			(SO_ERR() | 22)
#define ERRCODE_SO_FALSE					(SO_WRN() | 23)
#define ERRCODE_SO_MK_NEED_GENERIC  		(SO_ERR() | 24)
#define ERRCODE_SO_PENDING			 		(SO_ERR() | 25)
#define ERRCODE_SO_NOT_INPLACEACTIVE		(SO_ERR() | 26)
#define ERRCODE_SO_LINDEX					(SO_ERR() | 27)
#define ERRCODE_SO_CANNOT_DOVERB_NOW		(SO_WRN() | 28)
#define ERRCODE_SO_OLEOBJ_INVALIDHWND		(SO_WRN() | 29)
#define ERRCODE_SO_NOVERBS					(SO_ERR() | 30)
#define ERRCODE_SO_INVALIDVERB				(SO_WRN() | 31)
#define ERRCODE_SO_MK_CONNECT				(SO_ERR() | 32)
#define ERRCODE_SO_NOTIMPL					(SO_ERR() | 33)
#define ERRCODE_SO_MK_CANTOPENFILE          (SO_ERR() | 34)

// Fehler Contexte
#define RID_SO_ERRCTX			32001

#define ERRCTX_SO_DOVERB		1



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
