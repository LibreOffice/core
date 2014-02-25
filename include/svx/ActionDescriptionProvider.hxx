/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_ACTIONDESCRIPTIONPROVIDER_HXX
#define INCLUDED_SVX_ACTIONDESCRIPTIONPROVIDER_HXX

#include <svx/svxdllapi.h>

// header for class OUString
#include <rtl/ustring.hxx>


/** This class provides localized descriptions for some basic actions done with objects.
The strings are intended to be provided to the user e.g. as description for undo actions in the menu.
The name of the object acted on and the type of action needs to be given as input parameter.
*/

class SVX_DLLPUBLIC ActionDescriptionProvider
{
public:
    enum ActionType
    {
        INSERT
        , DELETE
        , CUT
        , MOVE
        , RESIZE
        , ROTATE
        , TRANSFORM
        , FORMAT
        , MOVE_TOTOP
        , MOVE_TOBOTTOM
        , POS_SIZE
    };

public:
    static OUString createDescription( ActionType eActionType
                        , const OUString& rObjectName );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
