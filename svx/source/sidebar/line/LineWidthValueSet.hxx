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

#ifndef SVX_SIDEBAR_LINE_WIDTH_VALUE_SET_HXX
#define SVX_SIDEBAR_LINE_WIDTH_VALUE_SET_HXX

#include <svtools/valueset.hxx>
#include <vcl/image.hxx>

namespace svx { namespace sidebar {

class LineWidthValueSet
    : public ValueSet
{
public:
    LineWidthValueSet (Window* pParent, const ResId& rResId);
    virtual ~LineWidthValueSet (void);

    void SetUnit(XubString* str);
    void SetSelItem(sal_uInt16 nSel);
    sal_uInt16 GetSelItem();
    void SetImage(Image img);
    void SetCusEnable(bool bEnable);

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

protected:
    VirtualDevice*  pVDev;
    sal_uInt16          nSelItem;
    XubString*      strUnit;
    Image           imgCus;
    bool            bCusEnable;
};

} } // end of namespace svx::sidebar

#endif // SVX_SIDEBAR_LINE_WIDTH_VALUE_SET_HXX

// eof
