/*************************************************************************
 *
 *  $RCSfile: shellid.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SHELLID_HXX
#define _SHELLID_HXX

#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif

/*--------------------------------------------------------------------
    Beschreibung:   Mapping fuer UI-Parameter
 --------------------------------------------------------------------*/

#define SW_BASESHELL            (SFX_INTERFACE_SW_START + 1)
#define SW_TEXTSHELL            (SFX_INTERFACE_SW_START + 2)
#define SW_LISTSHELL            (SFX_INTERFACE_SW_START + 3)
#define SW_TABSHELL             (SFX_INTERFACE_SW_START + 4)
#define SW_FRAMESHELL           (SFX_INTERFACE_SW_START + 5)
#define SW_GRFSHELL             (SFX_INTERFACE_SW_START + 6)
#define SW_OLESHELL             (SFX_INTERFACE_SW_START + 7)
#define SW_DRAWSHELL            (SFX_INTERFACE_SW_START + 8)
#define SW_DRWTXTSHELL          (SFX_INTERFACE_SW_START + 9)

#define SW_INTERFACE_MODULE     (SFX_INTERFACE_SW_START + 10)
#define SW_DOCSHELL             (SFX_INTERFACE_SW_START + 11)
#define SW_VIEWSHELL            (SFX_INTERFACE_SW_START + 12)
#define SW_PAGEPREVIEW          (SFX_INTERFACE_SW_START + 13)
#define SW_GLOSDOCSHELL         (SFX_INTERFACE_SW_START + 14)
#define SW_SRC_VIEWSHELL        (SFX_INTERFACE_SW_START + 15)

#define SW_BEZIERSHELL          (SFX_INTERFACE_SW_START + 16)
#define SW_DRAWBASESHELL        (SFX_INTERFACE_SW_START + 17)
#define SW_DRAWFORMSHELL        (SFX_INTERFACE_SW_START + 18)

#define SW_WEBDOCSHELL          (SFX_INTERFACE_SW_START + 19)
#define SW_WEBVIEWSHELL         (SFX_INTERFACE_SW_START + 20)
#define SW_WEBBASESHELL         (SFX_INTERFACE_SW_START + 21)
#define SW_WEBTEXTSHELL         (SFX_INTERFACE_SW_START + 22)
#define SW_WEBFRAMESHELL        (SFX_INTERFACE_SW_START + 23)
#define SW_WEBGLOSDOCSHELL      (SFX_INTERFACE_SW_START + 24)
#define SW_WEBLISTSHELL         (SFX_INTERFACE_SW_START + 25)
#define SW_WEBTABSHELL          (SFX_INTERFACE_SW_START + 26)
#define SW_WEBGRFSHELL          (SFX_INTERFACE_SW_START + 27)
#define SW_WEBOLESHELL          (SFX_INTERFACE_SW_START + 28)
#define SW_WEBDRAWBASESHELL     (SFX_INTERFACE_SW_START + 29)
#define SW_WEBDRAWFORMSHELL     (SFX_INTERFACE_SW_START + 30)

#endif // _UIPARAM_HXX
