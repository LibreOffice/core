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

#ifndef _HTMLFORM_HXX
#define _HTMLFORM_HXX


enum HTMLEventType
{
    HTML_ET_ONSUBMITFORM,   HTML_ET_ONRESETFORM,
    HTML_ET_ONGETFOCUS,     HTML_ET_ONLOSEFOCUS,
    HTML_ET_ONCLICK,        HTML_ET_ONCLICK_ITEM,
    HTML_ET_ONCHANGE,       HTML_ET_ONSELECT,
    HTML_ET_END
};

extern HTMLEventType __FAR_DATA aEventTypeTable[];
extern const sal_Char * __FAR_DATA aEventListenerTable[];
extern const sal_Char * __FAR_DATA aEventMethodTable[];
extern const sal_Char * __FAR_DATA aEventSDOptionTable[];
extern const sal_Char * __FAR_DATA aEventOptionTable[];




#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
