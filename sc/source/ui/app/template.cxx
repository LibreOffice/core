/*************************************************************************
 *
 *  $RCSfile: template.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

#include "template.hxx"

SEG_EOFGLOBALS()

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(template_01)

ScTemplateDlg::ScTemplateDlg(Window * pParent, USHORT nAppResource) :
//  SfxTemplateDlg( pParent, nAppResource )
    SfxTemplateDialog( pParent )
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(template_07)

__EXPORT ScTemplateDlg::~ScTemplateDlg()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(template_02)

BOOL ScTemplateDlg::New(String &rNewName)
{
    return TRUE;
}

#pragma SEG_FUNCDEF(template_03)

void ScTemplateDlg::Edit(const String &)
{
}

#pragma SEG_FUNCDEF(template_04)

BOOL ScTemplateDlg::Delete(const String &)
{
    return TRUE;
}

#pragma SEG_FUNCDEF(template_05)

void ScTemplateDlg::InvalidateTemplates()
{
}

#pragma SEG_FUNCDEF(template_06)

void ScTemplateDlg::ToggleApplyTemplate()
{
}


// STATIC DATA -----------------------------------------------------------

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.7  2000/09/17 14:08:54  willem.vandorp
    OpenOffice header added.

    Revision 1.6  2000/08/31 16:38:18  willem.vandorp
    Header and footer replaced

    Revision 1.5  1995/01/30 21:25:46  MO
    + Dtor implementiert


      Rev 1.4   30 Jan 1995 22:25:46   MO
   + Dtor implementiert

      Rev 1.3   17 Jan 1995 17:02:22   TRI
   Pragmas zur Segmentierung eingebaut

      Rev 1.2   06 Dec 1994 11:26:22   MO
   SfxTemplateDlg -> SfxTemplateDialog

      Rev 1.1   09 Nov 1994 19:54:18   NN
   SfxTemplateDlg Konstruktor angepasst

      Rev 1.0   08 Nov 1994 16:42:30   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


