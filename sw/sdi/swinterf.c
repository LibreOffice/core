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

#include <stdio.h>

#include <svinterf.h>
#include <cmdid.h>

static char pEmpty[] = "";

extern void *pSvClient;

/* WRITER */
short SwInsertDocument (void *pHandle,
                        const char *pFileName,
                        const char *pFilter)
{
    return 0;
}

/**************************************************************************/

/* WRITER */
short SwDeleteTableColumns (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_TABLE_DELETE_COL | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwDeleteTableRows (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_TABLE_DELETE_ROW | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwGotoStartOfTable (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_START_TABLE | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwGotoEndOfTable   (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_END_TABLE | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwGotoNextTable    (void *pHandle)
{
    return 0;
}

/* WRITER */
short SwGotoPrevTable   (void *pHandle)
{
    return 0;
}

/* WRITER */
short SwSelectTableRow    (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_TABLE_SELECT_ROW | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwSelectTableColumn (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_TABLE_SELECT_COL | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwTableSelect       (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_TABLE_SELECT_ALL | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwInsertTable (void *pHandle, const char *pName,
                     unsigned short nRow, unsigned short nColumn)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_INSERT_TABLE | 0x10000,
                                     pHandle,0,"hh",nRow,nColumn);
    return 0;

}

/**************************************************************************/

/* WRITER */
short SwCharLeft(void *pHandle, unsigned short nCount, unsigned short bSelect)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_CHAR_LEFT | 0x10000,
                                     pHandle,0,"hb",nCount,bSelect);
    return 0;
}

/* WRITER */
short SwCharRight(void *pHandle, unsigned short nCount, unsigned short bSelect)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_CHAR_RIGHT | 0x10000,
                                     pHandle,0,"hb",nCount,bSelect);
    return 0;
}

/* WRITER */
short SwLineUp(void *pHandle, unsigned short nCount, unsigned short bSelect)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_LINE_UP | 0x10000,
                                     pHandle,0,"hb",nCount,bSelect);
    return 0;
}

/* WRITER */
short SwLineDown(void *pHandle, unsigned short nCount, unsigned short bSelect)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_LINE_DOWN | 0x10000,
                                     pHandle,0,"hb",nCount,bSelect);
    return 0;
}

/**************************************************************************/

/* WRITER */
const char *SwGetSelectedText(void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallStringFunction(SvIPCGetClient(),FN_GET_SELECTED_TEXT,
                                      pHandle,0,pEmpty);
    return NULL;
}

/* WRITER */
short SwInsertString(void *pHandle,const char *pString)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_INSERT_STRING | 0x10000,
                                     pHandle,0,"s",pString);
    return 0;
}

/* WRITER */
extern short SwInsertLineBreak(void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_INSERT_LINEBREAK | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/**************************************************************************/

/* WRITER */
short SwDelLeft (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_BACKSPACE | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwDelRight (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_DELETE | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwDelLine (void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_DELETE_WHOLE_LINE | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/**************************************************************************/

/* WRITER */
short SwCharDlg(void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_FORMAT_CHAR_DLG | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}

/* WRITER */
short SwParagrDlg(void *pHandle)
{
    if (SvIPCIsConnected())
        return pIPCCallShortFunction(SvIPCGetClient(),FN_FORMAT_PARA_DLG | 0x10000,
                                     pHandle,0,pEmpty);
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
