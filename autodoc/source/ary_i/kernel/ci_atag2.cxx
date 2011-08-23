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

#include <precomp.h>
#include <ary_i/ci_atag2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/disdocum.hxx>


namespace ary
{
namespace inf
{

void    DocuTag_Display::Display_TextToken(
                            const csi::dsapi::DT_TextToken & ) {}
void	DocuTag_Display::Display_White() {}
void    DocuTag_Display::Display_MupType(
                            const csi::dsapi::DT_MupType & ) {}
void    DocuTag_Display::Display_MupMember(
                            const csi::dsapi::DT_MupMember & ) {}
void    DocuTag_Display::Display_MupConst(
                            const csi::dsapi::DT_MupConst & ) {}
void    DocuTag_Display::Display_Style(
                            const csi::dsapi::DT_Style & ) {}
void    DocuTag_Display::Display_EOL() {}


}   // namespace inf
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
