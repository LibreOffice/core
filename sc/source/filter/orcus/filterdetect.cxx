/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <unotools/mediadescriptor.hxx>
#include <tools/stream.hxx>
#include <orcus_utils.hxx>

#include <orcus/format_detection.hpp>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace
{
class OrcusFormatDetect : public ::cppu::WeakImplHelper<css::document::XExtendedFilterDetection,
                                                        css::lang::XServiceInfo>
{
public:
    explicit OrcusFormatDetect();

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual OUString SAL_CALL
    detect(css::uno::Sequence<css::beans::PropertyValue>& rMediaDescSeq) override;

private:
};

OrcusFormatDetect::OrcusFormatDetect() {}

OUString OrcusFormatDetect::getImplementationName()
{
    return u"com.sun.star.comp.sc.OrcusFilterDetect"_ustr;
}

sal_Bool OrcusFormatDetect::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> OrcusFormatDetect::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ExtendedTypeDetection"_ustr };
}

OUString OrcusFormatDetect::detect(css::uno::Sequence<css::beans::PropertyValue>& rMediaDescSeq)
{
    utl::MediaDescriptor aMediaDescriptor(rMediaDescSeq);
    bool bAborted
        = aMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ABORTED, false);
    if (bAborted)
        return OUString();

    css::uno::Reference<css::io::XInputStream> xInputStream(
        aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM], css::uno::UNO_QUERY);

    CopiedTempStream aTemp(xInputStream);
    auto aContent = toFileContent(aTemp.getFileName());
    orcus::format_t eFormat = orcus::detect(aContent.str());

    switch (eFormat)
    {
        case orcus::format_t::gnumeric:
            return u"Gnumeric XML"_ustr;
        case orcus::format_t::xls_xml:
            return u"calc_MS_Excel_2003_XML"_ustr;
        case orcus::format_t::parquet:
            return u"Apache Parquet"_ustr;
        case orcus::format_t::xml:
            return u"generic_XML"_ustr;
        case orcus::format_t::json:
            return u"generic_JSON"_ustr;
        default:;
    }

    return OUString();
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sc_OrcusFormatDetect_get_implementation(css::uno::XComponentContext*,
                                                          css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OrcusFormatDetect());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
