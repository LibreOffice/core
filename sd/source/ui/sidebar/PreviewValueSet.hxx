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

#ifndef SD_SIDEBAR_PANELS_PREVIEW_VALUE_SET_HXX
#define SD_SIDEBAR_PANELS_PREVIEW_VALUE_SET_HXX

#include <svtools/valueset.hxx>


namespace sd { namespace sidebar {


/** Adapt the svtools valueset to the needs of the master page controlls.
*/
class PreviewValueSet
    : public ValueSet
{
public:
    PreviewValueSet (::Window* pParent);
    ~PreviewValueSet (void);

    void SetRightMouseClickHandler (const Link& rLink);
    virtual void Resize (void);

    void SetPreviewSize (const Size& rSize);

    sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    sal_Int32 GetPreferredHeight (sal_Int32 nWidth);

    /** Set the number of rows and columns according to the current number
        of items.  Call this method when new items have been inserted.
    */
    void Rearrange (bool bForceRequestResize = false);

protected:
    virtual void MouseButtonDown (const MouseEvent& rEvent);

private:
    Link maRightMouseClickHandler;
    Size maPreviewSize;
    const int mnBorderWidth;
    const int mnBorderHeight;
    const int mnMaxColumnCount;

    sal_uInt16 CalculateColumnCount (int nWidth) const;
    sal_uInt16 CalculateRowCount (sal_uInt16 nColumnCount) const;
};

} } // end of namespace sd::sidebar

#endif
