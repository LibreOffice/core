/*************************************************************************
 *
 *  $RCSfile: property.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
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

#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#define _TOOLKIT_HELPER_PROPERTY_HXX_

#include <sal/types.h>

namespace com {
namespace sun {
namespace star {
namespace uno {
    class Type;
    class Any;
} } } }

namespace rtl {
    class OUString;
}


#define BASEPROPERTY_NOTFOUND                0

#define BASEPROPERTY_TEXT                    1  // ::rtl::OUString
#define BASEPROPERTY_BACKGROUNDCOLOR         2  // sal_Int32
#define BASEPROPERTY_FILLCOLOR               3  // sal_Int32
#define BASEPROPERTY_TEXTCOLOR               4  // sal_Int32
#define BASEPROPERTY_LINECOLOR               5  // sal_Int32
#define BASEPROPERTY_BORDER                  6  // sal_Int16
#define BASEPROPERTY_ALIGN                   7  // sal_Int16
#define BASEPROPERTY_FONTDESCRIPTOR          8  // ::com::sun::star::awt::FontDescriptor, war mal Font, aber nie gespeichert...
#define BASEPROPERTY_DROPDOWN                9  // sal_Bool
#define BASEPROPERTY_MULTILINE              10  // sal_Bool
#define BASEPROPERTY_STRINGITEMLIST         11  // UStringSequence
#define BASEPROPERTY_HSCROLL                12  // sal_Bool
#define BASEPROPERTY_VSCROLL                13  // sal_Bool
#define BASEPROPERTY_TABSTOP                14  // sal_Bool
#define BASEPROPERTY_STATE                  15  // sal_Int16
#define BASEPROPERTY_FONT_TYPE              16  // OLD: Font_Type
#define BASEPROPERTY_FONT_SIZE              17  // OLD: Font_Size
#define BASEPROPERTY_FONT_ATTRIBS           18  // OLD: Font_Attribs
#define BASEPROPERTY_DEFAULTCONTROL         19  // ::rtl::OUString (ServiceName)
#define BASEPROPERTY_LABEL                  20  // ::rtl::OUString
#define BASEPROPERTY_LINECOUNT              21  // sal_Int16
#define BASEPROPERTY_EXTDATEFORMAT          22  // sal_Int16
#define BASEPROPERTY_DATESHOWCENTURY        23  // sal_Bool
#define BASEPROPERTY_EXTTIMEFORMAT          24  // sal_Int16
#define BASEPROPERTY_NUMSHOWTHOUSANDSEP     25  // sal_Bool
#define BASEPROPERTY_CURRENCYSYMBOL         26  // ::rtl::OUString
#define BASEPROPERTY_SPIN                   27  // sal_Bool
#define BASEPROPERTY_STRICTFORMAT           28  // sal_Bool
#define BASEPROPERTY_DECIMALACCURACY        29  // sal_Int16
#define BASEPROPERTY_DATE                   30  // sal_Int32
#define BASEPROPERTY_DATEMIN                31  // sal_Int32
#define BASEPROPERTY_DATEMAX                32  // sal_Int32
#define BASEPROPERTY_TIME                   33  // sal_Int32
#define BASEPROPERTY_TIMEMIN                34  // sal_Int32
#define BASEPROPERTY_TIMEMAX                35  // sal_Int32
#define BASEPROPERTY_VALUE_INT32            36  // sal_Int32
#define BASEPROPERTY_VALUEMIN_INT32         37  // sal_Int32
#define BASEPROPERTY_VALUEMAX_INT32         38  // sal_Int32
#define BASEPROPERTY_VALUESTEP_INT32        39  // sal_Int32
#define BASEPROPERTY_EDITMASK               40  // ::rtl::OUString
#define BASEPROPERTY_LITERALMASK            41  // ::rtl::OUString
#define BASEPROPERTY_IMAGEURL               42  // ::rtl::OUString
#define BASEPROPERTY_READONLY               43  // sal_Bool
#define BASEPROPERTY_ENABLED                44  // sal_Bool
#define BASEPROPERTY_PRINTABLE              45  // sal_Bool
#define BASEPROPERTY_ECHOCHAR               46  // sal_Int16
#define BASEPROPERTY_MAXTEXTLEN             47  // sal_Int16
#define BASEPROPERTY_HARDLINEBREAKS         48  // sal_Int16
#define BASEPROPERTY_AUTOCOMPLETE           49  // sal_Bool
#define BASEPROPERTY_MULTISELECTION         50  // sal_Bool
#define BASEPROPERTY_SELECTEDITEMS          51  // INT16Sequence
#define BASEPROPERTY_VALUE_DOUBLE           52  // DOUBLE
#define BASEPROPERTY_VALUEMIN_DOUBLE        53  // DOUBLE
#define BASEPROPERTY_VALUEMAX_DOUBLE        54  // DOUBLE
#define BASEPROPERTY_VALUESTEP_DOUBLE       55  // DOUBLE
#define BASEPROPERTY_TRISTATE               56  // sal_Bool
#define BASEPROPERTY_DEFAULTBUTTON          57  // sal_Bool
#define BASEPROPERTY_HELPURL                58  // ::rtl::OUString
#define BASEPROPERTY_AUTOTOGGLE             59  // sal_Bool
#define BASEPROPERTY_FOCUSSELECTIONHIDE     60  // sal_Bool
#define BASEPROPERTY_FORMATKEY              61  // sal_Bool
#define BASEPROPERTY_FORMATSSUPPLIER        62  // ::com::sun::star::util::XNumberFormatsSupplier
#define BASEPROPERTY_EFFECTIVE_VALUE        63  // Any (double or string)
#define BASEPROPERTY_TREATASNUMBER          64  // sal_Bool
#define BASEPROPERTY_EFFECTIVE_DEFAULT      65  // Any (double or string)
#define BASEPROPERTY_EFFECTIVE_MIN          66  // Double
#define BASEPROPERTY_EFFECTIVE_MAX          67  // Double
#define BASEPROPERTY_CURSYM_POSITION        68  // sal_Bool


// Keine gebundenen Properties, werden immer aus der Property BASEPROPERTY_FONTDESCRIPTOR entnommen.
#define BASEPROPERTY_FONTDESCRIPTORPART_START           1000
#define BASEPROPERTY_FONTDESCRIPTORPART_NAME            1000    // ::rtl::OUString, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME       1001    // ::rtl::OUString, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_FAMILY          1002    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_CHARSET         1003    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT          1004    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT          1005    // Float, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_SLANT           1006    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE       1007    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT       1008    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_WIDTH           1009    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_PITCH           1010    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH       1011    // Float, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION     1012    // Float, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_KERNING         1013    // sal_Bool, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE    1014    // sal_Bool, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_TYPE            1015    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_END             1015

#define PROPERTY_ALIGN_LEFT                     0
#define PROPERTY_ALIGN_CENTER                   1
#define PROPERTY_ALIGN_RIGHT                    2

#define PROPERTY_STATE_OFF                      0
#define PROPERTY_STATE_ON                       1
#define PROPERTY_STATE_DONTCARE                 2

sal_uInt16                          GetPropertyId( const ::rtl::OUString& rPropertyName );
const ::com::sun::star::uno::Type*  GetPropertyType( sal_uInt16 nPropertyId );
const ::rtl::OUString&              GetPropertyName( sal_uInt16 nPropertyId );
sal_Int16                           GetPropertyAttribs( sal_uInt16 nPropertyId );
sal_uInt16                          GetPropertyOrderNr( sal_uInt16 nPropertyId );
sal_Bool                            DoesDependOnOthers( sal_uInt16 nPropertyId );
sal_Bool                            CompareProperties( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 );




#endif // _TOOLKIT_HELPER_PROPERTY_HXX_


