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
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XComponent> m_xDstDoc;

public:
    explicit MathTypeFilter(const uno::Reference<uno::XComponentContext>& xContext);
    virtual ~MathTypeFilter();

    // XFilter
    virtual sal_Bool SAL_CALL filter(const uno::Sequence<beans::PropertyValue>& rDescriptor) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel() throw (uno::RuntimeException, std::exception) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument(const uno::Reference<lang::XComponent>& xDoc) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (uno::RuntimeException, std::exception) override;
    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw (uno::RuntimeException, std::exception) override;
};

MathTypeFilter::MathTypeFilter(const uno::Reference< uno::XComponentContext >& rxContext)
    : m_xContext(rxContext)
{
}

MathTypeFilter::~MathTypeFilter()
{
}

sal_Bool MathTypeFilter::filter(const uno::Sequence<beans::PropertyValue>& rDescriptor) throw(uno::RuntimeException, std::exception)
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
                    if (SmModel* pModel = dynamic_cast<SmModel*>(m_xDstDoc.get()))
                    {
                        SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
                        OUString aText = pDocShell->GetText();
                        MathType aEquation(aText);
                        bSuccess = aEquation.Parse(aStorage) == 1;
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
        SAL_WARN("starmath", "Exception caught: " << rException.Message);
    }
    return bSuccess;
}

void MathTypeFilter::cancel() throw(uno::RuntimeException, std::exception)
{
}

void MathTypeFilter::setTargetDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    m_xDstDoc = xDoc;
}

OUString MathTypeFilter::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Math.MathTypeFilter");
}

sal_Bool MathTypeFilter::supportsService(const OUString& rServiceName) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> MathTypeFilter::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.document.ImportFilter")
    };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* SAL_CALL com_sun_star_comp_Math_MathTypeFilter_get_implementation(uno::XComponentContext* pComponent, uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new MathTypeFilter(pComponent));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
