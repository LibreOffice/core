/*************************************************************************
 *
 *  $RCSfile: fmservs.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:03:16 $
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
#ifndef _SVX_FMSERVS_HXX
#define _SVX_FMSERVS_HXX

#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif

namespace svxform
{

    DECLARE_CONSTASCII_USTRING(FM_NUMBER_FORMATTER);

    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_EDIT);      // alter service name (5.0)
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_TEXTFIELD);

    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_LISTBOX);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_COMBOBOX);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_RADIOBUTTON);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_GROUPBOX);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_FIXEDTEXT);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_COMMANDBUTTON);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_CHECKBOX);

    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_GRID);      // alter service name (5.0)
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_GRIDCONTROL);

    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_IMAGEBUTTON);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_FILECONTROL);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_TIMEFIELD);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_DATEFIELD);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_NUMERICFIELD);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_CURRENCYFIELD);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_PATTERNFIELD);
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_FORMATTEDFIELD);

    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_HIDDEN);    // alter service name (5.0)
    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_HIDDENCONTROL);

    DECLARE_CONSTASCII_USTRING(FM_COMPONENT_IMAGECONTROL);

    DECLARE_CONSTASCII_USTRING(FM_CONTROL_GRID);
    DECLARE_CONSTASCII_USTRING(FM_CONTROL_GRIDCONTROL);

    DECLARE_CONSTASCII_USTRING(FM_FORM_CONTROLLER);
    DECLARE_CONSTASCII_USTRING(SRV_SDB_CONNECTION);
    DECLARE_CONSTASCII_USTRING(SRV_SDB_INTERACTION_HANDLER);

    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_FORM);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_TEXTFIELD);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_LISTBOX);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_COMBOBOX);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_RADIOBUTTON);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_GROUPBOX);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_FIXEDTEXT);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_COMMANDBUTTON);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_CHECKBOX);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_GRIDCONTROL);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_IMAGEBUTTON);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_FILECONTROL);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_TIMEFIELD);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_DATEFIELD);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_NUMERICFIELD);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_CURRENCYFIELD);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_PATTERNFIELD);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_HIDDENCONTROL);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_IMAGECONTROL);
    DECLARE_CONSTASCII_USTRING(FM_SUN_COMPONENT_FORMATTEDFIELD);

    DECLARE_CONSTASCII_USTRING(FM_SUN_CONTROL_GRIDCONTROL);

    void ImplSmartRegisterUnoServices();

}   // namespace svxform

#endif // _SVX_FMSERVS_HXX

