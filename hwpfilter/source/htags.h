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



/* NAME $Id: htags.h,v 1.3 2008-04-10 12:08:00 rt Exp $
 * PURPOSE
 *
 * NOTES
 *
 * HISTORY
 *        frog - Aug 6, 1998: Created.
 */

#ifndef _HTAGS_H_
#define _HTAGS_H_

#ifdef __GNUG__
#pragma interface
#endif

class HWPFile;
/**
 * @short Embedded image
 */
struct EmPicture
{
    int   size;
    char  name[16];
    char  type[16];
    uchar *data;

    EmPicture(int size);
    ~EmPicture(void);

    bool Read(HWPFile& hwpf);
};
/**
 * @short HyperText
 */
struct HyperText
{
    kchar filename[256];
    hchar bookmark[16];
    char  macro[325];
    uchar type;
    char reserve[3];
    bool Read(HWPFile& hwpf);
};
/**
 * @short Win32 OLE object
 */
struct OlePicture
{
    int   size;
    ulong signature;
#ifdef WIN32
     IStorage *pis;
#else
     char *pis;
#endif
    OlePicture(int tsize);
    ~OlePicture(void);

    bool Read(HWPFile& hwpf);
};
#endif                                            /* _HTAGS_H_ */
