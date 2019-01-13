/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_MTFXMLDUMP_HXX
#define INCLUDED_VCL_MTFXMLDUMP_HXX

#include <sal/config.h>
#include <vcl/dllapi.h>
#include <vcl/metaactiontypes.hxx>
#include <o3tl/enumarray.hxx>

namespace tools { class XmlWriter; }
class GDIMetaFile;
class SvStream;

enum class MetaActionType;

class VCL_DLLPUBLIC MetafileXmlDump final
{
    o3tl::enumarray<MetaActionType, bool> maFilter;

    void writeXml(const GDIMetaFile& rMetaFile, tools::XmlWriter& rWriter);

public:
    MetafileXmlDump();
    ~MetafileXmlDump();

    void filterActionType(const MetaActionType nActionType, bool bShouldFilter);
    void filterAllActionTypes();

    /** The actual result that will be used for testing.
    */
    void dump(const GDIMetaFile& rMetaFile, SvStream& rStream);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
