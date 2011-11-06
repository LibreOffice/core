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


#ifndef _SVX_RADIOBTNBOX_HXX
#define _SVX_RADIOBTNBOX_HXX

// include ---------------------------------------------------------------

#include <svx/simptabl.hxx>

namespace svx {

// class SvxRadioButtonListBox -------------------------------------------

class SvxRadioButtonListBox : public SvxSimpleTable
{
    using SvxSimpleTable::SetTabs;

private:
    Point               m_aCurMousePoint;

protected:
    virtual void        SetTabs();
    virtual void        MouseButtonUp( const MouseEvent& _rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );

public:
    SvxRadioButtonListBox( Window* _pParent, const ResId& _rId );
    ~SvxRadioButtonListBox();

    void                HandleEntryChecked( SvLBoxEntry* _pEntry );

    const Point&        GetCurMousePoint() const;
};

} // end of namespace ::svx

#endif // #ifndef _SVX_RADIOBTNBOX_HXX

