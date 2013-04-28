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

#ifndef _SVX_COLORVALUESET_HXX
#define _SVX_COLORVALUESET_HXX

#include <svtools/valueset.hxx>
#include "svx/svxdllapi.h"
#include <boost/shared_ptr.hpp>

//////////////////////////////////////////////////////////////////////////////

class XColorList;
typedef ::boost::shared_ptr< XColorList > XColorListSharedPtr;

class SVX_DLLPUBLIC SvxColorValueSet : public ValueSet
{
private:
protected:
public:
    SvxColorValueSet(Window* pParent, WinBits nWinStyle = WB_ITEMBORDER);
    SvxColorValueSet(Window* pParent, const ResId& rResId);

    sal_uInt32 getMaxRowCount() const;
    sal_uInt32 getEntryEdgeLength() const;
    sal_uInt32 getColumnCount() const;

    void addEntriesForXColorList(const XColorListSharedPtr aXColorList, sal_uInt32 nStartIndex = 1);
    Size layoutAllVisible(sal_uInt32 nEntryCount);
    Size layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount);
};

//////////////////////////////////////////////////////////////////////////////

#endif // _SVX_COLORVALUESET_HXX

// eof
