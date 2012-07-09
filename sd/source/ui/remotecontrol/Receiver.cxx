/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2012 Andrzej J.R. Hunt
 *
 * LibreOffice - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * LibreOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * Libreoffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with LibreOffice.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "Receiver.hxx"

Receiver::Receiver()
{
    g_type_init ();
}

Receiver::~Receiver()
{
}

void Receiver::parseCommand(char* aCommand, XSlideShowController *aController)
{
    JsonParser *parser;
    JsonNode *root;
    GError *error;

    parser = json_parser_new ();
    error = NULL;
    json_parser_load_from_data( parser, aCommand, aCommand.size, error );

    if (error) {
    }

    root = json_parser_get_root( parser );
    JsonObject *aObject = json_node_get_object( root );
    char* aInstruction = json_node_get_string( json_object_get_member( "command" ) );

    switch ( aInstruction )
    {
    case "transition_next":
        aController->gotoNextEffect();
      // Next slide;
        break;
    case "transition_previous":
        aController->gotoPreviousEffect();
        break;
    case "goto_slide":
        //
        break;
    };
}
