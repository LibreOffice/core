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

#ifndef _BASICSTATUS_HXX
#define _BASICSTATUS_HXX

#include <svtools/taskbar.hxx>
#include <vcl/status.hxx>

#define ST_MESSAGE          1
#define ST_LINE             2
#define ST_PROF             3

class BasicFrame;

class StatusLine : public TaskBar
{
protected:
    BasicFrame* pFrame;
    DECL_LINK( ActivateTask, TaskToolBox* );

public:
    StatusLine( BasicFrame* );
    void Message( const String& );           // Show text
    void Pos( const String& s );             // Show text position
    void SetProfileName( const String& s );  // Current Profile
    void LoadTaskToolBox();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
