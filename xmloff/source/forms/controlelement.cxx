/*************************************************************************
 *
 *  $RCSfile: controlelement.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-13 13:43:42 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#include "controlelement.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlElement
    //=====================================================================
    //---------------------------------------------------------------------
    const sal_Char* OControlElement::getElementName(ElementType _eType)
    {
        switch (_eType)
        {
            case TEXT: return "text";
            case TEXT_AREA: return "textarea";
            case PASSWORD: return "password";
            case FILE: return "file";
            case FORMATTED_TEXT: return "formatted-text";
            case FIXED_TEXT: return "fixed-text";
            case COMBOBOX: return "combobox";
            case LISTBOX: return "listbox";
            case BUTTON: return "button";
            case IMAGE: return "image";
            case CHECKBOX: return "checkbox";
            case RADIO: return "radio";
            case FRAME: return "frame";
            case IMAGE_FRAME: return "image-frame";
            case HIDDEN: return "hidden";
            case GRID: return "grid";

            default: return "generic-control";
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.1  2000/12/06 17:29:46  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 06.12.00 14:02:39  fs
 ************************************************************************/

