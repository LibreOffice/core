/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlelement.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:22:20 $
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

