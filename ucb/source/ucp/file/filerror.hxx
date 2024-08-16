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

#pragma once

enum class TaskHandlerErr {

// Error codes used to deliver the resulting exceptions

    NO_ERROR                            ,
    UNSUPPORTED_COMMAND                 ,
    WRONG_SETPROPERTYVALUES_ARGUMENT   ,
    WRONG_GETPROPERTYVALUES_ARGUMENT   ,
    WRONG_OPEN_ARGUMENT                ,
    WRONG_DELETE_ARGUMENT              ,
    WRONG_TRANSFER_ARGUMENT            ,
    WRONG_INSERT_ARGUMENT              ,
    WRONG_CREATENEWCONTENT_ARGUMENT    ,
    UNSUPPORTED_OPEN_MODE              ,

    DELETED_STATE_IN_OPEN_COMMAND      ,
    INSERTED_STATE_IN_OPEN_COMMAND     ,

    OPEN_FILE_FOR_PAGING              ,
    NOTCONNECTED_FOR_PAGING           ,
    BUFFERSIZEEXCEEDED_FOR_PAGING     ,
    IOEXCEPTION_FOR_PAGING            ,
    READING_FILE_FOR_PAGING           ,

    OPEN_FOR_INPUTSTREAM              ,
    OPEN_FOR_STREAM                   ,
    OPEN_FOR_DIRECTORYLISTING         ,

    NOFRESHINSERT_IN_INSERT_COMMAND   ,
    NONAMESET_INSERT_COMMAND          ,
    NOCONTENTTYPE_INSERT_COMMAND      ,

    NO_OPEN_FILE_FOR_OVERWRITE        ,
    NO_OPEN_FILE_FOR_WRITE            ,
    NOTCONNECTED_FOR_WRITE            ,
    BUFFERSIZEEXCEEDED_FOR_WRITE      ,
    IOEXCEPTION_FOR_WRITE             ,
    FILEIOERROR_FOR_WRITE             ,
    FILEIOERROR_FOR_NO_SPACE          ,
    FILESIZE_FOR_WRITE                ,
    INPUTSTREAM_FOR_WRITE             ,
    NOREPLACE_FOR_WRITE               ,
    ENSUREDIR_FOR_WRITE               ,

    FOLDER_EXISTS_MKDIR               ,
    INVALID_NAME_MKDIR                ,
    CREATEDIRECTORY_MKDIR             ,

    NOSUCHFILEORDIR_FOR_REMOVE        ,
    VALIDFILESTATUS_FOR_REMOVE        ,
    OPENDIRECTORY_FOR_REMOVE          ,
    DELETEFILE_FOR_REMOVE             ,
    DELETEDIRECTORY_FOR_REMOVE        ,
    FILETYPE_FOR_REMOVE               ,
    VALIDFILESTATUSWHILE_FOR_REMOVE   ,
    DIRECTORYEXHAUSTED_FOR_REMOVE     ,

    TRANSFER_ACCESSINGROOT            ,
    TRANSFER_INVALIDSCHEME            ,
    TRANSFER_INVALIDURL               ,
    TRANSFER_DESTFILETYPE             ,
    TRANSFER_BY_MOVE_SOURCE           ,
    TRANSFER_BY_MOVE_SOURCESTAT       ,
    KEEPERROR_FOR_MOVE                ,
    NAMECLASH_FOR_MOVE                ,
    NAMECLASHMOVE_FOR_MOVE            ,
    NAMECLASHSUPPORT_FOR_MOVE         ,
    OVERWRITE_FOR_MOVE                ,
    RENAME_FOR_MOVE                   ,
    RENAMEMOVE_FOR_MOVE               ,

    TRANSFER_BY_COPY_SOURCE           ,
    TRANSFER_BY_COPY_SOURCESTAT       ,
    KEEPERROR_FOR_COPY                ,
    OVERWRITE_FOR_COPY                ,
    RENAME_FOR_COPY                   ,
    RENAMEMOVE_FOR_COPY               ,
    NAMECLASH_FOR_COPY                ,
    NAMECLASHMOVE_FOR_COPY            ,
    NAMECLASHSUPPORT_FOR_COPY         ,

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
