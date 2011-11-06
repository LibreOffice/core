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



#ifndef _SGFFILT_HXX
#define _SGFFILT_HXX
#include <tools/urlobj.hxx>

sal_uInt8 CheckSgfTyp(SvStream& rInp, sal_uInt16& nVersion);
sal_Bool SgfBMapFilter(SvStream& rInp, SvStream& rOut);
sal_Bool SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf);
sal_Bool SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, INetURLObject aIniPath );

// Konstanten fÅr CheckSgfTyp()
#define SGF_BITIMAGE   1     /* Bitmap                      */
#define SGF_SIMPVECT   2     /* Einfaches Vectorformat      */
#define SGF_POSTSCRP   3     /* Postscript file             */
#define SGF_STARDRAW   7     /* StarDraw SGV-Datei          */
#define SGF_DONTKNOW 255     /* Unbekannt oder kein SGF/SGV */

#define SGV_VERSION    3     /* SGV mit anderer Version wird abgewiesen */
                             /* 3 entspricht StarDraw 2.00/2.01 MÑrz'93 */
#endif  //_SGFFILT_HXX
