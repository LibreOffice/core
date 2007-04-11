/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SettingsExportHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:21:44 $
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

#ifndef _XMLOFF_SETTINGSEXPORTHELPER_HXX
#define _XMLOFF_SETTINGSEXPORTHELPER_HXX

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_FORMULA_SYMBOLDESCRIPTOR_HPP_
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

namespace com
{
    namespace sun { namespace star {
        namespace container { class XNameAccess; class XIndexAccess; }
        namespace util { struct DateTime; }
    } }
}
class XMLSettingsExportHelper
{
    SvXMLExport&    rExport;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringSubstitution > mxStringSubsitution;

    const rtl::OUString msPrinterIndependentLayout;
    const rtl::OUString msColorTableURL;
    const rtl::OUString msLineEndTableURL;
    const rtl::OUString msHatchTableURL;
    const rtl::OUString msDashTableURL;
    const rtl::OUString msGradientTableURL;
    const rtl::OUString msBitmapTableURL;

    void ManipulateSetting( com::sun::star::uno::Any& rAny, const rtl::OUString& rName ) const;

    void CallTypeFunction(const com::sun::star::uno::Any& rAny,
                        const rtl::OUString& rName) const;

    void exportBool(const sal_Bool bValue, const rtl::OUString& rName) const;
    void exportByte(const sal_Int8 nValue, const rtl::OUString& rName) const;
    void exportShort(const sal_Int16 nValue, const rtl::OUString& rName) const;
    void exportInt(const sal_Int32 nValue, const rtl::OUString& rName) const;
    void exportLong(const sal_Int64 nValue, const rtl::OUString& rName) const;
    void exportDouble(const double fValue, const rtl::OUString& rName) const;
    void exportString(const rtl::OUString& sValue, const rtl::OUString& rName) const;
    void exportDateTime(const com::sun::star::util::DateTime& aValue, const rtl::OUString& rName) const;
    void exportSequencePropertyValue(
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps,
        const rtl::OUString& rName) const;
    void exportbase64Binary(
        const com::sun::star::uno::Sequence<sal_Int8>& aProps,
        const rtl::OUString& rName) const;
    void exportMapEntry(const com::sun::star::uno::Any& rAny,
                        const rtl::OUString& rName,
                        const sal_Bool bNameAccess) const;
    void exportNameAccess(
        const com::sun::star::uno::Reference<com::sun::star::container::XNameAccess>& aNamed,
        const rtl::OUString& rName) const;
    void exportIndexAccess(
        const com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> aIndexed,
        const rtl::OUString rName) const;

    void exportSymbolDescriptors(
                    const com::sun::star::uno::Sequence < com::sun::star::formula::SymbolDescriptor > &rProps,
                    const rtl::OUString rName) const;
    void exportForbiddenCharacters(
                    const com::sun::star::uno::Any &rAny,
                    const rtl::OUString rName) const;

public:
    XMLSettingsExportHelper(SvXMLExport& rExport);
    ~XMLSettingsExportHelper();

    void exportSettings(
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps,
        const rtl::OUString& rName) const;
};

#endif

