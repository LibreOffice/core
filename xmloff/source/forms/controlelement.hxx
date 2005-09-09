/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlelement.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:04:04 $
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
#define _XMLOFF_FORMS_CONTROLELEMENT_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlElement
    //=====================================================================
    /** helper for translating between control types and XML tags
    */
    class OControlElement
    {
    public:
        enum ElementType
        {
            TEXT = 0,
            TEXT_AREA,
            PASSWORD,
            FILE,
            FORMATTED_TEXT,
            FIXED_TEXT,
            COMBOBOX,
            LISTBOX,
            BUTTON,
            IMAGE,
            CHECKBOX,
            RADIO,
            FRAME,
            IMAGE_FRAME,
            HIDDEN,
            GRID,
            VALUERANGE,
            GENERIC_CONTROL,

            UNKNOWN // must be the last element
        };

    protected:
        /** ctor.
            <p>This default constructor is protected, 'cause this class is not intended to be instantiated
            directly. Instead, the derived classes should be used.</p>
        */
        OControlElement() { }

    public:
        /** retrieves the tag name to be used to describe a control of the given type

            <p>The retuned string is the pure element name, without any namespace.</p>

            @param  _eType
                the element type
        */
        static const sal_Char* getElementName(ElementType _eType);
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_CONTROLELEMENT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4.402.1  2005/09/05 14:38:54  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.4  2003/12/11 12:07:32  kz
 *  INTEGRATION: CWS frmcontrols01 (1.3.62); FILE MERGED
 *  2003/10/22 13:17:42 fs 1.3.62.1: #21277# new implementations for exporting/importing scrollbar controls as form:value-range elements
 *
 *  Revision 1.3.62.1  2003/10/22 13:17:42  fs
 *  #21277# new implementations for exporting/importing scrollbar controls as form:value-range elements
 *
 *  Revision 1.3  2003/06/12 09:49:08  vg
 *  INTEGRATION: CWS mh11rc (1.2.152); FILE MERGED
 *  2003/06/06 11:08:47 mh 1.2.152.1: join: from cws_srx644_ooo11beta2
 *
 *  Revision 1.2.152.1  2003/06/06 11:08:47  mh
 *  join: from cws_srx644_ooo11beta2
 *
 *  Revision 1.2.130.1  2003/05/21 13:37:06  waratah
 *  1858: Simple patch to comment out trailing text after #endif, waring removal
 *
 *  Revision 1.2  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.1  2000/12/06 17:30:27  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 06.12.00 14:00:25  fs
 ************************************************************************/

