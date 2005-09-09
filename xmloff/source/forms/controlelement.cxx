/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlelement.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:03:48 $
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
            case VALUERANGE: return "value-range";

            default: return "generic-control";
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4.402.1  2005/09/05 14:38:54  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.4  2003/12/11 12:07:12  kz
 *  INTEGRATION: CWS frmcontrols01 (1.3.210); FILE MERGED
 *  2003/10/22 13:17:41 fs 1.3.210.1: #21277# new implementations for exporting/importing scrollbar controls as form:value-range elements
 *
 *  Revision 1.3.210.1  2003/10/22 13:17:41  fs
 *  #21277# new implementations for exporting/importing scrollbar controls as form:value-range elements
 *
 *  Revision 1.3  2001/02/13 13:43:42  fs
 *  text-area -> textarea
 *
 *  Revision 1.2  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.1  2000/12/06 17:29:46  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 06.12.00 14:02:39  fs
 ************************************************************************/

