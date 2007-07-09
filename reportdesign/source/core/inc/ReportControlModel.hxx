/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportControlModel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:15 $
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

#ifndef RPT_REPORTCONTROLMODEL_HXX
#define RPT_REPORTCONTROLMODEL_HXX

#ifndef RPT_REPORTCOMPONENT_HXX
#include "ReportComponent.hxx"
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include "com/sun/star/style/VerticalAlignment.hpp"
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include "com/sun/star/awt/FontDescriptor.hpp"
#endif
#ifndef _COM_SUN_STAR_REPORT_XFORMATCONDITION_HPP_
#include "com/sun/star/report/XFormatCondition.hpp"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include "com/sun/star/container/XContainer.hpp"
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#include <com/sun/star/lang/Locale.hpp>
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif


namespace reportdesign
{
    bool operator==( const ::com::sun::star::awt::FontDescriptor& _lhs, const ::com::sun::star::awt::FontDescriptor& _rhs );

    inline bool operator!=( const ::com::sun::star::awt::FontDescriptor& _lhs, const ::com::sun::star::awt::FontDescriptor& _rhs )
    {
        return !( _lhs == _rhs );
    }

    struct OFormatProperties
    {
        ::sal_Int16                                         nAlign;
        ::com::sun::star::awt::FontDescriptor               aFontDescriptor;
        ::com::sun::star::lang::Locale                      aCharLocale;
        ::sal_Int16                                         nFontEmphasisMark;
        ::sal_Int16                                         nFontRelief;
        ::sal_Int32                                         nTextColor;
        ::sal_Int32                                         nTextLineColor;
        ::sal_Int32                                         nCharUnderlineColor;
        ::sal_Int32                                         nBackgroundColor;
        ::rtl::OUString                                     sCharCombinePrefix;
        ::rtl::OUString                                     sCharCombineSuffix;
        ::rtl::OUString                                     sHyperLinkURL;
        ::rtl::OUString                                     sHyperLinkTarget;
        ::rtl::OUString                                     sHyperLinkName;
        ::rtl::OUString                                     sVisitedCharStyleName;
        ::rtl::OUString                                     sUnvisitedCharStyleName;
        short                                               aVerticalAlignment;
        ::sal_Int16                                         nCharEscapement;
        ::sal_Int16                                         nCharCaseMap;
        ::sal_Int16                                         nCharKerning;
        ::sal_Int8                                          nCharEscapementHeight;
        ::sal_Bool                                          m_bBackgroundTransparent;
        ::sal_Bool                                          bCharFlash;
        ::sal_Bool                                          bCharAutoKerning;
        ::sal_Bool                                          bCharCombineIsOn;
        ::sal_Bool                                          bCharHidden;
        ::sal_Bool                                          bCharShadowed;
        ::sal_Bool                                          bCharContoured;
        OFormatProperties();
    };
    class OReportControlModel
    {
        void checkIndex(sal_Int32 _nIndex);
        OReportControlModel(OReportControlModel&);
        void operator =(OReportControlModel&);
    public:
        ::cppu::OInterfaceContainerHelper                   aContainerListeners;
        OReportComponentProperties                          aComponent;
        OFormatProperties                                   aFormatProperties;
        ::com::sun::star::container::XContainer*            m_pOwner;
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition> >
                                                            m_aFormatConditions;
        osl::Mutex&                                         m_rMutex;
        ::rtl::OUString                                     aDataField;
        ::rtl::OUString                                     aConditionalPrintExpression;
        sal_Bool                                            bPrintWhenGroupChange;

        OReportControlModel(osl::Mutex& _rMutex
                            ,::com::sun::star::container::XContainer* _pOwner
                            ,::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext)
            :aContainerListeners(_rMutex)
            ,aComponent(_xContext)
            ,m_pOwner(_pOwner)
            ,m_rMutex(_rMutex)
            ,bPrintWhenGroupChange(sal_False)
        {}

        void dispose(oslInterlockedCount& _rRefCount);

        // XContainer
        void addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        void removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        ::sal_Bool hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

        // XIndexReplace
        void replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexContainer
        void insertByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        void removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexAccess
        ::sal_Int32 getCount(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        static bool isInterfaceForbidden(const ::com::sun::star::uno::Type& _rType);
    };
}
#endif // RPT_REPORTCONTROLMODEL_HXX

