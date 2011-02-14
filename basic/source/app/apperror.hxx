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

#include <appwin.hxx>
#include <msgedit.hxx>

class AppError : public AppWin
{
protected:
    DECL_LINK( EditChange, MsgEdit * );
public:
    TYPEINFO();
    AppError( BasicFrame*, String );
    ~AppError();
//  long Command( short nID );
    virtual long InitMenu( Menu* );
    virtual long DeInitMenu( Menu* );
    sal_uInt16 GetLineNr();
    FileType GetFileType();
    MsgEdit* GetMsgTree()           { return ((MsgEdit*)pDataEdit); }
    virtual sal_Bool ReloadAllowed(){ return !StarBASIC::IsRunning(); }
    virtual void LoadIniFile();     // (re)load ini file after change
    DirEntry aBaseDir;
};

