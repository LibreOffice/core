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



#ifndef _SVX_XSFLCLIT_HXX
#define _SVX_XSFLCLIT_HXX

#include <svx/xcolit.hxx>

//------------------------------
// class XSecondaryFillColorItem
//------------------------------
class SVX_DLLPUBLIC XSecondaryFillColorItem : public XColorItem
{
public:
            SVX_DLLPRIVATE XSecondaryFillColorItem() {}
            SVX_DLLPRIVATE XSecondaryFillColorItem(sal_Int32 nIndex, const Color& rTheColor);
            XSecondaryFillColorItem(const String& rName, const Color& rTheColor);
            SVX_DLLPRIVATE XSecondaryFillColorItem(SvStream& rIn);

    SVX_DLLPRIVATE virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    SVX_DLLPRIVATE virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;

    SVX_DLLPRIVATE virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion( sal_uInt16 nFileFormatVersion ) const;

};

#endif

