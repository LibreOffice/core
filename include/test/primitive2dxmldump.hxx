/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_PRIMITIVE2DXMLDUMP_HXX
#define INCLUDED_TEST_PRIMITIVE2DXMLDUMP_HXX

#include <sal/config.h>
#include <test/testdllapi.hxx>
#include <libxml/tree.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vector>

namespace tools { class XmlWriter; }

class OOO_DLLPUBLIC_TEST Primitive2dXmlDump final
{
private:
    std::vector<bool> maFilter;
    void decomposeAndWrite(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence, tools::XmlWriter& rWriter);

public:
    Primitive2dXmlDump();
    ~Primitive2dXmlDump();

    /** The actual result that will be used for testing.

        This function normally uses a SvMemoryStream for its operation; but
        can use a physical file when a filename is specified in
        pTempStreamName - this is useful when creating the test, to dump the
        file for examination.
    */
    xmlDocPtr dumpAndParse(const drawinglayer::primitive2d::Primitive2DContainer& aPrimitive2DSequence, const OUString& rTempStreamName = OUString());
};

#endif // INCLUDED_TEST_PRIMITIVE2DXMLDUMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
