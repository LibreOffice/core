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



/* NAME $Id: himgutil.h,v 1.4 2008-06-04 09:58:33 vg Exp $
 * PURPOSE
 *   WIN32 등의 xv image 라이브러리가 없는 경우,
 *   이미지 파일의 형식을 반환하는데 사용한다.
 * NOTES
 *
 * HISTORY
 *        frog - Dec 23, 1998: Created.
 */

#ifndef _HIMGUTIL_H_
#define _HIMGUTIL_H_

/**
 * Graphics file format identifiers.
 */
#define RFT_ERROR    -1                           /* couldn't open file, or whatever... */
#define RFT_UNKNOWN   0
#define RFT_GIF       1
#define RFT_PM        2
#define RFT_PBM       3
#define RFT_XBM       4
#define RFT_SUNRAS    5
#define RFT_BMP       6
#define RFT_UTAHRLE   7
#define RFT_IRIS      8
#define RFT_PCX       9
#define RFT_JFIF     10
#define RFT_TIFF     11
#define RFT_PDSVICAR 12
#define RFT_COMPRESS 13
#define RFT_PS       14
#define RFT_IFF      15
#define RFT_TARGA    16
#define RFT_XPM      17
#define RFT_XWD      18
#define RFT_FITS     19
#define RFT_WMF      20

// function declaration
struct EmPicture;

/**
 * Extract the name from given object
 * @param empic Object having name to extract
 * @returns Name of embeded picture
 */
const char *GetEmbImgname(const EmPicture *empic);
#endif                                            /* _HIMGUTIL_H_ */
