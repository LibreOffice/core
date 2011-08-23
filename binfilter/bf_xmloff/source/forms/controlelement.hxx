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

#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#define _XMLOFF_FORMS_CONTROLELEMENT_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
namespace binfilter {

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
            GENERIC_CONTROL,

            UNKNOWN	// must be the last element
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
            
            @param	_eType
                the element type
        */
        static const sal_Char* getElementName(ElementType _eType);
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_CONTROLELEMENT_HXX_

