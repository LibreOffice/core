/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formatnormalizer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:16 $
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

#ifndef REPORTDESIGN_FORMATNORMALIZER_HXX
#define REPORTDESIGN_FORMATNORMALIZER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFORMATTEDFIELD_HPP_
#include <com/sun/star/report/XFormattedField.hpp>
#endif
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <vector>

//........................................................................
namespace rptui
{
//........................................................................

    class OReportModel;
    //====================================================================
    //= FormatNormalizer
    //====================================================================
    class FormatNormalizer : public ::boost::noncopyable
    {
    public:
        struct Field
        {
            ::rtl::OUString sName;
            sal_Int32       nDataType;
            sal_Int32       nScale;
            sal_Bool        bIsCurrency;

            Field() : sName(), nDataType( 0 ), nScale( 0 ), bIsCurrency( 0 ) { }
        };
        typedef ::std::vector< Field >  FieldList;

    private:
        const OReportModel&                                                                 m_rModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >     m_xReportDefinition;

        /// is our field list dirty?
        FieldList                       m_aFields;
        bool                            m_bFieldListDirty;

    public:
        FormatNormalizer( const OReportModel& _rModel );
        ~FormatNormalizer();

        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    private:
        bool    impl_lateInit();

        void    impl_onDefinitionPropertyChange( const ::rtl::OUString& _rChangedPropName );
        void    impl_onFormattedProperttyChange( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _rxFormatted, const ::rtl::OUString& _rChangedPropName );

        bool    impl_ensureUpToDateFieldList_nothrow();

        void    impl_adjustFormatToDataFieldType_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _rxFormatted );
    };

//........................................................................
} // namespace rptui
//........................................................................

#endif // REPORTDESIGN_FORMATNORMALIZER_HXX
