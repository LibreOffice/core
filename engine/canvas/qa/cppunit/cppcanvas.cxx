/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vcl/wrkwin.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <vclfactory.hxx>
#include <canvas.hxx>

using namespace ::com::sun::star;

static std::ostream& operator<<(std::ostream& rStream, const KernArray& rArray)
{
    if (rArray.empty())
    {
        rStream << "{ }";
        return rStream;
    }

    rStream << "{ ";
    for (size_t i = 0; i < rArray.size() - 1; i++)
        rStream << rArray[i] << ", ";
    rStream << rArray.back();
    rStream << " }";
    return rStream;
}

static std::ostream& operator<<(std::ostream& rStream, const boost::container::vector<bool>& rVec)
{
    if (rVec.empty())
    {
        rStream << "{ }";
        return rStream;
    }

    rStream << "{ ";
    for (size_t i = 0; i < rVec.size() - 1; i++)
        rStream << std::boolalpha << bool(rVec[i]) << ", ";
    rStream << std::boolalpha << bool(rVec.back());
    rStream << " }";
    return rStream;
}

#include <test/bootstrapfixture.hxx>

class CanvasTest : public test::BootstrapFixture
{
public:
    CanvasTest()
        : BootstrapFixture(true, false)
    {
    }
};

CPPUNIT_TEST_FIXTURE(CanvasTest, testTdf155810)
{
    GDIMetaFile aInputMetaFile, aOutputMetaFile;
    KernArray aDXArray;
    boost::container::vector<bool> aKashidaArray;

    // First create a meta file with a text array action that has Kashida adjustments.
    {
        ScopedVclPtrInstance<VirtualDevice> pDev;

        vcl::Font aFont(u"Noto Naskh Arabic"_ustr, u"Regular"_ustr, Size(0, 72));
        pDev->SetFont(aFont);

        aInputMetaFile.Record(pDev.get());

        OUString aText(u"خالد"_ustr);
        pDev->GetTextArray(aText, &aDXArray);

        auto nKashida = 200;
        aDXArray[0] += nKashida;
        aDXArray[2] += nKashida;
        aKashidaArray = { true, false, true, false };

        pDev->DrawTextArray(Point(0, 0), aText, aDXArray, aKashidaArray, 0, -1);

        aInputMetaFile.Stop();
    }

    // Then draw it using canvas
    {
        ScopedVclPtrInstance<VirtualDevice> pDev;

        aOutputMetaFile.Record(pDev.get());

        vclcanvas::Canvas aCanvas(*pDev);

        ::basegfx::B2DHomMatrix aMatrix;
        aMatrix.scale(1, 1);
        auto pRenderer = cppcanvas::VCLFactory::createRenderer(aCanvas, aMatrix, aInputMetaFile);
        pRenderer->draw();

        aOutputMetaFile.Stop();
    }

    // Then check that the text array drawn by the canvas renderer didn’t loose
    // the Kashida insertion positions.
    {
        auto pInputAction = aInputMetaFile.GetAction(aInputMetaFile.GetActionSize() - 1);
        auto pOutputAction = aOutputMetaFile.GetAction(aOutputMetaFile.GetActionSize() - 2);

        CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTARRAY, pInputAction->GetType());
        CPPUNIT_ASSERT_EQUAL(pInputAction->GetType(), pOutputAction->GetType());

        MetaTextArrayAction* pInputTextAction = static_cast<MetaTextArrayAction*>(pInputAction);
        MetaTextArrayAction* pOutputTextAction = static_cast<MetaTextArrayAction*>(pOutputAction);

        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetDXArray(), aDXArray);
        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetDXArray(), pOutputTextAction->GetDXArray());

        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetKashidaArray(), aKashidaArray);
        CPPUNIT_ASSERT_EQUAL(pInputTextAction->GetKashidaArray(),
                             pOutputTextAction->GetKashidaArray());
    }

    {
        auto pOutputAction = aOutputMetaFile.GetAction(aOutputMetaFile.GetActionSize() - 2);

        CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTARRAY, pOutputAction->GetType());

        MetaTextArrayAction* pOutputTextAction = static_cast<MetaTextArrayAction*>(pOutputAction);
        boost::container::vector<bool> aSubsetKashidaArray({ true, false, true, false });

        CPPUNIT_ASSERT_EQUAL(aSubsetKashidaArray, pOutputTextAction->GetKashidaArray());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
