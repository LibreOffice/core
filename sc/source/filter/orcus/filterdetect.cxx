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
#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <unotools/mediadescriptor.hxx>
#include <tools/stream.hxx>
#include <orcus_utils.hxx>

#include <orcus/format_detection.hpp>
#include <orcus/orcus_json.hpp>
#include <orcus/orcus_xml.hpp>

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
    comphelper::SequenceAsHashMap aMediaDescriptor(rMediaDescSeq);
    bool bAborted
        = aMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ABORTED, false);
    if (bAborted)
        return OUString();

    OUString aType; // type to test against
    aMediaDescriptor[utl::MediaDescriptor::PROP_TYPENAME] >>= aType;
    if (aType.isEmpty())
        return OUString();

    static const std::unordered_map<OUString, orcus::format_t> aMap = {
        { "Gnumeric XML", orcus::format_t::gnumeric },
        { "calc_MS_Excel_2003_XML", orcus::format_t::xls_xml },
        { "Apache Parquet", orcus::format_t::parquet },
        { "generic_XML", orcus::format_t::xml },
        { "generic_JSON", orcus::format_t::json },
    };

    orcus::format_t eFormat{};

    if (auto it = aMap.find(aType); it != aMap.end())
        eFormat = it->second;
    else
        return OUString();

    css::uno::Reference<css::io::XInputStream> xInputStream(
        aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM], css::uno::UNO_QUERY);

    CopiedTempStream aTemp(xInputStream);
    auto aContent = toFileContent(aTemp.getFileName());
    bool bValid = orcus::detect(aContent.str(), eFormat);
    if (!bValid)
        return OUString();

    switch (eFormat)
    {
        case orcus::format_t::json:
            // make sure this JSON doc has at least one linkable range
            bValid = orcus::orcus_json::has_range(aContent.str());
            break;
        case orcus::format_t::xml:
            // make sure this XML doc has at least one linkable range
            bValid = orcus::orcus_xml::has_range(aContent.str());
            break;
        default:;
    }

    return bValid ? aType : OUString();
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sc_OrcusFormatDetect_get_implementation(css::uno::XComponentContext*,
                                                          css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OrcusFormatDetect());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
