/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linedescriptor.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:16:00 $
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
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#define _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROL_HXX_
#include "brwcontrol.hxx"
#endif
#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

#define LINE_FLAG_EVENT             0x0001
    // TODO: this could be handled with virtual properties, and a IPropertyHandler implementation, and PROP_FLAGs

    struct PropertyUIDescriptor
    {
        /// the type of the control which should be used to represent this property at the UI
        BrowserControlType          eControlType;

        /** a user-defined control, if the property cannot be represented by a standard control
            Only evaluated if eControlType equals BCT_USERDEFINED
        */
        IBrowserControl*            pControl;

        /** a list of possible values for the property
            Only evaluated if eControlType equals BCT_LISTBOX or BCT_COMBOBOX
        */
        ::std::vector< ::rtl::OUString >
                                    aListValues;

        /** a help id to give to the browse button, if necessary.
            The line has a browse button if and only if this is not 0.
        */
        sal_uInt32                  nButtonHelpId;

        /** if the property supports two buttons, this is the help id of the second one
            The line has a second button if and only if this is not 0.
            Only evaluated if nButtonHelpId is not 0.
        */
        sal_uInt32                  nAdditionalButtonHelpId;

        /** an image to display on the browse button. If not set, the button
            will contain the usual "..." text for browse buttons.
            Only evaluated if nButtonHelpId is not 0.
        */
        Image                       aButtonImage;

        /** an image to display on the second button.
            Only evaluated if nAdditionalButtonHelpId is not 0.
        */
        Image                       aAdditionalButtonImage;

        /** should the UI for this property get an indent, so it looks like a sub-aspect
            of some super-ordinate property?
        */
        sal_Bool                    bIndent : 1;

        /// is the UI for this property locked (i.e. read-only)?
        sal_Bool                    bIsLocked : 1;

        /** does the (numeric) property have a min- and max value
            If this is false, nMinValue and nMaxValue are to be ignored
        */
        sal_Bool                    bHaveMinMax : 1;

        /// for numeric fields: number of digits
        sal_uInt16                  nDigits;
        /// for numeric fields only: minimum value
        sal_Int32                   nMinValue;
        /// for numeric fields only: maximum value
        sal_Int32                   nMaxValue;
        /// for numeric fields only: measurement unit to display values
        FieldUnit                   eDisplayUnit;
        /// for numeric fields only: measurement unit to transfer values
        FieldUnit                   eValueUnit;

        PropertyUIDescriptor()
        {
            clear();
        }

        void clear()
        {
            eControlType = BCT_UNDEFINED;
            pControl = NULL;
            aListValues.resize( 0 );
            nButtonHelpId = nAdditionalButtonHelpId = 0;
            aButtonImage = aAdditionalButtonImage = Image();
            bIndent = bIsLocked = bHaveMinMax = sal_False;
            nDigits = 0;
            nMinValue = 0;
            nMaxValue = -1;
            eDisplayUnit = eValueUnit = FUNIT_NONE;
        }
    };

    class IBrowserControl;
    //========================================================================
    //= OLineDescriptor
    //========================================================================
    struct OLineDescriptor : public PropertyUIDescriptor
    {
        String                      sName;          // the name of the property
        String                      sTitle;         // the human readable label for the property
        String                      sValue;         // the current value of the property, in it's string representation
        sal_uInt32                  nHelpId;        // a help id to give to the input control

        sal_uInt16                  nFlags;                 // special flag, see LINE_FLAG_*
        sal_Bool                    bUnknownValue    : 1;   // is the property value currently "unknown"?
        sal_Bool                    bHasDefaultValue : 1;   // does the property support a "standard" value?

        // TODO: this class is a wild mix of property meta data, and information about a current property
        // value. Can this be separated? Doesn't part of this belong into IPropertyInfoService?

        OLineDescriptor()
        {
            clear();
        }

        void clear()
        {
            PropertyUIDescriptor::clear();

            sName = sTitle = sValue = String();
            nHelpId = 0;
            nFlags = 0;
            bUnknownValue = bHasDefaultValue = sal_False;
        }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

