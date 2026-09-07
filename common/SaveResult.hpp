/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/StateEnum.hpp>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>

#include <string>

/// The answer Core gives to a .uno:Save request.
namespace SaveResult
{

// What Core did with the document. A save asked for with dontSaveIfUnmodified skips a document
// that carries no changes, and Core answers that skip with success false and the result string
// "unmodified".
STATE_ENUM(Outcome,
           Saved, ///< Core wrote a new version of the document.
           Unmodified, ///< Core wrote nothing, the document carried no changes.
           Failed); ///< Core tried to write the document and could not.

/// A .uno:Save answer, parsed once.
struct Result
{
    Outcome outcome = Outcome::Failed;
    /// The result string Core sent. Empty when Core sent none.
    std::string text;
};

/// Reads the outcome out of a .uno:Save unocommandresult payload.
inline Result parse(const Poco::JSON::Object::Ptr& json)
{
    Result parsed;

    if (json->has("result"))
    {
        const Poco::Dynamic::Var resultVar = json->get("result");
        const auto& resultObject = resultVar.extract<Poco::JSON::Object::Ptr>();
        if (resultObject->get("type").toString() == "string")
            parsed.text = resultObject->get("value").toString();
    }

    if (json->get("success").toString() == "true")
        parsed.outcome = Outcome::Saved;
    else if (parsed.text == "unmodified")
        parsed.outcome = Outcome::Unmodified;
    else
        parsed.outcome = Outcome::Failed;

    return parsed;
}

} // namespace SaveResult

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
