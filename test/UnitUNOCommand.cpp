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

/*
 * Unit test for UNO command execution functionality.
 */

#include <config.h>

#include <memory>
#include <ostream>
#include <set>
#include <string>

#include <Poco/Exception.h>
#include <regex>
#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <helpers.hpp>

namespace
{
void testStateChanged(const std::string& filename, std::set<std::string>& commands)
{
    const auto testname = "stateChanged_" + filename + ' ';

    std::regex reUno("\\.[a-zA-Z]*\\:[a-zA-Z]*\\=");

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("UnitEachView");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
        socketPoll, filename, Poco::URI(helpers::getTestServerURI()), testname);
    helpers::SocketProcessor(testname, socket,
        [&](const std::string& msg)
        {
            if (msg.starts_with("statechanged: {"))
            {
                // Payload is JSON, the commandName key has the command name.
                Poco::JSON::Parser parser;
                std::string json = msg.substr(strlen("statechanged: "));
                const Poco::Dynamic::Var var = parser.parse(json);
                const auto& root = var.extract<Poco::JSON::Object::Ptr>();
                std::string commandName = root->get("commandName").toString();
                commands.erase(commandName + "=");
            }
            else
            {
                std::smatch matches;
                if (std::regex_search(msg, matches, reUno))
                {
                    // Payload is a commandName=...status... plain text format.
                    commands.erase(matches[0].str());
                }
            }

            return !commands.empty();
        });

    if (!commands.empty())
    {
        std::ostringstream ostr;
        ostr << filename << " : Missing Uno Commands: " << std::endl;
        for (auto & itUno : commands)
        {
            ostr << itUno << std::endl;
        }

        LOK_ASSERT_FAIL(ostr.str());
    }
}
}

