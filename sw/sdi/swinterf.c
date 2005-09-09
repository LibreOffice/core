/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swinterf.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:39:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


