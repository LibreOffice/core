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



#ifndef _SWFORMATCLIPBOARD_HXX
#define _SWFORMATCLIPBOARD_HXX

// header for class SwWrtShell
#include <wrtsh.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
// header for class SfxStyleSheetBasePool
#include <svl/style.hxx>

//-----------------------------------------------------------------------------
/** This class acts as data container and execution class for the format paintbrush feature in writer.
*/

class SwFormatClipboard
{
public:
    SwFormatClipboard();
    ~SwFormatClipboard();

    bool HasContent() const;
    bool HasContentForThisType( int nSelectionType ) const;
    bool CanCopyThisType( int nSelectionType ) const;

    void Copy( SwWrtShell& rWrtShell, SfxItemPool& rPool, bool bPersistentCopy=false );
    void Paste( SwWrtShell& rWrtShell, SfxStyleSheetBasePool* pPool
        , bool bNoCharacterFormats=false, bool bNoParagraphFormats=false );
    void Erase();

private:
    int         m_nSelectionType;
    SfxItemSet* m_pItemSet;
    SfxItemSet* m_pTableItemSet;

    String m_aCharStyle;
    String m_aParaStyle;
    //no frame style because it contains position information

    bool   m_bPersistentCopy;
};

#endif
