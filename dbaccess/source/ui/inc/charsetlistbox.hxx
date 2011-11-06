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



#ifndef CHARSETLISTBOX_HXX
#define CHARSETLISTBOX_HXX

#include "charsets.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/lstbox.hxx>

class SfxItemSet;

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= CharSetListBox
    //====================================================================
    class CharSetListBox : public ListBox
    {
    public:
        CharSetListBox( Window* _pParent, const ResId& _rResId );
        ~CharSetListBox();

        void    SelectEntryByIanaName( const String& _rIanaName );
        bool    StoreSelectedCharSet( SfxItemSet& _rSet, const sal_uInt16 _nItemId );

    private:
        OCharsetDisplay     m_aCharSets;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // CHARSETLISTBOX_HXX
