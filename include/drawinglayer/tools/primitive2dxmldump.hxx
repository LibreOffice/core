/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DRAWINGLAYER_TOOLS_PRIMITIVE2DXMLDUMP_HXX
#define INCLUDED_DRAWINGLAYER_TOOLS_PRIMITIVE2DXMLDUMP_HXX

#include <sal/config.h>
#include <drawinglayer/drawinglayerdllapi.h>
#include <libxml/tree.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <test/xmldocptr.hxx>
#include <vector>

namespace tools { class XmlWriter; }

namespace drawinglayer
{

class DRAWINGLAYER_DLLPUBLIC Primitive2dXmlDump
{
private:
    std::vector<bool> maFilter;

protected:
    void runDecomposeAndRecurse(const drawinglayer::primitive2d::BasePrimitive2D* pBasePrimitive, tools::XmlWriter& rWriter);
    void decomposeAndWrite(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence, tools::XmlWriter& rWriter);
    static OUString idToString(sal_uInt32 nId);

public:
    Primitive2dXmlDump();
    virtual ~Primitive2dXmlDump();

    /** Dumps the input primitive sequence to xml into a file or memory stream and parses the xml for testing.
     *
     * if rStreamName is set, then the xml content will be dumped into a file,
     * otherwise if the rStreamName is empty, then the content will be dumped
     * into a memory stream.
     *
     */
    xmlDocUniquePtr dumpAndParse(const drawinglayer::primitive2d::Primitive2DContainer& aPrimitive2DSequence, const OUString& rStreamName = OUString());

    /** Dumps the input primitive sequence to xml into a file. */
    void dump(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence, const OUString& rStreamName);

    /** overridden by svx::ExtendedPrimitive2dXmlDump */
    virtual bool decomposeAndWrite( const drawinglayer::primitive2d::BasePrimitive2D& /*rPrimitive2DSequence*/, ::tools::XmlWriter& /*rWriter*/) { return false; }
};

}

#endif // INCLUDED_DRAWINGLAYER_TOOLS_PRIMITIVE2DXMLDUMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
