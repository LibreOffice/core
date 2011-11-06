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



#ifndef SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX
#define SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX

#include <sfx2/sfxsids.hrc>
#include <sfx2/tabdlg.hxx>
#include <editeng/flstitem.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= TextControlCharAttribDialog
    //====================================================================
    class TextControlCharAttribDialog : public SfxTabDialog
    {
    private:
        SvxFontListItem m_aFontList;

    public:
        TextControlCharAttribDialog( Window* pParent, const SfxItemSet& _rCoreSet, const SvxFontListItem& _rFontList );
        ~TextControlCharAttribDialog();

    protected:
        virtual void PageCreated( sal_uInt16 _nId, SfxTabPage& _rPage );
    };

    //====================================================================
    //= TextControlParaAttribDialog
    //====================================================================
    class TextControlParaAttribDialog : public SfxTabDialog
    {
    public:
        TextControlParaAttribDialog( Window* _pParent, const SfxItemSet& _rCoreSet );
        ~TextControlParaAttribDialog();
    };
//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX

