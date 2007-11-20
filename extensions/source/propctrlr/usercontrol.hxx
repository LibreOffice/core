/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: usercontrol.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:53:16 $
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

#ifndef _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#include "commoncontrol.hxx"
#endif
#define _ZFORLIST_DECLARE_TABLE
#ifndef _FMTFIELD_HXX_
#include <svtools/fmtfield.hxx>
#endif
#ifndef SVTOOLS_FILEURLBOX_HXX
#include <svtools/fileurlbox.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_
#include "standardcontrol.hxx"
#endif

class SvNumberFormatsSupplierObj;

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= NumberFormatSampleField
    //========================================================================
    class NumberFormatSampleField : public ControlWindow< FormattedField >
    {
    private:
        typedef ControlWindow< FormattedField > BaseClass;

    public:
        NumberFormatSampleField( Window* _pParent, WinBits _nStyle )
            :BaseClass( _pParent, _nStyle )
        {
        }

        void SetFormatSupplier( const SvNumberFormatsSupplierObj* pSupplier );

    protected:
        virtual long PreNotify( NotifyEvent& rNEvt );
    };

    //========================================================================
    //= OFormatSampleControl
    //========================================================================
    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, NumberFormatSampleField > OFormatSampleControl_Base;
    class OFormatSampleControl : public OFormatSampleControl_Base
    {
    public:
        OFormatSampleControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException);

        inline void SetFormatSupplier( const SvNumberFormatsSupplierObj* _pSupplier )
        {
            getTypedControlWindow()->SetFormatSupplier( _pSupplier );
        }

        /** returns the default preview value for the given format key
        *
        * \param _pNF the number formatter
        * \param _nFormatKey the format key
        * \return current date or time or the value 1234.56789
        */
        static double getPreviewValue(SvNumberFormatter* _pNF,sal_Int32 _nFormatKey);
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
    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< FormattedField > > OFormattedNumericControl_Base;
    class OFormattedNumericControl : public OFormattedNumericControl_Base
    {
    private:
        sal_Int32   m_nLastDecimalDigits;

    public:
        OFormattedNumericControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP);

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException);

        void SetFormatDescription( const FormatDescription& rDesc );

        // make some FormattedField methods available
        void SetDecimalDigits(sal_uInt16 nPrecision) { getTypedControlWindow()->SetDecimalDigits(nPrecision); m_nLastDecimalDigits = nPrecision; }
        void SetDefaultValue(double dDef) { getTypedControlWindow()->SetDefaultValue(dDef); }
        void EnableEmptyField(sal_Bool bEnable) { getTypedControlWindow()->EnableEmptyField(bEnable); }
        void SetThousandsSep(sal_Bool bEnable) { getTypedControlWindow()->SetThousandsSep(bEnable); }

    protected:
        ~OFormattedNumericControl();
    };

    //========================================================================
    //= OFileUrlControl
    //========================================================================
    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< ::svt::FileURLBox > > OFileUrlControl_Base;
    class OFileUrlControl : public OFileUrlControl_Base
    {
    public:
        OFileUrlControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~OFileUrlControl();
    };

    //========================================================================
    //= OTimeDurationControl
    //========================================================================
    class OTimeDurationControl : public ONumericControl
    {
    public:
        OTimeDurationControl( ::Window* pParent, WinBits nWinStyle );
        ~OTimeDurationControl();

        // XPropertyControl
        ::sal_Int16 SAL_CALL getControlType() throw (::com::sun::star::uno::RuntimeException);

    private:
        DECL_LINK( OnCustomConvert, MetricField* );
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_

