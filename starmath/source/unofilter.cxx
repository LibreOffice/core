/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <document.hxx>
#include <mathtype.hxx>
#include <unomodel.hxx>

using namespace ::com::sun::star;

/// Invokes the MathType importer via UNO.
class MathTypeFilter : public cppu::WeakImplHelper
    <
    document::XFilter,
    document::XImporter,
    lang::XServiceInfo
    >
{
    uno::Reference<lang::XComponent> m_xDstDoc;

public:
    MathTypeFilter();

    // XFilter
    sal_Bool SAL_CALL filter(const uno::Sequence<beans::PropertyValue>& rDescriptor) override;
    void SAL_CALL cancel() override;

    // XImporter
    void SAL_CALL setTargetDocument(const uno::Reference<lang::XComponent>& xDoc) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

MathTypeFilter::MathTypeFilter() = default;

sal_Bool MathTypeFilter::filter(const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    bool bSuccess = false;
    try
    {
        utl::MediaDescriptor aMediaDesc(rDescriptor);
        aMediaDesc.addInputStream();
        uno::Reference<io::XInputStream> xInputStream;
        aMediaDesc[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
        std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream));
        if (pStream)
        {
            if (SotStorage::IsStorageFile(pStream.get()))
            {
                tools::SvRef<SotStorage> aStorage(new SotStorage(pStream.get(), false));
                // Is this a MathType Storage?
                if (aStorage->IsStream("Equation Native"))
                {
                    if (auto pModel = dynamic_cast<SmModel*>(m_xDstDoc.get()))
                    {
                        auto pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
                        OUString aText = pDocShell->GetText();
                        MathType aEquation(aText);
                        bSuccess = aEquation.Parse(aStorage.get());
                        if (bSuccess)
                        {
                            pDocShell->SetText(aText);
                            pDocShell->Parse();
                        }
                    }
                }
            }
        }
    }
    catch (const uno::Exception& rException)
    {
        SAL_WARN("starmath", "Exception caught: " << rException);
    }
    return bSuccess;
}

void MathTypeFilter::cancel()
{
}

void MathTypeFilter::setTargetDocument(const uno::Reference< lang::XComponent >& xDoc)
{
    m_xDstDoc = xDoc;
}

OUString MathTypeFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.Math.MathTypeFilter");
}

sal_Bool MathTypeFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> MathTypeFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.document.ImportFilter")
    };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* SAL_CALL com_sun_star_comp_Math_MathTypeFilter_get_implementation(uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new MathTypeFilter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
