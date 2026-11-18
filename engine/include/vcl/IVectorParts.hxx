/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/string.hxx>
#include <vcl/dllapi.h>

#include <string>
#include <string_view>
#include <vector>

namespace tools
{
class JsonWriter;
}

namespace vcl
{
/**
 * The parts of a document as vector primitives. A part is a page of a text document, a page of
 * a presentation or a drawing, or a sheet of a spreadsheet. The vector primitives of a part
 * carry the id of the part as their partId.
 */
class VCL_DLLPUBLIC IVectorParts
{
public:
    /**
     * Answers rCommand, a command with its parameters like
     * ".uno:VectorPrimitives?partid=...&mode=0", when its name is VectorPrimitives or
     * VectorForgetPart, and writes the answer to rJsonWriter. Returns false, with nothing
     * written, for a command of any other name, VectorRenderingGraphics and VectorRenderingFont
     * included, which getVectorResource() answers.
     */
    virtual bool answerVectorCommand(tools::JsonWriter& rJsonWriter, std::string_view rCommand) = 0;

    /**
     * Returns the answer to rCommand, a VectorRenderingGraphics or a VectorRenderingFont command
     * with its parameters: a line of JSON, then the bytes of the image or of the font file after
     * a newline when there is one. Empty when the document holds no vector content.
     */
    virtual std::string getVectorResource(std::string_view rCommand) = 0;

    /**
     * Writes the vector primitives delta of a part: the objects that changed since the part was
     * last pushed, then moves that mark to the current version of the part. rPartId is the id of
     * the part, and nMode the page list the part sits in. Writes nothing when nothing changed
     * since the last push or when no part carries that id.
     */
    virtual void pushVectorPrimitivesDelta(tools::JsonWriter& rJsonWriter,
                                           std::string_view rPartId, int nMode) = 0;

    /**
     * The ids of the parts, in the order the parts lie in the document. A presentation, a
     * drawing and a spreadsheet name their parts through vcl::ITiledRenderable::getPartId(), and
     * their list here is empty.
     */
    virtual std::vector<OString> getVectorPartIds() = 0;

    /**
     * Drops the state the document keeps of the part rPartId since it first served the part. A
     * part served again later counts its versions above the ones of the dropped state.
     */
    virtual void forgetVectorPart(std::string_view rPartId) = 0;

protected:
    ~IVectorParts();
};
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
