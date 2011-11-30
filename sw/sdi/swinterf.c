/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


