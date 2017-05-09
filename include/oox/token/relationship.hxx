/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/dllapi.h>

#include <rtl/ustring.hxx>

namespace oox {

enum class Relationship
{
    HYPERLINK,
    PACKAGE,
    HDPHOTO,
    OLEOBJECT,
    CHART,
    IMAGE,
    FONT,
    STYLES,
    FOOTNOTES,
    ENDNOTES,
    COMMENTS,
    NUMBERING,
    HEADER,
    FOOTER,
    FONTTABLE,
    SETTINGS,
    THEME,
    GLOSSARYDOCUMENT,
    CUSTOMXML,
    CUSTOMXMLPROPS,
    CONTROL,
    ACTIVEXCONTROLBINARY,
    OFFICEDOCUMENT,
    DIAGRAMDATA,
    DIAGRAMLAYOUT,
    DIAGRAMQUICKSTYLE,
    DIAGRAMCOLORS,
    DIAGRAMDRAWING
};

OUString OOX_DLLPUBLIC getRelationship(Relationship eRelationship);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
