/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/kit/ParsedCommand.hxx>

#include <tools/urlobj.hxx>

KitParsedCommand KitParsedCommand::parse(std::string_view aCommand)
{
    KitParsedCommand aParsed;

    static constexpr std::string_view aPrefix = ".uno:";
    if (!aCommand.starts_with(aPrefix))
        return aParsed;

    aCommand.remove_prefix(aPrefix.size());

    enum class State
    {
        Name,
        Key,
        Value
    };

    State eState = State::Name;
    std::string_view aKey;
    size_t nStart = 0; // start of the current name, key or value

    for (size_t i = 0; i <= aCommand.size(); ++i)
    {
        const bool bEnd = i == aCommand.size();
        const char c = bEnd ? '\0' : aCommand[i];

        switch (eState)
        {
            case State::Name:
                if (bEnd || c == '?')
                {
                    aParsed.aName = aCommand.substr(0, i);
                    eState = State::Key;
                    nStart = i + 1;
                }
                break;
            case State::Key:
                if (c == '=')
                {
                    aKey = aCommand.substr(nStart, i - nStart);
                    eState = State::Value;
                    nStart = i + 1;
                }
                else if (bEnd || c == '&')
                {
                    // A key without a value, or an empty parameter to skip.
                    if (i > nStart)
                        aParsed.aParams[aCommand.substr(nStart, i - nStart)]
                            = std::string_view();
                    nStart = i + 1;
                }
                break;
            case State::Value:
                // A value may itself contain '=', as in formula==SUM(A1:A5).
                if (bEnd || c == '&')
                {
                    aParsed.aParams[aKey] = aCommand.substr(nStart, i - nStart);
                    eState = State::Key;
                    nStart = i + 1;
                }
                break;
        }
    }

    return aParsed;
}

std::optional<std::string_view> KitParsedCommand::get(std::string_view aKey) const
{
    auto it = aParams.find(aKey);
    if (it == aParams.end())
        return std::nullopt;
    return it->second;
}

OUString KitParsedCommand::getDecoded(std::string_view aKey) const
{
    const std::optional<std::string_view> oValue = get(aKey);
    if (!oValue)
        return OUString();
    return INetURLObject::decode(OUString::fromUtf8(*oValue),
                                 INetURLObject::DecodeMechanism::WithCharset);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
