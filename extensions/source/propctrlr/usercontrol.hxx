/*************************************************************************
 *
 *  $RCSfile: usercontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:04:00 $
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

#ifndef _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#include "commoncontrol.hxx"
#endif
#define _ZFORLIST_DECLARE_TABLE
#ifndef _FMTFIELD_HXX_
#include <svtools/fmtfield.hxx>
#endif

class SvNumberFormatsSupplierObj;

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OFormatDescriptionControl
    //========================================================================
    class OFormatDescriptionControl : public OCommonBehaviourControl, FormattedField
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OFormatDescriptionControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP);

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            virtual void                    SetFormatSupplier(const SvNumberFormatsSupplierObj* pSupplier);
    };

    //========================================================================
    //= FormatDescription
    //========================================================================
    struct FormatDescription
    {
        SvNumberFormatsSupplierObj*     pSupplier;
        sal_Int32                       nKey;
    };

    //========================================================================
    //= OFormattedNumericControl
    //========================================================================
    class OFormattedNumericControl : public OCommonBehaviourControl, FormattedField
    {
            sal_Int32                       m_nLastDecimalDigits;

    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OFormattedNumericControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP);
                                            ~OFormattedNumericControl();

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            virtual void                    SetFormatDescription(const FormatDescription& rDesc);

            // make some FormattedField methods available
            virtual void                    SetDecimalDigits(sal_uInt16 nPrecision) { FormattedField::SetDecimalDigits(nPrecision); m_nLastDecimalDigits = nPrecision; }
            virtual void                    SetDefaultValue(double dDef) { FormattedField::SetDefaultValue(dDef); }
            virtual void                    EnableEmptyField(sal_Bool bEnable) { FormattedField::EnableEmptyField(bEnable); }
            virtual void                    SetThousandsSep(sal_Bool bEnable) { FormattedField::SetThousandsSep(bEnable); }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_

