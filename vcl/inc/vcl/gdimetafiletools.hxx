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

#ifndef _SV_GDIMETAFILETOOLS_HXX
#define _SV_GDIMETAFILETOOLS_HXX

#include <vcl/gdimtf.hxx>

//////////////////////////////////////////////////////////////////////////////
// #121267# Added tooling to be able to support old exporters which are based on
// metafiles as graphic content, but do not implement using the contained clip
// regions.
// The given metafile will internall yclip it's graphic content against the
// included clip regions so that it is safe to ignore clip actions there. This
// is not done completely, but implemented and extended as needed (on demand)
// since all this is a workarund; the better and long term solution will be to
// reimplement these im/exports to use primitives and not metafiles as bese
// information.

void VCL_DLLPUBLIC clipMetafileContentAgainstOwnRegions(GDIMetaFile& rSource);

//////////////////////////////////////////////////////////////////////////////
// Allow to check if a Metafile contains clipping or not

bool VCL_DLLPUBLIC usesClipActions(const GDIMetaFile& rSource);

//////////////////////////////////////////////////////////////////////////////

#endif // _SV_GDIMETAFILETOOLS_HXX
