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

#ifndef SD_TEXT_LOGGER_HXX
#define SD_TEXT_LOGGER_HXX

#include <tools/link.hxx>

class String;
class VclWindowEvent;

namespace sd { namespace notes {

class EditWindow;

class TextLogger
{
public:
    static TextLogger& Instance (void);

    void AppendText (const char* sText);
    void AppendText (const String& sText);
    void AppendNumber (long int nValue);

    void ConnectToEditWindow (EditWindow* pWindow);

private:
    static TextLogger* spInstance;
    EditWindow* mpEditWindow;

    TextLogger (void);

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
};

} } // end of namespace ::sd::notes

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
