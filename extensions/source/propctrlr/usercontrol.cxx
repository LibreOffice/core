/*************************************************************************
 *
 *  $RCSfile: usercontrol.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-13 15:36:13 $
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
#include "usercontrol.hxx"
#endif

#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //==================================================================
    // class OFormatDescriptionControl
    //==================================================================
    //------------------------------------------------------------------
    OFormatDescriptionControl::OFormatDescriptionControl( Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,FormattedField(pParent, nWinStyle)
    {
        SetModifyHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));

        autoSizeWindow();
    }

    //------------------------------------------------------------------
    long OFormatDescriptionControl::PreNotify( NotifyEvent& rNEvt )
    {
        // want to handle two keys myself : Del/Backspace should empty the window (setting my prop to "standard" this way)
        if (EVENT_KEYINPUT == rNEvt.GetType())
        {
            sal_uInt16 nKey = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

            if ((KEY_DELETE == nKey) || (KEY_BACKSPACE == nKey))
            {
                SetText(String());
                ModifiedHdl(this);
                return sal_True;
            }
        }

        if (OCommonBehaviourControl::handlePreNotify(rNEvt))
            return sal_True;

        // it wasn't a PropCommonControl event => let our window handle it
        return FormattedField::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------
    void OFormatDescriptionControl::SetProperty(const ::rtl::OUString& rString, sal_Bool bIsUnknown)
    {
        if (bIsUnknown || (rString == m_sStandardString) || !rString.getLength())
            // unknown or standard -> no text
            SetText(String());
        else
        {
            // else set the new format key, the text will be reformatted
            SetValue(1234.56789);
            SetFormatKey(String(rString).ToInt32());
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString OFormatDescriptionControl::GetProperty() const
    {
        if (!GetText().Len())
            return m_sStandardString;
        else
            return String::CreateFromInt32(GetFormatKey());
    }

    //------------------------------------------------------------------
    void OFormatDescriptionControl::SetFormatSupplier(const SvNumberFormatsSupplierObj* pSupplier)
    {
        if (pSupplier)
        {
            TreatAsNumber(sal_True);

            SvNumberFormatter* pFormatter = pSupplier->GetNumberFormatter();
            SetFormatter(pFormatter, sal_True);
            SetValue(1234.56789);
        }
        else
        {
            TreatAsNumber(sal_False);
            SetFormatter(NULL, sal_True);
            SetText(String());
        }
    }

    //==================================================================
    // class OFormattedNumericControl
    //==================================================================
    DBG_NAME(OFormattedNumericControl);
    //------------------------------------------------------------------
    OFormattedNumericControl::OFormattedNumericControl( Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,FormattedField(pParent, nWinStyle)
    {
        DBG_CTOR(OFormattedNumericControl,NULL);

        SetModifyHdl(LINK( this, OFormattedNumericControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OFormattedNumericControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OFormattedNumericControl, LoseFocusHdl));

        autoSizeWindow();

        TreatAsNumber(sal_True);

        m_nLastDecimalDigits = GetDecimalDigits();
    }

    //------------------------------------------------------------------
    OFormattedNumericControl::~OFormattedNumericControl()
    {
        DBG_DTOR(OFormattedNumericControl,NULL);
    }

    //------------------------------------------------------------------
    long OFormattedNumericControl::PreNotify( NotifyEvent& rNEvt )
    {
        if (OCommonBehaviourControl::handlePreNotify(rNEvt))
            return sal_True;

        // it wasn't a PropCommonControl event => let our window handle it
        return FormattedField::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------
    void OFormattedNumericControl::SetProperty(const ::rtl::OUString& rString, sal_Bool bIsUnknown)
    {
        if (bIsUnknown || (rString == m_sStandardString) || !rString.getLength())
            // unknown or standard -> no text
            SetText(String());
        else
        {
            SetValue(String(rString).ToDouble());
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString OFormattedNumericControl::GetProperty() const
    {
        if (!GetText().Len())
            return m_sStandardString;
        else
        {
            String sReturn;
            SolarMath::DoubleToString(
                sReturn, const_cast<OFormattedNumericControl*>(this)->GetValue(), 'F', m_nLastDecimalDigits,
                '.', sal_True);
                // always use a '.' (instead of the decimal separator given by m_aUsedInternational) :
                // when the returned string is interpreted nobody knows about this control or the used
                // international, so by convention we always use '.' for describing floating point numbers
                // 27.07.99 - 67901 - FS
            return sReturn;
        }
    }

    //------------------------------------------------------------------
    void OFormattedNumericControl::SetFormatDescription(const FormatDescription& rDesc)
    {
        if (rDesc.pSupplier)
        {
            TreatAsNumber(sal_True);

            SvNumberFormatter* pFormatter = rDesc.pSupplier->GetNumberFormatter();
            if (pFormatter != GetFormatter())
                SetFormatter(pFormatter, sal_True);
            SetFormatKey(rDesc.nKey);

            const SvNumberformat* pEntry = GetFormatter()->GetEntry(GetFormatKey());
            switch (pEntry->GetType() & ~NUMBERFORMAT_DEFINED)
            {
                case NUMBERFORMAT_NUMBER:
                case NUMBERFORMAT_CURRENCY:
                case NUMBERFORMAT_SCIENTIFIC:
                case NUMBERFORMAT_FRACTION:
                case NUMBERFORMAT_PERCENT:
                    m_nLastDecimalDigits = GetDecimalDigits();
                    break;
                case NUMBERFORMAT_DATETIME:
                case NUMBERFORMAT_DATE:
                case NUMBERFORMAT_TIME:
                    m_nLastDecimalDigits = 7;
                    break;
                default:
                    m_nLastDecimalDigits = 0;
                    break;
            }

        }
        else
        {
            TreatAsNumber(sal_False);
            SetFormatter(NULL, sal_True);
            SetText(String());
            m_nLastDecimalDigits = 0;
        }
    }

//............................................................................
} // namespace pcr
//............................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/12 11:34:02  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 09.01.01 10:23:42  fs
 ************************************************************************/

