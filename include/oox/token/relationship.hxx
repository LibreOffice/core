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
    ACTIVEXCONTROLBINARY,
    CHART,
    COMMENTS,
    COMMENTAUTHORS,
    CONTROL,
    CUSTOMXML,
    CUSTOMXMLPROPS,
    DIAGRAMCOLORS,
    DIAGRAMDATA,
    DIAGRAMDRAWING,
    DIAGRAMLAYOUT,
    DIAGRAMQUICKSTYLE,
    DRAWING,
    ENDNOTES,
    EXTERNALLINKPATH,
    FONT,
    FONTTABLE,
    FOOTER,
    FOOTNOTES,
    GLOSSARYDOCUMENT,
    HDPHOTO,
    HEADER,
    HYPERLINK,
    IMAGE,
    NOTESMASTER,
    NOTESSLIDE,
    NUMBERING,
    OFFICEDOCUMENT,
    OLEOBJECT,
    PACKAGE,
    SETTINGS,
    SHAREDSTRINGS,
    SLIDE,
    SLIDELAYOUT,
    SLIDEMASTER,
    STYLES,
    THEME,
    VBAPROJECT,
    VMLDRAWING,
    WORDVBADATA,
    WORKSHEET
};

OUString OOX_DLLPUBLIC getRelationship(Relationship eRelationship);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
