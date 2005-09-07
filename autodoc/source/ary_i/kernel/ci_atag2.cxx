/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_atag2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:22:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include <precomp.h>
#include <ary_i/ci_atag2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/disdocum.hxx>


namespace ary
{
namespace info
{

void    DocuTag_Display::Display_TextToken(
                            const csi::dsapi::DT_TextToken & ) {}
void    DocuTag_Display::Display_MupType(
                            const csi::dsapi::DT_MupType & ) {}
void    DocuTag_Display::Display_MupMember(
                            const csi::dsapi::DT_MupMember & ) {}
void    DocuTag_Display::Display_MupConst(
                            const csi::dsapi::DT_MupConst & ) {}
void    DocuTag_Display::Display_Style(
                            const csi::dsapi::DT_Style & ) {}
void    DocuTag_Display::Display_EOL() {}


}   // namespace info
}   // namespace ary

