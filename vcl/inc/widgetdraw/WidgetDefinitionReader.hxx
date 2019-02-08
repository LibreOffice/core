/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETDEFINITIONREADER_HXX
#define INCLUDED_VCL_INC_WIDGETDEFINITIONREADER_HXX

#include <vcl/dllapi.h>
#include <widgetdraw/WidgetDefinition.hxx>
#include <memory>
#include <rtl/ustring.hxx>
#include <tools/XmlWalker.hxx>

namespace vcl
{
class VCL_DLLPUBLIC WidgetDefinitionReader
{
private:
    OUString m_rDefinitionFile;
    OUString m_rResourcePath;

    void readDefinition(tools::XmlWalker& rWalker, WidgetDefinition& rWidgetDefinition,
                        ControlType eType);

    void readPart(tools::XmlWalker& rWalker, std::shared_ptr<WidgetDefinitionPart> rpPart);

    void readDrawingDefinition(tools::XmlWalker& rWalker,
                               std::shared_ptr<WidgetDefinitionState>& rStates);

public:
    WidgetDefinitionReader(OUString const& rDefinitionFile, OUString const& rResourcePath);
    bool read(WidgetDefinition& rWidgetDefinition);
};

} // end vcl namespace

#endif // INCLUDED_VCL_INC_WIDGETDEFINITIONREADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
