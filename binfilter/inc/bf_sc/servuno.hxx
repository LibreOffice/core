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

#ifndef SC_SERVUNO_HXX
#define SC_SERVUNO_HXX

#include <com/sun/star/uno/Sequence.hxx>

class String;

namespace binfilter {

class ScDocShell;


//!	AutoFormat wirklich hier oder besser global??????

#define SC_SERVICE_SHEET		0
#define SC_SERVICE_URLFIELD		1
#define SC_SERVICE_PAGEFIELD	2
#define SC_SERVICE_PAGESFIELD	3
#define SC_SERVICE_DATEFIELD	4
#define SC_SERVICE_TIMEFIELD	5
#define SC_SERVICE_TITLEFIELD	6
#define SC_SERVICE_FILEFIELD	7
#define SC_SERVICE_SHEETFIELD	8
#define SC_SERVICE_CELLSTYLE	9
#define SC_SERVICE_PAGESTYLE	10
#define SC_SERVICE_AUTOFORMAT	11
#define SC_SERVICE_CELLRANGES	12

//	drawing layer tables
#define SC_SERVICE_GRADTAB		13
#define SC_SERVICE_HATCHTAB		14
#define SC_SERVICE_BITMAPTAB	15
#define SC_SERVICE_TRGRADTAB	16
#define SC_SERVICE_MARKERTAB	17
#define SC_SERVICE_DASHTAB		18
#define SC_SERVICE_NUMRULES		19

#define SC_SERVICE_DOCDEFLTS	20
#define SC_SERVICE_DRAWDEFLTS	21

#define SC_SERVICE_DOCSPRSETT	22
#define SC_SERVICE_DOCCONF		23

#define SC_SERVICE_IMAP_RECT	24
#define SC_SERVICE_IMAP_CIRC	25
#define SC_SERVICE_IMAP_POLY	26

// #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#define SC_SERVICE_EXPORT_GOR	27
#define SC_SERVICE_IMPORT_GOR	28
#define SC_SERVICE_EXPORT_EOR	29
#define SC_SERVICE_IMPORT_EOR	30

#define SC_SERVICE_COUNT		31
#define SC_SERVICE_INVALID		USHRT_MAX


class ScServiceProvider
{
public:
                            // pDocShell wird nicht fuer alle Services benoetigt
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                            MakeInstance( sal_uInt16 nType, ScDocShell* pDocShell );
    static ::com::sun::star::uno::Sequence<rtl::OUString> GetAllServiceNames();
    static sal_uInt16		GetProviderType(const String& rServiceName);
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