/// Test suite for UNO commands.
class UnitUNOCommand : public UnitWSD
{
    TestResult testStateUnoCommandWriter();
    TestResult testStateUnoCommandCalc();
    TestResult testStateUnoCommandImpress();

public:
    UnitUNOCommand()
        : UnitWSD("UnitUNOCommand")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitUNOCommand::testStateUnoCommandWriter()
{
    std::set<std::string> writerCommands
    {
        ".uno:BackgroundColor=",
        ".uno:Bold=",
        ".uno:CenterPara=",
        ".uno:CharBackColor=",
        ".uno:CharBackgroundExt=",
        ".uno:CharFontName=",
        ".uno:Color=",
        ".uno:DefaultBullet=",
        ".uno:DefaultNumbering=",
        ".uno:FontColor=",
        ".uno:FontHeight=",
        ".uno:Italic=",
        ".uno:JustifyPara=",
        ".uno:OutlineFont=",
        ".uno:LeftPara=",
        ".uno:RightPara=",
        ".uno:Shadowed=",
        ".uno:SubScript=",
        ".uno:SuperScript=",
        ".uno:Strikeout=",
        ".uno:StyleApply=",
        ".uno:Underline=",
        ".uno:ModifiedStatus=",
        ".uno:Undo=",
        ".uno:Redo=",
        ".uno:Cut=",
        ".uno:Copy=",
        ".uno:Paste=",
        ".uno:SelectAll=",
        ".uno:InsertAnnotation=",
        ".uno:InsertRowsBefore=",
        ".uno:InsertRowsAfter=",
        ".uno:InsertColumnsBefore=",
        ".uno:InsertColumnsAfter=",
        ".uno:DeleteRows=",
        ".uno:DeleteColumns=",
        ".uno:DeleteTable=",
        ".uno:SelectTable=",
        ".uno:EntireRow=",
        ".uno:EntireColumn=",
        ".uno:EntireCell=",
        ".uno:InsertMode=",
        ".uno:StateTableCell=",
        ".uno:StatePageNumber=",
        ".uno:StateWordCount=",
        ".uno:SelectionMode=",
        ".uno:NumberFormatCurrency=",
        ".uno:NumberFormatPercent=",
        ".uno:NumberFormatDate="
    };

    try
    {
        testStateChanged("empty.odt", writerCommands);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitUNOCommand::testStateUnoCommandCalc()
{
    std::set<std::string> calcCommands
    {
        ".uno:BackgroundColor=",
        ".uno:Bold=",
        ".uno:CenterPara=",
        ".uno:CharBackColor=",
        ".uno:CharFontName=",
        ".uno:Color=",
        ".uno:FontHeight=",
        ".uno:Italic=",
        ".uno:JustifyPara=",
        ".uno:OutlineFont=",
        ".uno:LeftPara=",
        ".uno:RightPara=",
        ".uno:Shadowed=",
        ".uno:SubScript=",
        ".uno:SuperScript=",
        ".uno:Strikeout=",
        ".uno:StyleApply=",
        ".uno:Underline=",
        ".uno:ModifiedStatus=",
        ".uno:Undo=",
        ".uno:Redo=",
        ".uno:Cut=",
        ".uno:Copy=",
        ".uno:Paste=",
        ".uno:SelectAll=",
        ".uno:InsertAnnotation=",
        ".uno:InsertRowsBefore=",
        ".uno:InsertRowsAfter=",
        ".uno:InsertColumnsBefore=",
        ".uno:InsertColumnsAfter=",
        ".uno:DeleteRows=",
        ".uno:DeleteColumns=",
        ".uno:StatusDocPos=",
        ".uno:RowColSelCount=",
        ".uno:StatusPageStyle=",
        ".uno:InsertMode=",
        ".uno:StatusSelectionMode=",
        ".uno:StateTableCell=",
        ".uno:StatusBarFunc=",
        ".uno:WrapText=",
        ".uno:ToggleMergeCells=",
        ".uno:NumberFormatCurrency=",
        ".uno:NumberFormatPercent=",
        ".uno:NumberFormatDate="
    };

    try
    {
        testStateChanged("empty.ods", calcCommands);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitUNOCommand::testStateUnoCommandImpress()
{
    std::set<std::string> impressCommands
    {
        ".uno:Bold=",
        ".uno:CenterPara=",
        ".uno:CharBackColor=",
        ".uno:CharFontName=",
        ".uno:Color=",
        ".uno:DefaultBullet=",
        ".uno:DefaultNumbering=",
        ".uno:FontHeight=",
        ".uno:Italic=",
        ".uno:JustifyPara=",
        ".uno:OutlineFont=",
        ".uno:LeftPara=",
        ".uno:RightPara=",
        ".uno:Shadowed=",
        ".uno:SubScript=",
        ".uno:SuperScript=",
        ".uno:Strikeout=",
        ".uno:StyleApply=",
        ".uno:Underline=",
        ".uno:ModifiedStatus=",
        ".uno:Undo=",
        ".uno:Redo=",
        ".uno:InsertPage=",
        ".uno:DeletePage=",
        ".uno:DuplicatePage=",
        ".uno:Cut=",
        ".uno:Copy=",
        ".uno:Paste=",
        ".uno:SelectAll=",
        ".uno:InsertAnnotation=",
        ".uno:InsertRowsBefore=",
        ".uno:InsertRowsAfter=",
        ".uno:InsertColumnsBefore=",
        ".uno:InsertColumnsAfter=",
        ".uno:DeleteRows=",
        ".uno:DeleteColumns=",
        ".uno:SelectTable=",
        ".uno:EntireRow=",
        ".uno:EntireColumn=",
        ".uno:AssignLayout=",
        ".uno:PageStatus=",
        ".uno:LayoutStatus=",
        ".uno:Context=",
        ".uno:InsertSymbol=",
    };

    try
    {
        testStateChanged("empty.odp", impressCommands);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

void UnitUNOCommand::invokeWSDTest()
{
    UnitBase::TestResult result = testStateUnoCommandWriter();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testStateUnoCommandCalc();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testStateUnoCommandImpress();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitUNOCommand(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
