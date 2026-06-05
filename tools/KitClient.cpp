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

/*
 * Direct COKit client for testing and benchmarking.
 * Functions: Document loading, tile rendering, command execution
 */

#include <config.h>

#include <common/NumUtil.hpp>
#include <common/Png.hpp>
#include <common/Protocol.hpp>
#include <common/Util.hpp>
#include <kit/KitHelper.hpp>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKitInit.h>
#include <COKit/COKit.h>

#include <Poco/String.h>
#include <Poco/TemporaryFile.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sysexits.h>
#include <unistd.h>

using Poco::TemporaryFile;

bool EnableExperimental = false;

extern "C"
{
    static void myCallback(int type, const char* payload, void*)
    {
        std::cout << "Callback: " << kitCallbackTypeToString(type)
                  << " payload: " << payload << std::endl;
    }
}

/// The application class implementing a client.
class LOKitClient : public Poco::Util::Application
{
public:
protected:
    // coverity[root_function] : don't warn about uncaught exceptions
    int main(const std::vector<std::string>& args) override
    {
        if (args.size() != 2)
        {
            logger().fatal("Usage: lokitclient /path/to/lo/installation/program /path/to/document");
            return EX_USAGE;
        }

        COKit *loKit;
        COKitDocument *loKitDocument;

        loKit = cok_init(args[0].c_str());
        if (!loKit)
        {
            logger().fatal("COKit initialisation failed");
            return EX_UNAVAILABLE;
        }


        loKitDocument = loKit->pClass->documentLoad(loKit, args[1].c_str());
        if (!loKitDocument)
        {
            logger().fatal("Document loading failed: " + std::string(loKit->pClass->getError(loKit)));
            return EX_UNAVAILABLE;
        }

        loKitDocument->pClass->registerCallback(loKitDocument, myCallback, nullptr);

        loKitDocument->pClass->initializeForRendering(loKitDocument, nullptr);

        if (isatty(0))
        {
            std::cout << "Enter LOKit \"commands\", one per line. 'help' for help. EOF to finish." << std::endl;
        }

        while (!std::cin.eof())
        {
            std::string line;
            std::getline(std::cin, line);

            StringVector tokens(StringVector::tokenize(std::move(line), ' '));

            if (tokens.empty())
                continue;

            if (tokens.equals(0, "?") || tokens.equals(0, "help"))
            {
                std::cout <<
                    "Commands mimic COOL protocol but we talk directly to LOKit:" << std::endl <<
                    "    status" << std::endl <<
                    "        calls COKitDocument::getDocumentType, getParts, getPartName, getDocumentSize" << std::endl <<
                    "    tile part pixelwidth pixelheight docposx docposy doctilewidth doctileheight" << std::endl <<
                    "        calls COKitDocument::paintTile" << std::endl;
            }
            else if (tokens.equals(0, "status"))
            {
                if (tokens.size() != 1)
                {
                    std::cout << "? syntax" << std::endl;
                    continue;
                }
                std::cout << LOKitHelper::documentStatus(loKitDocument) << std::endl;
            }
            else if (tokens.equals(0, "tile"))
            {
                if (tokens.size() != 8)
                {
                    std::cout << "? syntax" << std::endl;
                    continue;
                }

                int partNumber(NumUtil::stoi(tokens[1]));
                int canvasWidth(NumUtil::stoi(tokens[2]));
                int canvasHeight(NumUtil::stoi(tokens[3]));
                int tilePosX(NumUtil::stoi(tokens[4]));
                int tilePosY(NumUtil::stoi(tokens[5]));
                int tileWidth(NumUtil::stoi(tokens[6]));
                int tileHeight(NumUtil::stoi(tokens[7]));

                std::vector<unsigned char> pixmap(canvasWidth*canvasHeight*4);
                loKitDocument->pClass->setPart(loKitDocument, partNumber);
                loKitDocument->pClass->paintTile(loKitDocument, pixmap.data(), canvasWidth, canvasHeight, tilePosX, tilePosY, tileWidth, tileHeight);

                if (!Util::windowingAvailable())
                    continue;

                std::vector<char> png;
                const auto mode = static_cast<COKitTileMode>(loKitDocument->pClass->getTileMode(loKitDocument));

                Png::encodeBufferToPNG(pixmap.data(), canvasWidth, canvasHeight, png, mode);

                TemporaryFile pngFile;
                std::ofstream pngStream(pngFile.path(), std::ios::binary);
                pngStream.write(png.data(), png.size());
                pngStream.close();

                if (std::system((std::string("display ") + pngFile.path()).c_str()) == -1)
                {
                    // Not worth it to display a warning, this is just a throwaway test program, and
                    // the developer running it surely notices if nothing shows up...
                }
            }
            else
            {
                std::cout << "? unrecognized" << std::endl;
            }
        }

        // Safest to just bluntly exit
        Util::forcedExit(EX_OK);
    }
};

namespace Util
{

void alertAllUsers(const std::string& cmd, const std::string& kind)
{
    std::cout << "error: cmd=" << cmd << " kind=" << kind << std::endl;
    (void) kind;
}

}

// coverity[root_function] : don't warn about uncaught exceptions
POCO_APP_MAIN(LOKitClient)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
