/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
